#include "processthread.h"

ProcessThread::ProcessThread(QObject *parent) :
    QThread(parent)
{
    oldX = oldY = 0;
    counter = 0;
    connect(parent, SIGNAL(process(QImage)), SLOT(processStuff(QImage)));
//    connect(bla, SIGNAL(renderedImage(QImage)),SLOT(picReceived(QImage)));
}
void ProcessThread::processStuff(QImage myImage)
{
    //image=myImage;
    /*
    mutex.lock();
    images[pointer1 % 30] = myImage.copy();
    counter++;
    mutex.unlock();
    pointer1++;
    if(pointer1 == 30000)
        pointer1 = 0;
    if(!isRunning)
    {
        startThread();
    }*/
}

void ProcessThread::startThread()
{
    isRunning = true;
    this->start();
}
void ProcessThread::stopThread()
{
    isRunning = false;
    this->terminate();
}

void ProcessThread::run()
{
    while(isRunning && counter != 0)
    {
        mutex.lock();
        image = images[pointer2 % 30].copy();
        mutex.unlock();
        pointer2++;
        if(pointer2 == 30000)
            pointer2 = 0;
        int width, height;
        width = image.width();
        height = image.height();
        double moment10 = 0, moment01 = 0, moment00 = 0;
        for(int x = 0 ; x < width;x++)
        {
            for(int y = 0 ; y < height;y++)
            {
                bleh.setRgb(image.pixel(x,y));
                int hue = bleh.hsvHue();
                int sat = bleh.hsvSaturation();
                int value = bleh.value();
                if(hue <= 267 && hue >= 167 && sat >= 150 && value >= 150)  //Yellow: between 72 and 53  Blue: between 267 and 167
                {
                    moment10 += 255 * x;
                    moment01 += 255 * y;
                    moment00 += 255;
                }
            }
        }
        if(moment00 > 255*10)
        {
            oldX = moment10/moment00;//((moment10/moment00)/width)*320;
            oldY = moment01/moment00;//((moment01/moment00)/height)*240;
        }
        mutex.lock();
        counter--;
        mutex.unlock();
        emit processFinished(oldX, oldY);
    }
    isRunning = false;
}
