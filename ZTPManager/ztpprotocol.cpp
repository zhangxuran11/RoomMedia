#include "ztpprotocol.h"
#include<QDebug>

ZTPprotocol::ZTPprotocol(QByteArray &bytes)
{
    load(bytes);
}
void ZTPprotocol::print()
{
    QList<QString> keyList = map.keys();
    for(int i = 0;i < keyList.count();i++)
    {
        qDebug()<<QString("%1 : %2").arg(keyList[i]).arg(QString::fromUtf8(map[keyList[i]]));
    }
}
static QList<QByteArray> split(const QByteArray& bytes,const QByteArray & sep)
{
    QList<QByteArray> resList;
    int off = 0,pos = 0;
    while(true)
    {
        pos = bytes.indexOf(sep,off);
        if(pos == -1)
        {
            resList.append(bytes.mid(off,-1));
            break;
        }
        else
        {
            resList.append(bytes.mid(off,pos-off));
        }
        off = pos + sep.length();
    }
    return resList;

}
//"&head&64位无符号长度&|&paraName1:|:paraValue1&|&paraName2:|:paraValue2&end&"
void ZTPprotocol::load(QByteArray& bytes)
{
    rawData = bytes;
    int pos = bytes.indexOf("&head&");
    bytes.remove(pos,17);
    pos = bytes.indexOf("&end&");
    bytes.remove(pos,5);
    QList<QByteArray> strList = split(bytes,"&|&");
    for(int i = 0;i<strList.length();i++)
    {
        QList<QByteArray> subList = split(strList[i],":|:");
        QString k = QString::fromUtf8(subList[0]);
        QByteArray v = subList[1];
        map.insert(k,v);
    }
}
void ZTPprotocol::clear()
{
    map.clear();
    rawData.clear();
}

void ZTPprotocol::removePara(const QString& paraName)
{
    map.remove(paraName);
}

void ZTPprotocol::addPara(const QString& paraName,const QString& paraValue)
{
    map.insert(paraName,paraValue.toUtf8());
}

void ZTPprotocol::addPara(const QString& paraName,const QByteArray& paraValue,EType type )
{
    if(type == FILE)
        map.insert(paraName,paraValue);
}
void ZTPprotocol::genarate()
{
    QList<QString> keyList = map.keys();
    rawData = "&head&00000000";
    for(int i = 0;i<keyList.length();i++)
    {
        rawData.append("&|&");
        rawData.append(keyList[i].toUtf8());
        rawData.append(":|:");
        rawData.append(map[keyList[i]]);
    }
    rawData.append("&end&");
    qint64 len = rawData.length();
    memcpy(rawData.data()+6,&len,8);
}
