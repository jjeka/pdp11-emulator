#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "vcpu.h"
#include <vector>
#include <QtWidgets/QLineEdit>
#include "screen.h"
#include <QAbstractListModel>
#include "disasmodel.h"
#include <QStandardItem>
#include <clickablelabel.h>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui_;
    Vcpu* vcpu_;
    std::vector<QLineEdit*> registerValues_;
    Screen* screen_;
    DisasModel* disasModel_;
    ClickableLabel* flagNRegister_;
    ClickableLabel* flagZRegister_;
    ClickableLabel* flagVRegister_;
    ClickableLabel* flagCRegister_;

    void changeBreakpoint_(QStandardItem * item);
    void refreshCpuState_();
    QString flagString_(QString name, bool value);
    void setEnabledWidgetsInLayout_(QLayout *layout, bool enabled);
    bool isChildWidgetOfAnyLayout_(QLayout *layout, QWidget *widget);
    void setVcpuControlsToState_(bool running);

Q_SIGNALS:
    void executionStopped();

public Q_SLOTS:
    virtual void on_startButton_clicked();
    virtual void on_pauseButton_clicked();
    virtual void on_resetButton_clicked();
    virtual void on_stepButton_clicked();

};

#endif // MAINWINDOW_H
