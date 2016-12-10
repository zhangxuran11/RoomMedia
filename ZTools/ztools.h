#ifndef ZTOOLS_H
#define ZTOOLS_H
#include<QtGlobal>
#include<QTimer>
class ZTools : public QObject
{
    Q_OBJECT
    ZTools();
    void (*_member)();
    void (*_member_with_para)(void*);
    void *_member_para;
    explicit ZTools(void (*member)()){_member = member;}
    ZTools(void (*member)(void*),void* data){_member_with_para = member;_member_para = data;}
private slots:
    void Proxy();
    void ProxyWithPara();
public:
    static void singleShot(int msecs,void (*member)());
    static void singleShot(int msecs,void (*member)(void*),void *data);
    static char* hex2ascii(int hex,char *,int len);
    static quint64 ascii2hex(const char *ascii,int len);
    static QByteArray readLineFromFile(const QString& fileName,int lineNo);
    static void msleep(int msecs);
    static int getCarID();
    static int getCarID_bk();
    static QString getCarIP();

};
#endif // ZTOOLS_H

