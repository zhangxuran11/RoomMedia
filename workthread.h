#ifndef WORKTHREAD_H
#define WORKTHREAD_H
#include "ztpmanager.h"
#include <QThread>
#include <QTimer>
#include<globalinfo.h>
class WorkThread : public QThread
{
    Q_OBJECT
    static WorkThread* _instance;
    ZTPManager* ztpmCh;
    ZTPManager* ztpmCarrierHeart;
    CallCtlUnit* callUnit;
    ZTPManager *netManager;
    QTimer *trainTtimer;  //列车控制器通信定时
    QTimer *carTimer;  //车厢控制器通信定时
    QTimer* recvCarrierHeartTimer;


    explicit WorkThread(QObject *parent = 0);
public:
    static WorkThread* getInstance();
private:

    
signals:
    
private slots:
    void OnRecvCarrierHeart();
    void test();

    void run();
    
    void broadcastProc();
    void tempTimeOutProc();
    void recvCarrierHeartTimeout();

    void slot_channel_proc();
};

#endif // WORKTHREAD_H




