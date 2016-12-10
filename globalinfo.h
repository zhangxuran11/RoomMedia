#ifndef GLOBALINFO_H
#define GLOBALINFO_H

#include <QObject>
#include<QMap>
#include"CQplayerGUI.h"
class ZTPManager;
class GlobalInfo : public QObject
{
    Q_OBJECT
    static GlobalInfo* _instance;
    explicit GlobalInfo(QObject *parent = 0);
public:
    QMap<QString,QString> chStateMap;//频道状态集合
    int train_id;
    int roomNr;
    int bedNr;
    int deviceNr;
    int carId;
    ZTPprotocol g_info;
    QString VServerIP;
    QString callUnitIP;
    QString sysCtlIp;
    static GlobalInfo* getInstance();
    ZTPManager* comZtpManager;

    CQplayerGUI *playerGui;



signals:

public slots:

};

#endif // GLOBALINFO_H
