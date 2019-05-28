#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QVBoxLayout>
#include<QFileDialog>
#include<QMessageBox>
#include "imagefilter.h"
#include "media.h"
#include "readpacketsthread.h"
#include <QContextMenuEvent>
#include <QInputDialog>
#include "displaymediatimer.h"
#include<QMenu>
static bool isPressSlider = false;
static bool isPlay = false;
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

        setMinimumWidth(400);
        setMinimumHeight(300);
        bottomAnimation = new QPropertyAnimation(ui->bottemWidget, "geometry");
        setMouseTracking(true);
        ui->openGLWidget->setMouseTracking(true);
        popMenu = new QMenu(this);
        leftRightMirrorAction = new QAction(this);
        upDownMirrorAction = new QAction(this);
        rgbAction = new QAction(this);
        grayAction = new QAction(this);
        netAddressAction = new QAction(this);
        leftRightMirrorAction->setText(QString::fromLocal8Bit("left right mirror"));
        upDownMirrorAction->setText(QString::fromLocal8Bit("up down mirror"));
        rgbAction->setText(QString::fromLocal8Bit("color"));
        grayAction->setText(QString::fromLocal8Bit("black"));
        netAddressAction->setText(QString::fromLocal8Bit("network stream"));
        connect(leftRightMirrorAction, &QAction::triggered, this, &MainWindow::mirrorLeftAndRight);
        connect(upDownMirrorAction, &QAction::triggered, this, &MainWindow::mirrorUpAndDown);
        connect(rgbAction, &QAction::triggered, this, &MainWindow::gray2Rgb);
        connect(grayAction, &QAction::triggered, this, &MainWindow::rgb2Gray);
        connect(netAddressAction, &QAction::triggered, this, &MainWindow::netAddressInput);

        startTimer(40);
//        mReadPacketsThread = ReadPacketsThread::getInstance();
//        mDisplayMediaTimer=DisplayMediaTimer::getInstance();
//        mMedia=Media::getInstance();
//        mImageFilter=ImageFilter::getInstance();

        mDisplayMediaTimer=new DisplayMediaTimer();
        mMedia=new Media();
        mReadPacketsThread=new ReadPacketsThread(mMedia);
        mImageFilter=new ImageFilter();
        QObject::connect(this,//信号发出的对象
            SIGNAL(sendPos(float)),//信号
            mReadPacketsThread,//槽接收的对象
            SLOT(receivePos(float))//槽
        );

        QObject::connect(mDisplayMediaTimer,//信号发出的对象
                SIGNAL(updateFrame(QImage*,ReadPacketsThread*)),//信号
                ui->openGLWidget,//槽接收的对象
                SLOT(setVideoImage(QImage*,ReadPacketsThread*))//槽
            );
        mReadPacketsThread->start();
//        mMedia=new Media;
        mDisplayMediaTimer->config(mImageFilter,mMedia,mReadPacketsThread); //加载配置

        mMedia->video->start();
        play();
}

void MainWindow::netAddressInput() {
    bool ok = FALSE;

    QString text = QInputDialog::getText( this,
    QString::fromLocal8Bit("network stream"),
    QString::fromLocal8Bit("please input the url"),
    QLineEdit::Normal, QString::null, &ok);
    if (ok&&!text.isEmpty()) {
        openNetAddressVideo(text);
    }

    else
        ;// 用户按下Cancel

}

void MainWindow::openNetAddressVideo(QString address)
{
    std::string file = address.toLocal8Bit().data();//防止有中文
    mMedia->setMediaFile(file.c_str())
        ->config(mReadPacketsThread,mDisplayMediaTimer);
    setWindowTitle(address);
    int total = mMedia->totalMs;
    ui->totalHour->setText(QString::number((int)(mMedia->totalMs / 1000 / 60 / 60)) + ":");//计算视频总的时分秒
    ui->totalMinute->setText(QString::number((int)(mMedia->totalMs / 1000 / 60) % 60) + ":");
    ui->totalSecond->setText(QString::number((int)(mMedia->totalMs / 1000) % 60 % 60));
    isPlay = false;
    play();
    float pos = 0;
    pos = (float)ui->volumeSlider->value() / (float)(ui->volumeSlider->maximum() + 1);
    if (mMedia->getAVFormatContext())
        mMedia->audio->setVolume(pos*SDL_MIX_MAXVOLUME);//初始化音量为最大音量的一半
}


//************************************
// Method:    contextMenuEvent
// FullName:  QTPlayer::contextMenuEvent
// Access:    public
// Returns:   void
// Qualifier:右击弹出菜单
// Parameter: QContextMenuEvent * event
//************************************
void MainWindow::contextMenuEvent(QContextMenuEvent *event) {
    popMenu->clear();
    popMenu->addAction(leftRightMirrorAction);
    popMenu->addAction(upDownMirrorAction);
    popMenu->addAction(rgbAction);
    popMenu->addAction(grayAction);
    popMenu->addAction(netAddressAction);

    //菜单出现的位置为当前鼠标的位置
    popMenu->exec(QCursor::pos());
    event->accept();
}

//************************************
// Method:    mouseDoubleClickEvent
// FullName:  QTPlayer::mouseDoubleClickEvent
// Access:    public
// Returns:   void
// Qualifier:鼠标双击最大化最小化
// Parameter: QMouseEvent * event
//************************************
void MainWindow::mouseDoubleClickEvent(QMouseEvent * event) {
    if (windowState() &  Qt::WindowFullScreen)
        showNormal();
    else
        showFullScreen();
}


//************************************
// Method:    mouseMoveEvent
// FullName:  QTPlayer::mouseMoveEvent
// Access:    public
// Returns:   void
// Qualifier:隐藏，显示底部
// Parameter: QMouseEvent * event
//************************************
void MainWindow::mouseMoveEvent(QMouseEvent *event) {
    int y = event->globalY();
    if (y >= height() - 100 || y <= 5) {
        showBottomInAnimation();
    }
    else {
        hideBottomInAnimation();
    }
}



MainWindow::~MainWindow()
{
    delete ui;
}

//************************************
// Method:    openVideoFile
// FullName:  QTPlayer::openVideoFile
// Access:    public
// Returns:   void
// Qualifier:选择打开视频文件
//************************************
void MainWindow::openVideoFile() {
    QString fileName = QFileDialog::getOpenFileName(this, "please select your video file!");
    if (fileName.isEmpty()) {
        return;
    }
    QStringList titles = fileName.split("/");
    QString postfix = titles.constLast().split(".").last();
    if (postfix != QString::fromLocal8Bit("mp4")
        && postfix != QString::fromLocal8Bit("rmvb")
        && postfix != QString::fromLocal8Bit("flv")
        && postfix != QString::fromLocal8Bit("avi")
        && postfix != QString::fromLocal8Bit("mkv")
        && postfix != QString::fromLocal8Bit("rm")
        && postfix != QString::fromLocal8Bit("wmv")
        ) {
        return;
    }
    std::string file = fileName.toLocal8Bit().data();//防止有中文
    qDebug()<<fileName<<endl;
    mMedia->setMediaFile(file.c_str())
        ->config(mReadPacketsThread,mDisplayMediaTimer);

    setWindowTitle(titles.constLast());
    int total =mMedia->totalMs;
    ui->totalHour->setText(QString::number((int)(mMedia->totalMs / 1000 / 60 / 60)) + ":");//计算视频总的时分秒
    ui->totalMinute->setText(QString::number((int)(mMedia->totalMs / 1000 / 60) % 60) + ":");
    ui->totalSecond->setText(QString::number((int)(mMedia->totalMs / 1000) % 60 % 60));
    isPlay = false;
    play();
    float pos = 0;
    pos = (float)ui->volumeSlider->value() / (float)(ui->volumeSlider->maximum() + 1);
    if (mMedia->getAVFormatContext())
        mMedia->audio->setVolume(pos*SDL_MIX_MAXVOLUME);//初始化音量为最大音量的一半
}

//************************************
// Method:    play
// FullName:  QTPlayer::play
// Access:    public
// Returns:   void
// Qualifier:视频播放
//************************************
void MainWindow::play() {
    const QString PAUSE = "QPushButton#playButton{border-image:url(:/QTPlayer/Resources/PauseP.png);}";

    const QString PLAY = "QPushButton#playButton{border-image:url(:/QTPlayer/Resources/PlayP.png);}";
    isPlay = !isPlay;
    mDisplayMediaTimer->setPlay(isPlay);
    if (isPlay)
    {
        ui->bottemWidget->setStyleSheet(PAUSE);
        if(mMedia->getAVFormatContext())
            mMedia->audio->audioPlay();
    }
    else
    {
        ui->bottemWidget->setStyleSheet(PLAY);
        if (mMedia->getAVFormatContext())
            SDL_CloseAudio();
    }
}



//************************************
// Method:    setVolume
// FullName:  QTPlayer::setVolume
// Access:    public
// Returns:   void
// Qualifier: 设置音量大小
// Parameter: int volume 音量大小0-128
//************************************
void MainWindow::setVolume(int volume) {
    float pos = 0;
    pos = (float)ui->volumeSlider->value() / (float)(ui->volumeSlider->maximum() + 1);
    if (mMedia->getAVFormatContext())
        mMedia->audio->setVolume(pos*SDL_MIX_MAXVOLUME);
}

//************************************
// Method:    showBottomInAnimation
// FullName:  QTPlayer::showBottomInAnimation
// Access:    private
// Returns:   void
// Qualifier:底部显示动画效果
//************************************
void MainWindow::showBottomInAnimation()
{
    if (ui->bottemWidget->y() == height()) {
        bottomAnimation->setDuration(500);
        bottomAnimation->setStartValue(QRect(ui->bottemWidget->x(), height(), ui->bottemWidget->width(), ui->bottemWidget->height()));

        bottomAnimation->setEndValue(QRect(ui->bottemWidget->x(), this->height() - ui->bottemWidget->height(), ui->bottemWidget->width(), ui->bottemWidget->height()));
        bottomAnimation->start();
    }
}

//************************************
// Method:    hideBottomInAnimation
// FullName:  QTPlayer::hideBottomInAnimation
// Access:    private
// Returns:   void
// Qualifier:底部隐藏动画效果
//************************************
void MainWindow::hideBottomInAnimation()
{
    if (ui->bottemWidget->y() == this->height() - ui->bottemWidget->height()) {
        bottomAnimation->setDuration(500);
        bottomAnimation->setStartValue(QRect(ui->bottemWidget->x(), this->height() - ui->bottemWidget->height(), ui->bottemWidget->width(), ui->bottemWidget->height()));

        bottomAnimation->setEndValue(QRect(ui->bottemWidget->x(), this->height(), ui->bottemWidget->width(), ui->bottemWidget->height()));
        bottomAnimation->start();
    }
}

//************************************
// Method:    timerEvent
// FullName:  QTPlayer::timerEvent
// Access:    public
// Returns:   void
// Qualifier:设置进度条和播放时间
// Parameter: QTimerEvent * e
//************************************
void MainWindow::timerEvent(QTimerEvent * e)
{
    if (mMedia->totalMs > 0)
    {
        double pts = (double)mMedia->audio->pts * 1000;
        double total = (double)mMedia->totalMs;
        double rate = pts / total;
        if (!isPressSlider&&isPlay) {
            ui->playslider->setValue(rate);
            ui->currentHour->setText(QString::number((int)(pts / 1000000 / 60 / 60))+":");
            ui->currentMinute->setText(QString::number((int)(pts / 1000000 / 60) % 60) + ":");
            ui->currentSecond->setText(QString::number((int)(pts / 1000000) % 60 % 60));
        }

    }
}

//************************************
// Method:    sliderPress
// FullName:  QTPlayer::sliderPress
// Access:    public
// Returns:   void
// Qualifier: 按下进度条
//************************************
void MainWindow::sliderPress()
{
    isPressSlider = true;
}

//************************************
// Method:    sliderRelease
// FullName:  QTPlayer::sliderRelease
// Access:    public
// Returns:   void
// Qualifier: 进度条释放
//************************************
void MainWindow::sliderRelease()
{
    isPressSlider = false;
    float pos = 0;
    pos = (float)ui->playslider->value() / (float)(ui->playslider->maximum() + 1);
    sendPos(pos);
}

//************************************
// Method:    mirrorUpAndDown
// FullName:  QTPlayer::mirrorUpAndDown
// Access:    public
// Returns:   void
// Qualifier:上下镜像操作
//************************************
void MainWindow::mirrorUpAndDown() {
    if (mMedia->getAVFormatContext() && isPlay)
        mImageFilter->addTask(XTask{ XTASK_MIRRORUPANDDOWN });
}

//************************************
// Method:    mirrorLeftAndRight
// FullName:  QTPlayer::mirrorLeftAndRight
// Access:    public
// Returns:   void
// Qualifier:左右镜像操作
//************************************
void MainWindow::mirrorLeftAndRight() {
    if (mMedia->getAVFormatContext() && isPlay)
        mImageFilter->addTask(XTask{ XTASK_MIRRORLEFTANDRIGHT });
}

//************************************
// Method:    gray2Rgb
// FullName:  QTPlayer::gray2Rgb
// Access:    public
// Returns:   void
// Qualifier: 灰度图转rgb
//************************************
void MainWindow::gray2Rgb()
{
    if (mMedia->getAVFormatContext() && isPlay)
        mImageFilter->addColorTask(ColorTask{ COLRTASK_GRAY_TO_RGBA });
}

//************************************
// Method:    rgb2Gray
// FullName:  QTPlayer::rgb2Gray
// Access:    public
// Returns:   void
// Qualifier:rgb图转灰度图
//************************************
void MainWindow::rgb2Gray()
{
    if (mMedia->getAVFormatContext() && isPlay) {
        for(int i=0;i<5;i++)
        mImageFilter->addColorTask(ColorTask{ COLRTASK_RGBA_TO_GRAY });
    }

}

//************************************
// Method:    resizeEvent
// FullName:  QTPlayer::resizeEvent
// Access:    public
// Returns:   void
// Qualifier: 窗体改变大小时，适配所有控件，以播放按钮为基准
// Parameter: QResizeEvent * e
//************************************
void MainWindow::resizeEvent(QResizeEvent *e) {
    ui->openGLWidget->resize(size());
    ui->bottemWidget->resize(this->width(), ui->bottemWidget->height());
    ui->bottemWidget->move(0,this->height() - ui->bottemWidget->height());

    ui->playButton->move((ui->bottemWidget->width()- ui->playButton->width())/2, (ui->bottemWidget->height() - ui->playButton->height()) / 2);
    ui->openButton->move(ui->playButton->x()+ ui->playButton->width()+30, ui->playButton->y());

    ui->volumeSlider->move(this->width()-ui->volumeSlider->width()-25, ui->bottemWidget->height() - ui->volumeSlider->height()-20);
    ui->playslider->move(25, ui->playButton->y()-20);
    ui->playslider->resize(ui->bottemWidget->width() - 50, ui->playslider->height());

    ui->totalLayout->setSpacing(0);
    ui->totalContainer->setLayout(ui->totalLayout);
    ui->totalContainer->move(ui->playslider->x() + ui->playslider->width() - ui->totalContainer->width()-10, ui->playslider->y()+10);

    ui->currentLayout->setSpacing(0);
    ui->currentContainer->setLayout(ui->currentLayout);
    ui->currentContainer->resize(ui->totalContainer->size());
    ui->currentContainer->move(ui->playslider->x()+10, ui->playslider->y() + 10);
    mDisplayMediaTimer->resetImage(width(), height());
}
