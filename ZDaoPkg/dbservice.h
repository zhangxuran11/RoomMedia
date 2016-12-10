#ifndef DBSERVICE_H
#define DBSERVICE_H
#include<QSqlDatabase>
#include<QMutex>
#include<QThreadPool>
#include<QFuture>
#include<unistd.h>
#include"resultset.h"
class DBService
{
    QThreadPool* dbThreadPool;
    bool vacuum_f;
    QMutex _mutex;
    QFuture<void> f_ret;
public:
    void beginVacuumThread();
    void cancelVacuumThread();
    QSqlDatabase database;
    bool isValid;
    explicit DBService(const QString& dbName,const QString& ConnName = "");
    ~DBService(){
        cancelVacuumThread();
        while(!f_ret.isFinished()){
            usleep(1000*10);
        }
        _mutex.lock();
        _mutex.unlock();
        close();
    }

    bool open();
    void close();
    bool  exec(const QString& sql,bool vaccum = true);
    bool  exec(const QStringList& sqlInserts,bool vaccum = true);
    ResultSet  query(const QString& sqlQuery);
private:
    void vacuum(){vacuum_f = true;}
    void vacuum_thread();
};

#endif // DBSERVICE_H


