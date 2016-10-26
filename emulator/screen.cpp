#include "screen.h"
#include <QPaintEvent>
#include <QPainter>

Screen::Screen(const void* screenData, unsigned width, unsigned height, QWidget *parent) :
    QWidget(parent),
    image_((const uchar*) screenData, width, height, QImage::Format_Grayscale8),
    timer_()
{
    QSize size(width, height);
    setMinimumSize(size);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(&timer_, &QTimer::timeout, this, [this]()
    {
        update();
    });
    timer_.start(1000 / 60);
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
}
