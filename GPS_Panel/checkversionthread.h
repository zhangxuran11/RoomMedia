#ifndef CHECKVERSIONTHREAD_H
#define CHECKVERSIONTHREAD_H

#include <QThread>
#include"ztpmanager.h"

class CheckVersionThread : public QThread
{
    Q_OBJECT
    ZTPManager* ztpManager;
    ZTPManager* gpsMapZtpm;
    ZTPManager* trainLineZtpm;
    QString curGPSMapMD5;
    QString curTrainLineDbMD5;
    bool f_mapIsValid;
    bool f_dbIsValid;
    void run();
public:
    explicit CheckVersionThread(QObject *parent = 0);
    
signals:
    void updateMap();
private:
    void updateTrainLineDbMD5();
    void updateGPSMapMD5();
private slots:
    void procVersion();
    void OnDownComplete();
};

#endif // CHECKVERSIONTHREAD_H
