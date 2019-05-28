#ifndef MEDIA_H
#define MEDIA_H
#include "audio.h"
#include "video.h"
#include <QMutex>
class ReadPacketsThread;
#include "readpacketsthread.h"
class DisplayMediaTimer;
#include "displaymediatimer.h"

extern "C" {

#include <libavformat\avformat.h>

}

class Media
{
public:
//    static Media *getInstance() {
//            static Media media;
//            return &media;
//        }
    Media();
    ~Media();
    Media * config(ReadPacketsThread *mReadPacketsThread,DisplayMediaTimer *mDisplayMediaTimer);
    Media * setMediaFile(const char*filename);
    bool checkMediaSizeValid();
    int getVideoStreamIndex();
    int getAudioStreamIndex();
    void enqueueVideoPacket(const AVPacket &packet);
    void enqueueAudioPacket(const AVPacket &packet);
    void startAudioPlay();
    AVFormatContext *getAVFormatContext();
    Video *video;
    Audio *audio;
    void close(ReadPacketsThread *mReadPacketsThread,DisplayMediaTimer *mDisplayMediaTimer);
    int totalMs = 0;
    int pts = 0;
private:
    AVFormatContext *pFormatCtx;
    const char *filename;
    QMutex mutex;
};

#endif // MEDIA_H
