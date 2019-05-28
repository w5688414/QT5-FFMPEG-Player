#ifndef DISPLAYMEDIATIMER_H
#define DISPLAYMEDIATIMER_H
#include <QTimer>
#include <QMutex>
#include<QImage>
#include <QObject>
#include "imagefilter.h"
class Media;
#include "media.h"
#include "readpacketsthread.h"
extern "C" {

#include <libswscale\swscale.h>
#include <libavutil\time.h>
#include <libavformat\avformat.h>
#include <libswresample\swresample.h>
}



class DisplayMediaTimer:public QObject
{
    Q_OBJECT
public:
//    static DisplayMediaTimer *getInstance() {
//        static DisplayMediaTimer displayMediaThread;
//        return &displayMediaThread;
//    }
    bool isPlaying();//获取播放状态
    void setPlay(bool isPlaying);//设置播放状态
    ~DisplayMediaTimer();
    void resetImage(int width,int height);
    DisplayMediaTimer();
    void config(ImageFilter *mImageFilter,Media *mMedia,ReadPacketsThread* mReadPacketsThread);
public slots:
    void synchronizeAudioAndVideo();
private:
    bool isPlay = false;
    QTimer *timer;//视频定时刷新器
    uchar *buf;
    QImage *img;
    int imageWidth = 800;
    int imageHeight = 600;
    QMutex mutex;
    Media *mMedia;
    ImageFilter *mImageFilter;
    ReadPacketsThread* mReadPacketsThread;

signals:
    void updateFrame(QImage *image,ReadPacketsThread* mReadPacketsThread);//发送到opengl
};

#endif // DISPLAYMEDIATIMER_H
