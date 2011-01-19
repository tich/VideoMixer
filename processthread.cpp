#include "processthread.h"

ProcessThread::ProcessThread(QObject *parent) :
    QThread(parent)
{
    oldbX = oldbY = oldyX = oldyY = 0;
    counter = 0;
    stopRequested = false;
    connect(parent, SIGNAL(process(QImage)), SLOT(processStuff(QImage)));
//    connect(bla, SIGNAL(renderedImage(QImage)),SLOT(picReceived(QImage)));
}
void ProcessThread::processStuff(QImage myImage)
{
    //image=myImage;

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
    }
}

void ProcessThread::startThread()
{
    isRunning = true;
    stopRequested = false;
    this->start();
}
void ProcessThread::stopThread()
{
    stopRequested = true;
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
        if(image.isNull())
            continue;
        pointer2++;
        if(pointer2 == 30000)
            pointer2 = 0;
        int width, height;
        width = image.width();
        height = image.height();
        double momentb10 = 0, momentb01 = 0, momentb00 = 0;
        double momenty10 = 0, momenty01 = 0, momenty00 = 0;
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
                    momentb10 += 255 * x;
                    momentb01 += 255 * y;
                    momentb00 += 255;
                }
                else if(hue <= 72 && hue >= 40 && sat >= 150 && value >= 150)
                {
                    momenty10 += 255 * x;
                    momenty01 += 255 * y;
                    momenty00 += 255;
                }
            }
        }
        if(momentb00 > 255*10)
        {
            oldbX = momentb10/momentb00;//((moment10/moment00)/width)*320;
            oldbY = momentb01/momentb00;//((moment01/moment00)/height)*240;
        }
        if(momenty00 > 255*10)
        {
            oldyX = momenty10/momenty00;
            oldyY = momenty01/momenty00;
        }
        mutex.lock();
        counter--;
        mutex.unlock();
        emit processFinished(oldbX, oldbY, oldyX, oldyY);
    }
    isRunning = false;
//    if(stopRequested)
//        return;
}
