#ifndef MEDIAPKGCOMMON_H
#define MEDIAPKGCOMMON_H

#include <gstreamer-0.10/gst/gst.h>
extern gboolean bus_callback(GstBus *bus,GstMessage *msg,gpointer data);
extern void pad_added_handler (GstElement *, GstPad *new_pad, GstElement *elem);


#endif // MEDIAPKGCOMMON_H
