#include "mytranslator.h"
#include <QFile>
MyTranslator::MyTranslator(QString iniFile)
{
    curLan = "En";
    configIniRead = new QSettings(iniFile, QSettings::IniFormat);
    configIniRead->setIniCodec("UTF-8");
    chNameIniRead = NULL;

}
void MyTranslator::loadChNameConfig(QString ip)
{
    QString cmdStr = QString("tftp  -g  -r %1  -l /srv/tftp/%1 %2").arg("ChNameConfig.ini").arg(ip);
    system(cmdStr.toAscii());
    if(QFile::exists("/srv/tftp/ChNameConfig.ini")) {
        chNameIniRead = new QSettings("/srv/tftp/ChNameConfig.ini", QSettings::IniFormat);
        chNameIniRead->setIniCodec("UTF-8");
    }
}
QString MyTranslator::tr(QString str)
{
    if(QFile::exists("/srv/tftp/ChNameConfig.ini") &&chNameIniRead == NULL )
    {
        chNameIniRead = new QSettings("/srv/tftp/ChNameConfig.ini", QSettings::IniFormat);
        chNameIniRead->setIniCodec("UTF-8");
    }
    if(chNameIniRead == NULL || (str != "ch1"  && str != "ch2" && str != "ch3" && str != "ch4"
             && str != "ch5" && str != "ch6" && str != "ch7" && str != "ch8"))
        return configIniRead->value("/"+str+"/"+curLan).toString();
    str = "Ch" + str.right(1);
    return chNameIniRead->value("/ChannelName/"+curLan + str).toString();
}
