#ifndef MYTRANSLATOR_H
#define MYTRANSLATOR_H
#include<QSettings>
#include<QDebug>
class MyTranslator
{
    QSettings *configIniRead;
    QSettings *chNameIniRead;
public:
    QString curLan;
    void loadChNameConfig(QString ip);
    void setLan(QString lan){curLan = lan;}
    MyTranslator(QString iniFile);
    QString tr(QString str);
};

#endif // MYTRANSLATOR_H
