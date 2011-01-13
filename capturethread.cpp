#include "capturethread.h"

static void xioctl(int fh, int request, void *arg)
{
        int r;

        do {
                r = v4l2_ioctl(fh, request, arg);
        } while (r == -1 && ((errno == EINTR) || (errno == EAGAIN)));

        if (r == -1) {
                fprintf(stderr, "error %d, %s\n", errno, strerror(errno));
                return;
        }
}

CaptureThread::CaptureThread(QObject *parent) : QThread(parent)
{
    devam=false;
    fd = -1;
}

void CaptureThread::run(){
    //do real stuff
    fd = -1;
    dev_name = (char*)"/dev/video0";

    if(!deviceName.isEmpty()){
        qDebug()<<"dev:"<<deviceName.toAscii().constData();
        dev_name=(char*)deviceName.toAscii().constData();
    }

    fd = v4l2_open(dev_name, O_RDWR | O_NONBLOCK, 0);
    if (fd < 0) {
           qDebug("Cannot open device");
           //exit(EXIT_FAILURE);
           quit();
           return;
    }


    static struct v4lconvert_data *v4lconvert_data;
    static struct v4l2_format src_fmt;
    static unsigned char *dst_buf;

    CLEAR(fmt);
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width       = 640;
    fmt.fmt.pix.height      = 480;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB24;
    fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;
    xioctl(fd, VIDIOC_S_FMT, &fmt);
    if (fmt.fmt.pix.pixelformat != V4L2_PIX_FMT_RGB24) {
           printf("Libv4l didn't accept RGB24 format. Can't proceed.\n");
           //exit(EXIT_FAILURE);
           return;
    }
    if ((fmt.fmt.pix.width != 640) || (fmt.fmt.pix.height != 480))
           printf("Warning: driver is sending image at %dx%d\n",
                   fmt.fmt.pix.width, fmt.fmt.pix.height);



/*std::string tm1="deneme";
std::string tm2="mp4";
    videoEncoder->InitFile(tm1,tm2);*/

    v4lconvert_data = v4lconvert_create(fd);
    if (v4lconvert_data == NULL)
        qDebug("v4lconvert_create");
    if (v4lconvert_try_format(v4lconvert_data, &fmt, &src_fmt) != 0)
        qDebug("v4lconvert_try_format");
    xioctl(fd, VIDIOC_S_FMT, &src_fmt);
    dst_buf = (unsigned char*)malloc(fmt.fmt.pix.sizeimage);

    CLEAR(req);
    req.count = 2;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;
    xioctl(fd, VIDIOC_REQBUFS, &req);

    buffers = (buffer*)calloc(req.count, sizeof(*buffers));
    for (n_buffers = 0; n_buffers < req.count; ++n_buffers) {
           CLEAR(buf);

           buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
           buf.memory      = V4L2_MEMORY_MMAP;
           buf.index       = n_buffers;

           xioctl(fd, VIDIOC_QUERYBUF, &buf);

           buffers[n_buffers].length = buf.length;
           buffers[n_buffers].start = v4l2_mmap(NULL, buf.length,
                         PROT_READ | PROT_WRITE, MAP_SHARED,
                         fd, buf.m.offset);

           if (MAP_FAILED == buffers[n_buffers].start) {
                   qDebug("mmap");
                   //exit(EXIT_FAILURE);
                   return;
           }
    }

    for (unsigned int i = 0; i < n_buffers; ++i) {
           CLEAR(buf);
           buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
           buf.memory = V4L2_MEMORY_MMAP;
           buf.index = i;
           xioctl(fd, VIDIOC_QBUF, &buf);
    }
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    xioctl(fd, VIDIOC_STREAMON, &type);
    qDebug()<<"buf.timecode.type:"<< buf.timecode.type;

    int di=0;
    char header [50];
    sprintf(header,"P6\n%d %d 255\n",fmt.fmt.pix.width,fmt.fmt.pix.height);
    devam=true;
    while(devam){
        /* bu döngü datanın birikmesini sağlıyor */
        do {
                if(fd < 0)
                    return;
                FD_ZERO(&fds);
                FD_SET(fd, &fds);

                /* Timeout. */
                tv.tv_sec = 2;
                tv.tv_usec = 0;

                r = select(fd + 1, &fds, NULL, NULL, &tv);
        } while ((r == -1 && (errno = EINTR)));
        if (r == -1) {
                qDebug("select");
                //exit(1) ;
                return;
        }

        CLEAR(buf);
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        xioctl(fd, VIDIOC_DQBUF, &buf);
            
        if (v4lconvert_convert(v4lconvert_data,
                                &src_fmt,
                                &fmt,
                                (unsigned char*)buffers[buf.index].start, buf.bytesused,
                                dst_buf, fmt.fmt.pix.sizeimage) < 0) {
                if (errno != EAGAIN)
                        qDebug("v4l_convert");
        }

        unsigned char* asil=(unsigned char*)malloc(fmt.fmt.pix.sizeimage+qstrlen(header));
        memmove(asil, dst_buf, fmt.fmt.pix.sizeimage);
        memmove(asil+qstrlen(header), asil, fmt.fmt.pix.sizeimage);
        memcpy(asil,header,qstrlen(header));

        QImage *qq=new QImage();
        if(qq->loadFromData(asil,fmt.fmt.pix.sizeimage+qstrlen(header),"PPM")){
            emit renderedImage(*qq);
/*            if(mutex.tryLock()  && devam)
            {
                image = *qq;//emit renderedImage(*qq);
                mutex.unlock();
            }*/

        }
        free(asil);
        delete qq;

        xioctl(fd, VIDIOC_QBUF, &buf);
        di++;
    }
    mutex.unlock();
    qDebug()<<"buf.timecode.type:"<< buf.timecode.type;
    qDebug()<<"totalFrameCount:"<<di;

    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    xioctl(fd, VIDIOC_STREAMOFF, &type);
    for (unsigned int i = 0; i < n_buffers; ++i)
           v4l2_munmap(buffers[i].start, buffers[i].length);

    v4l2_close(fd);

}
CaptureThread::~CaptureThread()
{

    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    xioctl(fd, VIDIOC_STREAMOFF, &type);
    for (unsigned int i = 0; i < n_buffers; ++i)
           v4l2_munmap(buffers[i].start, buffers[i].length);

    v4l2_close(fd);

    fd = -1;
    condition.wakeOne();

    wait();
}

void CaptureThread::stopUlan()
{
    try{
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    xioctl(fd, VIDIOC_STREAMOFF, &type);
    /*for (int i = 0; i < n_buffers; ++i)
           v4l2_munmap(buffers[i].start, buffers[i].length);*/

        v4l2_close(fd);
    }catch(...){}
    fd = -1;
    mutex.unlock();
}

void CaptureThread::startUlan()
{
    qDebug()<<"start oluo";
    this->start();
}
/*
void CaptureThread::onStopCapture()
{
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    xioctl(fd, VIDIOC_STREAMOFF, &type);
    for (int i = 0; i < n_buffers; ++i)
           v4l2_munmap(buffers[i].start, buffers[i].length);
    v4l2_close(fd);
}*/



void CaptureThread::setDeviceName(QString fileN) {
    deviceName=fileN;
}
