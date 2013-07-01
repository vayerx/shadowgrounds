
//must include stdint.h first with __STDC_CONSTANT_MACROS defined
//otherwise we are lacking the UINT64_C() macro
#define __STDC_CONSTANT_MACROS
#include <stdint.h>

#include "treader.h"

#include "../../system/Logger.h"
#include "../../system/Miscellaneous.h"

#include <boost/bind.hpp>
#include <istorm3d_streambuffer.h>
#include "igios.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/log.h>
#include <libswscale/swscale.h>
}

static const unsigned int MAX_BUFFERED_FRAMES = 50;

VideoBackgroundLoader::VideoBackgroundLoader()
    : mContext(0),
    mState(VideoBackgroundLoader::STOPPED),
    quitRequested(false),
    backgroundReaderWaiting(false),
    toRGB_convert_ctx(NULL),
    sws_flags(SWS_BICUBIC)
{
    static bool avinit = false;
    if (!avinit) {
        av_register_all();
        av_log_set_level(AV_LOG_QUIET);
        avinit = true;
    }
    mContext = new avctx;
}

VideoBackgroundLoader::~VideoBackgroundLoader()
{
    stop();
    if (mContext->audiobuffer) delete[] mContext->audiobuffer;
    if (mContext->audioopen) avcodec_close(mContext->audiocodecctx);
    if (mContext->readframe) av_free(mContext->readframe);
    if (mContext->drawframe) av_free(mContext->drawframe);
    if (mContext->drawbuffer) delete[] mContext->drawbuffer;
    mContext->frames.clear();
    if (mContext->videoopen) avcodec_close(mContext->videocodecctx);
    if (mContext->fileopen) avformat_close_input(&mContext->formatctx);

    if (toRGB_convert_ctx != NULL) {
        sws_freeContext(toRGB_convert_ctx);
        toRGB_convert_ctx = NULL;
    }

    delete mContext;
    mContext = NULL;
}

bool VideoBackgroundLoader::init(const char *filename, IStorm3D_StreamBuilder *builder)
{
    if (avformat_open_input(&mContext->formatctx, filename, 0, 0) != 0) {
        LOG_WARNING( strPrintf("Failed to load video '%s'.", filename).c_str() );
        return false;
    } else { mContext->fileopen = true; }

    if (avformat_find_stream_info(mContext->formatctx, 0) < 0) {
        LOG_WARNING("Failed to find stream information.");
        return false;
    }

    mContext->videoindex = mContext->audioindex = -1;
    for (unsigned int i = 0; i < mContext->formatctx->nb_streams; ++i) {
        if (mContext->formatctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO && mContext->videoindex ==
            -1) mContext->videoindex = i;
        else if (mContext->formatctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO && mContext->audioindex ==
                 -1) mContext->audioindex = i;
    }

    if (mContext->videoindex == -1) {
        LOG_WARNING("No video streams detected.");
        return false;
    } else { mContext->videocodecctx = mContext->formatctx->streams[mContext->videoindex]->codec; }

    mContext->videocodec = avcodec_find_decoder(mContext->videocodecctx->codec_id);
    if (!mContext->videocodec) {
        LOG_WARNING("Unable to find suitable video codec.");
        return false;
    }

    if (mContext->videocodec->capabilities & CODEC_CAP_TRUNCATED)
        mContext->videocodecctx->flags |= CODEC_FLAG_TRUNCATED;

    if (avcodec_open2(mContext->videocodecctx, mContext->videocodec, 0) < 0) {
        LOG_WARNING("Unable to open video codec.");
        return false;
    } else { mContext->videoopen = true; }

    if (mContext->videocodecctx->time_base.num > 1000 && mContext->videocodecctx->time_base.den ==
        1) mContext->videocodecctx->time_base.den = 1000;
    mContext->fpsnumerator = mContext->videocodecctx->time_base.num;
    mContext->fpsdenominator = mContext->videocodecctx->time_base.den;

    mContext->videowidth = mContext->videocodecctx->width;
    mContext->videoheight = mContext->videocodecctx->height;

    if ( !( mContext->readframe = avcodec_alloc_frame() ) ) {
        LOG_WARNING("Unable to allocate read frame.");
        return false;
    }

    if ( !( mContext->drawframe = avcodec_alloc_frame() ) ) {
        LOG_WARNING("Unable to allocate draw frame.");
        return false;
    }
    unsigned int bytes = avpicture_get_size(PIX_FMT_RGB24, mContext->videowidth, mContext->videoheight);
    mContext->drawbuffer = new unsigned char[bytes * 4 + 1];
    if (!mContext->drawbuffer) {
        LOG_WARNING("Unable to allocate draw buffer.");
        return false;
    }
    avpicture_fill( (AVPicture *)mContext->drawframe, mContext->drawbuffer, PIX_FMT_RGB24, mContext->videowidth,
                    mContext->videoheight );

    if (mContext->audioindex != -1 && builder) {
        mContext->audiocodecctx = mContext->formatctx->streams[mContext->audioindex]->codec;
        mContext->audiocodec = avcodec_find_decoder(mContext->audiocodecctx->codec_id);
        if (!mContext->audiocodec) {
            LOG_WARNING("Unable to find suitable audio codec.");
        } else {
            if (avcodec_open2(mContext->audiocodecctx, mContext->audiocodec, 0) < 0) {
                LOG_WARNING("Unable to open audio codec.");
            } else {
                mContext->audioopen = true;
                mContext->audiobuffersize = (AVCODEC_MAX_AUDIO_FRAME_SIZE * 3) / 2;
                mContext->audiobuffer = new int16_t[mContext->audiobuffersize];
                if (!mContext->audiobuffer) {
                    LOG_WARNING("Unable to allocate audio buffer.");
                } else {
                    builder->setStereo(mContext->audiocodecctx->channels);
                    builder->setFrequency(mContext->audiocodecctx->sample_rate);
                    builder->setBits(16);
                    mContext->audiostream = builder->getStream();
                    mContext->audiostream->activate();
                }
            }
        }
    }

    return true;
}

void VideoBackgroundLoader::restart()
{
    stop();

    {
        boost::mutex::scoped_lock lock(backgroundMutex);

        mContext->frames.clear();
        if (backgroundReaderWaiting) {
            backgroundReaderWaiting = false;
            framesFullCVar.notify_one();
        }
    }

    av_seek_frame(mContext->formatctx, mContext->videoindex, 0, AVSEEK_FLAG_ANY);
    av_seek_frame(mContext->formatctx, mContext->audioindex, 0, AVSEEK_FLAG_ANY);

    start();
}

bool VideoBackgroundLoader::finished()
{
    {
        boost::mutex::scoped_lock lock(backgroundMutex);
        if ( !mContext->frames.empty() )
            // there are still buffered frames, definitely not finished
            return false;
    }

    {
        // no buffered frames, check state
        boost::mutex::scoped_lock lock(backgroundMutex);

        if (mState == STOPPED)
            return true;
        else
            return false;
    }
}

void VideoBackgroundLoader::start()
{
    boost::mutex::scoped_lock lock(backgroundMutex);
    if (mState == STOPPED) {
        mState = STARTING;
        mThread = boost::thread( boost::bind(&VideoBackgroundLoader::startLoadingThread, this) );
    }
}

void VideoBackgroundLoader::stop()
{
    {
        boost::mutex::scoped_lock lock(backgroundMutex);
        if (mState != STOPPED)
            mState = STOPPING;
        quitRequested = true;

        if (mContext->audiostream) {
            mContext->audiostream->deactivate();
            mContext->audiostream.reset();
        }

        // wake it up if it's sleeping
        if (backgroundReaderWaiting) {
            backgroundReaderWaiting = false;
            framesFullCVar.notify_one();
        }
    }

    mThread.join();
}

void VideoBackgroundLoader::getVideoInfo(unsigned int *fps_num, unsigned int *fps_den, unsigned int *w, unsigned int *h)
{
    *fps_num = mContext->fpsnumerator;
    *fps_den = mContext->fpsdenominator;
    *w = mContext->videowidth;
    *h = mContext->videoheight;
}

bool VideoBackgroundLoader::readFrame(char *buffer, const unsigned int w, const unsigned int h)
{
    boost::shared_array<unsigned char> frame;
    {
        boost::mutex::scoped_lock lock(backgroundMutex);
        if ( mContext->frames.size() ) {
            frame = mContext->frames.front();
            mContext->frames.pop_front();

            // wake the reader up if it was sleeping
            if (backgroundReaderWaiting) {
                backgroundReaderWaiting = false;
                framesFullCVar.notify_one();
            }
        } else {
            // no frames to get
            return false;
        }
    }

    uint32_t *buf1 = (uint32_t *)buffer;
    uint8_t *buf2 = frame.get();
    unsigned int sh = mContext->videoheight, sw = mContext->videowidth;

    for (unsigned int y = 0; y < sh; ++y) {
        for (unsigned int x = 0; x < sw; ++x) {
            buf1[y * w + x] = 0xff000000 | *(uint32_t *)( buf2 + (y * sw * 3 + x * 3) );
        }
    }
    return true;
}

void VideoBackgroundLoader::startLoadingThread()
{
    {
        boost::mutex::scoped_lock lock(backgroundMutex);
        if (mState != STARTING) {
            mState = STOPPED;
            return;
        }
        mState = RUNNING;
    }
    AVPacket packet;

    while (true) {
        {
            boost::mutex::scoped_lock lock(backgroundMutex);

            if (quitRequested)
                return;

            // move from initial or stopping to stopped
            if (mState != RUNNING) {
                mState = STOPPED;
                return;
            }
        }

        if (av_read_frame(mContext->formatctx, &packet) >= 0) {
            if (packet.stream_index == mContext->videoindex) {
                int framedone = 0;
                avcodec_decode_video2(mContext->videocodecctx, mContext->readframe, &framedone, &packet);
                if (framedone) {
                    size_t bsize = (mContext->videowidth * mContext->videoheight * 3 + 4);
                    boost::shared_array<unsigned char> buffer(new unsigned char[bsize]);
                    if (buffer) {
                        toRGB_convert_ctx = sws_getCachedContext(toRGB_convert_ctx,
                                                 mContext->videowidth, mContext->videoheight,
                                                 mContext->videocodecctx->pix_fmt,
                                                 mContext->videowidth, mContext->videoheight,
                                                 PIX_FMT_RGB24,
                                                 sws_flags, NULL, NULL, NULL);

                        sws_scale(toRGB_convert_ctx,
                                  mContext->readframe->data,
                                  mContext->readframe->linesize,
                                  0, mContext->videoheight,
                                  mContext->drawframe->data,
                                  mContext->drawframe->linesize);
                        unsigned int sh = mContext->videoheight, sw = mContext->videowidth;
                        memcpy(buffer.get(), mContext->drawframe->data[0], sw * sh * 3);

                        {
                            boost::mutex::scoped_lock lock(backgroundMutex);
                            mContext->frames.push_back(buffer);

                            if (quitRequested)
                                return;

                            // if too many buffered frames, sleep
                            if (mContext->frames.size() >= MAX_BUFFERED_FRAMES) {
                                backgroundReaderWaiting = true;
                                framesFullCVar.wait(lock);
                                backgroundReaderWaiting = false;
                            }
                        }
                    }
                }
            } else if (packet.stream_index == mContext->audioindex && mContext->audiobuffer) {
                int size = mContext->audiobuffersize;
                avcodec_decode_audio3(mContext->audiocodecctx, mContext->audiobuffer, &size, &packet);
                if (size) {
                    unsigned long long duration =
                        (unsigned long long)( (double(mContext->audiocodecctx->time_base.num)
                                               / mContext->audiocodecctx->time_base.den) * 1000 );
                    // mContext->audiostream->activate();
                    mContext->audiostream->addSample( (char *)mContext->audiobuffer, size, mContext->audiotime,
                                                      duration );
                    mContext->audiotime += duration;
                }
            }
            av_free_packet(&packet);
        } else { break; }
    }
    {
        boost::mutex::scoped_lock lock(backgroundMutex);
        mState = STOPPED;
    }
}

TReader::TReader()
{
    mLoader = new VideoBackgroundLoader();
    fps_numerator = 1;
    fps_denominator = 1;
    frame_width = 1;
    frame_height = 1;
}

TReader::~TReader()
{
    finish();
    delete mLoader;
}

int TReader::init()
{
    return false;
}

int TReader::read_info(const char *filename, IStorm3D_StreamBuilder *builder)
{
    if ( !mLoader->init(filename, builder) ) return true;
    mLoader->getVideoInfo(&fps_numerator, &fps_denominator, &frame_width, &frame_height);
    mLoader->start();
    return false;
}

int TReader::nextframe()
{
    return mLoader->finished();
}

int TReader::read_pixels(char *buffer, unsigned int w, unsigned int h)
{
    return !mLoader->readFrame(buffer, w, h);
}

int TReader::finish()
{
    if ( !mLoader->finished() )
        mLoader->stop();
    return 1;
}

void TReader::restart()
{
    mLoader->restart();
}
