#include "sdpsrcparser.h"
#include <QtCore/QFile>
#include<QStringList>
#include <QtDebug>
#include<QNetworkAccessManager>
#include<QNetworkReply>
#include<QEventLoop>
#include<QTimer>
void SdpsrcParser::httpRead()
{
    waitData.append(reply->readAll());
}
void SdpsrcParser::httpFinshed()
{
    if(reply->error())
    {
        waitData.clear();
        qDebug()<<"error :"<<reply->errorString();
    }
    wait_q.quit();

}
QByteArray& SdpsrcParser::waitReply(int msecs)
{
    waitData.clear();
    connect(reply,SIGNAL(readyRead()),this,SLOT(httpRead()));
    connect(reply,SIGNAL(finished()),this,SLOT(httpFinshed()));
    QTimer::singleShot(msecs,&wait_q,SLOT(quit()));
    wait_q.exec();

    return waitData;
}
SdpsrcParser::SdpsrcParser(const QString& url)
{
    m_isValid = false;
    media = NOTHING;
    QNetworkAccessManager qnam;
    reply = qnam.get(QNetworkRequest(QUrl(url)));
    QByteArray& bytes = SdpsrcParser::waitReply(500);
    if(bytes == "")
    {
        qDebug()<<"No fund sdp data!!";
        return;
    }
    QList<QByteArray> byteList = bytes.split('\n');
    for(int i = 0;i< byteList.count();i++)
    {
        QByteArray line = byteList[i].trimmed();
        if(line.data()[1]!= '=')
            continue;
        parseLine(line);
    }

    m_isValid = true;

}

void SdpsrcParser::parseLine(QByteArray& line)
{
    QString str(line);

    const char cmd = str.toUtf8().data()[0];
    str = str.mid(2);
    switch(cmd)
    {
    case 'c':
        broadcastIP = str.split(QChar(' '))[2].split(QChar('/'))[0];
        break;
    case 'm':
    {
        QString tmp;
        tmp = str.split(QChar(' '))[0];
        if(tmp=="video")
            media = VIDEO;
        else if(tmp=="audio")
            media = AUDIO;
        else
            media = NOTHING;
        tmp = str.split(QChar(' '))[1];
        if(media == VIDEO)
            v_port = tmp.toInt();
        else if(media == AUDIO)
            a_port = tmp.toInt();
        break;
    }
    case 'a':
        QStringList strList;
        if(media == VIDEO)
        {
            strList = str.split(QChar(':'));
            if(strList[0].trimmed() == "rtpmap")
            {
                strList = strList[1].split(QChar(' '),QString::SkipEmptyParts);
                v_payload = strList[0].toInt();
                v_coding = strList[1].split(QChar('/'))[0];
                v_clock_rate = strList[1].split(QChar('/'))[1].toInt();
            }
            else if(strList[0].trimmed() == "fmtp")
            {
                strList = strList[1].split(QRegExp("\\s+|;"),QString::SkipEmptyParts);
                for(int i = 0;i < strList.length ();i++)
                {
                    int index;
                    index = strList[i].indexOf('=');
                    if(index == -1)
                        continue;
                    if(strList[i].trimmed().left(index) == "packetization-mode")
                        v_packetization_mode = strList[i].trimmed().mid(index+1);
                    else if(strList[i].trimmed().left(index) == "sprop-parameter-sets")
                        v_sprop_parameter_sets = strList[i].trimmed().mid(index+1);
                    else if(strList[i].trimmed().left(index) == "profile-level-id")
                        v_profile_level_id = strList[i].trimmed().mid(index+1);

                }
            }
        }
        else if(media == AUDIO)
        {
            strList = str.split(QChar(':'));
            if(strList[0].trimmed() == "rtpmap")
            {
                strList = strList[1].split(QChar(' '),QString::SkipEmptyParts);
                a_payload = strList[0].toInt();
                a_coding = strList[1].split(QChar('/'))[0];
                a_clock_rate = strList[1].split(QChar('/'))[1].toInt();
                a_encoding_params = strList[1].split(QChar('/'))[2];
            }
            else if(strList[0].trimmed() == "fmtp")
            {
                strList = strList[1].split(QRegExp("\\s+|;"),QString::SkipEmptyParts);
                for(int i = 0;i < strList.length ();i++)
                {
                    int index;
                    index = strList[i].indexOf('=');
                    if(index == -1)
                        continue;
                    if(strList[i].trimmed().left(index) == "profile-level-id")
                        a_profile_level_id = strList[i].trimmed().mid(index+1);
                    else if(strList[i].trimmed().left(index) == "mode")
                        a_mode = strList[i].trimmed().mid(index+1);
                    else if(strList[i].trimmed().left(index) == "sizelength")
                        a_sizelength = strList[i].trimmed().mid(index+1);
                    else if(strList[i].trimmed().left(index) == "indexlength")
                        a_indexlength = strList[i].trimmed().mid(index+1);
                    else if(strList[i].trimmed().left(index) == "indexdeltalength")
                        a_indexdeltalength = strList[i].trimmed().mid(index+1);
                    else if(strList[i].trimmed().left(index) == "config")
                        a_config = strList[i].trimmed().mid(index+1);

                }
            }
        }

        break;
    }

}


const char* SdpsrcParser::get_a_caps_string()
{
    if(a_caps_string ==  "")
    {
        a_caps_string = "application/x-rtp, media=(string)audio, payload=(int)" +QString::number(a_payload)+",clock-rate=(int)"+QString::number(a_clock_rate)+",encoding-name=(string)"+
                a_coding+",encoding-params=(string)"+a_encoding_params+",profile-level-id=(string)"+a_profile_level_id+",mode=(string)"+a_mode+",sizelength=(string)"+a_sizelength+
                ",indexlength=(string)"+a_indexlength+",indexdeltalength=(string)"+a_indexdeltalength+",config=(string)"+a_config;
    }
    static QByteArray buff = a_caps_string.toUtf8();
    return buff.data();

}
void SdpsrcParser::printf()
{
    qDebug("broadcastIP:%s\n",broadcastIP.toUtf8().data());
    qDebug("video:\n");
    qDebug("v_port:%d\n",v_port);
    qDebug("v_payload:%d\n",v_payload);
    qDebug("v_coding:%s\n",v_coding.toUtf8().data());
    qDebug("v_packetization_mode:%s\n",v_packetization_mode.toUtf8().data());
    qDebug("v_clock_rate:%d\n",v_clock_rate);
    qDebug("v_sprop_parameter_sets:%s\n",v_sprop_parameter_sets.toUtf8().data());
    qDebug("v_profile_level_id:%s\n",v_profile_level_id.toUtf8().data());
    qDebug("audio:\n");
    qDebug("a_port:%d\n",a_port);
    qDebug("a_payload:%d\n",a_payload);
    qDebug("a_coding:%s\n",a_coding.toUtf8().data());
    qDebug("a_clock_rate:%d\n",a_clock_rate);
    qDebug("a_encoding_params:%s\n",a_encoding_params.toUtf8().data());
    qDebug("a_profile_level_id:%s\n",a_profile_level_id.toUtf8().data());
    qDebug("a_mode:%s\n",a_mode.toUtf8().data());
    qDebug("a_sizelength:%s\n",a_sizelength.toUtf8().data());
    qDebug("a_indexlength:%s\n",a_indexlength.toUtf8().data());
    qDebug("a_indexdeltalength:%s\n",a_indexdeltalength.toUtf8().data());
    qDebug("a_config:%s\n",a_config.toUtf8().data());
    qDebug("a_caps_string:%s\n",get_a_caps_string());
}
