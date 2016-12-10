#include "ztpmanager.h"
#include"ztpprotocol.h"
#include "fragment.h"
#include <QtAlgorithms>
#include<QThread>
#include"ztptask.h"
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
    #include <QtConcurrent>
#else
    #include <QtCore>
#endif
#include<QEventLoop>

#if defined(Q_OS_LINUX)
    #include<unistd.h>
#elif defined(Q_OS_WIN)
    #include<windows.h>
#endif

static ZTPTask ztpTask;
ZTPManager::ZTPManager(QHostAddress host,quint16 port,
                       QHostAddress groupAddress, QObject *parent):
QObject(parent)
{
   _Socketlistener.bind(host,port,QUdpSocket::ShareAddress);
   commonInit(groupAddress);
}

ZTPManager::ZTPManager(quint16 port,
           QHostAddress groupAddress, QObject *parent):
    QObject(parent)
{
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    _Socketlistener.bind(QHostAddress::Any,port,QUdpSocket::ShareAddress);
#else
    _Socketlistener.bind(QHostAddress::AnyIPv4,port,QUdpSocket::ShareAddress);
#endif
    commonInit(groupAddress);
    qsrand(QDateTime::currentMSecsSinceEpoch());
}

void ZTPManager::commonInit(const QHostAddress& groupAddress)
{
    #if QT_VERSION < QT_VERSION_CHECK(5,0,0)
    QThreadPool::globalInstance()->setMaxThreadCount(QThreadPool::globalInstance()->maxThreadCount()+1);
    #endif
    ztpThreadPool = new QThreadPool;
    ztpThreadPool->setMaxThreadCount(1);
    if(groupAddress.toIPv4Address()>0xe0000000 && groupAddress.toIPv4Address()<0xf0000000)
    {
        _Socketlistener.joinMulticastGroup(groupAddress);
        //_Socketlistener.setSocketOption(QAbstractSocket::MulticastTtlOption, 5);
    }
    connect(&_Socketlistener,SIGNAL(readyRead()),this,SLOT(onRead()),Qt::DirectConnection);
    MTU = 60000;
    _timeout = 3000;

    moveToThread(&ztpTask);
    connect(this,SIGNAL(registTask(const QByteArray &,const QString &,quint16)),this,
            SLOT(procPkg(const QByteArray &,const QString &,quint16)),Qt::QueuedConnection);
}

ZTPManager::~ZTPManager()
{
    //此析构有问题   尽量避免使用 ,即使使用也尽量在与ztpmanager直接链接的线程里用(待解决)
    _Socketlistener.close();
    for(QMap<quint16,FragmentList*>::iterator it = workMap.begin();it !=workMap.end();it++)
    {
        delete it.value();
    }
    ztpListMutex.lock();
    for(QList<ZTPprotocol*>::iterator it = ztpList.begin();it !=ztpList.end();it++)
    {
        delete *it;
    }
    ztpListMutex.unlock();
}
ZTPManager::ResultState ZTPManager::getOneZtp(ZTPprotocol& ztp)
{
    //qDebug("entry file:%s line:%d func:%s thread:%ld",__FILE__,__LINE__,__FUNCTION__,QThread::currentThreadId());
    ztpListMutex.lock();
    if(ztpList.isEmpty())
    {
        qDebug()<<"ZTP LIST is empty!!";
        //qDebug("leave file:%s line:%d func:%s thread:%ld",__FILE__,__LINE__,__FUNCTION__,QThread::currentThreadId());
        ztpListMutex.unlock();
        return FAILED;
    }
    ZTPprotocol* pztp = ztpList.takeFirst();
    ztpListMutex.unlock();
    ztp = *pztp;    //ZZZZZZZZZZZZZZZZZZZZ
    delete pztp;
    //qDebug("leave file:%s line:%d func:%s thread:%ld",__FILE__,__LINE__,__FUNCTION__,QThread::currentThreadId());
    return SUCCESS;
}
ZTPManager::ResultState ZTPManager::waitOneZtp(ZTPprotocol& ztp,int msecs)
{
    //qDebug("entry file:%s line:%d func:%s thread:%ld",__FILE__,__LINE__,__FUNCTION__,QThread::currentThreadId());
    QEventLoop q;
    QTimer::singleShot(msecs,&q,SLOT(quit()));
    connect(this,SIGNAL(readyRead()),&q,SLOT(quit()));
    q.exec();
    if(getOneZtp(ztp) == FAILED)
    {
        //qDebug("getOneZtp() timeout");
        //qDebug("leave file:%s line:%d func:%s thread:%ld",__FILE__,__LINE__,__FUNCTION__,QThread::currentThreadId());
        return TIMEOUT;
    }
    else
    {
        //qDebug("leave file:%s line:%d func:%s thread:%ld",__FILE__,__LINE__,__FUNCTION__,QThread::currentThreadId());
        return SUCCESS;
    }

}

ZTPManager::ResultState ZTPManager::SendOneZtp(ZTPprotocol& ztp,const QHostAddress &host,quint16 port)
{
	ztp.genarate();
    quint16 identifier = qrand()&0xffff; //用utc的低16位作为分片标识
	quint16 fragment_count = ztp.getRwaData().length()/MTU+1;

	quint16 fragment_offset = 1;
	for(int i = 0;i < fragment_count;i++)
	{
        if(i != 0)
            msleep(1);
        Fragment fragment;
        fragment.identifier = identifier; //用utc的低16位作为分片标识
        fragment.fragment_count = fragment_count;
        fragment.fragment_offset = fragment_offset++;
        fragment.data = ztp.getRwaData().left(MTU);
        ztp.getRwaData().remove(0,MTU);
        fragment.len = fragment.data.length();
        fragment.generate();
        int sendLen = _Socketlistener.writeDatagram(fragment.rawPkg,host,port);
        if(sendLen != fragment.rawPkg.length())
        {
            qDebug("send ZTP data error: has data %d bytes and actually send %d bytes!!",
                    fragment.len,sendLen);
            qDebug("ztp err: %s",_Socketlistener.errorString ().toLatin1().data());
            return FAILED;
        }
//        qDebug()<<"send---fragment : "<<fragment.identifier<<" "<<fragment.checksum<<" "<<fragment.fragment_count<<" "<<fragment.fragment_offset<<" "<<fragment.data.length();
    }
	return SUCCESS;
}
static bool lessThan(const Fragment* frag1, const Fragment* frag2)
 {
     return frag1->fragment_offset < frag2->fragment_offset;
 }
void ZTPManager::msleep(int msecs)
{
#if defined(Q_OS_LINUX)
    usleep(msecs * 1000);
#elif defined(Q_OS_WIN)
    Sleep(msecs);
#endif
}
void ZTPManager::procPkg(const QByteArray &recvBuff,const QString &remoteHost,quint16 remotePort)
{
    Fragment* fragment = new Fragment(recvBuff);
    if(!fragment->isValid())
    {
        fragment->print();
        return;
    }
    FragmentList* fragMentList = NULL;

//    qDebug()<<"fragment :"<<fragment->identifier<<" "<<fragment->checksum<<" "<<fragment->fragment_count<<" "<<fragment->fragment_offset<<" "<<fragment->len;
//    qDebug()<<"fragment isvalid:"<<fragment->isValid();
    if(!workMap.contains(fragment->identifier) || workMap[fragment->identifier] == NULL)
    {
        fragMentList = new FragmentList(fragment->identifier);
        connect(fragMentList,SIGNAL(timeout(quint16)),this,SLOT(onTimeout(quint16)));
        workMap.insert(fragment->identifier,fragMentList);
    }
    else
    {
        fragMentList =  workMap[fragment->identifier];
    }
    if(fragMentList == NULL)
    {
        return;
    }
//    workMap[fragment->identifier]->timer.start(_timeout);  //内存泄漏隐患
    fragMentList->fragment_list.append(fragment);
    if(fragMentList->fragment_list.length() == fragment->fragment_count)
    {
        qSort(fragMentList->fragment_list.begin(),fragMentList->fragment_list.end(),lessThan);
        QByteArray recvBuff;
        for(int i = 0; i < fragment->fragment_count;i++)
        {
            recvBuff.append(fragMentList->fragment_list[i]->data);
        }
        FragmentList* node = fragMentList;
        workMap.remove(fragment->identifier);
//        node->timer.stop();//内存泄漏隐患
        delete node;        // ZZZZZZZZZZZZZZZZZZZZZZZ
        qint64 len;
        memcpy(&len,recvBuff.data()+6,8);
        if(recvBuff.length()!=len)
        {
            qDebug("recv ZTP data error: has data %lld bytes and actually recv %d bytes!!\n",
                len,recvBuff.length());
        }
        ZTPprotocol* ztp = new ZTPprotocol(recvBuff);
        ztp->addPara("RemoteHost",remoteHost);
        ztp->addPara("RemotePort",QString::number(remotePort));

        ztpListMutex.lock();
        if(ztpList.length()>=500)
        {
            delete ztp;
            ztpListMutex.unlock();
            return;
        }
        ztpList.append(ztp);
        ztpListMutex.unlock();
        emit readyRead();
    }
}
void ZTPManager::onRead()
{
    while(_Socketlistener.hasPendingDatagrams())
    {
        QHostAddress remoteHost;
        quint16 remotePort;
        QByteArray recvBuff(_Socketlistener.pendingDatagramSize(),'\0');
        _Socketlistener.readDatagram(recvBuff.data(),recvBuff.length(),&remoteHost,&remotePort);
        emit registTask(recvBuff,remoteHost.toString(),remotePort);
    }
}

void ZTPManager::onTimeout(quint16 identifier){
    FragmentList* node = workMap[identifier];
//    node->timer.stop();//内存泄漏隐患
    workMap.remove(identifier);
    delete node;
}
