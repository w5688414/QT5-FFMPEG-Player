#ifndef FRAMEQUEUE_H
#define FRAMEQUEUE_H
#include <queue>
#include <QMutex>
#include <QWaitCondition>
extern "C"{

#include <libavcodec\avcodec.h>
#include <libavformat\avformat.h>
}

class FrameQueue
{
public:
    FrameQueue();
    static const int capacity = 30;
    bool enQueue(const AVFrame* frame);
    AVFrame * deQueue();
    int getQueueSize();
    void queueFlush();
private:
    std::queue<AVFrame*> queue;
    QMutex mutex;
    QWaitCondition cond;
};

#endif // FRAMEQUEUE_H
