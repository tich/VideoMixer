# -------------------------------------------------
# Project created by QtCreator 2010-11-16T08:34:28
# -------------------------------------------------
QT += opengl \
    phonon# \
    #multimedia
LIBS += -lv4l2
TARGET = VideoMixer
TEMPLATE = app
SOURCES += main.cpp \
    mainwindow.cpp \
    capture.cpp \
    capturethread.cpp \
    processthread.cpp \
    cameracapture.cpp \
    glwidget.cpp \
    gstvideoplayer.cpp
HEADERS += mainwindow.h \
    capture.h \
    capturethread.h \
    processthread.h \
    glwidget.h \
    cameracapture.h \
    gstvideoplayer.h
FORMS += mainwindow.ui
OTHER_FILES += TODO.txt
LIBS += -L/usr/lib/ \
    -lGLEW \
    -lPocoFoundation \
    -lgstreamer-0.10 \
    -lgobject-2.0 \
    -lgmodule-2.0 \
    -lgthread-2.0 \
    -lxml2 \
    -lpthread \
    -lz \
    -lm \
    -lglib-2.0

CONFIG += link_pkgconfig

######################################################################
#Could not link gtk+-2.0 with pkgcongig (Conflict with QtGui library)
######################################################################

PKGCONFIG += gstreamer-plugins-base-0.10 \
                         gstreamer-interfaces-0.10 \
                         #gstreamer-0.10 dbus-1 \
                         gstreamer-app-0.10
                         #gstreamer-plugins-bad-0.10
INCLUDEPATH += . \
    /usr/include/ \
    /usr/include/gtk-2.0/ \
    /usr/include/gtk-2.0/gtk/ \
    /usr/lib/gtk-2.0/include \
    /usr/include/glib-2.0/ \
    /usr/include/glib-2.0/glib/ \
    /usr/include/glib-2.0/gio/ \
    /usr/include/glib-2.0/gobject/ \
    /usr/include/atk-1.0/ \
    /usr/include/cairo/ \
    /usr/include/pango-1.0/ \
    /usr/lib/glib-2.0/include \
    /usr/include/gstreamer-0.10 \
    /usr/lib/glib-2.0/include \
    /usr/include/libxml2 \
    /usr/include/GL
