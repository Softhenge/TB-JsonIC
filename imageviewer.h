/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** Copyright (C) 2017 Softhenge LLC
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef IMAGEVIEWER_H
#define IMAGEVIEWER_H

#include <QMainWindow>
#include <QImage>
#ifndef QT_NO_PRINTER
#include <QPrinter>
#include <QJsonDocument>
#include <QJsonArray>
#include <QMap>
#include <QVariantList>
#include <QString>
#endif

class QAction;
class QLabel;
class QMenu;
class QScrollArea;
class QScrollBar;
class QPushButton;
class QVBoxLayout;
class QAbstractButton;
class QMessageBox;

class ImageLabel;

class ImageViewer : public QMainWindow
{
    Q_OBJECT

public:
    ImageViewer();
    bool loadFile(const QString &);

private slots:
    void open();
    void openFolder();
    void zoomIn();
    void zoomOut();
    void normalSize();
    //void fitToWindow();
    void about();
    void handleInitialCordinates(int x, int y);
    void handleIntermediateCordinates(int x, int y);
    void handleFinalCordinates(int x, int y);
    void handleSaveClicked();
    void handleFlush(QAbstractButton *clickedButton);
    void handleDropAreaClicked();
    void handleNextClicked();
    void handlePreviousClicked();
    void handleSaveAndNextClicked();

    void addWidgetToLayout(QWidget *layout, int alignment = 0);

    void closeEvent(QCloseEvent *event);

private:
    void initButtonsArea();
    void initJson();

private:
    void createActions();
    void createMenus();
    void updateActions();
    bool saveFile(const QString &fileName);
    void setImage(const QImage &newImage);
    void scaleImage(double factor);
    void adjustScrollBar(QScrollBar *scrollBar, double factor);
    void insertRectangle();


    QImage image;
    ImageLabel *imageLabel;
    QScrollArea *scrollArea;
    double scaleFactor;


    //Buttons area
    QPushButton *saveAreaButton;
    QPushButton *dropAreaButton;
    QPushButton *saveAndNextButton;
    QPushButton *nextButton;
    QPushButton *previousButton;
    QPushButton *flushJsonToFile;
    QMessageBox *flushToFileDialog;

    QVBoxLayout* buttonsLayout;

    int cordX1;
    int cordY1;
    int cordX2;
    int cordY2;

    QLabel* x1CoordianteLabel;
    QLabel* y1CoordianteLabel;
    QLabel* x2CoordianteLabel;
    QLabel* y2CoordianteLabel;
    QPixmap imagePixmap;

    QJsonDocument jsonDocToSave;
    QMap<QString, QJsonArray> jsonItems;

    QString fileName;
    QVector<QString> images;
    int currentImageIndex = 0;

#ifndef QT_NO_PRINTER
    QPrinter printer;
#endif

    QAction *zoomInAct;
    QAction *zoomOutAct;
    QAction *normalSizeAct;
    QAction *fitToWindowAct;

    int currentItemIndex = 0;
};

#endif
