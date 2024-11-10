#ifndef CONFIRM_H
#define CONFIRM_H

#include <ui/dialogs/web_dialog.h>
#include <QMap>

namespace Ui {
class Confirm;
}
namespace Dialogs
{
namespace Templates
{

class Confirm : public WebDialog
{
  Q_OBJECT

public:
  struct Settings
  {
    QString dialogHeader = tr("Dialog header");
    QMap<int, QVariantMap> buttonsProperties;
    QMap<int, QString> buttonsNames;
  };

  Confirm(QWidget *parent, QDialogButtonBox::StandardButtons buttons = QDialogButtonBox::Ok, QWidget *content = nullptr);
  Confirm(QWidget *parent, const Settings& settings, QDialogButtonBox::StandardButtons buttons = QDialogButtonBox::Ok, QWidget *content = nullptr);

  ~Confirm();

  void applySettings(const Settings& newSettings);
  Settings getSettings();
  QWidget* getContent();

  static Confirm* warning(QWidget *parent, QString header, QString text, QDialogButtonBox::StandardButtons buttons = QDialogButtonBox::Ok);
  static Confirm* confirmation(QWidget *parent, QString header, QString text, QDialogButtonBox::StandardButtons buttons = QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
  static Confirm* confirmationWithRedPrimary(QWidget *parent, QString header, QString text, QDialogButtonBox::StandardButtons buttons = QDialogButtonBox::Ok | QDialogButtonBox::Cancel, const QString &okBtnName = tr("Ok"));
  static Confirm* confirm(QWidget *parent, QString message, std::function<void()> onConfirm, std::function<void()> onIgnore, std::function<void()> onCancel);
  static Confirm* confirm(QWidget *parent, QString message, std::function<void()> onConfirm, std::function<void()> onCancel);

private:
  Ui::Confirm *ui;
  Settings mSettings;
  QWidget* mInternalContent;//content of WebDialog::Content, contents headers, button - cross, and such.
  //PUT YOUR INTERNAL DIALOG CONTENT INTO ui->externalContentContainer!!!


private:
  void setupUi();
};

}
}
#endif // CONFIRM_H
