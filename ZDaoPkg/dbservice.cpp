#include "dbservice.h"
#include <unistd.h>
#include<QFile>
#include<QSqlQuery>
#include<QSqlRecord>
#include<QDebug>
#include<QSqlError>
#include<QTime>
#if (QT_VERSION >= QT_VERSION_CHECK(5, 5, 0))
#include<QtConcurrent>
#else
#include<QtCore>
#endif

#include<QThread>
static bool volatile VacuumThreadRunF = true;
void DBService::beginVacuumThread()
{
    VacuumThreadRunF = true;
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    f_ret = QtConcurrent::run(dbThreadPool,this,DBService::vacuum_thread);
#else
    f_ret = QtConcurrent::run(this,&DBService::vacuum_thread);
#endif
}
void DBService::cancelVacuumThread()
{
    VacuumThreadRunF = false;
}
DBService::DBService(const QString& dbName,const QString& connName )
{
#if QT_VERSION < QT_VERSION_CHECK(5,0,0)
    QThreadPool::globalInstance()->setMaxThreadCount(QThreadPool::globalInstance()->maxThreadCount()+1);
#endif
    dbThreadPool = new QThreadPool;
    dbThreadPool->setMaxThreadCount(1);
    isValid = true;
    vacuum_f = false;
    if(connName == "")
        database = QSqlDatabase::addDatabase("QSQLITE");
    else
        database = QSqlDatabase::addDatabase("QSQLITE",connName);
    if(!QFile::exists(dbName))
    {
        qDebug()<<"db "<<dbName<<" is not exists!!";
        isValid = false;
        return;
    }
    database.setDatabaseName(dbName);
//    query("SELECT sqlite_version()").print();
    beginVacuumThread();
}
bool DBService::open()
{
    _mutex.lock();
    if(!database.open())
    {
        qDebug()<<database.lastError();
        _mutex.unlock();
        return false;
    }

    return true;
}
void DBService::close()
{
    database.close();
    _mutex.unlock();
}
ResultSet  DBService::query(const QString& sqlQuery)
{
    qDebug()<<"query sql : "<<sqlQuery;
    ResultSet results;
    isValid = true;
    if(!open())
    {
        isValid = false;
        return results;
    }
    QSqlQuery sql_query(database);
    sql_query.prepare(sqlQuery);
    if(!sql_query.exec())
    {
        isValid = false;
        qDebug()<<sql_query.lastError();
        close();
        return results;
    }
    else
    {
        QSqlRecord record = sql_query.record();
        while(sql_query.next())
        {
            ResultRow resMap;
            for(int i = 0;i < record.count();i++)
            {
//                if(sql_query.value(i).type() == QVariant::Double)
//                    qDebug()<<"data :"<<sql_query.value(i).toDouble();
//                else
//                    qDebug()<<"data :"<<sql_query.value(i).toString();
                resMap.insert(record.fieldName(i),sql_query.value(i).toString());
            }
            results.resultList.append(resMap);
        }
        close();
        return results;
    }
}

void DBService::vacuum_thread()
{
    while(VacuumThreadRunF)
    {
        if(vacuum_f)
        {
            qDebug("exec VACUUM");
            open();
            QSqlQuery sql_query(database);
            sql_query.exec("VACUUM");
            close();
            vacuum_f = false;
        }
//        qDebug("VACUUM thread sleep...");
        sleep(1);
    }
}
bool  DBService::exec(const QString& sql,bool vaccum)
{
//    qDebug()<<"exec sql : "<<sql;
    bool res = false;
    if(!open())
    {
        qDebug()<<database.lastError();
        return res;
    }
    else
    {
        QSqlQuery sql_query(database);
        sql_query.exec("PRAGMA foreign_keys = ON");
        if(!sql_query.exec(sql))
        {
            qDebug()<<sql_query.lastError();
        }
        else
        {
            qDebug()<<"inserted!";
            if(vaccum)
                vacuum();
//            sql_query.exec("VACUUM");
            res = true;
        }
    }
    close();
    return res;
}
bool  DBService::exec(const QStringList& sqlInserts,bool vaccum)
{

    QTime time;
    time.start();
    bool res = true;
    if(!open())
    {
        qDebug()<<database.lastError();
        return false;
    }
    else
    {
        database.transaction();
        QSqlQuery sql_query(database);
        sql_query.exec("PRAGMA foreign_keys = ON");
        for(int i = 0;i < sqlInserts.count();i++)
        {
//            qDebug()<<"exec sql : "<<sqlInserts[i];
            sql_query.prepare(sqlInserts[i]);
            if(!sql_query.exec())
            {
                qDebug()<<sql_query.lastError();
                res = false;
                break;
            }
            else
            {
//                qDebug()<<"inserted!" << i;
            }
//            qDebug("%d,%d",i,sqlInserts.count());
        }
        qDebug()<<"elapsed 0:"<<time.elapsed()/1000.0;
        if(res)
        {
            database.commit();
            if(vaccum)
                vacuum();
        }
        else
            database.rollback();
    }
    close();
    qDebug()<<"elapsed 1:"<<time.elapsed()/1000.0;
    return res;
}

