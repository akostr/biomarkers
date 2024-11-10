#ifndef PROJECT_TABLE_RENAME_DIALOG_H
#define PROJECT_TABLE_RENAME_DIALOG_H

#include <ui/dialogs/web_dialog.h>
#include <QSet>

namespace Ui {
class ProjectTableRenameDialog;
}
class QLabel;
namespace Dialogs
{
class ProjectTableRenameDialog : public WebDialog
{
  Q_OBJECT

public:
  struct EditTooltipsSet
  {
    QString empty;
    QString forbidden;
    QString notChecked;
    QString valid;
    QString invalidSymbol;
  };
  struct Settings
  {
    QString header                    = tr("Common information edit");
    QString titleHeader               = tr("Project title");
    QString statusHeader              = tr("Project status");
    bool titleRequired                = true;
    bool controlSymbols               = false;
    QString titlePlaceholder          = tr("Enter title here");
    int titleMaxLen                   = 50;
    QString okBtnText                 = tr("Save");
    QString cancelBtnText             = tr("Cancel");
    QString defaultTitle;
    QSet<QString> forbiddenTitles;
    EditTooltipsSet titleTooltipsSet = {tr("Title is empty"),
                                        tr("Title has alredy used"),
                                        tr("Not checked"),
                                        tr("Title is valid")};
    int projectId                     = -1;
  };
  ProjectTableRenameDialog(QWidget *parent, int projectId, QString projectTitle);
  ProjectTableRenameDialog(QWidget *parent, const Settings& settings);
  ~ProjectTableRenameDialog();

  void applySettings(const Settings& newSettings);
  Settings settings();
  QString getTitle();
  QString getStatusCode();
  QString getStatus();
  static Settings defaultSettings();

private:
  Ui::ProjectTableRenameDialog *ui;
  Settings mSettings;

  QPixmap mPMValid;
  QPixmap mPMInvalid;
  QPixmap mPMUnchecked;
  QWidget* mContent;

private:
  void setupUi();
  void setDialogDataValidity(bool isValid);
  void loadModels();
  QIcon createCircleIcon(QSize size, QColor color);

public slots:
  void Accept() override;
  void Open() override;
};
}//namespace Dialogs

#endif // PROJECT_TABLE_RENAME_DIALOG_H
