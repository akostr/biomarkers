#ifndef MARKUP_SAVE_TABLE_DIALOG_H
#define MARKUP_SAVE_TABLE_DIALOG_H

#include "ui/dialogs/templates/dialog.h"

class DialogLineEditEx;
class DialogTextEditEx;
class QCheckBox;

class MarkupSaveTableDialog : public Dialogs::Templates::Dialog
{
  Q_OBJECT

public:
  explicit MarkupSaveTableDialog(QWidget *parent, const QStringList& occupiedNames);
  ~MarkupSaveTableDialog();

  virtual void Accept() override;

  QString name();
  QString comment();
  bool generateMatrix();

private:
  QStringList mOccupiedNames;
  QPointer<DialogLineEditEx> mMarkupNameEdit;
  QPointer<DialogTextEditEx> mMarkupCommentEdit;
  QPointer<QCheckBox> mGenerateMatrixCheckBox;

private:
  void setupUi();
};

#endif // MARKUP_SAVE_TABLE_DIALOG_H
