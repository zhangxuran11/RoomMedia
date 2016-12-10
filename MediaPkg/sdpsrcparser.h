#ifndef SDPSRCPARSER_H
#define SDPSRCPARSER_H
#include<QObject>
#include<QEventLoop>
#include<QNetworkReply>
class SdpsrcParser : public QObject
{

    Q_OBJECT
public:

    bool m_isValid;
    QString broadcastIP;
    int     v_port;
    int     v_payload;
    QString v_coding;
    QString v_packetization_mode;
    int     v_clock_rate;
    QString v_sprop_parameter_sets;
    QString v_profile_level_id;

    int     a_port;
    int     a_payload;
    QString a_coding;
    int     a_clock_rate;
    QString a_encoding_params;
    QString a_profile_level_id;
    QString a_mode;
    QString a_sizelength;
    QString a_indexlength;
    QString a_indexdeltalength;
    QString a_config;

    QString a_caps_string;
private:
    enum EMedia{NOTHING,VIDEO,AUDIO};
    EMedia media;

    void parseLine(QByteArray& line);
    QByteArray& waitReply(int msecs = 300);
    QByteArray waitData;
    QNetworkReply *reply;

    QEventLoop wait_q; //用于等待退出的循环

private slots:
    void httpRead();
    void httpFinshed();
public:
    SdpsrcParser(const QString& url);
    bool isValid()const{return m_isValid;}
    const char* get_a_caps_string();


    void printf();
};

#endif // SDPSRCPARSER_H
