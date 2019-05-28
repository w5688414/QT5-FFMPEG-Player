#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include<QPropertyAnimation>
#include"media.h"
#include "readpacketsthread.h"
#include "displaymediatimer.h"
#include "imagefilter.h"
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavdevice/avdevice.h"
}

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void resizeEvent(QResizeEvent *e);
    void contextMenuEvent(QContextMenuEvent *event);
    void mirrorLeftAndRight();
    void mirrorUpAndDown();
    void gray2Rgb();
    void rgb2Gray();
    void netAddressInput();
    void openNetAddressVideo(QString address);
    void mouseDoubleClickEvent(QMouseEvent * event);
    void mouseMoveEvent(QMouseEvent *event);
public slots:
    void openVideoFile();
    void timerEvent(QTimerEvent *e);
    void sliderPress();
    void sliderRelease();
    void play();
    void setVolume(int volume);
signals:
    void sendPos(float pos);
private:
    QPropertyAnimation *bottomAnimation;
    bool isFullScreen = false;
    void showBottomInAnimation();
    void hideBottomInAnimation();
    QMenu *popMenu;
    QAction *leftRightMirrorAction;
    QAction *upDownMirrorAction;
    QAction *rgbAction;
    QAction *grayAction;
    QAction *netAddressAction;
    Media *mMedia;
    ReadPacketsThread *mReadPacketsThread;;
    DisplayMediaTimer *mDisplayMediaTimer;
    ImageFilter *mImageFilter;
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
