#include "treader.h"

#ifdef USE_LIBAVCODEC

#include <boost/bind.hpp>
#include <istorm3d_streambuffer.h>
#include "igios.h"
#include <libavutil/log.h>

VideoBackgroundLoader::VideoBackgroundLoader() : mContext(0), mState(VideoBackgroundLoader::STOPPED)
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
	if (mContext->audiobuffer) delete [] mContext->audiobuffer;
	if (mContext->audioopen) avcodec_close(mContext->audiocodecctx);
	if (mContext->readframe) av_free(mContext->readframe);
	if (mContext->drawframe) av_free(mContext->drawframe);
	if (mContext->drawbuffer) delete [] mContext->drawbuffer;
	for (std::list<unsigned char*>::iterator it = mContext->frames.begin(); it != mContext->frames.end(); ++it)
		delete [] *it;
	if (mContext->videoopen) avcodec_close(mContext->videocodecctx);
	if (mContext->fileopen) av_close_input_file(mContext->formatctx);
	delete mContext;
}

bool VideoBackgroundLoader::init(const char *filename, IStorm3D_StreamBuilder *builder)
{
	if (av_open_input_file(&mContext->formatctx, filename, 0, 0, 0) != 0) {
		igiosWarning("Failed to load video '%s'.\n", filename);
		return false;
	} else mContext->fileopen = true;

	if (av_find_stream_info(mContext->formatctx) < 0) {
		igiosWarning("Failed to find stream information.\n");
		return false;
	}

	mContext->videoindex = mContext->audioindex = -1;
	for (unsigned int i = 0; i < mContext->formatctx->nb_streams; ++i) {
		if (mContext->formatctx->streams[i]->codec->codec_type == CODEC_TYPE_VIDEO && mContext->videoindex == -1) mContext->videoindex = i;
		else if (mContext->formatctx->streams[i]->codec->codec_type == CODEC_TYPE_AUDIO && mContext->audioindex == -1) mContext->audioindex = i;
	}

	if (mContext->videoindex == -1) {
		igiosWarning("No video streams detected.\n");
		return false;
	} else mContext->videocodecctx = mContext->formatctx->streams[mContext->videoindex]->codec;

	mContext->videocodec = avcodec_find_decoder(mContext->videocodecctx->codec_id);
	if (!mContext->videocodec) {
		igiosWarning("Unable to find suitable video codec.\n");
		return false;
	}

	if (mContext->videocodec->capabilities & CODEC_CAP_TRUNCATED)
		mContext->videocodecctx->flags |= CODEC_FLAG_TRUNCATED;

	if (avcodec_open(mContext->videocodecctx, mContext->videocodec) < 0) {
		igiosWarning("Unable to open video codec.\n");
		return false;
	} else mContext->videoopen = true;

	if(mContext->videocodecctx->time_base.num>1000 && mContext->videocodecctx->time_base.den==1) mContext->videocodecctx->time_base.den=1000;
	mContext->fpsnumerator = mContext->videocodecctx->time_base.num;
	mContext->fpsdenominator = mContext->videocodecctx->time_base.den;

	mContext->videowidth = mContext->videocodecctx->width;
	mContext->videoheight = mContext->videocodecctx->height;

	if (!(mContext->readframe = avcodec_alloc_frame())) {
		igiosWarning("Unable to allocate read frame.\n");
		return false;
	}

	if (!(mContext->drawframe = avcodec_alloc_frame())) {
		igiosWarning("Unable to allocate draw frame.\n");
		return false;
	}
	unsigned int bytes = avpicture_get_size(PIX_FMT_RGB24, mContext->videowidth, mContext->videoheight);
	mContext->drawbuffer = new unsigned char[bytes * 4 + 1];
	if (!mContext->drawbuffer) {
		igiosWarning("Unable to allocate draw buffer.\n");
		return false;
	}
	avpicture_fill((AVPicture*)mContext->drawframe, mContext->drawbuffer, PIX_FMT_RGB24, mContext->videowidth, mContext->videoheight);

	if (mContext->audioindex != -1 && builder) {
		mContext->audiocodecctx = mContext->formatctx->streams[mContext->audioindex]->codec;
		mContext->audiocodec = avcodec_find_decoder(mContext->audiocodecctx->codec_id);
		if (!mContext->audiocodec) igiosWarning("Unable to find suitable audio codec.\n");
		else {
			if (avcodec_open(mContext->audiocodecctx, mContext->audiocodec) < 0) igiosWarning("Unable to open audio codec.\n");
			else {
				mContext->audioopen = true;
				mContext->audiobuffersize = (AVCODEC_MAX_AUDIO_FRAME_SIZE * 3) / 2;
				mContext->audiobuffer = new int16_t[mContext->audiobuffersize];
				if (!mContext->audiobuffer) igiosWarning("Unable to allocate audio buffer.\n");
				else {
					builder->setStereo(mContext->audiocodecctx->channels);
					builder->setFrequency(mContext->audiocodecctx->sample_rate);
					builder->setBits(16);
					mContext->audiostream = builder->getStream();
				}
			}
		}
	}

	return true;
}

void VideoBackgroundLoader::restart()
{
	stop();
	
	for (std::list<unsigned char*>::iterator it = mContext->frames.begin(); it != mContext->frames.end(); ++it)
		delete [] *it;

	av_seek_frame(mContext->formatctx, mContext->videoindex, 0, AVSEEK_FLAG_ANY);
	av_seek_frame(mContext->formatctx, mContext->audioindex, 0, AVSEEK_FLAG_ANY);

	start();
}

bool VideoBackgroundLoader::finished() 
{
	{
		boost::mutex::scoped_lock lock(mContextMutex);
		if (!mContext->frames.empty()) return false;
	}
	boost::mutex::scoped_lock lock(mStateMutex);
	if (mState == STOPPED) return true;
	else return false;
}

void VideoBackgroundLoader::start() 
{
	boost::mutex::scoped_lock lock(mStateMutex);
	if (mState == STOPPED) {
		mState = STARTING;
		mThread = boost::thread(boost::bind(&VideoBackgroundLoader::startLoadingThread, this));
	}
}

void VideoBackgroundLoader::stop() 
{
	bool join = false;
	{
		boost::mutex::scoped_lock lock(mStateMutex);
		if (mState != STOPPED) {
			mState = STOPPING;
			join = true;
		}
	}
	if (join) mThread.join();
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
	unsigned char *frame;
	{
		boost::mutex::scoped_lock lock(mContextMutex);
		if (mContext->frames.size()) {
			frame = mContext->frames.front();
			mContext->frames.pop_front();
		} else return false;
	}

	uint32_t *buf1 = (uint32_t*)buffer;
	uint8_t *buf2 = frame;
	unsigned int sh = mContext->videoheight, sw = mContext->videowidth;
	for (unsigned int y = 0; y < sh; ++y) {
		for (unsigned int x = 0; x < sw; ++x) {
			buf1[y * w + x] = 0xff000000 | *(uint32_t*)(buf2 + (y * sw * 3 + x * 3));
		}
	}
	delete [] frame;
	return true;
}

void VideoBackgroundLoader::startLoadingThread() 
{
	{
		boost::mutex::scoped_lock lock(mStateMutex);
		if (mState != STARTING) {
			mState = STOPPED;
			return;
		}
		mState = RUNNING;
	}
	AVPacket packet;

	for (;;) {
		for (;;) {
			{
				boost::mutex::scoped_lock lock(mStateMutex);
				if (mState != RUNNING) {
					mState = STOPPED;
					return;
				}
			}
			bool wait = false;
			{
				boost::mutex::scoped_lock lock(mContextMutex);
				if (mContext->frames.size() == 25) wait = true;
			}
			if (!wait) break;
			else usleep(100);
		}
		if (av_read_frame(mContext->formatctx, &packet) >= 0) {
			if (packet.stream_index == mContext->videoindex) {
				int framedone = 0;
				avcodec_decode_video(mContext->videocodecctx, mContext->readframe, &framedone, packet.data, packet.size);
				if (framedone) {
					unsigned char *buffer = new unsigned char[mContext->videowidth * mContext->videoheight * 3];
					if (buffer) {
						img_convert((AVPicture*)mContext->drawframe, PIX_FMT_RGB24,
												(AVPicture*)mContext->readframe, mContext->videocodecctx->pix_fmt,
												mContext->videowidth, mContext->videoheight);
						unsigned int sh = mContext->videoheight, sw = mContext->videowidth;
						memcpy(buffer, mContext->drawframe->data[0], sw * sh * 3);
						boost::mutex::scoped_lock lock(mContextMutex);
						mContext->frames.push_back(buffer);
					}
				}
			} else if (packet.stream_index == mContext->audioindex && mContext->audiobuffer) {
				int size = mContext->audiobuffersize;
				avcodec_decode_audio2(mContext->audiocodecctx, mContext->audiobuffer, &size, packet.data, packet.size);
				if (size) {
					unsigned long long duration = (unsigned long long)((double(mContext->audiocodecctx->time_base.num) / mContext->audiocodecctx->time_base.den) * 1000);
					mContext->audiostream->activate();
					mContext->audiostream->addSample((char*)mContext->audiobuffer, size, mContext->audiotime, duration);
					mContext->audiotime += duration;
				}
			}
			av_free_packet(&packet);
		} else break;
	}
	{
		boost::mutex::scoped_lock lock(mStateMutex);
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
	if (!mLoader->init(filename, builder)) return true;
	mLoader->getVideoInfo(&fps_numerator, &fps_denominator, &frame_width, &frame_height);
	mLoader->start();
	return false;
}

int TReader::nextframe()
{
	return mLoader->finished();
}

int TReader::read_pixels(char* buffer, unsigned int w, unsigned int h)
{
	return !mLoader->readFrame(buffer, w, h);
}

int TReader::finish()
{
	if (!mLoader->finished()) mLoader->stop();
	return 1;
}

void TReader::restart()
{
	mLoader->restart();
}

#else

TReader::TReader()
{
	fps_numerator = 1;
	fps_denominator = 1;
	frame_width = 1;
	frame_height = 1;
}

TReader::~TReader() {}

int TReader::init()
{
	return 1;
}

int TReader::read_info(const char *filename, IStorm3D_StreamBuilder *builder)
{
	return 1;
}

int TReader::nextframe()
{
	return 1;
}

int TReader::read_pixels(char* buffer, unsigned int w, unsigned int h)
{
	return 1;
}

int TReader::finish()
{
	return 1;
}

void TReader::restart()
{
}

#endif
