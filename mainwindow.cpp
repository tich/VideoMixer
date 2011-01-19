#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "capturethread.h"
#include "cameracapture.h"
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
    m_videoWidget->setGeometry(0,0,ui->widget->width(),ui->widget->height());
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
    timer = new QTimer(this);
    //connect(timer, SIGNAL(timeout()), this, SLOT(update()));
    /*
    bla = new CaptureThread(this);
    proc = new ProcessThread(this);
    connect(proc, SIGNAL(processFinished(int,int)), SLOT(getCoord(int,int)));
    setAttribute(Qt::WA_PaintOnScreen,true);
    */

    bleh = new CameraThread(this);
    bleh->startThread(NULL);
    glwidget = new GLWidget(ui->widget_2);
    proc = new ProcessThread(this);
    connect(proc, SIGNAL(processFinished(int,int,int,int)), SLOT(getCoord(int,int,int,int)));
    connect(bleh, SIGNAL(processMe(QImage)), proc, SLOT(processStuff(QImage)));
    timer2 = new QTimer(this);
    timer2->setInterval(20);//40=25Hz
    connect(timer2, SIGNAL(timeout()), this, SLOT(update()));
    glwidget->setGeometry(0,0,ui->widget_2->width(),ui->widget_2->height());
    //timer2->start();

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

    glwidget->repaint();
    glwidget->updateGL();
    glwidget->paintEngine();
    if(glwidget->updatesEnabled() && glwidget->isVisible())
        ui->label_3->setText("bla " + QString::number(count));
    else
        ui->label_3->setText("bloup" + QString::number(count++));
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
    ui->label->setGeometry(110+posyX, 10+posyY, ui->label->geometry().width(), ui->label->geometry().height());
    ui->widget->setGeometry(370+posbX, 10+posbY, ui->widget->geometry().width(), ui->widget->geometry().height());
}

void MainWindow::setPicture(QImage Image)
{
    //myImage = Image.mirrored(true,false);
    //emit process(Image.mirrored(true,false));
    QPixmap troll = QPixmap::fromImage(Image);
    troll = troll.scaled(320, 240,Qt::IgnoreAspectRatio,Qt::FastTransformation);
    ui->label->setPixmap(troll);
}

MainWindow::~MainWindow()
{
    //bla->stopUlan();
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
    ui->label->setGeometry(ui->label->geometry().x(), 10+(100-value), ui->label->geometry().width(), ui->label->geometry().height());
    ui->widget->setGeometry(ui->widget->geometry().x(), 10+(100-value), ui->widget->geometry().width(), ui->widget->geometry().height());
}
