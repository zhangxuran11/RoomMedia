#ifndef RESULTSET_H
#define RESULTSET_H
#include<QList>
#include<QMap>
#include<QVariant>
#include<QString>
class ResultRow
{
    QMap<QString,QString> _resMap;

    friend class DBService;
    void insert(const QString& key,const QString& value);
public:
    const QString getPara(const QString& paraName)const{return _resMap[paraName];}
    double getParaDouble(const QString& paraName)const{return _resMap[paraName].toDouble();}
    float getParaFloat(const QString& paraName)const{return _resMap[paraName].toFloat();}
    int getParaInt(const QString& paraName)const{return _resMap[paraName].toInt();}
    int count()const{return _resMap.count();}
    QString getPara(int index)const{return _resMap[_resMap.keys()[index]];}
    void print();
};
class ResultSet
{
    QList<ResultRow> resultList;
    friend class DBService;
public:
    ResultSet();
    int count()const{return resultList.count();}
    const ResultRow& operator [] (int i)const{return resultList[i];}
    void print();
};

#endif // RESULTSET_H
