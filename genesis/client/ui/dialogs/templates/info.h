#ifndef INFO_H
#define INFO_H

#include <ui/dialogs/web_dialog.h>
#include <QMap>

namespace Ui {
class info;
}

namespace Dialogs
{
namespace Templates
{

class Info : public WebDialog
{
  Q_OBJECT

public:
  struct Settings
  {
    QString dialogHeader = tr("Dialog header");
    QString contentHeader = tr("Content header");
    QMap<int, QVariantMap> buttonsProperties;
    QMap<int, QString> buttonsNames;
  };

  Info(QWidget *parent, QDialogButtonBox::StandardButtons buttons = QDialogButtonBox::Ok, QWidget *content = nullptr);
  Info(QWidget *parent, const Settings& settings, QDialogButtonBox::StandardButtons buttons = QDialogButtonBox::Ok, QWidget *content = nullptr);
  ~Info();

  void applySettings(const Settings& newSettings);
  Settings getCurrentSettings();
  QWidget* getContent();
  void setContentHeader(const QString& content);

private:
  Ui::info *ui;
  Settings mSettings;
  QWidget* mInternalContent;//content of WebDialog::Content, contents headers, button - cross, and such.
  //PUT YOUR INTERNAL DIALOG CONTENT INTO ui->externalContentContainer!!!


private:
  void setupUi();
};
}
}

#endif // INFO_H
