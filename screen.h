#ifndef SCREEN_H
#define SCREEN_H

#include <QImage>
#include <QTimer>
#include <QWidget>

class Screen : public QWidget
{
    Q_OBJECT

    QImage image_;
    QTimer timer_;

public:
    Screen(const void* screenData, unsigned width, unsigned height, QWidget *parent = 0);
    virtual void paintEvent(QPaintEvent *event);
    virtual QSize sizeHint();

signals:

public slots:
};

#endif // SCREEN_H
