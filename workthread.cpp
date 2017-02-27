#include "workthread.h"
#include"config.h"
#include"ztpmanager.h"
#include"ztools.h"
#include<QTimer>
#include<QtCore>
#include"CQplayerGUI.h"
#include"zutility.h"
#include"stackpanel.h"
#include"math.h"
static int debug = 0;
WorkThread* WorkThread::_instance = NULL;
WorkThread::WorkThread(QObject *parent) :
    QThread(parent)
{
}
WorkThread* WorkThread::getInstance()
{
    if(_instance == NULL)
        _instance = new WorkThread();
    return _instance;
}

void WorkThread::test()
{
    QTimer *t = new QTimer;
    delete t;
    t = NULL;
    t->setInterval(1000);
    return;
}
static QTimer* tempTimer = new QTimer;
static QTimer* outTempTimer = new QTimer;
void WorkThread::tempTimeOutProc()
{
    QTimer* timer = dynamic_cast<QTimer*>(sender());
    if(GlobalInfo::getInstance()->playerGui->ui->frameMovie->geometry() != GlobalInfo::getInstance()->playerGui->media->row_rect)
        return;
    else if(timer == tempTimer)
    {
        GlobalInfo::getInstance()->playerGui->ui->innerTempLabel->setText("25");
    }
    else if(timer == outTempTimer)
    {
        CQplayerGUI* gui = GlobalInfo::getInstance()->playerGui;
        gui->ui->outTempLabel->setText(QString("33"));
    }
}
void WorkThread::slot_recv_car_id_map()
{
    ZTPprotocol ztp;
    recv_car_id_map_ztpm->getOneZtp(ztp);
    if(ztp.getPara("T") == "CAR_ID_MAP" && ztp.getPara("GLOBAL_ID").toInt() == ZTools::getCarGlobalID() )
    {
        if(ztp.getPara("CAR_ID").toInt() !=ZTools::getCarID())
        {
            ZTools::setCarID(ztp.getPara("CAR_ID").toInt());
        }
    }
}
void WorkThread::run()
{
    recv_car_id_map_ztpm = new ZTPManager(8323,QHostAddress("224.102.228.40"));
    connect(recv_car_id_map_ztpm,SIGNAL(readyRead()),this,SLOT(slot_recv_car_id_map()));

    recvCarrierHeartTimer = new QTimer;
    recvCarrierHeartTimer->setInterval(10000);
    recvCarrierHeartTimer->setSingleShot(false);
    connect(recvCarrierHeartTimer,SIGNAL(timeout()),this,SLOT(recvCarrierHeartTimeout()));
    recvCarrierHeartTimer->start();

    ztpmCh = new ZTPManager(8315,QHostAddress(BROADCAST_IP));//多媒体频道广播
    connect(ztpmCh,SIGNAL(readyRead()),this,SLOT(slot_channel_proc()));

    ztpmCarrierHeart = new ZTPManager(8317,QHostAddress("224.102.228.40"));
    connect(ztpmCarrierHeart,SIGNAL(readyRead()),this,SLOT(OnRecvCarrierHeart()));
    tempTimer->start(5000);
    outTempTimer->start(5000);
    connect(tempTimer,SIGNAL(timeout()),this,SLOT(tempTimeOutProc()));
    connect(outTempTimer,SIGNAL(timeout()),this,SLOT(tempTimeOutProc()));
    callUnit = new CallCtlUnit;
    callUnit->start();
    GlobalInfo::getInstance()->comZtpManager = new ZTPManager(QHostAddress(BROADCAST_IP),BROADCAST_PORT);//用于收通用包
    connect(GlobalInfo::getInstance()->comZtpManager,SIGNAL(readyRead()),this,SLOT(broadcastProc()));
    GlobalInfo::getInstance()->comZtpManager = new ZTPManager;//专用于发送
    ZTPManager* ztpm = new ZTPManager(3321,QHostAddress(BROADCAST_IP));//厕所状态
    connect(ztpm,SIGNAL(readyRead()),this,SLOT(broadcastProc()));
    ztpm = new ZTPManager(8313,QHostAddress(BROADCAST_IP));//车外温度
    connect(ztpm,SIGNAL(readyRead()),this,SLOT(broadcastProc()));
    ztpm = new ZTPManager(8314,QHostAddress(BROADCAST_IP));//包间温度
    connect(ztpm,SIGNAL(readyRead()),this,SLOT(broadcastProc()));
    ZTPManager* g_infoZtpm = new ZTPManager(8311,QHostAddress(BROADCAST_IP));//系统控制器消息
    connect(g_infoZtpm,SIGNAL(readyRead()),this,SLOT(broadcastProc()));
    exec();
}
static void proBroadSignal(bool broadSignal)
{
    CQplayerGUI* player = GlobalInfo::getInstance()->playerGui;
    if(broadSignal && !player->media->isSilence())
        player->media->setSilence(true);
    else if(!broadSignal && player->media->isSilence())
        player->media->setSilence(false);
}
void WorkThread::broadcastProc()
{
    //qDebug()<<"in the broadcastProc";
    ZTPprotocol ztp;
//    GlobalInfo::getInstance()->comZtpManager->getOneZtp(ztp);
    ZTPManager* ztpm = dynamic_cast<ZTPManager*>(sender());
    if(ztpm == NULL)
        return;
    ztpm->getOneZtp(ztp);
    if(ztp.getPara("T") == "Temperature" && ztp.getPara("CAR_ID").toInt() == ZTools::getCarID() && ztp.getPara("ROOM_NR").toInt() == GlobalInfo::getInstance()->roomNr)
    {
//        ztp.print();
        tempTimer->start(5000);
        //qDebug()<<"温度: "<<ztp.getPara("value");
        //QString temp = QString::number(ztp.getPara("value").toFloat(),'f',1);
        int d = ztp.getPara("value").toFloat();
        char buf[10];
        sprintf(buf,"%02d",d);
//        int temp = ztp.getPara("value").toFloat()+0.5;
        //GlobalInfo::getInstance()->playerGui->ui->innerTempLabel->setAlignment(Qt::AlignCenter);
        if(GlobalInfo::getInstance()->playerGui->ui->frameMovie->geometry() == GlobalInfo::getInstance()->playerGui->media->row_rect)
            GlobalInfo::getInstance()->playerGui->ui->innerTempLabel->setText(QString(buf));
    }
    else if(ztp.getPara("T") == "OutTemperature")
    {
        outTempTimer->start(5000);
        CQplayerGUI* gui = GlobalInfo::getInstance()->playerGui;
        int d = ztp.getPara("VALUE").toFloat();
        char buf[10];
        sprintf(buf,"%02d",d);
        if(GlobalInfo::getInstance()->playerGui->ui->frameMovie->geometry() == GlobalInfo::getInstance()->playerGui->media->row_rect)
            gui->ui->outTempLabel->setText(QString(buf));

    }
    else if(0 &&ztp.getPara("T") == "CarTemperature" && ztp.getPara("CAR_ID").toInt() == ZTools::getCarID())
    {
        //外温
        outTempTimer->start(5000);
        CQplayerGUI* gui = GlobalInfo::getInstance()->playerGui;
        int d = ztp.getPara("OUT VALUE").toFloat();
        char buf[10];
        sprintf(buf,"%02d",d);
        if(GlobalInfo::getInstance()->playerGui->ui->frameMovie->geometry() == GlobalInfo::getInstance()->playerGui->media->row_rect)
            gui->ui->outTempLabel->setText(QString(buf));
    }
    else if(ztp.getPara("T") == "G_INFO")
    {
        CQplayerGUI* gui = GlobalInfo::getInstance()->playerGui;
        gui->stackPanel->setSecsOff(ztp.getPara("SECS_OFF").toInt());
        if(gui->ui->frameMovie->geometry() == gui->media->row_rect)
        {
            QString speed = QString::number((int)round(ztp.getPara("Speed").toDouble()));
            gui->ui->trainIDLabel->setText(ztp.getPara("TrainNum"));
            gui->ui->speedLabel->setText(speed);
            gui->ui->arriveTimeLabel->setText(ztp.getPara("ArriveTime"));
            gui->ui->nextStationTimeLabel->setText(ztp.getPara("NextStationTime"));
            GlobalInfo::getInstance()->g_info.addPara("StartStation",ztp.getPara("StartStation"));
            GlobalInfo::getInstance()->g_info.addPara("EndStation",ztp.getPara("EndStation"));
            GlobalInfo::getInstance()->g_info.addPara("PreStation",ztp.getPara("PreStation"));
            GlobalInfo::getInstance()->g_info.addPara("NextStation",ztp.getPara("NextStation"));
            GlobalInfo::getInstance()->g_info.addPara("CurrentStationEn",ztp.getPara("CurrentStationEn"));
            GlobalInfo::getInstance()->g_info.addPara("StartStation_th",ztp.getPara("StartStation_th"));
            GlobalInfo::getInstance()->g_info.addPara("EndStation_th",ztp.getPara("EndStation_th"));
            GlobalInfo::getInstance()->g_info.addPara("PreStation_th",ztp.getPara("PreStation_th"));
            GlobalInfo::getInstance()->g_info.addPara("NextStation_th",ztp.getPara("NextStation_th"));
            GlobalInfo::getInstance()->g_info.addPara("CurrentStationTh",ztp.getPara("CurrentStationTh"));
            if(gui->translator->curLan == "En") {
                gui->ui->startStaionLabel->setText(ztp.getPara("StartStation"));
                gui->ui->endStationLabel->setText(ztp.getPara("EndStation"));
                gui->ui->preStationLabel->setText(ztp.getPara("PreStation"));
                gui->ui->nextStationLabel->setText(ztp.getPara("NextStation"));
                gui->ui->curStationLabel->setText(ztp.getPara("CurrentStationEn"));

            }
            else{
                gui->ui->startStaionLabel->setText(ztp.getPara("StartStation_th"));
                gui->ui->endStationLabel->setText(ztp.getPara("EndStation_th"));
                gui->ui->preStationLabel->setText(ztp.getPara("PreStation_th"));
                gui->ui->nextStationLabel->setText(ztp.getPara("NextStation_th"));
                gui->ui->curStationLabel->setText(ztp.getPara("CurrentStationTh"));
            }
        }
        int train_id = ztp.getPara("TrainNum").toInt();
        if(GlobalInfo::getInstance()->train_id != train_id)
        {
            GlobalInfo::getInstance()->train_id = train_id;
            GlobalInfo::getInstance()->playerGui->stackPanel->setTrainId(train_id);
            GlobalInfo::getInstance()->playerGui->stackPanel->loadTimeTable(train_id);
            GlobalInfo::getInstance()->playerGui->stackPanel->OnUpdateMap();
        }
    }
    else if(ztp.getPara("T") == "IPaddress1")
    {
        if(GlobalInfo::getInstance()->VServerIP != ztp.getPara("mainvideoserver")){
            GlobalInfo::getInstance()->VServerIP = ztp.getPara("mainvideoserver");
            GlobalInfo::getInstance()->playerGui->translator->loadChNameConfig(GlobalInfo::getInstance()->VServerIP);
            }
        if(GlobalInfo::getInstance()->callUnitIP != ztp.getPara("roomcallcell"))
            GlobalInfo::getInstance()->callUnitIP = ztp.getPara("roomcallcell");
        if(GlobalInfo::getInstance()->sysCtlIp != ztp.getPara("syscontroller"))
            GlobalInfo::getInstance()->sysCtlIp = ztp.getPara("syscontroller");

        if(GlobalInfo::getInstance()->VServerIP != NULL &&
            GlobalInfo::getInstance()->callUnitIP != NULL &&
            GlobalInfo::getInstance()->sysCtlIp != NULL)
            delete ztpm;
    }
    else if(ztp.getPara("T") == "CS_STATE" && ztp.getPara("CAR_ID").toInt() == ZTools::getCarID())
    {
        CQplayerGUI* gui = GlobalInfo::getInstance()->playerGui;
        static QString pre_xiaobian = "--",pre_dabian = "--",pre_dabian2 = "--",pre_linyu = "--";
        QString cur_xiaobian = ztp.getPara("CS1");
        QString cur_dabian = ztp.getPara("CS2");
        QString cur_dabian2 = ztp.getPara("CS3") ;
        QString cur_linyu = ztp.getPara("CS4");
        ztp.print();
        if(gui->ui->frameMovie->geometry() == gui->media->row_rect )
        {

            if(cur_xiaobian != pre_xiaobian )
            {
                if( cur_xiaobian == "Y")
                {
                    gui->ui->wcStateLabel1->setStyleSheet(QString::fromUtf8("border-image: url(:/images/xiaobian_out.png);"));
                }
                else
                {
                    gui->ui->wcStateLabel1->setStyleSheet(QString::fromUtf8( "border-image: url(:/images/xiaobian_in.png);"));
                }
            }

            if(cur_dabian != pre_dabian )
            {
                if(cur_dabian == "Y")
                {
                    gui->ui->wcStateLabel2->setStyleSheet(QString::fromUtf8( "border-image: url(:/images/dabian_out.png);"));
                }
                else
                {
                    gui->ui->wcStateLabel2->setStyleSheet(QString::fromUtf8( "border-image: url(:/images/dabian_in.png);"));
                }
            }
            if(cur_dabian2 != pre_dabian2 )
            {
                if(cur_dabian2 == "Y")
                {
                    gui->ui->wcStateLabel3->setStyleSheet(QString::fromUtf8( "border-image: url(:/images/dabian_out.png);"));
                }
                else
                {
                    gui->ui->wcStateLabel3->setStyleSheet(QString::fromUtf8( "border-image: url(:/images/dabian_in.png);"));
                }
            }
            if(cur_linyu != pre_linyu )
            {
                if(cur_linyu == "Y"){
                    gui->ui->wcStateLabel4->setStyleSheet(QString::fromUtf8( "border-image: url(:/images/water_out.png);"));
                }
                else
                {
                    gui->ui->wcStateLabel4->setStyleSheet(QString::fromUtf8( "border-image: url(:/images/water_in.png);"));
                }
            }

            pre_xiaobian = cur_xiaobian;
            pre_dabian = cur_dabian;
            pre_dabian2 = cur_dabian2;
            pre_linyu = cur_linyu;
        }
    }
    else if(ztp.getPara("T") == "BroadCast")
    {
        bool broadSignal = false;
        if(ztp.getPara("HasBroadcast") == "Y" ||
                (ztp.getPara("HasBroadcast_lc") == "Y" &&
                 ztp.getPara("CarId").toInt() == ZTools::getCarID() ))
        {
           broadSignal = true;
        }
        proBroadSignal(broadSignal);
    }

}
void WorkThread::OnRecvCarrierHeart()
{
    QString printCmd;
    ZTPprotocol ztp;
    ztpmCarrierHeart->getOneZtp(ztp);
    if(ztp.getPara("T") != "CarrierHeart")
        return;
    GlobalInfo::getInstance()->playerGui->versionCtrl->setOnline(true);
    recvCarrierHeartTimer->start();
    if(QFile::exists("/carrier_record.txt"))
    {
        QFile file("/carrier_record.txt");
        if(file.size() > 10*1024*1024)
        {
            QFile::remove("/carrier_record.txt");
        }
    }
    if(debug)
    {
        printCmd = "echo Recv CarrierHeart    : " + QTime::currentTime().toString() + " >> /carrier_record.txt";
        system(printCmd.toAscii().data());
    }
//    qDebug()<<"Room Recv CarrierHeart ..." << QTime::currentTime();
    if(!QFile::exists("/tmp/DevExist"))
    {
        qDebug("Create DevExist ...");
        if(debug)
        {
            printCmd = "echo /tmp/DevExist do not exists and create it   : " + QTime::currentTime().toString() + " >> /carrier_record.txt";
            system(printCmd.toAscii().data());
        }
        system("touch /tmp/DevExist ");
    }
    else if(0)
    {
        qDebug("Has DevExist ...");
        printCmd = "echo /tmp/DevExist had exists and ignore ...   : " + QTime::currentTime().toString() + " >> /carrier_record.txt";
        system(printCmd.toAscii().data());
    }
}

void WorkThread::recvCarrierHeartTimeout()
{
    GlobalInfo::getInstance()->playerGui->versionCtrl->setOnline(false);
}
void WorkThread::slot_channel_proc(){
    static QString ch[9];
    static int init = 1;
    if(init == 1){
        init = 0;
        qDebug()<<"init slot_channel_proc ...";
        ch[0] = "play";
        GlobalInfo::getInstance()->chStateMap.insert("Ch"+QString::number(0),ch[0]);
        for( int i = 1 ;i < 9;i++) {
            ch[i] = "stop";
            GlobalInfo::getInstance()->chStateMap.insert("Ch"+QString::number(i),ch[i]);
        }
    }
    ZTPprotocol ztp;
    ztpmCh->getOneZtp(ztp);
    if(ztp.getPara("T") != "T_ChId")
        return;
//    ztp.print();
    for(int i = 1;i<9;i++){ //只判断1~8,不判断Ch0
        QString chId = "Ch"+QString::number(i);
        if(ztp.getPara(chId) != ch[i]){
//            qDebug()<<"ztp.getPara(chId)  "<<ztp.getPara(chId) ;
//            qDebug()<<"ch["<<i<<"] "<<ch[i] ;
            bool flag = true;
//            static int errCnt = 0;
            if(i ==GlobalInfo::getInstance()->playerGui->getCurrentChannel()) {
                if(ch[i] == "play" && ztp.getPara(chId) == "paused" && GlobalInfo::getInstance()->playerGui->getPlayState() == CQplayerGUI::PLAYING)
                {
//                    errCnt = 20;
                    GlobalInfo::getInstance()->playerGui->paused();
                }
                if( ch[i] == "paused" && ztp.getPara(chId) == "play" && GlobalInfo::getInstance()->playerGui->getPlayState() == CQplayerGUI::PAUSED )
                {
                    GlobalInfo::getInstance()->chStateMap[chId] = "play";//临时改一下以便通过播放
                    flag = GlobalInfo::getInstance()->playerGui->playback(i);
//                    if(!flag)
//                        errCnt--;
                }
            }
            ch[i] = ztp.getPara(chId);

            if(!flag )
                GlobalInfo::getInstance()->playerGui->onStopButton_clicked();
            GlobalInfo::getInstance()->chStateMap[chId] = ch[i];
            qDebug()<<chId<<" change to "<<GlobalInfo::getInstance()->chStateMap[chId];
        }
    }
}
