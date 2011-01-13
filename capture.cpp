#include "capture.h"

CameraThread::CameraThread(QObject *parent) :
    QThread(parent)
{
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
    isRunning = false;
    this->terminate();
}

void CameraThread::run()
{
    CameraCapture *camera=new CameraCapture();

    if (!camera->initialize_pipeline (NULL, NULL))
            printf("Failed to initialize pipeline!\n");
    else
            camera->toggle_camera_stream();
    return;
}
