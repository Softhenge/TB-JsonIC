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

#include <QtWidgets>
#ifndef QT_NO_PRINTER
#include <QPrintDialog>
#endif

#include <QJsonObject>
#include <QJsonArray>
#include <QRectF>
#include <QPainter>
#include <QMessageBox>
#include <QFileInfo>

#include "imagelabel.h"
#include "imageviewer.h"

QString jsonFileName = QDir::currentPath() +"/input.json";

// inputImages is the expected input folder name
// if another directory is used, just rename inputImages to the directory name which will be used for training input
QString imageDir("inputImages");

ImageViewer::ImageViewer()
   : imageLabel(new ImageLabel)
   , scrollArea(new QScrollArea)
   , scaleFactor(1)
   , saveAreaButton(new QPushButton)
   , dropAreaButton(new QPushButton)
   , saveAndNextButton(new QPushButton)
   , nextButton(new QPushButton)
   , previousButton(new QPushButton)
   , flushJsonToFile(new QPushButton)
   , flushToFileDialog(new QMessageBox)
   , buttonsLayout(new QVBoxLayout)
{
    imageLabel->setBackgroundRole(QPalette::Base);
    imageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    imageLabel->setScaledContents(true);

    scrollArea->setBackgroundRole(QPalette::Dark);
    scrollArea->setWidget(imageLabel);
    scrollArea->setVisible(true);

    QHBoxLayout* mainLayout = new QHBoxLayout;
    mainLayout->insertWidget(0, scrollArea);
    mainLayout->insertLayout(1, buttonsLayout);
    QWidget* main = new QWidget;
    main->setLayout(mainLayout);
    setCentralWidget(main);

    createActions();

    resize(QGuiApplication::primaryScreen()->availableSize() * 3 / 5);

    bool res = connect(imageLabel, SIGNAL(mousePositionInitial(int, int)), this, SLOT(handleInitialCordinates(int, int)));
    Q_ASSERT(res);
    res = connect(imageLabel, SIGNAL(mousePositionFinal(int, int)), this, SLOT(handleFinalCordinates(int, int)));
    Q_ASSERT(res);
    res = connect(imageLabel, SIGNAL(mousePositionIntermediate(int, int)), this, SLOT(handleIntermediateCordinates(int, int)));
    Q_ASSERT(res);

    initButtonsArea();
    initJson();
}

bool ImageViewer::loadFile(const QString &fileName)
{
    QImageReader reader(fileName);
    reader.setAutoTransform(true);
    const QImage newImage = reader.read();
    if (newImage.isNull()) {
        QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
                                 tr("Cannot load %1: %2")
                                 .arg(QDir::toNativeSeparators(fileName), reader.errorString()));
        return false;
    }

    setImage(newImage);

    setWindowFilePath(fileName);

    const QString message = tr("Opened \"%1\", %2x%3, Depth: %4")
        .arg(QDir::toNativeSeparators(fileName)).arg(image.width()).arg(image.height()).arg(image.depth());
    statusBar()->showMessage(message);
    saveAreaButton->setEnabled(true);
    dropAreaButton->setEnabled(true);
    return true;
}

void ImageViewer::setImage(const QImage &newImage)
{
    image = newImage;
    imagePixmap = QPixmap::fromImage(image);
    imageLabel->setPixmap(imagePixmap);
    scaleFactor = 1.0;

    scrollArea->setVisible(true);
    //fitToWindowAct->setEnabled(true);
    updateActions();

    //if (!fitToWindowAct->isChecked())
        imageLabel->adjustSize();
}


static void initializeImageFileDialog(QFileDialog &dialog, QFileDialog::AcceptMode acceptMode)
{
    static bool firstDialog = true;

    if (firstDialog) {
        firstDialog = false;
        const QStringList picturesLocations = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);
        dialog.setDirectory(picturesLocations.isEmpty() ? QDir::currentPath() : picturesLocations.last());
    }

    QStringList mimeTypeFilters;
    const QByteArrayList supportedMimeTypes = acceptMode == QFileDialog::AcceptOpen
        ? QImageReader::supportedMimeTypes() : QImageWriter::supportedMimeTypes();
    foreach (const QByteArray &mimeTypeName, supportedMimeTypes)
        mimeTypeFilters.append(mimeTypeName);
    mimeTypeFilters.sort();
    dialog.setMimeTypeFilters(mimeTypeFilters);
    dialog.selectMimeTypeFilter("image/jpeg");
    if (acceptMode == QFileDialog::AcceptSave)
        dialog.setDefaultSuffix("jpg");
}

void ImageViewer::open()
{
    QFileDialog dialog(this, tr("Open File"));
    initializeImageFileDialog(dialog, QFileDialog::AcceptOpen);

    while (dialog.exec() == QDialog::Accepted && !loadFile(dialog.selectedFiles().first())) {}
    if (!dialog.selectedFiles().isEmpty()) {
        images.clear();
        currentImageIndex = 0;
        nextButton->setEnabled(false);
        saveAndNextButton->setEnabled(false);
        previousButton->setEnabled(false);
        fileName = dialog.selectedFiles().first();
    }
}

void ImageViewer::zoomIn()
{
    scaleImage(1.25);
}

void ImageViewer::zoomOut()
{
    scaleImage(0.8);
}

void ImageViewer::normalSize()
{
    imageLabel->adjustSize();
    scaleFactor = 1.0;
}

//void ImageViewer::fitToWindow()
//{
//    bool fitToWindow = fitToWindowAct->isChecked();
//    scrollArea->setWidgetResizable(fitToWindow);
//    imagePixmap = QPixmap::fromImage(image);
//    // TODO: scale factor is inacture now
//    scaleFactor = (double)imageLabel->size().height() / (double)imagePixmap.size().height();

//    if (!fitToWindow)
//        normalSize();
//    updateActions();
//}

void ImageViewer::about()
{
    //TODO: need to modify this
    QMessageBox* aboutMessageBox = new QMessageBox();
    QPixmap exportSuccess(":/icon/softhenge.png");
    aboutMessageBox->setIconPixmap(exportSuccess);
    aboutMessageBox->setAttribute(Qt::WA_DeleteOnClose, true);
    aboutMessageBox->setWindowTitle(tr("TensorBox Json Input Creator"));
    aboutMessageBox->setText(tr("<p>The purpose of this tool to simplify the process of input json file creation for Tensorbox."
                                "The application provides interface to load images and select the regions of interests "
                                "and save them into a json file in the expected format of Tensorbox.</p>"));
    aboutMessageBox->show();
}

void ImageViewer::createActions()
{
    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));

    QAction *openAct = fileMenu->addAction(tr("&Open..."), this, &ImageViewer::open);
    openAct->setShortcut(QKeySequence::Open);

    fileMenu->addAction(tr("&Open Folder..."), this, &ImageViewer::openFolder);

    fileMenu->addSeparator();

    QAction *exitAct = fileMenu->addAction(tr("E&xit"), this, &QWidget::close);
    exitAct->setShortcut(tr("Ctrl+Q"));

    QMenu *viewMenu = menuBar()->addMenu(tr("&View"));

    zoomInAct = viewMenu->addAction(tr("Zoom &In (25%)"), this, &ImageViewer::zoomIn);
    zoomInAct->setShortcut(QKeySequence::ZoomIn);
    zoomInAct->setEnabled(false);

    zoomOutAct = viewMenu->addAction(tr("Zoom &Out (25%)"), this, &ImageViewer::zoomOut);
    zoomOutAct->setShortcut(QKeySequence::ZoomOut);
    zoomOutAct->setEnabled(false);

    normalSizeAct = viewMenu->addAction(tr("&Normal Size"), this, &ImageViewer::normalSize);
    normalSizeAct->setShortcut(tr("Ctrl+S"));
    normalSizeAct->setEnabled(false);

    viewMenu->addSeparator();

//    fitToWindowAct = viewMenu->addAction(tr("&Fit to Window"), this, &ImageViewer::fitToWindow);
//    fitToWindowAct->setEnabled(false);
//    fitToWindowAct->setCheckable(true);
//    fitToWindowAct->setShortcut(tr("Ctrl+F"));

    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));

    helpMenu->addAction(tr("&About"), this, &ImageViewer::about);
    helpMenu->addAction(tr("About &Qt"), &QApplication::aboutQt);
}

void ImageViewer::updateActions()
{
    zoomInAct->setEnabled(true);
    zoomOutAct->setEnabled(true);
    normalSizeAct->setEnabled(true);
}

void ImageViewer::scaleImage(double factor)
{
    Q_ASSERT(imageLabel->pixmap());
    scaleFactor *= factor;
    imageLabel->resize(scaleFactor * imageLabel->pixmap()->size());

    adjustScrollBar(scrollArea->horizontalScrollBar(), factor);
    adjustScrollBar(scrollArea->verticalScrollBar(), factor);

    zoomInAct->setEnabled(scaleFactor < 3.0);
    zoomOutAct->setEnabled(scaleFactor > 0.333);
}

void ImageViewer::adjustScrollBar(QScrollBar *scrollBar, double factor)
{
    scrollBar->setValue(int(factor * scrollBar->value()
                            + ((factor - 1) * scrollBar->pageStep()/2)));
}

void ImageViewer::addWidgetToLayout(QWidget* layout, int alignment)
{
    buttonsLayout->insertWidget(currentItemIndex++, layout);
    if (alignment != 0) {
        Qt::AlignmentFlag align = (Qt::AlignmentFlag)alignment;
        buttonsLayout->setAlignment(layout, align);
    }
}


//************************************ Mostly Softhenge code is here
void ImageViewer::initButtonsArea()
{
    //Buttons area
    x1CoordianteLabel = new QLabel;
    addWidgetToLayout(x1CoordianteLabel, Qt::AlignTop);
    y1CoordianteLabel = new QLabel;
    addWidgetToLayout(y1CoordianteLabel, Qt::AlignTop);
    x2CoordianteLabel = new QLabel;
    addWidgetToLayout(x2CoordianteLabel, Qt::AlignTop);
    y2CoordianteLabel = new QLabel;
    addWidgetToLayout(y2CoordianteLabel, Qt::AlignTop);

    saveAreaButton->setText("Save Area");
    saveAreaButton->setEnabled(false);
    addWidgetToLayout(saveAreaButton);
    connect(saveAreaButton, SIGNAL(clicked()), this, SLOT(handleSaveClicked()));

    dropAreaButton->setText("Drop Area");
    dropAreaButton->setEnabled(false);
    addWidgetToLayout(dropAreaButton);
    connect(dropAreaButton, SIGNAL(clicked()), this, SLOT(handleDropAreaClicked()));

    saveAndNextButton->setText("Save, Next Image");
    saveAndNextButton->setEnabled(false);
    addWidgetToLayout(saveAndNextButton);
    connect(saveAndNextButton, SIGNAL(clicked()), this, SLOT(handleSaveAndNextClicked()));

    nextButton->setText("Next Image");
    nextButton->setEnabled(false);
    addWidgetToLayout(nextButton);
    connect(nextButton, SIGNAL(clicked()), this, SLOT(handleNextClicked()));

    previousButton->setText("Previous Image");
    previousButton->setEnabled(false);
    addWidgetToLayout(previousButton);
    connect(previousButton, SIGNAL(clicked()), this, SLOT(handlePreviousClicked()));

    flushJsonToFile->setText("Flush To File");
    flushJsonToFile->setEnabled(false);
    addWidgetToLayout(flushJsonToFile);
    flushToFileDialog->setStandardButtons(QMessageBox::Ok|QMessageBox::Cancel);
    flushToFileDialog->setText("Flush rectangles to the file");
    connect(flushJsonToFile, SIGNAL(clicked()), flushToFileDialog, SLOT(show()));
    connect(flushToFileDialog, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(handleFlush(QAbstractButton*)));
}

void ImageViewer::initJson()
{
    // load all jsons from the file to not lose append when saving
    QFile file(jsonFileName);
    if (file.exists() && file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString val = file.readAll();
        file.close();
        jsonDocToSave = QJsonDocument::fromJson(val.toUtf8());

        QJsonArray jsonArray = jsonDocToSave.array();
        for(QJsonArray::const_iterator it = jsonArray.constBegin();it != jsonArray.constEnd(); ++it) {
            QJsonObject jsonObject = (*it).toObject();
            // image name maps to rects
            QString fileName = jsonObject.value("image_path").toString();
            QJsonArray rects = jsonObject.value("rects").toArray();
            jsonItems.insert(fileName, rects);
        }
    }
}

void ImageViewer::handleFlush(QAbstractButton* clickedButton)
{
    QMessageBox::StandardButton button = flushToFileDialog->standardButton(clickedButton);
    if (button != QMessageBox::Ok) {
        return;
    }
    QJsonArray items;
    foreach (QString key, jsonItems.keys()) {
        QJsonObject item;
        item.insert("image_path", key);
        item.insert("rects", jsonItems.value(key));
        items.append(item);
    }
    QJsonDocument doc;
    doc.setArray(items);
    QFile saveFile(jsonFileName);
    if (saveFile.open(QIODevice::WriteOnly)){
        saveFile.write(doc.toJson());
    }
}

void ImageViewer::handleSaveClicked()
{
    if (cordX1 >= cordX2 || cordY1 >= cordY2) {
        QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
                                 tr("Empty or Incorrect Area selected"));
        return;
    }
    if (!x1CoordianteLabel->text().isEmpty() && !x2CoordianteLabel->text().isEmpty() &&
            !y1CoordianteLabel->text().isEmpty() && !y2CoordianteLabel->text().isEmpty()) {
        // save the rect
        insertRectangle();
        if (!flushJsonToFile->isEnabled()) {
            flushJsonToFile->setEnabled(true);
        }
    }
    handleDropAreaClicked();
}

void ImageViewer::handleDropAreaClicked()
{
    // clear all the data related to current rect
    x1CoordianteLabel->clear();
    x2CoordianteLabel->clear();
    y1CoordianteLabel->clear();
    y2CoordianteLabel->clear();
    imagePixmap = QPixmap::fromImage(image);
    imageLabel->setPixmap(imagePixmap);
}

void ImageViewer::handleInitialCordinates(int x, int y)
{
    cordX1 = x;
    cordY1 = y;
    x1CoordianteLabel->setText("x1: " + QString::number(x));
    y1CoordianteLabel->setText("y1: " + QString::number(y));
}

void ImageViewer::handleIntermediateCordinates(int x, int y)
{
    // create pixmap and draw a rectangle on it
    imagePixmap = QPixmap::fromImage(image);
    // if there's a scale need to normalize to current image view
    QRectF rectangle(cordX1 / scaleFactor, cordY1 / scaleFactor, (x - cordX1) / scaleFactor, (y - cordY1) / scaleFactor);
    QPainter painter(&imagePixmap);
    painter.setPen(QColor(Qt::green));
    painter.setBrush(Qt::Dense4Pattern);
    painter.drawRect(rectangle); // drawing code
    imageLabel->setPixmap(imagePixmap);
    // update final coordinate values
    handleFinalCordinates(x, y);
}

void ImageViewer::handleFinalCordinates(int x, int y)
{
    cordX2 = x;
    cordY2 = y;
    x2CoordianteLabel->setText("x2: " + QString::number(x));
    y2CoordianteLabel->setText("y2: " + QString::number(y));
}


void ImageViewer::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event);
    emit flushJsonToFile->clicked();
}


void ImageViewer::insertRectangle()
{
    QString name = imageDir + "/" + QFileInfo(fileName).fileName();
    QJsonArray rects;
    if (jsonItems.contains(name)){
        rects = jsonItems.value(name);
    }
    QJsonObject rectangle;
    rectangle.insert("x1", float(cordX1));
    rectangle.insert("x2", float(cordX2));
    rectangle.insert("y1", float(cordY1));
    rectangle.insert("y2", float(cordY2));
    rects.append(rectangle);
    jsonItems.insert(name, rects);
}

void ImageViewer::openFolder()
{
    QFileDialog dialog(this, tr("Open Folder"));
    const QStringList picturesLocations = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);
    QString dir = dialog.getExistingDirectory(this, tr("Open Folder"),
                                                    picturesLocations.isEmpty() ? QDir::currentPath() : picturesLocations.last(),
                                                    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (!dir.isEmpty() && QFileInfo(dir).isDir()) {
        QStringList filters;
        filters << "*.png" << "*.jpg" << "*.bmp" << "*.gif" << "*.tif" << "*.jpeg";
        QFileInfoList imagesList = QDir(dir).entryInfoList(filters, QDir::Files|QDir::NoDotAndDotDot);

        if (!images.isEmpty()) {
            images.clear();
        }
        foreach (const QFileInfo& imageInfo, imagesList) {
            images.push_back(imageInfo.absoluteFilePath());
        }
        currentImageIndex = -1;
        while (currentImageIndex < images.size()) {
            ++currentImageIndex;
            fileName = images[currentImageIndex];
            if (loadFile(fileName)) {
                break;
            }
        }
        nextButton->setEnabled(true);
        saveAndNextButton->setEnabled(true);
    }
}

void ImageViewer::handleSaveAndNextClicked()
{
    handleSaveClicked();
    handleNextClicked();
}

void ImageViewer::handleNextClicked()
{
    while (currentImageIndex < images.size()) {
        ++currentImageIndex;
        fileName = images[currentImageIndex];
        if (loadFile(fileName)) {
            break;
        }
    }
    if (currentImageIndex >= images.size() - 1) {
        nextButton->setEnabled(false);
        saveAndNextButton->setEnabled(false);
    }
    previousButton->setEnabled(true);
}
void ImageViewer::handlePreviousClicked()
{
    while (currentImageIndex > 0) {
        --currentImageIndex;
        fileName = images[currentImageIndex];
        if (loadFile(fileName)) {
            break;
        }
    }
    if (currentImageIndex <= 0) {
        previousButton->setEnabled(false);
    }
    nextButton->setEnabled(true);
    saveAndNextButton->setEnabled(true);
}
