#ifndef __THEORA_READER__
#define __THEORA_READER__

#include <boost/utility.hpp>

class IStorm3D_Stream;
class IStorm3D_StreamBuilder;

#include <boost/thread.hpp>
#include <boost/shared_array.hpp>
#include <boost/shared_ptr.hpp>

struct AVFormatContext;
struct AVCodecContext;
struct AVCodec;
struct AVFrame;
struct SwsContext;

struct avctx : public boost::noncopyable {
    bool               fileopen, videoopen, audioopen;
    AVFormatContext   *formatctx;
    AVCodecContext    *videocodecctx, *audiocodecctx;
    int                videoindex, audioindex;
    AVCodec           *videocodec, *audiocodec;

    std::list<boost::shared_array<unsigned char> > frames;
    AVFrame           *readframe, *drawframe;
    unsigned char     *drawbuffer;

    unsigned int       videowidth, videoheight;
    unsigned int       fpsnumerator, fpsdenominator;

    short             *audiobuffer;
    unsigned int       audiobuffersize;
    unsigned long long audiotime;
    boost::shared_ptr<IStorm3D_Stream> audiostream;
    avctx() : fileopen(false), videoopen(false), audioopen(false),
        formatctx(0), videocodecctx(0), audiocodecctx(0),
        videoindex(-1), audioindex(-1),
        videocodec(0), audiocodec(0),
        readframe(0), drawframe(0), drawbuffer(0),
        videowidth(1), videoheight(1),
        fpsnumerator(1), fpsdenominator(1),
        audiobuffer(0), audiobuffersize(0), audiotime(0) { }
};

class VideoBackgroundLoader : public boost::noncopyable {
    avctx *mContext;
    enum ENUM_LOADER_STATE {
        STARTING = 1,
        RUNNING,
        STOPPING,
        STOPPED,
    };
    ENUM_LOADER_STATE mState;
    boost::mutex backgroundMutex;
    boost::thread mThread;
    bool quitRequested;

    // background loader waits on this when there are
    // maximum allowed buffered frames
    // associated mutex is mContextMutex
    boost::condition_variable framesFullCVar;
    bool backgroundReaderWaiting;

    SwsContext *toRGB_convert_ctx;
    int sws_flags;

public:
    VideoBackgroundLoader();
    ~VideoBackgroundLoader();
    bool init(const char *filename, IStorm3D_StreamBuilder *builder);
    bool finished();
    void start();
    void stop();
    void restart();
    void getVideoInfo(unsigned int *fps_num, unsigned int *fps_den, unsigned int *w, unsigned int *h);
    bool readFrame(char *buffer, const unsigned int w, const unsigned int h);
private:
    void startLoadingThread();
};

class TReader : public boost::noncopyable {
private:
    VideoBackgroundLoader * mLoader;
public:
    unsigned int fps_numerator, fps_denominator, frame_width, frame_height;

    TReader();
    ~TReader();

    int read_info(const char *filename, IStorm3D_StreamBuilder *builder);
    int init();
    int nextframe();
    int read_pixels(char *buffer, unsigned int w, unsigned int h);
    int finish();
    void restart();
};

#endif
