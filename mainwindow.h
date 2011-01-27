#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtGui/QGraphicsView>
#include <phonon/mediaobject.h>
#include <phonon/videowidget.h>
#include <phonon/audiodataoutput.h>
#include <QTimer>
#include "capturethread.h"
#include "processthread.h"
#include "capture.h"
#include "gstvideoplayer.h"
#include "glwidget.h"

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::MainWindow *ui;
    QGraphicsScene *scene;
    QTimer *timer;
    QTimer *timer2;
    Phonon::MediaObject *m_MediaObject;
    Phonon::VideoWidget *m_videoWidget;
    QPixmap image[10];
    QImage myImage;
    CaptureThread *bla;
    gstVideoPlayer *vid;
    CameraThread *bleh;
    ProcessThread *proc;
    GLWidget * glwidget, *glwidget2;
    int count;
    int pointer;
    bool begin;
private slots:
    void on_verticalSlider_valueChanged(int value);
    void update();
    void setPicture(QImage);
    void getCoord(int,int,int,int);
signals:
    void process(QImage);
};

#endif // MAINWINDOW_H
