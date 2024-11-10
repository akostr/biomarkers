#ifndef TABLERENAMEDIALOG_H
#define TABLERENAMEDIALOG_H

#include <ui/dialogs/templates/dialog.h>
#include <QObject>
#include <QWidget>

class DialogLineEditEx;

using Btns = QDialogButtonBox::StandardButton;

class TableRenameDialog : public Dialogs::Templates::Dialog
{
  Q_OBJECT
public:
  TableRenameDialog(QWidget *parent, QString title = "");
  ~TableRenameDialog();
  virtual void Accept() override;
  QString getTitle();

  void SetLineEditCaption(const QString& caption);

private:
  void setupUi();

private:
  QPointer<DialogLineEditEx> mEdit;
  QString defaultTitle;
};

#endif // TABLERENAMEDIALOG_H
