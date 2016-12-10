#ifndef CQIPC_H
#define CQIPC_H

#include <iostream>
#include <gstreamer-0.10/gst/gst.h>
#include"CQGstbasic.h"
class CQplayerGUI;



class CQipc :public CQGstBasic
{
    Q_OBJECT
public:
    CQipc(int windowID,QObject *parent = NULL);
    bool loadURL(const QString& url);
private:
    void _updateDecoder();
    void _updateDemux();

};

#endif // CQIPC_H
