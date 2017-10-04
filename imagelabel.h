#ifndef IMAGELABEL_H
#define IMAGELABEL_H

#include <QLabel>

class ImageLabel: public QLabel
{
    Q_OBJECT

public:
    virtual void mousePressEvent(QMouseEvent* ev);
    virtual void mouseReleaseEvent(QMouseEvent* ev);
    virtual void mouseMoveEvent(QMouseEvent* ev);

signals:
    void mousePositionInitial(int x1, int y1);
    void mousePositionFinal(int x2, int y2);
    void mousePositionIntermediate(int x2, int y2);

};

#endif // IMAGELABEL_H
