#include "zutility.h"
#include <QHostAddress>
#include<QNetworkInterface>
#include<QStringList>
ZUtility::ZUtility()
{

}

 QString ZUtility::getVideoUrl(int channel)
{
//     if(channel == 1)
//         return "udp://225.0.0.1:2002";
//     if(channel == 2)
//         return "udp://225.0.0.1:2001";
//    if(channel == 9)
//        return "udp://239.255.42.42:1234";
//    else
        return QString("udp://225.0.0.1:200%1").arg(channel);
}
//QString ZUtility::getVideoUrl(QString VServerIP,int channel)
//{
//   if(channel == 9)
//       return "http://192.168.1.123:8090/test2.sdp";
//   else
//       return QString("http://"+VServerIP +":%1/test%2.sdp").arg(8090+2*channel).arg(channel);
//}

int ZUtility::getRoomNr()
{
    QList<QHostAddress> list = QNetworkInterface::allAddresses();
    foreach(QHostAddress address,list)
    {
        QString ip = address.toString();
        if(ip.startsWith("192.168."))
        {
            return (ip.split(QChar('.'))[3].toInt() - 21)/2 + 1;
        }
    }
    return 0;
}
int ZUtility::getBedNr()
{
    QList<QHostAddress> list = QNetworkInterface::allAddresses();
    foreach(QHostAddress address,list)
    {
        QString ip = address.toString();
        if(ip.startsWith("192.168."))
        {
            return ip.split(QChar('.'))[3].toInt();
        }
    }
    return 0;
}

int ZUtility::getDeviceNr()//主 1 次 0
{
    QList<QHostAddress> list = QNetworkInterface::allAddresses();
    foreach(QHostAddress address,list)
    {
        QString ip = address.toString();
        if(ip.startsWith("192.168."))
        {
            return (ip.split(QChar('.'))[3].toInt() - 21)%2;
        }
    }
    return -1;
}

