#include "gstvideoplayer.h"

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


bool gstVideoPlayer::buffer_ready;
bool gstVideoPlayer::cold_start;
GstAppSink **gstVideoPlayer::appsink;
GstBuffer **gstVideoPlayer::buffer;
bool gstVideoPlayer::scanning=false;
int gstVideoPlayer::length=0;

gstVideoPlayer::gstVideoPlayer()
{
        scanning_status = 0;
}


gboolean gstVideoPlayer::initialize_pipeline(QStringList Files)
{

    GstElement *video_src, *decode_bin, *csp_filter, *image_sink, *queue;
    GstCaps *caps;
    GstBus *bus;

    //initialize flags
    buffer_ready=false;
    cold_start=true;

    /* Initialize Gstreamer */
    gst_init(NULL, NULL);

    /* Create pipeline and attach a callback to it's message bus */
    length = Files.length();
    pipelines = new GstElement*[Files.length()];
    appsink = new GstAppSink*[Files.length()];
    for(int i =0; i<length;i++)
        appsink[i] = NULL;
    buffer = new GstBuffer*[Files.length()];
    printf("length: %d\n", Files.length());

    for(int i=0; i<length;i++)
    {
        QString temp = Files.takeAt(0);
        printf("Loading %s\n", temp.toStdString().c_str());
        //QString templol = temp.append(QString::number(i));
        QString name = "bla";
        pipeline = gst_pipeline_new(name.append(QString::number(i)).toStdString().c_str());

        /******** Is this needed? *********/
        Q_ASSERT(pipeline);
        gst_object_ref (GST_OBJECT (pipeline)); //Take ownership
        gst_object_sink (GST_OBJECT (pipeline));
        /*********************************/

        bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));
        gst_bus_add_watch(bus, (GstBusFunc)bus_callback, NULL);
        gst_object_unref(GST_OBJECT(bus));

        /* Create elements */

        /* Camera video stream comes from a Video4Linux driver */
        video_src = gst_element_factory_make("filesrc", "file_src");
        g_object_set(G_OBJECT(video_src), "location", temp.toStdString().c_str(), NULL);

        decode_bin = gst_element_factory_make("decodebin2", "decode_bin");
        queue = gst_element_factory_make("queue", "queue_stuff");
        /* Colorspace filter is needed to make sure that sinks understands the stream coming from the camera */
        csp_filter = gst_element_factory_make("ffmpegcolorspace", "csp_filter");

        /* A dummy sink for the image stream. Goes to bitheaven AppSink*/
        image_sink = gst_element_factory_make("appsink", "image_sink");

        /* Check that elements are correctly initialized */
        if(!(pipeline && video_src && csp_filter && decode_bin && image_sink && queue))
        {
                g_critical("Couldn't create pipeline elements");
                return FALSE;
        }

        /* Add elements to the pipeline. This has to be done prior to linking them */
        gst_bin_add_many(GST_BIN(pipeline), video_src, decode_bin, queue, csp_filter,image_sink,NULL);

        /* Link the camera source and colorspace filter using capabilitie specified */
        if(!gst_element_link_filtered(video_src, decode_bin,NULL))
        {
            printf("Couldn't link video_src\n");
            return FALSE;
        }
        g_signal_connect(decode_bin, "new-decoded-pad", G_CALLBACK (decodebin_new_decoded_pad_cb), queue);
        //gst_caps_unref(caps);
        /*if(!gst_element_link_filtered(decode_bin, queue, NULL))
        {
            printf("Couldn't link decode_bin\n");
            return FALSE;
        }*/
        if(!gst_element_link(queue, csp_filter))
        {
            printf("Couldn't link queue\n");
            return FALSE;
        }
        caps = gst_caps_from_string("video/x-raw-rgb");
        /* Link the image-branch of the pipeline.*/
        if(!gst_element_link_filtered(csp_filter,image_sink, caps))
        {
            printf("Couldn't link stuff\n");
            return FALSE;
        }
        gst_caps_unref(caps);

        /* Set image sink to emit handoff-signal before throwing away  it's buffer */
        g_object_set (G_OBJECT (image_sink), "emit-signals", TRUE, NULL);
        int *index = new int;
        *index = i;
        g_signal_connect (image_sink, "new-buffer", G_CALLBACK (new_buffer_added), index);
        pipelines[i] = pipeline;
        gst_element_set_state(pipelines[i], GST_STATE_PLAYING);
    }
        return TRUE;
}
void gstVideoPlayer::decodebin_new_decoded_pad_cb(GstElement *decodebin, GstPad *pad, gboolean last, gpointer user_data)
{
    GstPad *queue_pad;
    GstElement* queue = (GstElement*)user_data;
    /**
     * The last discovered pad will always be the one hooked up to
     * the sink.
     **/
    GstCaps *caps;
    GstStructure *str;

    /* check media type */
    caps = gst_pad_get_caps (pad);
    str = gst_caps_get_structure (caps, 0);
    const gchar *tex = gst_structure_get_name(str);

    if(g_strrstr(tex,"video"))
    {
        printf("New video!\n");
        queue_pad = gst_element_get_pad (queue, "sink");
        gst_pad_link (pad, queue_pad);
    }
    else if(g_strrstr(tex,"audio"))
    {
        printf("New audio!\n");
    }
}

void gstVideoPlayer::toggle_play_state(int video_stream, int play)
 {
        if (!play) {
                // we therefore want to stop the scan
                scanning_status = 0;
                gst_element_set_state (pipelines[video_stream], GST_STATE_PAUSED); //READY
                scanning=false;
        }
        else
        {
                scanning_status = 1;
                gst_element_set_state (pipelines[video_stream], GST_STATE_PLAYING);
                scanning=true;

        }
 }

void gstVideoPlayer::new_buffer_added (GstAppSink *_appsink, gpointer user_data)
{
        if(cold_start)
        {
                //initialize appsink
                int *index = (int*)user_data;
                printf("\nAppsink initialized! %d\n", *index);
                appsink[*index]=_appsink;
                gst_app_sink_set_drop(appsink[*index],true);
                //turn off emit new_buffer_added
                gst_app_sink_set_emit_signals(appsink[*index],false);
                gst_app_sink_set_max_buffers(appsink[*index],1);
                delete index;
                cold_start=false;
                buffer_ready=true;
                scanning=true;
                for(int i=0;i<length;i++)
                {
                    if(appsink[i] == NULL)
                    {
                        printf("Still null\n");
                        cold_start=true;
                        buffer_ready=false;
                        scanning=false;
                    }
                }
        }
}

void gstVideoPlayer::refresh_buffer(int video_stream){
        if(appsink!=NULL){
                buffer[video_stream] = gst_app_sink_pull_buffer(appsink[video_stream]);
                buffer_ready=true;
        };
}

/* Callback that gets called whenever pipeline's message bus has
 * a message */
gboolean gstVideoPlayer::bus_callback (GstBus * bus, GstMessage * message)
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
