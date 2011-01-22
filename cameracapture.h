/* ***********************************************************************************************************
 * THE GSTREAMER PIPELINE CODE TAKEN FROM THE MAEMO 5 CAMERA EXAMPLE
 * http://wiki.maemo.org/Documentation/Maemo_5_Developer_Guide/Using_Multimedia_Components/Camera_API_Usage
 * ************************************************************************************************************/
#ifndef CAMERA_H
#define CAMERA_H

#include <gst/app/gstappsink.h>
#include <QObject>
class AppData;

class CameraCapture : public QObject
{
    public:
        CameraCapture();
    	void toggle_camera_stream();
    	static gboolean bus_callback (GstBus * bus, GstMessage * message);
    	gboolean initialize_pipeline (int *argc, char ***argv);
    	static void new_buffer_added (GstAppSink *appsink, gpointer user_data);
        static bool buffer_ready;
        static int count;
        static GstAppSink * appsink;
        static void refresh_buffer();
        static bool cold_start;
        static bool scanning;
        static GstBuffer * buffer;

    private:
        GstElement *pipeline;
     	int scanning_status;
    	int processing;
    	int delay;
    	char distance[100];
    	int init();
};
#endif

