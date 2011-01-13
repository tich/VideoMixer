#ifndef CAPTURETHREAD_H
#define CAPTURETHREAD_H

#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include "libv4l2.h"
#include <linux/videodev2.h>
#include "libv4lconvert.h"
#include <QApplication>
#include <QDataStream>
#include <QString>
#include <QDebug>
#include <QBuffer>
#include <QImage>
#include <typeinfo>

#include <iostream>
#include <QFile>
//#include "QVideoEncoder.h"
//#include "VideoEncoder.h"

#define CLEAR(x) memset(&(x), 0, sizeof(x))

struct buffer {
        void   *start;
        size_t length;
};

class CaptureThread : public QThread
{
    Q_OBJECT

public:
    CaptureThread(QObject *parent = 0);
     ~CaptureThread();
    bool devam;

    struct v4l2_format              fmt;
    struct v4l2_buffer              buf;
    struct v4l2_requestbuffers      req;
    enum v4l2_buf_type              type;
    fd_set                          fds;
    struct timeval                  tv;
    int                             r, fd;
    unsigned int                    n_buffers;
    char                            *dev_name;
    char                            out_name[256];
    FILE                            *fout;


    struct buffer                   *buffers;

    void stopUlan();
    void startUlan();
    void setDeviceName(QString);
    QMutex mutex;
protected:
    void run();
signals:
    void renderedImage(const QImage &image);

private:
    QWaitCondition condition;
    QString deviceName;
};

#endif // CAPTURETHREAD_H
