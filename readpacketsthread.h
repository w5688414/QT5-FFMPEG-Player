#ifndef READPACKETSTHREAD_H
#define READPACKETSTHREAD_H
#include<QThread>
#include <QMutex>
class Media;
#include "media.h"
class ReadPacketsThread : public QThread
{
    Q_OBJECT
public:
//    static ReadPacketsThread * getInstance() {
//        static ReadPacketsThread rpt;
//        return &rpt;
//    }
    ReadPacketsThread(Media* mMedia);
    void run();
    ~ReadPacketsThread();
    bool getIsPlaying();
    float currentPos = 0;
    bool isSeek = false;
    void setPlaying(bool isPlaying);
public slots:
    void receivePos(float pos);
private:
    QMutex mutex;
    bool isPlay = false;
    Media *mMedia;
};

#endif // READPACKETSTHREAD_H
