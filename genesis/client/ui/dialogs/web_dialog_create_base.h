#pragma once

#include "web_dialog_create.h"
#include "functional"

class QLineEdit;

class WebDialogCreateBase : public WebDialogCreate
{
  Q_OBJECT

public:
  typedef std::function<void(QString)> HandleFunc;

public:
  explicit WebDialogCreateBase(QWidget* parent, 
                               const QString& subjectAccusative, 
                               const QString& subjectGenetive, 
                               HandleFunc func);
  ~WebDialogCreateBase();

  virtual QString GetCurrentName() const;

protected:
  void Accept();

protected:
  //// Ui
  QPointer<QWidget>       Body;
  QPointer<QVBoxLayout>   BodyLayout;

  //// Input
  QPointer<QLineEdit>     Name;

  HandleFunc Handle;
};
