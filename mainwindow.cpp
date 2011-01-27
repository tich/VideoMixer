#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "capturethread.h"
#include "cameracapture.h"
#include "gstvideoplayer.h"
#include "glwidget.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    //setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(this);
    // this code is inside a class that inherits a QWidget
  /*
    m_MediaObject = new Phonon::MediaObject(this);
    m_MediaObject->clearQueue ();
    m_MediaObject->enqueue(Phonon::MediaSource("/home/tich/Desktop/vid.flv")); // or setCurrentSource
    m_MediaObject->play();
*/
    m_videoWidget = new Phonon::VideoWidget(ui->widget);
    //m_videoWidget->setGeometry(0,0,ui->widget->width(),ui->widget->height());
    //Phonon::createPath(m_MediaObject, m_videoWidget);
    /*
    m_videoWidget = new Phonon::VideoWidget(this);
    m_videoWidget->setGeometry(500,500,500,500);
    Phonon::createPath(m_MediaObject, m_videoWidget);
     */
    // if you want to grab the entire window
    //QPixmap ecran = QPixmap::grabWindow(QApplication::desktop()->winId());
    //scene = new QGraphicsScene;
    count = pointer = 0;
    begin = false;
    //QPixmap image = QPixmap::grabWindow(ui->widget->winId());
    //scene->addPixmap(image);
    //timer = new QTimer(this);
    //connect(timer, SIGNAL(timeout()), this, SLOT(update()));
    /*
    bla = new CaptureThread(this);
    proc = new ProcessThread(this);
    connect(proc, SIGNAL(processFinished(int,int)), SLOT(getCoord(int,int)));
    setAttribute(Qt::WA_PaintOnScreen,true);
    */

    bleh = new CameraThread(this);
    bleh->startThread(NULL);
    proc = new ProcessThread(this);
    connect(proc, SIGNAL(processFinished(int,int,int,int)), SLOT(getCoord(int,int,int,int)));
    connect(bleh, SIGNAL(processMe(QImage)), proc, SLOT(processStuff(QImage)));

    vid =new gstVideoPlayer();
    QStringList Files;
    Files.append("/home/tich/Desktop/IMG_1483.MOV");
    Files.append("/home/tich/Desktop/vid.flv");
    if (!vid->initialize_pipeline (Files))
    {
        printf("Failed to initialize video pipeline!\n");
        return;
    }
    glwidget = glwidget2 = 0;
    timer2 = new QTimer(this);
    timer2->setInterval(40);//40=25Hz
    connect(timer2, SIGNAL(timeout()), this, SLOT(update()));
    timer2->start();
    /*
    connect(bla, SIGNAL(renderedImage(QImage)),SLOT(setPicture(QImage)));
    bla->setDeviceName("/dev/video0");
    bla->startUlan();
    proc->startThread();
    */
    //timer->start(100);
}

void MainWindow::update()
{
    if(gstVideoPlayer::scanning && glwidget==NULL && glwidget2 ==NULL)
    {
        vid->toggle_play_state(0, 1);
        vid->toggle_play_state(1, 1);
        printf("Toggle play state\n");
        glwidget = new GLWidget(ui->widget_2,0);
        glwidget2 = new GLWidget(ui->widget,1);
        glwidget->setGeometry(0,0,ui->widget_2->width(),ui->widget_2->height());
        glwidget2->setGeometry(0,0,ui->widget->width(),ui->widget->height());
        glwidget2->setVisible(true);
        glwidget->setVisible(true);
    }
    else if(glwidget != NULL && glwidget2 != NULL && gstVideoPlayer::scanning)
    {
        glwidget->repaint();
        glwidget->updateGL();
        glwidget->paintEngine();
        glwidget2->repaint();
        glwidget2->updateGL();
        glwidget2->paintEngine();
        if(glwidget->updatesEnabled() && glwidget->isVisible())
            ui->label_3->setText("bla " + QString::number(count));
        else
            ui->label_3->setText("bloup" + QString::number(count++));
    }
    /*
    image[count % 10] = QPixmap::grabWindow(ui->widget->winId());
    count++;
    //QPixmap image = QPixmap::grabWindow(ui->widget->winId());
    if(count == 9)
        begin = true;
    if(begin)
    {
       ui->label->setPixmap(image[pointer % 10]);
       pointer++;
    }*/
/*
        QPixmap troll = QPixmap::fromImage(thresh);
        troll = troll.scaled(320, 240,Qt::IgnoreAspectRatio,Qt::FastTransformation);
        ui->label_2->setPixmap(troll);
    }*/
}
void MainWindow::getCoord(int posbX, int posbY, int posyX, int posyY)
{
    //ui->label_3->setText(QString::number(posY));
    ui->widget->setGeometry(110+posyX, 10+posyY, ui->widget->geometry().width(), ui->widget->geometry().height());
    ui->widget_2->setGeometry(370+posbX, 10+posbY, ui->widget_2->geometry().width(), ui->widget_2->geometry().height());
}

void MainWindow::setPicture(QImage Image)
{
    //myImage = Image.mirrored(true,false);
    //emit process(Image.mirrored(true,false));
    //QPixmap troll = QPixmap::fromImage(Image);
    //troll = troll.scaled(320, 240,Qt::IgnoreAspectRatio,Qt::FastTransformation);
    //ui->label->setPixmap(troll);
}

MainWindow::~MainWindow()
{
    printf("Terminating...\n");
    if(bleh)
    {
        bleh->stopThread();
        delete bleh;
    }
    if(proc)
    {
        proc->stopThread();
        delete proc;
    }
    if(timer2)
    {
        timer2->stop();
        delete timer2;
    }
    if(glwidget)
        delete glwidget;
    if(glwidget2)
        delete glwidget2;
    if(vid)
        delete vid;
    delete ui;
}

void MainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void MainWindow::on_verticalSlider_valueChanged(int value)
{
    ui->widget->setGeometry(ui->widget->geometry().x(), 10+(100-value), ui->widget->geometry().width(), ui->widget->geometry().height());
}
