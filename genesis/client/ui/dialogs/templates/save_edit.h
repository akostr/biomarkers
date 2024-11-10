#ifndef SAVE_EDIT_H
#define SAVE_EDIT_H

#include <ui/dialogs/web_dialog.h>

class QLabel;
namespace Ui {
class SaveEdit;
}

namespace Dialogs
{
namespace Templates
{

class SaveEdit : public WebDialog
{
  Q_OBJECT
public:
  struct EditTooltipsSet
  {
    QString empty;
    QString forbidden;
    QString notChecked;
    QString valid;
  };
  struct Settings
  {
    QString header                    = tr("Caption in header");
    QString subHeader                 = "";
    QString titleHeader               = tr("Title title");
    QString commentHeader             = tr("Comment title");
    bool titleRequired                = true;
    bool commentRequired              = false;
    QString titlePlaceholder          = tr("Enter title info here");
    QString commentPlaceholder        = tr("Enter comment info here");
    int titleMaxLen                   = 50;
    int commentMaxLen                 = 70;
    QString okBtnText                 = tr("Save");
    QString cancelBtnText             = tr("Cancel");
    QString defaultTitle;
    QString defaultComment;
    QStringList forbiddenTitles;
    EditTooltipsSet titleTooltipsSet;
    EditTooltipsSet commentTooltipsSet;
  };

  SaveEdit(QWidget *parent);
  SaveEdit(QWidget *parent, const Settings& settings);
  ~SaveEdit();

  void applySettings(const Settings& newSettings);
  Settings settings();
  QString getTitle();
  QString getComment();

private:
  Ui::SaveEdit *ui;
  Settings mSettings;

  QPixmap mPMValid;
  QPixmap mPMInvalid;
  QPixmap mPMUnchecked;
  QWidget* mContent;

private:
  void setupUi();
  void setDialogDataValidity(bool isValid);

  // WebDialog interface
public slots:
  void Accept() override;
};
}
}

#endif // SAVE_EDIT_H
