#ifndef ZTPTASK_H
#define ZTPTASK_H
#include <QThread>
#include<QDebug>
class ZTPTask :public QThread
{
    Q_OBJECT
public:
    ZTPTask();

protected slots:
    void run()
    {
        exec();
    }
};
#endif // ZTPTASK_H
