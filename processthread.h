#ifndef PROCESSTHREAD_H
#define PROCESSTHREAD_H

#include <QThread>
#include <QMutex>
#include <QImage>
#include <QColor>
#include "capturethread.h"

class ProcessThread : public QThread
{
Q_OBJECT
public:
    explicit ProcessThread(QObject *parent = 0);
    void startThread();
    void stopThread();
    QMutex mutex;

private:
    bool isRunning;
    QImage images[30];
    QImage image;
    QColor bleh;
    int pointer1;
    int pointer2;
    int oldbX, oldbY;
    int oldyX, oldyY;
    int counter;
    bool stopRequested;
protected:
    void run();
signals:
    void processFinished(int, int, int, int);
public slots:
    //void picReceived(QImage);
    void processStuff(QImage);

};

#endif // PROCESSTHREAD_H
