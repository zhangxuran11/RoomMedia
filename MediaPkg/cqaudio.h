#ifndef CQAUDIO_H
#define CQAUDIO_H
#include "CQGstbasic.h"
#include"ztpmanager.h"
#include<QWidget>
#include "stackpanel.h"
class CQAudio : public CQGstBasic
{
    Q_OBJECT
    StackPanel* s;
public:
    CQAudio(QWidget* _frameMovie,StackPanel* stackPanel,QObject* parent);
    bool disableUpateImage;
private:
    QWidget* frameMovie;
    ZTPManager* ztpm;
    void _updateDemux(){}
    void _updateDecoder(){}
    bool loadURL(const QString&){return true;}
private slots:
    void slot_recvCtrl();
};

#endif // CQAUDIO_H
