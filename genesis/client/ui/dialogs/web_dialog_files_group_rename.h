#ifndef WEBDIALOGFILESGROUPRENAME_H
#define WEBDIALOGFILESGROUPRENAME_H

#include "web_dialog.h"

class QLineEdit;

class WebDialogFilesGroupRename : public WebDialog
{
  Q_OBJECT
public:
  WebDialogFilesGroupRename(int groupId, const QString &groupName, QWidget* parent);
  ~WebDialogFilesGroupRename();

  QString GetGroupName();

private:
  void SetupUi();

  QPointer<QLineEdit> GroupNameLineEdit;
  int GroupId;
  QString GroupName;
};

#endif // WEBDIALOGFILESGROUPRENAME_H
