#pragma once

#include <QDialog>
#include <QThread>

#include <functional>

class QLabel;

class FunctionThread : public QThread
{
  Q_OBJECT

public:
  FunctionThread(std::function<void()> fn, QObject* parent = nullptr)
    : QThread(parent)
    , fn(fn)
  {}

signals:
  void funcFinished();

protected:
  void run()
  {
    fn();
    emit funcFinished();
  }

  std::function<void()> fn;
};

//// Progress dialog
class ProgressDialog : public QDialog
{
  Q_OBJECT

public:
  explicit ProgressDialog(QWidget* parent, const QString& message = QString());
  explicit ProgressDialog(QWidget* parent,
                          std::function<void()> fn, const QString& message = QString(),
                          std::function<void()> stopFn = nullptr, const QString& stopMessage = QString());
  ~ProgressDialog();

  void SetupUi();
  
  virtual void keyPressEvent(QKeyEvent* event) override;
  virtual void accept() override;
  virtual void reject() override;

  static void RunFunction(QWidget* parent,
                          std::function<void()> fn, const QString& message = QString(),
                          std::function<void()> stopFn = nullptr, const QString& stopMessage = QString());

public slots:
  void StopFunction();

protected:
  FunctionThread* Thread;

private:
  std::function<void()> Function;
  std::function<void()> FunctionStop;

  QLabel* TextLabel;
  QPushButton* ButtonStop;

  QString RuntimeMsg;
  QString StopMsg;
};
