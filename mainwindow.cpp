#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtWidgets/QLabel>
#include "aspectratiowidget.h"
#include <QThread>
#include <QMessageBox>
#include <QFileDialog>

#define FLAG_REGISTERS()        \
    FLAG_REGISTER(N, Negative)  \
    FLAG_REGISTER(Z, Zero)      \
    FLAG_REGISTER(V, Overflow)  \
    FLAG_REGISTER(C, Carry)

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui_(new Ui::MainWindow),
    vcpu_(new Vcpu("rom.bin", [this](){emit executionStopped();})),
    registerValues_(vcpu_->getNRegisters()),
    screen_(new Screen(vcpu_->getFramebuffer(), vcpu_->getDisplayWidth(), vcpu_->getDisplayHeight())),
    disasModel_(new DisasModel(vcpu_))
{
    connect(this, &MainWindow::executionStopped, this, &MainWindow::executionStopped_);

    ui_->setupUi(this);
    ui_->stateLayout->insertWidget(0, new AspectRatioWidget(screen_, screen_->sizeHint().width(), screen_->sizeHint().height()));
    ui_->disasView->setModel(disasModel_);
    ui_->disasView->setFocusPolicy(Qt::NoFocus);
    ui_->disasView->setSelectionMode(QAbstractItemView::NoSelection);

    for (unsigned i = 0; i < vcpu_->getNRegisters(); i++)
    {
        QString label = vcpu_->getRegisterName(i).c_str();
        QLineEdit* value = new QLineEdit();

        connect(value, &QLineEdit::editingFinished, [this, value, i]()
        {
            bool ok;
            int val = value->text().toInt(&ok, 8);
            if (ok)
                vcpu_->getRegister(i) = val;
            value->setText(QString().sprintf("%.8o", (unsigned) vcpu_->getRegister(i)));
        });

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

    createMenus_();
}

void MainWindow::refreshCpuState_()
{
    for (unsigned i = 0; i < vcpu_->getNRegisters(); i++)
        registerValues_[i]->setText(QString().sprintf("%.8o", (unsigned) vcpu_->getRegister(i)));

    disasModel_->reload();

    ui_->disasView->scrollTo(disasModel_->index(vcpu_->getPC() / sizeof (uint16_t)), QAbstractItemView::PositionAtTop);

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

    executionStopped_();
}

void MainWindow::on_resetButton_clicked()
{
    vcpu_->reset();

    executionStopped_();
}

void MainWindow::on_stepButton_clicked()
{
    setVcpuControlsToState_(true);

    vcpu_->step();

    executionStopped_();
}

void MainWindow::setVcpuControlsToState_(bool running)
{
    setEnabledWidgetsInLayout_(ui_->cpuLayout, !running);
    ui_->startButton->setEnabled(!running && vcpu_->getStatus() == VCPU_STATUS_OK);
    ui_->stepButton->setEnabled(!running && vcpu_->getStatus() == VCPU_STATUS_OK);

    ui_->pauseButton->setEnabled(running);
}

void MainWindow::executionStopped_()
{
    checkVcpuStatus_();
    setVcpuControlsToState_(false);
    refreshCpuState_();
}

void MainWindow::checkVcpuStatus_()
{
    switch (vcpu_->getStatus())
    {
    case VCPU_STATUS_OK:
        break;
    case VCPU_STATUS_FAIL_OPEN_ROM:
        QMessageBox::warning(this, "Warning", "Unable to load default ROM file. Select a ROM file in File->Open ROM.");
        break;
    case VCPU_STATUS_WRONG_ROM_SIZE:
        QMessageBox::critical(this, "Error", "Error loading ROM file. Select a valid ROM file in File->Open ROM.");
        break;
    case VCPU_STATUS_WRITE_FROM_READONLY:
        QMessageBox::critical(this, "Error", "Attempt to write into read-only memory. Unable to continue execution");
        break;
    case VCPU_STATUS_NOT_IMPLEMENTED_INSTRUCTION:
        QMessageBox::critical(this, "Error", "Attempt to execute an unimplemented instruction. Unable to continue execution");
        break;
    case VCPU_STATUS_INVALID_INSTRUCTION:
        QMessageBox::critical(this, "Error", "Attempt to execute an invalid instruction. Unable to continue execution");
        break;
    }
}

void MainWindow::show()
{
    QMainWindow::show();

    executionStopped_();
}

void MainWindow::createMenus_()
{
    QMenu* fileMenu = menuBar()->addMenu("&File");

    QAction* openAct = new QAction("&Open ROM File", this);
    openAct->setShortcuts(QKeySequence::Open);
    connect(openAct, &QAction::triggered, this, [this]()
    {
        QString fileName = QFileDialog::getOpenFileName(this, "Open ROM file");
        vcpu_->reset(fileName.toStdString());
    });
    fileMenu->addAction(openAct);

    QAction* exitAct = new QAction("&Exit", this);
    exitAct->setShortcuts(QKeySequence::Quit);
    connect(exitAct, &QAction::triggered, this, [this]()
    {
        QApplication::quit();
    });
    fileMenu->addAction(exitAct);

    QAction* aboutAct = new QAction("&About", this);
    aboutAct->setShortcuts(QKeySequence::HelpContents);
    connect(aboutAct, &QAction::triggered, this, [this]()
    {
        QMessageBox::about(this, "About PDP 11 emulator",
                                 "Version " APP_VERSION "<br>"
                                 "Authors: Nikitenko Evgeny & Ivanov Alexey<br>"
                                 "Github: <a href='https://github.com/jjeka/pdp11-emulator'>github.com/jjeka/pdp11-emulator</a>");
    });
    menuBar()->addAction(aboutAct);
}
