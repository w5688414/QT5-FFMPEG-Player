#ifndef VIDEOOPENGLWIDGET_H
#define VIDEOOPENGLWIDGET_H

#include <QWidget>
#include <QOpenGLWidget>
#include "readpacketsthread.h"
namespace Ui {
class VideoOpenGLWidget;
}

class VideoOpenGLWidget : public QOpenGLWidget
{
    Q_OBJECT
public:
    VideoOpenGLWidget(QWidget *parent = Q_NULLPTR);
    ~VideoOpenGLWidget();
    void paintEvent(QPaintEvent *e);
public slots:
    void setVideoImage(QImage* img,ReadPacketsThread* mReadPacketsThread);
private:
    Ui::VideoOpenGLWidget *ui;
};

#endif // VIDEOOPENGLWIDGET_H
