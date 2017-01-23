#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtWidgets/QLabel>
#include "aspectratiowidget.h"
#include <QThread>
#include <QMessageBox>
#include <QFileDialog>
#include <QKeyEvent>
#include "keycodes.h"
#include <QKeyEvent>

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

    saved_conv_ticks_ = 0;
    saved_no_conv_ticks_ = 0;
    saved_instr_num_ = 0;

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
            value->setText(QString().sprintf("%.6o", (unsigned) vcpu_->getRegister(i)));
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


    //QString caption1 = "Elapsed ticks:", caption2 = "with conveyor", caption3 = "without conveyor";
    QHBoxLayout* conveyorLayout1 = new QHBoxLayout();
    QHBoxLayout* conveyorLayout2 = new QHBoxLayout();

    QLabel* label_caption11 = new QLabel("with conveyor:");
    QLabel* label_caption21 = new QLabel("without conveyor:");
    QLabel* label_with_conv = new QLabel("0", this);
    QLabel* label_without_conv = new QLabel("0", this);
    conveyorLayout1->addWidget(label_caption11);
    conveyorLayout1->addWidget(label_with_conv);
    conveyorLayout2->addWidget(label_caption21);
    conveyorLayout2->addWidget(label_without_conv);

    QLabel* label_caption12 = new QLabel("per instruction:");
    QLabel* label_caption22 = new QLabel("per instruction:");
    QLabel* label_with_conv_avg = new QLabel("0", this);
    QLabel* label_without_conv_avg = new QLabel("0", this);
    conveyorLayout1->addWidget(label_caption12);
    conveyorLayout1->addWidget(label_with_conv_avg);
    conveyorLayout2->addWidget(label_caption22);
    conveyorLayout2->addWidget(label_without_conv_avg);


    ui_->registersLayout->addWidget(new QLabel(tr("Elapsed ticks")));
    ui_->registersLayout->addRow(conveyorLayout1);
    ui_->registersLayout->addRow(conveyorLayout2);
    conveyorValues_.push_back(label_with_conv);
    conveyorValues_.push_back(label_without_conv);
    conveyorValues_.push_back(label_with_conv_avg);
    conveyorValues_.push_back(label_without_conv_avg);
    createMenus_();
    setFocusPolicy(Qt::StrongFocus);
}

void MainWindow::refreshCpuState_()
{
    for (unsigned i = 0; i < vcpu_->getNRegisters(); i++)
        registerValues_[i]->setText(QString().sprintf("%.6o", (unsigned) vcpu_->getRegister(i)));

    disasModel_->reload();

    ui_->disasView->scrollTo(disasModel_->index(vcpu_->getPC() / sizeof (uint16_t)), QAbstractItemView::PositionAtTop);

    #define FLAG_REGISTER(name, fullname)                                               \
        flag##name##Register_->setText(flagString_(#name, vcpu_->get##fullname##Flag()));

    FLAG_REGISTERS()
    #undef FLAG_REGISTER

    uint64_t instr_num = vcpu_->get_instr_num();
    uint64_t ticks_conv = vcpu_->get_ticks_with_conv();
    uint64_t ticks_no_conv = vcpu_->get_ticks_without_conv();

    uint64_t t = saved_conv_ticks_;
    uint64_t tt = saved_no_conv_ticks_;
    uint64_t ttt = saved_instr_num_;

    saved_conv_ticks_ = ticks_conv;
    saved_no_conv_ticks_ = ticks_no_conv;
    saved_instr_num_ = instr_num;

    conveyorValues_[0]->setText(QString().sprintf("%" PRId64, ticks_conv));
    //conveyorValues_[0]->setText(QString().sprintf("%" PRId64, 1000000000000));
    conveyorValues_[1]->setText(QString().sprintf("%" PRId64, ticks_no_conv));
    if (instr_num - ttt <= 0)
    {
        conveyorValues_[2]->setText(QString().sprintf("NaN"));
        conveyorValues_[3]->setText(QString().sprintf("NaN"));
    }
    else
    {
        while (instr_num - ttt > 100000)
        {
            instr_num /= 10;
            ticks_conv /= 10;
            ticks_no_conv /= 10;
            t /= 10;
            tt /= 10;
            ttt /= 10;
        }
        conveyorValues_[2]->setText(QString().sprintf("%f", ((double)(ticks_conv - t))/((double)(instr_num - ttt))));
        conveyorValues_[3]->setText(QString().sprintf("%f", ((double)(ticks_no_conv - tt))/((double)(instr_num - ttt))));
    }
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
   if (layout == NULL || widget == NULL)
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
    case VCPU_STATUS_INVALID_OPCODE:
        QMessageBox::critical(this, "Error", "Attempt to execute an instruction with invalid opcode. Unable to continue execution");
        break;
    case VCPU_STATUS_INVALID_MEMORY_ACCESS:
        QMessageBox::critical(this, "Error", "Invalid memory access. Unable to continue execution");
        break;
    case VCPU_STATUS_INVALID_PC:
        QMessageBox::critical(this, "Error", "Invalid (odd) PC. Unable to continue execution");
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
                                 "Authors: Nikitenko Evgeny, Ivanov Alexey & Samara Oleksa<br>"
                                 "Github: <a href='https://github.com/jjeka/pdp11-emulator'>github.com/jjeka/pdp11-emulator</a>");
    });
    menuBar()->addAction(aboutAct);
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    #define VCPU_KEYCODE(code)                  \
        case Qt::Key_ ## code:                  \
            keyCode = VCPU_KEYCODE_ ## code;    \
            break;

    #define VCPU_KEYCODE_(code, val) VCPU_KEYCODE(code)

    uint8_t keyCode = 0;

    switch(event->key())
    {
        #include "keycodestable.h"
    }

    if (keyCode)
        vcpu_->getKeyboard().keyPressed(keyCode, event->modifiers() & Qt::ControlModifier, event->modifiers() & Qt::ShiftModifier);
}
