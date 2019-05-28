#include "readpacketsthread.h"
#include "Media.h"
#include <QMutexLocker>
extern "C" {

#include <libavformat\avformat.h>

}
static bool isExit = false;

ReadPacketsThread::ReadPacketsThread(Media* mMedia)
{
    this->mMedia = mMedia;
}

//************************************
// Method:    run
// FullName:  ReadPacketsThread::run
// Access:    public
// Returns:   void
// Qualifier:读取音视频包的线程处理函数
//************************************
void ReadPacketsThread::run()
{
    AVPacket packet;
    while (!isExit) {
        QMutexLocker locker(&mutex);
        if (!isPlay) {//还没开始播放
            locker.unlock();
            msleep(100);
            continue;
        }

        if (mMedia->audio == nullptr || mMedia->video == nullptr) {
            break;
        }
        if (isSeek) {//是否跳转视频的标识，在解压音视频读包时进行跳转
            int64_t stamp = 0;
            stamp = currentPos * mMedia->video->getVideoStream()->duration;

            int ret = av_seek_frame(mMedia->getAVFormatContext(), mMedia->getVideoStreamIndex(),
                stamp, AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_FRAME);
            mMedia->audio->clearPacket();//要清空所有队列
            mMedia->video->clearPackets();
            mMedia->video->clearFrames();
            isSeek = false;
        }

        bool isInvalid = mMedia->checkMediaSizeValid();//音视队列超过一定量时先不读包，等待包消费
        if (isInvalid) {
            locker.unlock();
            msleep(10);
            continue;
        }
        if (!mMedia->getAVFormatContext()) {
            locker.unlock();
            msleep(10);
            continue;
        }
        int ret = av_read_frame(mMedia->getAVFormatContext(), &packet);//读帧音视频包
        if (ret < 0)
        {
            if (ret == AVERROR_EOF) {//读包出错
                break;
            }

            if (mMedia->getAVFormatContext()->pb->error == 0) // 没有错误就等待下次读
            {
                locker.unlock();
                msleep(100);
                continue;
            }
            else {
                break;
            }

        }
        if (mMedia->audio!=nullptr&& packet.stream_index == mMedia->getAudioStreamIndex()) // 音频包队列此处入队
        {
            locker.unlock();
            mMedia->enqueueAudioPacket(packet);
        }

        else if (mMedia->video != nullptr&& packet.stream_index == mMedia->getVideoStreamIndex()) // 视频包队列此处入队
        {
            locker.unlock();
            mMedia->enqueueVideoPacket(packet);
        }
        else {
            av_packet_unref(&packet);
        }

    }
    if(packet.size>=0)
        av_packet_unref(&packet);//包没数据不能释放
}



ReadPacketsThread::~ReadPacketsThread()
{
    QMutexLocker locker(&mutex);
    isExit = true;
    locker.unlock();
    wait();
}

//************************************
// Method:    getIsPlaying
// FullName:  ReadPacketsThread::getIsPlaying
// Access:    public
// Returns:   bool
// Qualifier: 获取视频播放状态
//************************************
bool ReadPacketsThread::getIsPlaying()
{
    return isPlay;
}

//************************************
// Method:    setPlaying
// FullName:  ReadPacketsThread::setPlaying
// Access:    public
// Returns:   void
// Qualifier: 设置播放状态
// Parameter: bool isPlaying
//************************************
void ReadPacketsThread::setPlaying(bool isPlaying)
{
    this->isPlay = isPlaying;
}

//************************************
// Method:    receivePos
// FullName:  ReadPacketsThread::receivePos
// Access:    public
// Returns:   void
// Qualifier: 接收跳转的位置，跳转标识设置true
// Parameter: float pos 视频跳转位置
//************************************
void ReadPacketsThread::receivePos(float pos) {
    currentPos = pos;
    isSeek = true;
}
