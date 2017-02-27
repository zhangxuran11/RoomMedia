#define __MAIN__
#include "CQplayerGUI.h"
#include "ui_CQplayerGUI.h"
#include "config.h"
#include "unistd.h"
#include<QDebug>
#include <iostream>
#include <QTimer>
#include <QFile>
#include<QtCore>
#include <QtGui/QApplication>
#include <QTranslator>
#include "callctlunit.h"
#include "roompanel.h"
#include "sdpsrcparser.h"
#include"workthread.h"
#include"softwatchdog.h"
#include"zutility.h"
#include"ztools.h"
#include"versionsender.h"
using namespace std;
static bool heartSwitch = true;
static bool disableBtn = false;
static QTimer disableTimer;
void CQplayerGUI::sendHeart()
{
    static ZTPManager* ztpm = new ZTPManager;
    static QString carIp = ZTools::getCarIP();
    if(!heartSwitch)
        return;

    ZTPprotocol ztp;
    ztp.addPara("T","Heart");
    ztp.addPara("CarIP",carIp);
    ztpm->SendOneZtp(ztp,QHostAddress("224.102.228.41"),6600);
    qDebug("Video send >>>");
    ztp.print();
    qDebug("Video send <<<");
}
void CQplayerGUI::slot_turnOffHeart()
{
    ZTPprotocol ztp;
    ZTPManager* ztpm = (ZTPManager*)sender();
    ztpm->getOneZtp(ztp);
    ztp.print();
    if(ztp.getPara("T") == "debug" && ztp.getPara("heartSwitch") == "off")
        heartSwitch = false;
    if(ztp.getPara("T") == "debug" && ztp.getPara("heartSwitch") == "on")
        heartSwitch = true;
}
void CQplayerGUI::slot_disableTime()
{
    disableBtn = false;
}
CQplayerGUI::CQplayerGUI(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CQplayerGUI)
{
    translator = new MyTranslator(":/language_thai.ini");
    disableTimer.setSingleShot(true);
    disableTimer.setInterval(3000);
    connect(&disableTimer,SIGNAL(timeout()),this,SLOT(slot_disableTime()));
    versionCtrl = new VersionSender("RoomMedia",1,1,12);

    ztpmForTest = new ZTPManager(8319,QHostAddress("224.102.228.40"));
    connect(ztpmForTest,SIGNAL(readyRead()),this,SLOT(slot_procTestZtp()));


    ZTPManager* ztpm = new ZTPManager(6633,QHostAddress("224.102.228.41"));
    connect(ztpm,SIGNAL(readyRead()),this,SLOT(slot_turnOffHeart()));
    QTimer* timer = new QTimer();
    timer->setSingleShot(false);
    timer->setInterval(3000);
    timer->start();
    connect(timer,SIGNAL(timeout()),this,SLOT(sendHeart()));
    ui->setupUi(this);



    btn[0] = ui->btn_ch0;
    btn[1] = ui->btn_ch1;
    btn[2] = ui->btn_ch2;
    btn[3] = ui->btn_ch3;
    btn[4] = ui->btn_ch4;
    btn[5] = ui->btn_ch5;
    btn[6] = ui->btn_ch6;
    btn[7] = ui->btn_ch7;
    btn[8] = ui->btn_ch8;

    stackPanel = new StackPanel(translator,ui->frameMovie);
    stackPanel->hide();
    GlobalInfo* global = GlobalInfo::getInstance();
    global->playerGui = this;
    WorkThread::getInstance()->start();
    setWindowFlags(Qt::FramelessWindowHint|(windowFlags() & (~Qt::WindowCloseButtonHint)));

    serial = new RoomPanel();
    cqAudio = new CQAudio(ui->frameMovie,stackPanel,0);
    media = new CQMedia(ui->frameMovie->winId(),0);
    connect(media,SIGNAL(error(CQGstBasic::ErrorType)),this,SLOT(onMediaSignal(CQGstBasic::ErrorType)));
    ui->sliderVlm->setSliderPosition(10);
//    QTimer::singleShot(1000,this,SLOT(on_btn_ch0_clicked()));
    channel = -1;
    playState = STOP;
    screenInit();

    updateTimer = new QTimer;
    updateTimer->setSingleShot(false);
    updateTimer->setInterval(1000);
    connect(updateTimer,SIGNAL(timeout()),this,SLOT(updateTime()));
    updateTimer->start();

    switchLanguage = new QTimer;
    switchLanguage->setSingleShot(false);
    connect(switchLanguage,SIGNAL(timeout()),this,SLOT(refresh()));
    switchLanguage->setInterval(10000);
    switchLanguage->start();
}
void CQplayerGUI::onMediaSignal(CQGstBasic::ErrorType type)
{
    if(type == CQGstBasic::TIMEOUT)
    {
        qDebug("recv timeout");
        play(channel);
    }
    else if(type == CQGstBasic::QOS)
    {
        qDebug("recv qos");
        play(channel);
    }
    else if(type == CQGstBasic::STOP)
        onStopButton_clicked();
}
void CQplayerGUI::screenInit()
{
    //屏幕GPIO(1_7)初始化
    {
        int _io = 7;
        QString io = QString::number(_io);
        if(!QFile::exists("/sys/class/gpio/gpio"+io))
        {
            QString cmd = "echo "+io+" > /sys/class/gpio/export";
            if(system(cmd.toAscii().data()))
            {
                qDebug()<<"create /sys/class/gpio/gpio" + io+"failed!!";
            }
        }
        if(QFile::exists("/sys/class/gpio/gpio"+io))
        {
            //QFile f("/sys/class/gpio/gpio"+io+"/value");
            QFile f("/sys/class/gpio/gpio"+io+"/direction");
            //f.setFileName("/sys/class/gpio/gpio"+io+"/direction");
            f.open(QFile::ReadWrite);
            f.write("out");
            f.close();

            f.setFileName("/sys/class/gpio/gpio"+io+"/value");
            if(!f.open(QFile::ReadWrite))
                qDebug("open file value failed!!");
            f.write("1");
            f.close();
        }
        else
            qDebug()<<"not exits gpio"+io;
    }

    screen_toggle = ON;

}
void CQplayerGUI::screenSwitch(EToggle toggle)
{
    if(toggle == OFF)//关
    {
        QFile f("/sys/class/gpio/gpio7/value");
        f.open(QFile::ReadWrite);
        f.write("0");
        f.close();
        media->stop();
        screen_toggle = OFF;
    }
    else
    {
        QFile f("/sys/class/gpio/gpio7/value");
        f.open(QFile::ReadWrite);
        f.write("1");
        f.close();
//        play(channel);
        screen_toggle = ON;
    }


}

void CQplayerGUI::onStopButton_clicked()
{
    stop();
}
void CQplayerGUI::onPlayButton_clicked()
{
    play(channel);
}

CQplayerGUI::~CQplayerGUI()
{
    delete ui;
    ui = NULL;
    gst_deinit();
    delete media;
    delete serial;
}
static bool flag = false;

enum PlayMode{S_PLAY,S_STOP};
static void execute_recipe(PlayMode mode)
{
    QProcess::execute("/appbin/recipeclient");
    if(mode == S_PLAY)
        GlobalInfo::getInstance()->playerGui->onPlayButton_clicked();

    flag = false;
    return;
}
void CQplayerGUI::on_recipeButton_clicked()
{
    if(flag == true)
        return;
    flag = true;
    if(media->getPlayingState() == CQMedia::PLAYING)
    {
        onStopButton_clicked();
        QtConcurrent::run(execute_recipe,S_PLAY);
    }
    else
    {
        QtConcurrent::run(execute_recipe,S_STOP);
    }
}
void CQplayerGUI::stop()
{
    if(getCurrentChannel() < 0 || playState == STOP)
        return;
    btn[getCurrentChannel()]->setStyleSheet("background-color: qlineargradient(spread:pad, x1:0, y1:1, x2:0, y2:0, "
            "stop:0 rgba(123, 174, 193, 255), stop:0.566502 rgba(174, 214, 226, 255), stop:1 rgba(197, 229, 242, 255));"
            "border: 0px inset gray ;"
            "border-radius: 5px;");
    stackPanel->hide();
    media->stop();
    playState = STOP;
}
void CQplayerGUI::paused()
{
    if(getCurrentChannel() < 0 || playState == STOP || playState == PAUSED)
        return;
//    btn[getCurrentChannel()]->setStyleSheet("background-color: qlineargradient(spread:pad, x1:0, y1:1, x2:0, y2:0, "
//            "stop:0 rgba(123, 174, 193, 255), stop:0.566502 rgba(174, 214, 226, 255), stop:1 rgba(197, 229, 242, 255));"
//            "border: 0px inset gray ;"
//            "border-radius: 5px;");
    stackPanel->hide();
    media->stopWithoutSignal();
    playState = PAUSED;
}



void CQplayerGUI::play(int _channel)
{
    if(disableBtn == true)
        return;
    if( playState == PAUSED || (media->getPlayingState() == CQMedia::PLAYING && getCurrentChannel() == _channel)  )
    {
        qDebug()<<"Current Channel " <<_channel<<" is playing...";
        return;
    }
    stop();
    if(GlobalInfo::getInstance()->chStateMap["Ch"+QString::number(_channel)] != "play")
    {
        qDebug()<<"Current Ch"<<_channel<< "not play";
        return;
    }
    channel = _channel;
    if(channel == 0)
    {
        qDebug()<<"Begin play GPS channel...";
        stackPanel->show();
        playState = PLAYING;
        btn[getCurrentChannel()]->setStyleSheet("border: 0px inset gray;border-radius: 5px;"
            "background-color: qlineargradient(spread:repeat, x1:0.458, y1:0, x2:0.463, y2:1, stop:0 rgba(240, 240, 240, 191),"
            "stop:0.0640394 rgba(162, 162, 162, 193), stop:0.137931 rgba(237, 249, 242, 255), stop:0.231527 "
            "rgba(147, 156, 169, 224), stop:0.512315 rgba(55, 60, 95, 191), stop:0.758621 rgba(147, 156, 169, 224),"
            "stop:0.91133 rgba(240, 249, 240, 255), stop:0.975369 rgba(0, 0, 0, 81));");
        disableBtn = true;
        disableTimer.start(1000);
        return;
    }
    QString videoUrl = ZUtility::getVideoUrl( _channel);
    if(videoUrl != "")
    {
        if(media->play(videoUrl))
        {
            playState = PLAYING;
            btn[getCurrentChannel()]->setStyleSheet("border: 0px inset gray;border-radius: 5px;"
                "background-color: qlineargradient(spread:repeat, x1:0.458, y1:0, x2:0.463, y2:1, stop:0 rgba(240, 240, 240, 191),"
                "stop:0.0640394 rgba(162, 162, 162, 193), stop:0.137931 rgba(237, 249, 242, 255), stop:0.231527 "
                "rgba(147, 156, 169, 224), stop:0.512315 rgba(55, 60, 95, 191), stop:0.758621 rgba(147, 156, 169, 224),"
                "stop:0.91133 rgba(240, 249, 240, 255), stop:0.975369 rgba(0, 0, 0, 81));");
            disableBtn = true;
            disableTimer.start(1000);
        }
    }

}


bool CQplayerGUI::playback(int _channel)
{
    bool res = false;
    if(GlobalInfo::getInstance()->chStateMap["Ch"+QString::number(_channel)] != "play")
    {
        qDebug()<<"Current Ch"<<_channel<< "not play";
        return res;
    }
    channel = _channel;
    QString videoUrl = ZUtility::getVideoUrl( _channel);
    if(videoUrl != "")
    {
        res = media->play(videoUrl);
        if(res)
        {
            playState = PLAYING;
        }
    }
    return res;
}
void CQplayerGUI::on_btn_ch1_clicked()
{
    play(1);
}
void CQplayerGUI::on_btn_ch2_clicked()
{
    play(2);
}
void CQplayerGUI::on_btn_ch3_clicked()
{
    play(3);
}
void CQplayerGUI::on_btn_ch4_clicked()
{
     play(4);
}
void CQplayerGUI::on_btn_ch5_clicked()
{
    play(5);
}
void CQplayerGUI::on_btn_ch6_clicked()
{
    play(6);
}
void CQplayerGUI::on_btn_ch7_clicked()
{
     play(7);
}
void CQplayerGUI::on_btn_ch8_clicked()
{
    play(8);
}
void CQplayerGUI::on_btn_off_clicked()
{
    if(0)
    {
        play(9);
    }
    else
    {
        screenSwitch(OFF);
    }
}

void CQplayerGUI::on_btn_ch0_clicked()
{
    if(stackPanel->s_off)
        return;
    play(0);
}

void CQplayerGUI::on_sliderVlm_valueChanged(int volume)
{
    if(media != NULL)
        media->setVolume(volume);
}

void CQplayerGUI::updateTime()
{
    if(ui->frameMovie->geometry() == media->row_rect)   {
        ui->dateTimeLabel->setText(QDateTime::currentDateTime().toString("20yy-MM-dd                hh:mm:ss"));
    }
}


void CQplayerGUI::updateDaemon()
{
    system("ps | grep [r]oom-daemon | awk '{print $1}'|  xargs kill -9");
    QFile::copy(":/room-daemon","/tmp/room-daemon");
    system("chmod +x /tmp/room-daemon");
    system("/tmp/room-daemon 1>/dev/null 2>&1  &");
    system("rm /tmp/room-daemon");
}
void CQplayerGUI::slot_procTestZtp()
{
    ZTPprotocol ztp;
    ztpmForTest->getOneZtp(ztp);                //当设备是ROOM时,CARID为传床位号
    if(ztp.getPara("T") == "TEST" && ztp.getPara("CARID").toInt() == GlobalInfo::getInstance()->bedNr &&  ztp.getPara("DEV") == "ROOM"
            &&  ztp.getPara("CMD") == "EXIT_WORKTHREAD")
    {
        versionCtrl->setTestOffline();
    }
}

void CQplayerGUI::on_btn_scr_full_clicked()
{
    if(!disableBtn)
    {
        media->screenFull();
        disableBtn = true;
        disableTimer.start(3000);

    }
//    if(media->getPlayingState() == CQMedia::PLAYING)
//        media->paused();
//    else if(media->getPlayingState() == CQMedia::PAUSED)
//        media->play();
}
//void CQplayerGUI::mouseDoubleClickEvent ( QMouseEvent *  )
static int clickCnt = 0;
void CQplayerGUI::mouseReleaseEvent(QMouseEvent *e)
{
    static int init = 0;
    static QTimer timer;
    static QPoint p;
    touchAdjust(e);
    if(init == 0){
        init++;
        timer.setSingleShot(true);
        timer.setInterval(1000);
        connect(&timer,SIGNAL(timeout()),this,SLOT(clickStateReset()));
    }
    if(!ui->frameMovie->geometry().contains(e->globalPos()))
        return;
    clickCnt++;
    if(clickCnt == 1){  //第一次点击
        p = e->pos();
        timer.start();
    }
    else {  //第二次点击
        clickCnt = 0;
        timer.stop();
        if((e->pos() - p).manhattanLength() < 200)//两次点击距离10pix之内为有效双击
            media->screenNormal();
    }
}
void CQplayerGUI::clickStateReset(){    //双击超时
    clickCnt = 0;
}
void CQplayerGUI::touchAdjust(QMouseEvent *e)
{
    static int init = 0;
    static QTimer timer;
    if(init == 0){
        init++;
        timer.setSingleShot(true);
        timer.setInterval(1000);
        connect(&timer,SIGNAL(timeout()),this,SLOT(clickStateReset()));
    }
    if(ui->frameMovie->geometry() != media->row_rect)
        return;
    qDebug()<<ui->logWidget->geometry()<<"    "<<e->globalPos();
    if(!ui->logWidget->geometry().contains(e->globalPos()))
        return;
    clickCnt++;
    timer.start();
    if(clickCnt >= 3) {
        clickCnt = 0;
        timer.stop();
        system("rm /etc/pointercal.xinput  /.pointercal/ -rf");
		stop();
        system("xinput_calibrator_once.sh &");
    }
}
void CQplayerGUI::refresh()
{

    if(translator->curLan == "En")
    {
        translator->curLan = "Th";
    }
    else
    {
        translator->curLan = "En";
    }
    if(ui->frameMovie->geometry() != media->row_rect)
        return;

    ui->_speedLabel->setText(translator->tr("Speed"));
    ui->_speedUnitLabel->setText(translator->tr("km/h"));
    ui->labelVolume->setText(translator->tr("Volume"));
    ui->btn_scr_full->setText(translator->tr("FULL"));
    ui->btn_ch0->setText(translator->tr("GPS"));
    ui->btn_ch1->setText(translator->tr("ch1"));
    ui->btn_ch2->setText(translator->tr("ch2"));
    ui->btn_ch3->setText(translator->tr("ch3"));
    ui->btn_ch4->setText(translator->tr("ch4"));
    ui->btn_ch5->setText(translator->tr("ch5"));
    ui->btn_ch6->setText(translator->tr("ch6"));
    ui->btn_ch7->setText(translator->tr("ch7"));
    ui->btn_ch8->setText(translator->tr("ch8"));
    ui->_startStaionLabel->setText(translator->tr("Origin station"));
    ui->_endStationLabel->setText(translator->tr("Destination station"));
    ui->_preStationLabel->setText(translator->tr("Previous station"));
    ui->_nextStationLabel->setText(translator->tr("Next station"));
    ui->_carIDLabel->setText(translator->tr("CAR NO"));
    char buf[10];
    sprintf(buf,"%02d",ZTools::getCarID());
    ui->carIDLabel->setText(buf);
    ui->_trainIDLabel->setText(translator->tr("Train No"));
    ui->_curStationLabel->setText(translator->tr("This Station"));
    ui->_arriveTimeLabel->setText(translator->tr("Destination station time"));
    ui->_nextStationTimeLabel->setText(translator->tr("Next station time"));
    ui->_TempLabel->setText(translator->tr("TEMPERATURE"));
    ui->_outTempLabel->setText(translator->tr("Outside T"));
    ui->_innerTempLabel->setText(translator->tr("Room T"));
    if(translator->curLan == "En") {
        ui->startStaionLabel->setText(GlobalInfo::getInstance()->g_info.getPara("StartStation"));
        ui->endStationLabel->setText(GlobalInfo::getInstance()->g_info.getPara("EndStation"));
        ui->preStationLabel->setText(GlobalInfo::getInstance()->g_info.getPara("PreStation"));
        ui->nextStationLabel->setText(GlobalInfo::getInstance()->g_info.getPara("NextStation"));
        ui->curStationLabel->setText(GlobalInfo::getInstance()->g_info.getPara("CurrentStationEn"));
    }
    else{
        ui->startStaionLabel->setText(GlobalInfo::getInstance()->g_info.getPara("StartStation_th"));
        ui->endStationLabel->setText(GlobalInfo::getInstance()->g_info.getPara("EndStation_th"));
        ui->preStationLabel->setText(GlobalInfo::getInstance()->g_info.getPara("PreStation_th"));
        ui->nextStationLabel->setText(GlobalInfo::getInstance()->g_info.getPara("NextStation_th"));
        ui->curStationLabel->setText(GlobalInfo::getInstance()->g_info.getPara("CurrentStationTh"));
    }
    stackPanel->updateLan();

}
