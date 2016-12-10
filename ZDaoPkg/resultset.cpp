#include "resultset.h"

#include<QDebug>
void ResultRow::print()
{
    for(int i = 0;i < _resMap.keys().length();i++)
    {
        qDebug()<<_resMap.keys()[i]<<" : "<<_resMap[_resMap.keys()[i]];
    }
}


ResultSet::ResultSet()
{
}
void ResultRow::insert(const QString& key,const QString& value)
{
    _resMap.insert(key,value);
}
void ResultSet::print()
{
    for(int i = 0;i < resultList.length();i++)
    {
        resultList[i].print();
    }
}
