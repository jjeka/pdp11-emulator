#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtWidgets/QLabel>
#include "aspectratiowidget.h"

#define FLAG_REGISTERS()        \
    FLAG_REGISTER(N, Negative)  \
    FLAG_REGISTER(Z, Zero)      \
    FLAG_REGISTER(V, Overflow)  \
    FLAG_REGISTER(C, Carry)

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui_(new Ui::MainWindow),
    vcpu_(new Vcpu("rom.bin")),
    registerValues_(vcpu_->getNRegisters()),
    screen_(new Screen(vcpu_->getFramebuffer(), vcpu_->getDisplayWidth(), vcpu_->getDisplayHeight())),
    disasModel_(new DisasModel(vcpu_))
{
    ui_->setupUi(this);
    ui_->stateLayout->insertWidget(0, new AspectRatioWidget(screen_, screen_->sizeHint().width(), screen_->sizeHint().height()));
    ui_->disasView->setModel(disasModel_);
    ui_->disasView->setFocusPolicy(Qt::NoFocus);
    ui_->disasView->setSelectionMode(QAbstractItemView::NoSelection);

    for (unsigned i = 0; i < vcpu_->getNRegisters(); i++)
    {
        QString label = vcpu_->getRegisterName(i).c_str();
        QLineEdit* value = new QLineEdit();

        ui_->registersLayout->addRow(label, value);
        registerValues_[i] = value;
    }

    QHBoxLayout* flagsLayout = new QHBoxLayout();

    #define FLAG_REGISTER(name, fullname)                                                       \
        flag##name##Register_ = new ClickableLabel();                                           \
        flagsLayout->addWidget(flag##name##Register_);                                          \
        connect(flag##name##Register_, &ClickableLabel::clicked, [this]()                       \
        {                                                                                       \
            vcpu_->set##fullname##Flag(!vcpu_->get##fullname##Flag());                          \
            flag##name##Register_->setText(flagString_(#name, vcpu_->get##fullname##Flag()));   \
        });

    FLAG_REGISTERS()
    #undef FLAG_REGISTER

    flagsLayout->addStretch();
    ui_->registersLayout->addRow("Flags", flagsLayout);

    refreshCpuState_();
    setVcpuControlsToState_(false);
}

void MainWindow::refreshCpuState_()
{
    //ui_->disasView->setCurrentIndex(disasModel_.index(353451, 0));
    for (unsigned i = 0; i < vcpu_->getNRegisters(); i++)
    {
        QString registerValue;
        registerValue.sprintf("0x%.4x", (unsigned) vcpu_->getRegister(i));
        registerValues_[i]->setText(registerValue);
    }

    disasModel_->reload();

    ui_->disasView->scrollTo(disasModel_->index(vcpu_->getPC() / sizeof (uint16_t)));

    #define FLAG_REGISTER(name, fullname)                                               \
        flag##name##Register_->setText(flagString_(#name, vcpu_->get##fullname##Flag()));

    FLAG_REGISTERS()
    #undef FLAG_REGISTER
}

QString MainWindow::flagString_(QString name, bool value)
{
    return value ? "<b>" + name + "</b>" : name;
}

MainWindow::~MainWindow()
{
    delete ui_;
    delete disasModel_;
    delete vcpu_;
}

bool MainWindow::isChildWidgetOfAnyLayout_(QLayout *layout, QWidget *widget)
{
   if (layout == NULL or widget == NULL)
      return false;

   if (layout->indexOf(widget) >= 0)
      return true;

   foreach(QObject *o, layout->children())
   {
      if (isChildWidgetOfAnyLayout_((QLayout*)o,widget))
         return true;
   }

   return false;
}

void MainWindow::setEnabledWidgetsInLayout_(QLayout *layout, bool enabled)
{
   if (layout == NULL)
      return;

   QWidget *pw = layout->parentWidget();
   if (pw == NULL)
      return;

   foreach(QWidget *w, pw->findChildren<QWidget*>())
   {
      if (isChildWidgetOfAnyLayout_(layout,w))
         w->setEnabled(enabled);
   }
}


void MainWindow::on_startButton_clicked()
{
    setVcpuControlsToState_(true);

    vcpu_->start();
}

void MainWindow::on_pauseButton_clicked()
{
    vcpu_->pause();

    setVcpuControlsToState_(false);
    refreshCpuState_();
}

void MainWindow::on_resetButton_clicked()
{
    //vcpu_->reset();
}

void MainWindow::on_stepButton_clicked()
{
    setVcpuControlsToState_(true);
    vcpu_->step();
    setVcpuControlsToState_(false);

}

void MainWindow::setVcpuControlsToState_(bool running)
{
    setEnabledWidgetsInLayout_(ui_->cpuLayout, !running);
    ui_->startButton->setEnabled(!running);
    ui_->stepButton->setEnabled(!running);

    ui_->pauseButton->setEnabled(running);
}
