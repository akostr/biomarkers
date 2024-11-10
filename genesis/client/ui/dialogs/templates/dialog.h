#ifndef DIALOG_H
#define DIALOG_H

#include <ui/dialogs/web_dialog.h>
#include <QMap>

namespace Ui {
class Dialog;
}
namespace Dialogs
{
namespace Templates
{

class Dialog : public WebDialog
{
  Q_OBJECT

public:
  struct Settings
  {
    QString dialogHeader = tr("Dialog header");
    QMap<int, QVariantMap> buttonsProperties;
    QMap<int, QString> buttonsNames;
  };

  Dialog(QWidget *parent, QDialogButtonBox::StandardButtons buttons = QDialogButtonBox::Ok, QWidget *content = nullptr);
  Dialog(QWidget *parent, const Settings& settings, QDialogButtonBox::StandardButtons buttons = QDialogButtonBox::Ok, QWidget *content = nullptr);

  ~Dialog();

  void applySettings(const Settings& newSettings);
  QWidget* getContent();

  Settings getSettings() const;

private:
  Ui::Dialog *ui;
  Settings mSettings;
  QWidget* mInternalContent;//content of WebDialog::Content, contents headers, button - cross, and such.
  //PUT YOUR INTERNAL DIALOG CONTENT INTO ui->externalContentContainer!!!

private:
  void setupUi();
};

}
}
#endif // DIALOG_H
