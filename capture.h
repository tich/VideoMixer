#ifndef CAMERATHREAD_H
#define CAMERATHREAD_H

#include <QThread>
#include <QMutex>
#include <QTimer>
#include <QtGui/QWidget>
#include <qwidget.h>
#include "cameracapture.h"
#include "glwidget.h"

class CameraThread : public QThread
{
Q_OBJECT
public:
    explicit CameraThread(QObject *parent = 0);
    void startThread(QWidget *parent);
    void stopThread();
    QMutex mutex;
private:
    CameraCapture *camera;
    bool isRunning;
    bool first;
    int iwidth, iheight;
    QTimer *timer;
    QWidget *m_parent;

protected:
    void run();
private slots:
    void updateImage();
signals:
    void processMe(QImage);
};

#endif // CAMERATHREAD_H
