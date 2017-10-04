#include "imagelabel.h"

#include <QMouseEvent>


void ImageLabel::mousePressEvent(QMouseEvent* ev)
{

    emit mousePositionInitial(ev->localPos().x(), ev->localPos().y());
}

void ImageLabel::mouseReleaseEvent(QMouseEvent* ev)
{
    emit mousePositionFinal(ev->localPos().x(), ev->localPos().y());
}

void ImageLabel::mouseMoveEvent(QMouseEvent* ev)
{
    emit mousePositionIntermediate(ev->localPos().x(), ev->localPos().y());
}


