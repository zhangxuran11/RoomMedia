#include "CQmedia.h"
#include"sdpsrcparser.h"
#include"MediaPkgCommon.h"
#include<QDebug>
#include<QWidget>
#include"ztpmanager.h"
#include<unistd.h>
CQMedia::CQMedia(int winID,QWidget* win, DevType type,QObject* parent):
    CQGstBasic(winID,win,parent)
{
    if(type == ROOM||type == SUBVIDEO)
        qosMax = 20;
    else if(type == CAR)
        qosMax = 20;
    devType = type;
    silenceState = false;
    this->pipeline=gst_pipeline_new( "pipeline" );
    //构造各组件
    GstElement *udpsrc =  gst_element_factory_make("udpsrc","udpsrc");
    g_object_set(G_OBJECT(udpsrc),"do-timestamp",false,NULL);
    g_object_set(G_OBJECT(udpsrc),"timeout",(quint64)2000000,NULL);
    GstElement *queue0 = gst_element_factory_make("queue","queue0");
    g_object_set(G_OBJECT(queue0),"max-size-time",(guint64)0,NULL);
    GstElement *rtpmp2tdepay  =  gst_element_factory_make("rtpmp2tdepay","rtpmp2tdepay");

    GstElement *queue1 = gst_element_factory_make("queue","queue1");
    g_object_set(G_OBJECT(queue1),"max-size-time",(guint64)0,NULL);
    GstElement *aiurdemux = gst_element_factory_make("aiurdemux","aiurdemux");
    //g_object_set(G_OBJECT(aiurdemux),"streaming_latency",(guint64)5000,NULL);
    GstElement *queue2 = gst_element_factory_make("queue","queue2");
    g_object_set(G_OBJECT(queue2),"max-size-time",(guint64)0,NULL);
#ifdef ARM
    GstElement *v_dec = gst_element_factory_make("mfw_vpudecoder","v_dec");
//    g_object_set(G_OBJECT(v_dec),"parser",false,"dbkenable",false,"profiling",false,
//                 "framedrop",true,"min-latency",true,"fmt",(guint64)0,NULL);
    GstElement *v_sink  = gst_element_factory_make("imxv4l2sink","v_sink");
    g_object_set(G_OBJECT(v_sink),"sync",true,"x11enable",true,NULL);
#endif
#ifdef X86
    GstElement *v_dec = gst_element_factory_make("ffdec_h264","v_dec");
    GstElement *colorspace =  gst_element_factory_make("ffmpegcolorspace","colorspace");
    GstElement *v_sink  = gst_element_factory_make("ximagesink ","v_sink ");
    g_object_set(G_OBJECT(v_sink),"sync",true,NULL);
#endif
    GstElement *queue3 = gst_element_factory_make("queue","queue3");
    g_object_set(G_OBJECT(queue3),"max-size-time",(guint64)0,NULL);

#ifdef ARM
    GstElement *a_dec = gst_element_factory_make("beepdec","a_dec");
#endif

#ifdef X86
    GstElement *a_dec = gst_element_factory_make("ffdec_aac","a_dec");
#endif

    GstElement *a_conv = gst_element_factory_make("audioconvert","audioconvert");
    GstElement *capsfilter = gst_element_factory_make("capsfilter","capsfilter");
    GstCaps *a_caps = gst_caps_from_string("audio/x-raw-int, channels=2");
    g_object_set(G_OBJECT(capsfilter),"caps",a_caps,NULL);
    GstElement *audiovol = gst_element_factory_make("volume","audiovol");
    GstElement *a_sink = gst_element_factory_make("alsasink","a_sink");
    g_object_set(G_OBJECT(a_sink),"sync",true,NULL);
//串连组件
#ifdef ARM
    if(pipeline==NULL||udpsrc==NULL||queue0==NULL||rtpmp2tdepay==NULL||queue1==NULL||aiurdemux==NULL||
            queue2==NULL||v_dec==NULL||v_sink==NULL||queue3==NULL||a_dec==NULL||a_conv==NULL||capsfilter==NULL||audiovol==NULL||a_sink==NULL)
    {
        qDebug("video create failed\n");
    }
    gst_bin_add_many (GST_BIN (pipeline), udpsrc,queue0,rtpmp2tdepay,queue1,aiurdemux,
                      queue2,v_dec,queue3,a_dec,a_conv,capsfilter,audiovol,a_sink,v_sink,NULL);
    gst_element_link_many ( udpsrc,queue0,rtpmp2tdepay,queue1,aiurdemux,NULL);
    gst_element_link_many ( queue2,v_dec,v_sink,NULL);
    g_signal_connect (aiurdemux, "pad-added", G_CALLBACK (pad_added_handler),queue2);
    gst_element_link_many ( queue3,a_dec,a_conv,capsfilter,audiovol,a_sink,NULL);
    g_signal_connect (aiurdemux, "pad-added", G_CALLBACK (pad_added_handler), queue3);
#endif
#ifdef X86
    if(pipeline==NULL||udpsrc==NULL||queue0==NULL||rtpmp2tdepay==NULL||queue1==NULL||aiurdemux==NULL||
            queue2==NULL||v_dec==NULL||colorspace==NULL||v_sink==NULL||queue3==NULL||a_dec==NULL||a_conv==NULL||capsfilter==NULL||audiovol==NULL||a_sink==NULL)
    {
        qDebug("video create failed\n");
    }
    gst_bin_add_many (GST_BIN (pipeline), udpsrc,queue0,rtpmp2tdepay,queue1,aiurdemux,
                      queue2,v_dec,colorspace,v_sink,queue3,a_dec,a_conv,capsfilter,audiovol,a_sink,NULL);
    gst_element_link_many ( udpsrc,queue0,rtpmp2tdepay,queue1,aiurdemux,NULL);

    gst_element_link_many ( queue2,v_dec,colorspace,v_sink,NULL);
    g_signal_connect (aiurdemux, "pad-added", G_CALLBACK (pad_added_handler), queue2);
    gst_element_link_many ( queue3,a_dec,a_conv,capsfilter,audiovol,a_sink,NULL);
    g_signal_connect (aiurdemux, "pad-added", G_CALLBACK (pad_added_handler), queue3);

#endif


        //添加消息监听函数
    GstBus* bus=gst_pipeline_get_bus(GST_PIPELINE(pipeline));
    gst_bus_add_watch(bus,bus_callback,this);
    gst_object_unref(bus);
    gst_element_set_state(pipeline,GST_STATE_NULL);
    this->state=STOPPED;
}
void CQMedia::_emitTimeout()
{
    emitError(TIMEOUT);
}
bool CQMedia::loadURL(const QString& url)
{
    QString tmp = url;
    tmp.remove(0,6);
    QString ip = tmp.split(QChar(':'))[0];
    int port = tmp.split(QChar(':'))[1].toInt();
    QUdpSocket udp;
    udp.bind(port,QUdpSocket::ShareAddress);
    udp.joinMulticastGroup(QHostAddress(ip));
    QEventLoop q;
    QTimer::singleShot(1000,&q,SLOT(quit()));

    connect(&udp,SIGNAL(readyRead()),&q,SLOT(quit()));
    q.exec();
    if(udp.pendingDatagramSize() == -1)
    {
        //qDebug("has no avalid datapkg...");
        QTimer::singleShot(3500,this,SLOT(_emitTimeout()));
        return false;
    }
    qDebug("has avalid datapkg...");
    udp.leaveMulticastGroup(QHostAddress(ip));
    udp.close();

    this->url = url;
//    SdpsrcParser sdpSrc(url);
//    if(!sdpSrc.isValid())
//    {
//        qDebug()<<"SdpsrcParser invalid!!";
//        return false;
//    }
//    sdpSrc.printf();
    GstElement *udpsrc =  gst_bin_get_by_name(GST_BIN(pipeline),"udpsrc");
    GstCaps *caps = gst_caps_new_simple("application/x-rtp",NULL);
    //QString uri = "udp://"+sdpSrc.broadcastIP+":"+QString::number(sdpSrc.v_port);
//    QString uri = "udp://"+sdpSrc.broadcastIP+":"+QString::number(sdpSrc.v_port);
   // QString uri = "udp://239.255.42.42:1234";
    QString uri = url;

    g_object_set(G_OBJECT(udpsrc),"uri",uri.toUtf8().data(),"caps",caps,NULL);
    gst_object_unref(udpsrc);
    return true;

}
static double lineFun(int x)
{
    double y;
//    if(x <= 90)
//        y = x/90.0;
//    else
//        y = 9 * x/ 10.0 - 80;
//    y = x / 100.0;
    if(x < 50)
        y = 0.005*x;
    else
        y = 0.015*x-0.5;
    return y;
}
void CQMedia::setVolume(double _volume)
{
    volume=_volume;
    if(silenceState)
        return;
    GstElement *vol = gst_bin_get_by_name((GstBin *)pipeline,"audiovol");
    if(vol != NULL)
        g_object_set(vol,"volume",lineFun(volume),NULL);
}
double CQMedia::getVolume()
{
    return this->volume;
}

void CQMedia::setSilence(bool toggle)
{
    if(toggle)
    {
        GstElement *vol = gst_bin_get_by_name((GstBin *)pipeline,"audiovol");
        if(vol != NULL)
            g_object_set(vol,"volume",0.0,NULL);
    }
    else
    {
        GstElement *vol = gst_bin_get_by_name((GstBin *)pipeline,"audiovol");
        if(vol != NULL)
            g_object_set(vol,"volume",lineFun(volume),NULL);
    }
    silenceState = toggle;
}

void CQMedia::_updateDecoder()
{
    GstElement *videodecoder = gst_bin_get_by_name((GstBin*)pipeline,"v_dec");
    GstStateChangeReturn ret=gst_element_set_state(videodecoder,GST_STATE_NULL);
    if(GST_STATE_CHANGE_SUCCESS==ret)
    {
        qDebug("set videodecoder to null SUCCESS!!");
        if(gst_bin_remove(GST_BIN(pipeline),videodecoder))
        {
            qDebug("remove videodecoder success!!");
            int refCount = GST_OBJECT_REFCOUNT_VALUE(videodecoder);
            for(int i = 0;i < refCount; i++)
                gst_object_unref(videodecoder);
            videodecoder =  gst_element_factory_make("mfw_vpudecoder","v_dec");
            g_object_set(G_OBJECT(videodecoder),"parser",false,"dbkenable",false,"profiling",false,
                         "framedrop",true,"min-latency",true,"fmt",(guint64)0,NULL);
            GstElement *queue2 = gst_bin_get_by_name((GstBin*)pipeline,"queue2");
            GstElement *v_sink = gst_bin_get_by_name((GstBin*)pipeline,"v_sink");
            gst_bin_add_many (GST_BIN (pipeline),videodecoder,NULL);
            if(gst_element_link_many ( queue2,videodecoder,v_sink,NULL))
            {
                qDebug("link videodecoder  SUCCESS!!");
            }
            else
            {
                qDebug("link videodecoder  FAILED!!");
            }
            gst_object_unref(queue2);
            gst_object_unref(v_sink);
        }
        else
        {
            qDebug("remove videodecoder failed!!");
        }
    }
    else
    {
        qDebug("set videodecoder to null FAILED!!");
    }
}
void CQMedia::_updateDemux()
{
    GstElement *aiurdemux = gst_bin_get_by_name((GstBin*)pipeline,"aiurdemux");
    GstStateChangeReturn ret=gst_element_set_state(aiurdemux,GST_STATE_NULL);
    if(GST_STATE_CHANGE_SUCCESS==ret)
    {
        qDebug("set aiurdemux to null SUCCESS!!");
        if(gst_bin_remove(GST_BIN(pipeline),aiurdemux))
        {
            qDebug("remove aiurdemux success!!");
            gst_object_unref(aiurdemux);
            aiurdemux =  gst_element_factory_make("aiurdemux","aiurdemux");
            if(devType == ROOM || devType == SUBVIDEO)
                g_object_set(G_OBJECT(aiurdemux),"streaming_latency",(guint64)3000,NULL);
            else
                g_object_set(G_OBJECT(aiurdemux),"streaming_latency",(guint64)3000,NULL);

            GstElement *queue1 = gst_bin_get_by_name((GstBin*)pipeline,"queue1");
            GstElement *queue2 = gst_bin_get_by_name((GstBin*)pipeline,"queue2");
            GstElement *queue3 = gst_bin_get_by_name((GstBin*)pipeline,"queue3");
            //qDebug("queue3 is %p",queue3);
            gst_bin_add_many (GST_BIN (pipeline),aiurdemux,NULL);
            if(gst_element_link ( queue1,aiurdemux))
            {
                qDebug("link aiurdemux  SUCCESS!!");
            }
            else
            {
                qDebug("link aiurdemux  FAILED!!");
            }
            g_signal_connect (aiurdemux, "pad-added", G_CALLBACK (pad_added_handler), queue2);
            g_signal_connect (aiurdemux, "pad-added", G_CALLBACK (pad_added_handler), queue3);

            gst_object_unref(queue1);
            gst_object_unref(queue2);
            gst_object_unref(queue3);
        }
        else
        {
            qDebug("remove aiurdemux failed!!");
        }
    }
    else
    {
        qDebug("set aiurdemux to null FAILED!!");
    }
}
