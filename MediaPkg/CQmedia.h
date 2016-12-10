#ifndef CQMEDIA_H
#define CQMEDIA_H

#include <gstreamer-0.10/gst/gst.h>
#include "CQGstbasic.h"

class CQMedia :public CQGstBasic
{
    Q_OBJECT
public:
    enum DevType{ROOM,SUBVIDEO,CAR};
    CQMedia(int windowID,QWidget *win,DevType type = SUBVIDEO,QObject *parent = NULL);

    void setVolume(double volume);
    double getVolume();
    bool loadURL(const QString& url);
    void setSilence(bool toggle);
    bool isSilence()const{return silenceState;}
private:
    DevType devType;
    double volume;
    bool silenceState;

    void _unref(GstElement* elem);
    void _updateDecoder();
    void _updateDemux();
private slots:
    void _emitTimeout();
public slots:


};

#endif // CQMEDIA_H
