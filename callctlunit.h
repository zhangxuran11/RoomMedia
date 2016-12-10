#ifndef CALLCTLUNIT_H
#define CALLCTLUNIT_H
#include <QThread>
class CallCtlUnit : public QThread
{
    Q_OBJECT
    bool flag;//标志用户处于呼叫状态或者非呼叫状态
    int ioNr;
public:
    explicit CallCtlUnit(QObject *parent = 0);
    
signals:
    
public slots:
    void run();
    void sendRoomCall(bool _flag);
    
};

#endif // CALLCTLUNIT_H
