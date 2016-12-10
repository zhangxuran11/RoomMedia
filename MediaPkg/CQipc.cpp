#include "CQipc.h"
#include "MediaPkgCommon.h"
bool CQipc::loadURL(const QString& url)
{
    this->url = url;
    GstElement *v_src =  gst_bin_get_by_name(GST_BIN(pipeline),"v_src");
    g_object_set(G_OBJECT(v_src),"location",url.toUtf8().data(),NULL);
    gst_object_unref(v_src);
    return true;
}

CQipc::CQipc(int winID,QObject* parent):
    CQGstBasic(winID,0,parent)
{
    this->pipeline=gst_pipeline_new( "pipeline" );
    GstElement *v_src =  gst_element_factory_make("rtspsrc","v_src");
    GstElement *videoh264depay  =  gst_element_factory_make("rtph264depay","videoh264depay");
#ifdef ARM
    ///////////////////////////////////////////视频通道////////////////////////////////

    GstElement *videodecoder =  gst_element_factory_make("mfw_vpudecoder","videodecoder");
     GstElement *videosink =  gst_element_factory_make("imxv4l2sink","v_sink");
    g_object_set(G_OBJECT(videodecoder),"parser",false,"dbkenable",true,"profiling",true,
                 "framedrop",true,"min-latency",true,"fmt",(guint64)1,"loopback",true,"use-internal-buffer",true,NULL);
    if(pipeline==NULL||v_src==NULL||videoh264depay==NULL
            ||videodecoder==NULL||videosink==NULL)
    {
        printf("video create failed\n");
    }

    gst_bin_add_many (GST_BIN (pipeline), v_src,videoh264depay,videodecoder,videosink,NULL);
    g_signal_connect (v_src, "pad-added", G_CALLBACK (pad_added_handler), videoh264depay);
    gst_element_link_many ( videoh264depay,videodecoder,videosink,NULL);
    g_object_set(G_OBJECT(videosink),"sync",false,"x11enable",true,NULL);
#endif

#ifdef  X86
    ///////////////////////////////////////////视频通道////////////////////////////////
    GstElement *videodecoder =  gst_element_factory_make("ffdec_h264","videodecoder");
    GstElement *colorspace =  gst_element_factory_make("ffmpegcolorspace","colorspace");
    GstElement *videosink =  gst_element_factory_make("ximagesink","v_sink");
    if(pipeline==NULL||v_src==NULL||videoh264depay==NULL
            ||videodecoder==NULL||colorspace==NULL||videosink==NULL)
    {
        printf("video create failed\n");
    }
    gst_bin_add_many (GST_BIN (pipeline), v_src, videoh264depay,videodecoder,colorspace,videosink,NULL);
    g_signal_connect (v_src, "pad-added", G_CALLBACK (pad_added_handler), videoh264depay);
    gst_element_link_many ( videoh264depay,videodecoder,colorspace,videosink,NULL);
    g_object_set(G_OBJECT(videosink),"sync",false,NULL);

#endif


    ///////////////////////////////////////////属性设置////////////////////////////////


        //添加消息监听函数
    GstBus* bus=gst_pipeline_get_bus(GST_PIPELINE(pipeline));
    gst_bus_add_watch(bus,bus_callback,this);
    gst_object_unref(bus);
    gst_element_set_state(pipeline,GST_STATE_NULL);
    this->state=STOPPED;

}

void CQipc::_updateDecoder()
{
    GstElement *videodecoder = gst_bin_get_by_name((GstBin*)pipeline,"videodecoder");
    GstStateChangeReturn ret=gst_element_set_state(videodecoder,GST_STATE_NULL);
    if(GST_STATE_CHANGE_SUCCESS==ret)
    {
        qDebug("set videodecoder to null SUCCESS!!");
        if(gst_bin_remove(GST_BIN(pipeline),videodecoder))
        {
            qDebug("remove videodecoder success!!");
            gst_object_unref(videodecoder);

            videodecoder =  gst_element_factory_make("mfw_vpudecoder","videodecoder");
            g_object_set(G_OBJECT(videodecoder),"parser",false,"dbkenable",true,"profiling",true,
                         "framedrop",true,"min-latency",true,"fmt",(guint64)1,"loopback",true,"use-internal-buffer",true,NULL);

            GstElement *videoh264depay = gst_bin_get_by_name((GstBin*)pipeline,"videoh264depay");
            GstElement *videosink = gst_bin_get_by_name((GstBin*)pipeline,"v_sink");
            gst_bin_add_many (GST_BIN (pipeline),videodecoder,NULL);
            if(gst_element_link_many ( videoh264depay,videodecoder,videosink,NULL))
            {
                qDebug("link videodecoder  SUCCESS!!");
            }
            else
            {
                qDebug("link videodecoder  FAILED!!");
            }

            gst_object_unref(videoh264depay);
            gst_object_unref(videosink);

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
void CQipc::_updateDemux()
{
}
