#-------------------------------------------------
#
# Project created by QtCreator 2016-10-25T19:41:20
#
#-------------------------------------------------
# Project updated by Softhenge

DESTDIR = ../bin

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
qtHaveModule(printsupport): QT += printsupport

QT += core

TARGET = TB-JsonIC
TEMPLATE = app


HEADERS       = imageviewer.h \
                imagelabel.h
SOURCES       = imageviewer.cpp \
                main.cpp \
                imagelabel.cpp

FORMS    += mainwindow.ui


# install
target.path = $$[QT_INSTALL_EXAMPLES]/widgets/widgets/imageviewer
INSTALLS += target

wince {
   DEPLOYMENT_PLUGIN += qjpeg qgif
}

RESOURCES += \
    resource.qrc
