#ifndef MARKUPSAVEASDIALOG_H
#define MARKUPSAVEASDIALOG_H

#include "ui/dialogs/templates/dialog.h"

class DialogLineEditEx;

using Btns = QDialogButtonBox::StandardButton;
class MarkupSaveAsDialog : public Dialogs::Templates::Dialog
{
  Q_OBJECT
public:
  MarkupSaveAsDialog(QWidget *parent);
  ~MarkupSaveAsDialog();
  virtual void Accept() override;
  QString getTitle();

private:
  void setupUi();

private:
  QPointer<DialogLineEditEx> mMarkupNameEdit;
};

#endif // MARKUPSAVEASDIALOG_H
