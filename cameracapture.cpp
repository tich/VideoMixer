#include "cameracapture.h"

#include <gst/app/gstappbuffer.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <gst/gst.h>
#include <gst/gstbin.h>
#include <gst/interfaces/xoverlay.h>
#include <assert.h>
#include <time.h>
#include <pthread.h>
#include <sched.h>


bool CameraCapture::buffer_ready;
bool CameraCapture::cold_start;
GstAppSink *CameraCapture::appsink;
GstBuffer *CameraCapture::buffer;
int CameraCapture::count;
bool CameraCapture::scanning=false;

CameraCapture::CameraCapture()
{
        scanning_status = 0;
	count=0;
}
CameraCapture::~CameraCapture()
{
    printf("Stopping camera pipeline.\n");
    if(pipeline)
    {
       gst_element_set_state (pipeline, GST_STATE_NULL);
       gst_object_unref (pipeline);
    }
}

/* ***********************************************************************************************************
 * We are creating a pipeline like:  |v4l2camsrc (omap3cam)| |ffmpegcolorspace| |appsink|
 * Terminal command:
 * gst-launch -v -t v4l2camsrc num-buffers=1 driver-name=omap3cam ! "video/x-raw-yuv,framerate=499/100" ! \
 * ffmpegcolorspace ! "video/x-raw-rgb,framerate=499/100" ! appsink *
 *************************************************************************************************************/
gboolean CameraCapture::initialize_pipeline(int *argc, char ***argv) {

        GstElement *camera_src, *image_sink, *csp_filter, *image_filter;
	GstCaps *caps;
	GstBus *bus;

	//initialize flags
	buffer_ready=false;
	cold_start=true;
	count=0;

	/* Initialize Gstreamer */
	gst_init(argc, argv);

	/* Create pipeline and attach a callback to it's message bus */
	pipeline = gst_pipeline_new("test-camera");
	bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));
        gst_bus_add_watch(bus, (GstBusFunc)bus_callback, NULL);
	gst_object_unref(GST_OBJECT(bus));

	/* Create elements */
	/* Camera video stream comes from a Video4Linux driver */
        camera_src = gst_element_factory_make("v4l2src", "camera_src");
        //g_object_set(G_OBJECT(camera_src), "name", "v4l2src0", NULL); // thanks BBNS_ @ maemo irc aka Yun-Ta Tsai
        //g_object_set(G_OBJECT(camera_src), "device", "/dev/video0", NULL);
	/* Colorspace filter is needed to make sure that sinks understands the stream coming from the camera */
	csp_filter = gst_element_factory_make("ffmpegcolorspace", "csp_filter");
	/* Filter to convert stream to use format that the gdkpixbuf library can use */
	image_filter = gst_element_factory_make("ffmpegcolorspace", "image_filter");
	/* A dummy sink for the image stream. Goes to bitheaven AppSink*/
	image_sink = gst_element_factory_make("appsink", "image_sink");

	/* Check that elements are correctly initialized */
	if(!(pipeline && camera_src && csp_filter && image_sink && image_filter))
	{
		g_critical("Couldn't create pipeline elements");
		return FALSE;
	}

	/* Add elements to the pipeline. This has to be done prior to linking them */
        gst_bin_add_many(GST_BIN(pipeline), camera_src, csp_filter,image_filter,image_sink,NULL);

	/* Specify what kind of video is wanted from the camera */
	//caps = gst_caps_from_string("video/x-raw-yuv,format=(fourcc)UYVY,width=800,height=480"); //framerate=[1/30,30/1]
        //caps = gst_caps_from_string("video/x-raw-yuv,format=(fourcc)UYVY,width=640,height=480"); //framerate=[1/30,30/1]
	//caps = gst_caps_from_string("video/x-raw-yuv,format=(fourcc)UYVY,width=320,height=240"); //framerate=[1/30,30/1]
        caps = gst_caps_from_string("video/x-raw-rgb,width=320,height=240"); //framerate=[1/30,30/1]
	//caps = gst_caps_from_string("video/x-raw-yuv,format=(fourcc)UYVY,width=378,height=225"); //framerate=[1/30,30/1]
	//caps = gst_caps_from_string("video/x-raw-yuv,format=(fourcc)UYVY,width=592,height=400"); //framerate=[1/30,30/1]

	/* Link the camera source and colorspace filter using capabilitie specified */
        if(!gst_element_link_filtered(camera_src, csp_filter,caps))
        //if(!gst_element_link(camera_src, csp_filter))
	{
		return FALSE;
        }
        gst_caps_unref(caps);
	/* Connect Colorspace -> Image Filter -> Image Sink*/
	/* Define picture image_sink format*/
	//caps = gst_caps_from_string("video/x-raw-rgb,width=800,height=480");//framerate=[1/30,30/1]
	//caps = gst_caps_from_string("video/x-raw-rgb,width=640,height=480");//framerate=[1/30,30/1]
	//caps = gst_caps_from_string("video/x-raw-rgb,width=320,height=240");//framerate=[1/30,30/1]
        //caps = gst_caps_from_string("video/x-raw-rgb,width=400,height=240");//framerate=[1/30,30/1]
	//caps = gst_caps_from_string("video/x-raw-rgb,width=378,height=225");//framerate=[1/30,30/1]
	//caps = gst_caps_from_string("video/x-raw-rgb,width=592,height=400");//framerate=[1/30,30/1]

	/* Link the image-branch of the pipeline.*/
        if(!gst_element_link(csp_filter,image_sink)) return FALSE;
        //if(!gst_element_link_many(csp_filter,image_sink, NULL)) return FALSE;
        //if(!gst_element_link_filtered(image_filter, image_sink,caps)) return FALSE;

        //gst_caps_unref(caps);

	/* Set image sink to emit handoff-signal before throwing away  it's buffer */
	g_object_set (G_OBJECT (image_sink), "emit-signals", TRUE, NULL);
	g_signal_connect (image_sink, "new-buffer", G_CALLBACK (new_buffer_added), NULL);

	gst_element_set_state(pipeline, GST_STATE_PLAYING);

	return TRUE;
}

void CameraCapture::toggle_camera_stream()
 {
        if (scanning_status) {
 		// we therefore want to stop the scan
                scanning_status = 0;
                gst_element_set_state (pipeline, GST_STATE_PAUSED); //READY
 		scanning=false;
 	}
 	else
 	{
 		//processing=0;
                scanning_status = 1;
                gst_element_set_state (pipeline, GST_STATE_PLAYING);
 		scanning=true;

 	}
 }

void CameraCapture::new_buffer_added (GstAppSink *_appsink, gpointer user_data)
{
	if(cold_start)
	{
		//initialize appsink
		printf("\nAppsink initialized!\n");
		appsink=_appsink;
		gst_app_sink_set_drop(appsink,true);
		//turn off emit new_buffer_added
		gst_app_sink_set_emit_signals(appsink,false);
		gst_app_sink_set_max_buffers(appsink,1);
		cold_start=false;
		buffer_ready=true;
		scanning=true;
	}
}

void CameraCapture::refresh_buffer(){
	if(appsink!=NULL){
		//GstBuffer* buffer = gst_app_sink_pull_buffer(appsink);
		buffer = gst_app_sink_pull_buffer(appsink);
		buffer_ready=true;
		//printf("%d\n",count);
		if(count>100)
			count=0;
		count++;
	}
}

/* Callback that gets called whenever pipeline's message bus has
 * a message */
gboolean CameraCapture::bus_callback (GstBus * bus, GstMessage * message)
{
    gchar *message_str;
    GError *error;

    /* Report errors to the console */
    if (GST_MESSAGE_TYPE (message) == GST_MESSAGE_ERROR) {
        gst_message_parse_error (message, &error, &message_str);
        printf("GST error: %s\n", message_str);
        g_error ("GST error: %s\n", message_str);
        g_free (error);
        g_free (message_str);
    }

    /* Report warnings to the console */
    if (GST_MESSAGE_TYPE (message) == GST_MESSAGE_WARNING) {
        gst_message_parse_warning (message, &error, &message_str);
        printf("GST warning: %s\n", message_str);
        g_warning ("GST warning: %s\n", message_str);
        g_free (error);
        g_free (message_str);
    }

    if (GST_MESSAGE_TYPE (message) == GST_MESSAGE_APPLICATION)
    {
        fprintf(stdout, "Got a message from the GStreamer thread\n");
        printf("Got a message from the GStreamer thread\n");
        // unref/free this message
        gst_message_unref(message); // perhaps not needed....?
    }
    return TRUE;
}






