#ifndef ZTPMANAGER_H
#define ZTPMANAGER_H
#include<QList>
#include<QThread>
#include<QHostAddress>
#include<QUdpSocket>
#include<QDateTime>
#include<QByteArray>
#include<QMap>
#include <QStringList>
#include<QTimer>
#include<QMutex>
#include<QThreadPool>
#include "ztpprotocol.h"
#define REMOTE_HOST "RemoteHost"
#define REMOTE_PORT "RemotePort"
class ZTPprotocol;
class Fragment;
class FragmentList;
class ZTPManager : public QObject
{
    Q_OBJECT
    QThreadPool* ztpThreadPool;
    QMutex ztpListMutex;
    QList<ZTPprotocol*> ztpList;
    QMap<quint16,FragmentList*> workMap;
    QUdpSocket _Socketlistener;
    int MTU;
    int _timeout;
signals:
    void registTask(const QByteArray &,const QString&,quint16);//内部使用
    void readyRead();
public:
    enum ResultState
    {
        SUCCESS,
        TIMEOUT,
        FAILED
    };
    quint16 localPort()const{return _Socketlistener.localPort();}
    void setMTU(int _mtu){MTU = _mtu;}
    explicit ZTPManager(QHostAddress host = QHostAddress::Any,quint16 port = 0,
               QHostAddress groupAddress = QHostAddress::Any, QObject *parent = 0);
    explicit ZTPManager(quint16 port,QHostAddress groupAddress = QHostAddress::Any, QObject *parent = 0);
    ResultState getOneZtp(ZTPprotocol& ztp);//异步读取
    ResultState waitOneZtp(ZTPprotocol& ztp,int msecs = 3000);//同步读取
    ResultState SendOneZtp(ZTPprotocol& ztp,const QHostAddress& host,quint16 port);

    ~ZTPManager();

private:
    void msleep(int msecs);
    void commonInit(const QHostAddress& groupAddress);

private slots:
    void onRead();
    void onTimeout(quint16 identifier);
    void procPkg(const QByteArray &recvBuff,const QString &remoteHost,quint16 remotePort);
};



#endif // ZTPMANAGER_H
