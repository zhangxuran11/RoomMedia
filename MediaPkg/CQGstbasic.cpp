#include "CQGstbasic.h"
#include <gst/interfaces/xoverlay.h>
#include "MediaPkgCommon.h"
#include<QTimer>
#include<QWidget>
#include<QPoint>
#include<QDebug>
#include<QThread>
#include<QFile>
#include<QTime>
CQGstBasic::CQGstBasic(int _winID,QWidget*  ,QObject* parent):
    QObject(parent),winID(_winID)
{
    if(QFile::exists("/qos.txt"))
    {
        QFile file("/qos.txt");
        if(file.size() > 10*1024*1024)
        {
            QFile::remove("/qos.txt");
        }
    }
    if(_winID != 0)
        row_rect = QWidget::find(_winID)->geometry();
    //playOutTimer.setInterval(3000);
    playOutTimer.setSingleShot(true);
    connect(&playOutTimer,SIGNAL(timeout()),this,SLOT(_playTimeOutExec()), Qt::DirectConnection);

    qos_cnt = 0;
    //初始化CQMedia各个变量

    //初始化gstreamer
    if(gst_init_check(NULL,NULL,NULL)==false)
    {
        return;
    }
}
void CQGstBasic::printDot(const gchar* fileName)
{
    GST_DEBUG_BIN_TO_DOT_FILE(GST_BIN(pipeline),GST_DEBUG_GRAPH_SHOW_ALL,fileName);
}
CQGstBasic::~CQGstBasic()
{
    if(this->getPlayingState()!=STOPPED)
    {
        this->stop();
    }
}



bool CQGstBasic::paused()
{
    GstStateChangeReturn ret;
    //判断播放是否已开始
    if(getPlayingState()==PAUSED)
    {
        return true;
    }
    ret=gst_element_set_state(this->pipeline,GST_STATE_PAUSED);

    if(GST_STATE_CHANGE_FAILURE!=ret)
    {
        this->state=PAUSED;
        qDebug("PAUSED");
        return true;
    }
    return false;
}

bool CQGstBasic::play()
{

    if(getPlayingState()==PLAYING)
    {
        qDebug("The gstreamer had being playing.");
        return true;
    }
#ifdef ARM
    if(getPlayingState()==STOPPED)
    {
        _updateDemux();
        _updateDecoder();
    }
#endif

    GstStateChangeReturn ret=gst_element_set_state(this->pipeline,GST_STATE_PLAYING);
    if(GST_STATE_CHANGE_FAILURE!=ret)
    {
        this->state=PLAYING;
        qDebug("Playing ... ");
        updateOverlay();
        system("echo play > /tmp/play.txt");
        return true;
    }
    qDebug("Playing error");
    return false;
}

bool CQGstBasic::play(const QString& _url)
{
//    qDebug("entry file:%s line:%d func:%s thread:%ld",__FILE__,__LINE__,__FUNCTION__,QThread::currentThreadId());

    if(getPlayingState()==STOPPED)
    {
        if(!loadURL(_url))
        {
//            qDebug("leave file:%s line:%d func:%s thread:%ld",__FILE__,__LINE__,__FUNCTION__,QThread::currentThreadId());
            qDebug("Load url failed ...");
            return false;
        }
    }
//    qDebug("leave file:%s line:%d func:%s thread:%ld",__FILE__,__LINE__,__FUNCTION__,QThread::currentThreadId());
    return play();
}
bool CQGstBasic::stopWithoutSignal()
{
    GstStateChangeReturn ret;
    if (STOPPED == getPlayingState()) {
      return false;
    }
    ret = gst_element_set_state(this->pipeline, GST_STATE_NULL);
    if (GST_STATE_CHANGE_FAILURE != ret) {
        this->state = STOPPED;
        QWidget* widget = QWidget::find(winID);
        widget->update();
        system("echo stop > /tmp/play.txt");
        return true;
    }
    else
    {
        return false;
    }
}

bool CQGstBasic::stop()
{
//    qDebug("entry file:%s line:%d func:%s thread:%ld",__FILE__,__LINE__,__FUNCTION__,QThread::currentThreadId());
    GstStateChangeReturn ret;
    if (STOPPED == getPlayingState()) {
//      qDebug("leave file:%s line:%d func:%s thread:%ld",__FILE__,__LINE__,__FUNCTION__,QThread::currentThreadId());
      return false;
    }
    ret = gst_element_set_state(this->pipeline, GST_STATE_NULL);
    if (GST_STATE_CHANGE_FAILURE != ret) {
        this->state = STOPPED;
        emitError(STOP);
        if(winID != 0)
        {
            //GstElement *videosink = gst_bin_get_by_name((GstBin*)pipeline,"v_sink");
            QWidget* widget = QWidget::find(winID);
            widget->update();
        }
//        qDebug("leave file:%s line:%d func:%s thread:%ld",__FILE__,__LINE__,__FUNCTION__,QThread::currentThreadId());
        system("echo stop > /tmp/play.txt");
        return true;
    }
    else
    {
//        qDebug("leave file:%s line:%d func:%s thread:%ld",__FILE__,__LINE__,__FUNCTION__,QThread::currentThreadId());
        return false;

    }

}

void CQGstBasic::_playTimeOutExec(){
    qosClear();
    stop();
    qDebug()<<"_playTimeOutExec ";
    emitError(CQGstBasic::QOS);
}

void CQGstBasic::qosClear(){
    QString printCmd = QString("echo qos timer is timeout qos is cleared  : ")+QDateTime::currentDateTime().toString()+" >> /qos.txt";
    system(printCmd.toAscii().data());
    qos_cnt = 0;
}
void CQGstBasic::screenFull(){
    static QTimer timer;
    static int init = 0;
    if(init == 0) {
        init++;
        timer.setSingleShot(true);
        timer.setInterval(1000);
        connect(&timer,SIGNAL(timeout()),this,SLOT(play()));
    }
    if(QWidget::find(winID)->geometry() == QRect(0,0,1280,768))
        return;
    if(getPlayingState() == PLAYING){
        stopWithoutSignal();
        QWidget::find(winID)->setGeometry(0,0,1280,768);
        timer.start();
    }
}
void CQGstBasic::screenNormal(){
    static QTimer timer;
    static int init = 0;
    if(init == 0) {
        init++;
        timer.setSingleShot(true);
        timer.setInterval(1000);
        connect(&timer,SIGNAL(timeout()),this,SLOT(play()));
    }
    if(QWidget::find(winID)->geometry() == row_rect)
        return;
    if(getPlayingState() == PLAYING){
        stopWithoutSignal();
        QWidget::find(winID)->setGeometry(row_rect);
        timer.start();
    }
    else
    {
        QWidget::find(winID)->setGeometry(row_rect);
    }
}
void CQGstBasic::updateOverlay()
{
    GstElement *videosink = gst_bin_get_by_name((GstBin*)pipeline,"v_sink");
    if(videosink != NULL && GST_IS_X_OVERLAY(videosink))
    {
        gst_x_overlay_set_xwindow_id(GST_X_OVERLAY(videosink),this->winID);
        qDebug("Overlay GST Overlay.");
    }
}
