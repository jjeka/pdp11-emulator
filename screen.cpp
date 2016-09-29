#include "screen.h"
#include <QPaintEvent>
#include <algorithm>

Screen::Screen(const void* screenData, unsigned width, unsigned height, QWidget *parent) :
    QWidget(parent),
    image_((const uchar*) screenData, width, height, QImage::Format_Grayscale8)
{
    QSize size(width, height);
    setMinimumSize(size);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

QSize Screen::sizeHint()
{
    return image_.size();
}

void Screen::paintEvent(QPaintEvent*)
{
    qreal ratio = qreal(size().width()) / image_.width();
    QTransform transform;
    transform.scale(ratio, ratio);

    QPainter painter;
    painter.begin(this);
    painter.setTransform(transform);
    painter.drawImage(0, 0, image_);
    painter.end();

    update();
}
