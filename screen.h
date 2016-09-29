#ifndef SCREEN_H
#define SCREEN_H

#include <QGraphicsView>
#include <QImage>
#include <QGraphicsScene>

class Screen : public QWidget
{
    Q_OBJECT

    QImage image_;
public:
    Screen(const void* screenData, unsigned width, unsigned height, QWidget *parent = 0);
    virtual void paintEvent(QPaintEvent *event);
    virtual QSize sizeHint();

signals:

public slots:
};

#endif // SCREEN_H
