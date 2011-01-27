#include "capture.h"

CameraThread::~CameraThread()
{
    if(camera)
        delete camera;
}

CameraThread::CameraThread(QObject *parent) :
    QThread(parent)
{
    timer = new QTimer(parent);
    isRunning = false;
}

void CameraThread::startThread(QWidget *parent)
{
    isRunning = true;
    m_parent = parent;
    this->start();
}
void CameraThread::stopThread()
{
    printf("Stopping camera thread.\n");
    isRunning = false;
    this->terminate();
}

void CameraThread::run()
{
    camera =new CameraCapture();
    first = true;
    if (!camera->initialize_pipeline (NULL, NULL))
    {
        printf("Failed to initialize pipeline!\n");
        return;
    }
    else
        camera->toggle_camera_stream();
    timer->setInterval(20);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateImage()));
    timer->start();
    return;
}
void CameraThread::updateImage()
{
    if(!isRunning)
    {
        timer->stop();
        return;
    }
    if(CameraCapture::scanning)
    {
        CameraCapture::refresh_buffer(); //manually call appsink, not through the callback function
        if(CameraCapture::buffer!=NULL)
        {
            unsigned char* data=(unsigned char *) GST_BUFFER_DATA (CameraCapture::buffer);
            if(first)
            {
                QString bla(gst_caps_to_string(CameraCapture::buffer->caps));
                QString b = bla.remove(0, bla.indexOf("width=(int)")+11);
                QString width = b.left(b.indexOf(", height"));
                iwidth = width.toInt();
                b = bla.remove(0, bla.indexOf("height=(int)")+12);
                QString height = b.left(b.indexOf(", framerate"));
                iheight = height.toInt();
                first = false;
            }
            QImage image = QImage(data, iwidth, iheight, QImage::Format_RGB888).mirrored(true, false);
            emit processMe(image);
            gst_buffer_unref(CameraCapture::buffer);
        }
    }
}
