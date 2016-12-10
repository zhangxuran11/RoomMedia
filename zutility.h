#ifndef ZUTILITY_H
#define ZUTILITY_H
#include<QString>
class ZUtility
{
public:
    ZUtility();
    static QString getVideoUrl(int channel);
    static int getBedNr();
    static int getRoomNr();
    static int getDeviceNr();//主 0 次 1
};

#endif // zUTILITY_H
