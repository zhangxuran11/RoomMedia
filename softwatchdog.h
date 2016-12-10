#ifndef SOFTWATCHDOG_H
#define SOFTWATCHDOG_H

#include <QObject>
#include <QTimer>
class SoftWatchdog : public QObject
{
    Q_OBJECT
    QObject *obj;
    int elapse;//ms
    QTimer* timer;
    void (*deal)();
public:
    SoftWatchdog(QObject *ressourceObj,void (*method)(),QObject *parent = 0);
private slots:
    void cleanUp();
public slots:
    void feed();
    
};

#endif // SOFTWATCHDOG_H
