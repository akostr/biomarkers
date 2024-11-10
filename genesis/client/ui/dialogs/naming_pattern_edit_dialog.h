#ifndef NAMING_PATTERN_EDIT_DIALOG_H
#define NAMING_PATTERN_EDIT_DIALOG_H

#include "ui/dialogs/templates/dialog.h"
#include <QPointer>

namespace Ui {
class NamingPatternEditDialog;
}

class QRegularExpression;
class NamingPatternEditDialog : public Dialogs::Templates::Dialog
{
  Q_OBJECT

public:
  explicit NamingPatternEditDialog(QWidget *parent = nullptr);
  ~NamingPatternEditDialog();

  void Accept() override;

private:
  Ui::NamingPatternEditDialog *ui;
  QPointer<QWidget> mContent;
  QPointer<QWidget> mTagPopupWidget;
  QRegularExpression* mTagRegularExpression;

  //sample formatting
  QString mFormattingFontOpenTag;
  QString mFormattingErrorFontOpenTag;
  QString mFormattingFontCloseTag;
  QString mFormattingInsertPattern;

  static QMap<QString, QString> tagSampleMatch;

private:
  void updateSample(const QString& pattern);
  void applyChangesAndAccept();

protected:
  void setupUi();
  void loadCurrentPattern();

public:
  bool eventFilter(QObject *watched, QEvent *event) override;
};

#endif // NAMING_PATTERN_EDIT_DIALOG_H
