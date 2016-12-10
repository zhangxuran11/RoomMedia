#include "cqaudio.h"
static  gboolean my_bus_callback(GstBus *,GstMessage *msg,gpointer )
{
    qDebug("%s recive message:%s\n",gst_object_get_name (GST_MESSAGE_SRC(msg)),GST_MESSAGE_TYPE_NAME(msg));
    return true;
}
CQAudio::CQAudio(QWidget* _frameMovie,StackPanel* stackPanel,QObject* parent):
    CQGstBasic(0,0,parent),frameMovie(_frameMovie)
{
    s = stackPanel;
    disableUpateImage = false;
    this->pipeline=gst_pipeline_new( "pipeline" );
    //构造各组件
    GstElement *udpsrc =  gst_element_factory_make("udpsrc","cqa_udpsrc");
    g_object_set(G_OBJECT(udpsrc),"multicast-group","224.1.1.1","auto-multicast",true,"port",3000,NULL);
    GstCaps *caps = gst_caps_from_string("application/x-rtp,media=(string)audio,clock-rate=(int)8000,encoding-name=(string)PCMU,payload=(int)0,ssrc=(guint)1350777638,clock-base=(guint)2942119800,seqnum-base=(guint)47141");
    g_object_set(G_OBJECT(udpsrc),"caps",caps,NULL);

    GstElement *rtppcmudepay =  gst_element_factory_make("rtppcmudepay","cqa_rtppcmudepay");
    GstElement *mulawdec =  gst_element_factory_make("mulawdec","cqa_mulawdec");
    GstElement *audioconvert =  gst_element_factory_make("audioconvert","cqa_audioconvert");
    GstElement *capsfilter = gst_element_factory_make("capsfilter","cqa_capsfilter");
    GstCaps *cqa_caps = gst_caps_from_string("audio/x-raw-int, channels=2");
    g_object_set(G_OBJECT(capsfilter),"caps",cqa_caps,NULL);
    GstElement *alsasink =  gst_element_factory_make("alsasink","cqa_alsasink");

//串连组件
    if(pipeline==NULL||udpsrc==NULL||rtppcmudepay==NULL||mulawdec==NULL||audioconvert==NULL||capsfilter==NULL||alsasink==NULL)
    {
        qDebug("audio pipeline create failed\n");
    }
    gst_bin_add_many (GST_BIN (pipeline), udpsrc,rtppcmudepay,mulawdec,audioconvert,capsfilter,alsasink,NULL);
    gst_element_link_many ( udpsrc,rtppcmudepay,mulawdec,audioconvert,capsfilter,alsasink,NULL);

        //添加消息监听函数
    GstBus* bus=gst_pipeline_get_bus(GST_PIPELINE(pipeline));
    gst_bus_add_watch(bus,my_bus_callback,this);
    gst_object_unref(bus);
    gst_element_set_state(pipeline,GST_STATE_NULL);
    this->state=STOPPED;

    ztpm = new ZTPManager(8321,QHostAddress("224.102.228.40"));
    connect(ztpm,SIGNAL(readyRead()),this,SLOT(slot_recvCtrl()));
}
void CQAudio::slot_recvCtrl()
{
    ZTPprotocol ztp;
    ztpm->getOneZtp(ztp);
    //ztp.print();
    if(ztp.getPara("CMD") == "V-PAUSED")
    {
        s->off();
        if(!disableUpateImage)
            frameMovie->setStyleSheet(QString::fromUtf8("border-image: url(:/images/horn.png);"));
        if(ztp.getPara("FROM") == "ESPEAK")
            play();
        if(ztp.getPara("FROM") == "PA")
            stop();
    }
    else if(ztp.getPara("CMD") == "V-PLAY")
    {
        stop();
        //qDebug()<<" frameMovie->setStyleSheet(QString::fromUtf8(\"border-image: url(:/images/welcome.png);\"));";
        if(!disableUpateImage)
            frameMovie->setStyleSheet(QString::fromUtf8("border-image: url(:/images/welcome.png);"));
        s->on();
    }
}
