#ifndef PROJECT_TABLE_RENAME_DIALOG_REFACTOR_H
#define PROJECT_TABLE_RENAME_DIALOG_REFACTOR_H

#include <ui/dialogs/templates/dialog.h>

#include <QWidget>

namespace Ui {
class ProjectTableRenameDialogRefactor;
}

namespace Dialogs
{
class ProjectEditDialog : public Templates::Dialog
{
  Q_OBJECT

public:
  explicit ProjectEditDialog(
    int projectId,
    const QString& projectTitle,
    const QString& status,
    const QString& comment,
    const QString& codeWord,
    QWidget* parent = nullptr);
  ~ProjectEditDialog();

  QString getTitle();
  QString getStatusCode();
  QString getStatus();
  QIcon getStatusIcon();
  QString getComment();
  QString getCodeWord();

private:
  Ui::ProjectTableRenameDialogRefactor* ui;
  QWidget* mContent;

  int mProjectId = -1;
  QString mDefaultTitle;
  QString mEnglishStatus;
  QString mStatus;
  QString mComment;
  QString mCodeWord;

private:
  void setupUi();
  void setDialogDataValidity();
  void loadModels();
  QIcon createCircleIcon(QSize size, QColor color);

public slots:
  void Open() override;
};
}

#endif // PROJECT_TABLE_RENAME_DIALOG_REFACTOR_H
