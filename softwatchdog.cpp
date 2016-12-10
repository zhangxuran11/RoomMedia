#include "softwatchdog.h"
#include "globalinfo.h"
#include"ztpmanager.h"
SoftWatchdog::SoftWatchdog(QObject *ressourceObj,void (*method)(),QObject *parent ) :
    QObject(parent)
{
    deal = method;
    obj = ressourceObj;
    elapse = 5000;
    timer = new QTimer;
    timer->setInterval(elapse);
    timer->setSingleShot(true);
    timer->start();
    connect(timer,SIGNAL(timeout()),this,SLOT(cleanUp()));
}
void SoftWatchdog::cleanUp()
{
    deal();
//    delete obj;
//    delete timer;
//    delete this;
}
void SoftWatchdog::feed()
{
    timer->start(elapse);
}

