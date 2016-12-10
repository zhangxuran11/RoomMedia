#ifndef FRAGMENT_H
#define FRAGMENT_H
#include<QObject>
#include<QTimer>
#include<QDebug>
class Fragment//分片
{
    /******************************************************************

      |ˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉ|
     0|  identifier     | checksum          |31
    32|fragment count   | fragment offset   |63
    64|                len                  |95
    96|                data                 |
      |                ....                 |
      |_____________________________________|


    ******************************************************************/
public:
    quint16 identifier;
    quint16 checksum;
    quint16 fragment_count;
    quint16 fragment_offset;
    quint32 len;//仅数据正文长度
    QByteArray data;
    QByteArray rawPkg;
private:

    quint16 generateChecksum();
public:
    Fragment(){}
    explicit Fragment(const QByteArray& bytes);
    bool isValid(){return checksum == generateChecksum();}
    void generate();
    bool operator <(const Fragment& frag){return fragment_offset < frag.fragment_offset;}
    void print()
    {
        qDebug()<<"identifier :"<<identifier;
        qDebug()<<"checksum :"<<checksum;
        qDebug()<<"fragment_count :"<<fragment_count;
        qDebug()<<"fragment_offset :"<<fragment_offset;
        qDebug()<<"len  :"<<len;
        qDebug()<<"data :"<<data;
        qDebug()<<"rawPkg :"<<rawPkg;
        qDebug()<<"isValid :"<<isValid();
    }
};
class FragmentList :public QObject
{
    Q_OBJECT
signals:
    void timeout(quint16 identifier);
public:
    quint16 identifier;
//    QTimer timer;    //这里传输大包有内存泄漏隐患  如果后续希望实现大包传送可考虑开启此定时器  某种情况有问题  慎重实现!
    QList<Fragment*> fragment_list;
    FragmentList(quint16 identifier);
    ~FragmentList()
    {
        for(QList<Fragment*>::iterator it = fragment_list.begin();it !=fragment_list.end();it++)
        {
            delete *it;
        }
    }
private slots:
    void send_timeout(){emit timeout(identifier);}

};

#endif // FRAGMENT_H
