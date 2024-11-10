#include "progress_dialog.h"
#include "wait_indicator.h"

#include <QHBoxLayout>
#include <QPushButton>
#include <QKeyEvent>
#include <QLabel>

#ifndef DISABLE_CYBER_FRAC_STYLE
#include <style.h>
#endif

//// Progress dialog
ProgressDialog::ProgressDialog(QWidget* parent, const QString& message)
  : QDialog(parent)
  , Function()
  , Thread(nullptr)
  , TextLabel(nullptr)
  , ButtonStop(nullptr)
  , RuntimeMsg(message)
{
  SetupUi();
}

ProgressDialog::ProgressDialog(QWidget* parent,
                               std::function<void()> fn, const QString& message,
                               std::function<void()> stopFn, const QString &stopMessage)
  : QDialog(parent)
  , Function(fn)
  , FunctionStop(stopFn)
  , Thread(nullptr)
  , TextLabel(nullptr)
  , ButtonStop(nullptr)
  , RuntimeMsg(message)
  , StopMsg(stopMessage)
{
  SetupUi();
  
  Thread = new FunctionThread(fn, this);
  connect(Thread, SIGNAL(funcFinished()), this, SLOT(close()));
  Thread->start();
}

ProgressDialog::~ProgressDialog()
{
}

void ProgressDialog::SetupUi()
{
  setObjectName("progress");
  setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::FramelessWindowHint);

#ifndef DISABLE_CYBER_FRAC_STYLE
  setStyleSheet(Style::GetProgressStyle());
#endif

  QHBoxLayout* box = new QHBoxLayout();
  setContentsMargins(0, 0, 0, 0);
  box->setContentsMargins(6, 6, 6, 6);
  box->setSpacing(0);
  box->addSpacing(2);
  box->addWidget(new WaitIndicator(QString(), this, 28), 0);
  TextLabel = new QLabel(RuntimeMsg.isEmpty() ? tr("Please wait...") : RuntimeMsg, this);
  TextLabel->setObjectName("progress");
  box->addWidget(TextLabel, 1);
  box->addSpacing(18);

  if (FunctionStop == nullptr)
  {
    setLayout(box);
  }
  else
  {
    QVBoxLayout* vbox = new QVBoxLayout(this);
    vbox->addLayout(box);
    ButtonStop = new QPushButton(tr("Stop"), this);
    ButtonStop->setProperty("critical", true);
    vbox->addWidget(ButtonStop);
    connect(ButtonStop, SIGNAL(clicked(bool)), this, SLOT(StopFunction()));
  }
}

void ProgressDialog::keyPressEvent(QKeyEvent* event)
{
  if (event->key() == Qt::Key_Escape)
  {
    event->accept();
  }
  else
  {
    QDialog::keyPressEvent(event);
  }
}

void ProgressDialog::accept()
{
  if (Thread && Thread->isRunning())
    return;

  QDialog::accept();
}

void ProgressDialog::reject()
{
  if (Thread && Thread->isRunning())
    return;

  QDialog::reject();
}

void ProgressDialog::RunFunction(QWidget* parent,
                                 std::function<void()> fn, const QString& message,
                                 std::function<void()> stopFn, const QString &stopMessage)
{
  ProgressDialog dialog(parent, fn, message, stopFn, stopMessage);
  dialog.exec();
}

void ProgressDialog::StopFunction()
{
  disconnect(Thread, 0, this, 0);
  ButtonStop->hide();
  TextLabel->setText(StopMsg.isEmpty() ? tr("Stops...") : StopMsg);
  adjustSize();

  if (Thread->isRunning())
  {
    Thread->quit();
    if (!Thread->wait(5000))
    {
      Thread->terminate();
      Thread->wait();
    }
  }
  delete Thread;
  Thread = new FunctionThread(FunctionStop, this);
  connect(Thread, SIGNAL(funcFinished()), this, SLOT(close()));
  Thread->start();
}
