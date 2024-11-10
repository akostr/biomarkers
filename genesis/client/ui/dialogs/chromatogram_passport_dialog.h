#ifndef CHROMATOGRAMPASSPORTDIALOG_H
#define CHROMATOGRAMPASSPORTDIALOG_H

#include <ui/dialogs/templates/info.h>
#include <logic/structures/passport_form_structures.h>
#include <ui/widgets/passport/main_form.h>

namespace Ui {
class ChromatogramPassportDialogBody;
}

namespace Dialogs
{
using Btns = QDialogButtonBox::StandardButton;
class ChromatogramPassportDialog : public Templates::Info
{
  Q_OBJECT
public:
  ChromatogramPassportDialog(bool isFragment, const QJsonObject& json, QWidget* parent);
  ~ChromatogramPassportDialog();
  QJsonArray toJson();

  void Accept() override;

private:
  void setupUi();
  QString parseJson(const QJsonObject& json);

private:
  Ui::ChromatogramPassportDialogBody *ui;
  QList<PassportMainForm::ChromaData> mChromasData;
  QPointer<QWidget> mBody;
};
}//namespace Dialogs
#endif // CHROMATOGRAMPASSPORTDIALOG_H
