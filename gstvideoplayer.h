#ifndef GSTVIDEOPLAYER_H
#define GSTVIDEOPLAYER_H

#include <gst/app/gstappsink.h>
#include <QStringList>
#include <QObject>

class AppData;

class gstVideoPlayer : public QObject
{
public:
    gstVideoPlayer();
    void toggle_play_state(int video_stream);
    static gboolean bus_callback (GstBus * bus, GstMessage * message);
    gboolean initialize_pipeline (QStringList Files);
    static void new_buffer_added (GstAppSink *appsink, gpointer user_data);
    static void decodebin_new_decoded_pad_cb(GstElement *decodebin, GstPad     *pad, gboolean    last, gpointer user_data);
    static bool buffer_ready;
    static GstAppSink **appsink;
    static void refresh_buffer(int video_stream);
    static bool cold_start;
    static bool scanning;
    static GstBuffer ** buffer;
private:
    GstElement *pipeline;
    GstElement **pipelines;
    int scanning_status;
};

#endif // GSTVIDEOPLAYER_H
