#pragma once

#include "web_dialog.h"

class TreeModelDynamicMarkupVersions;
class QTableView;


/////////////////////////////////////////////////////
//// Web Dialog / open markup version
class WebDialogMarkupVersionList : public WebDialog
{
  Q_OBJECT

public:
  explicit WebDialogMarkupVersionList(QWidget* parent, QSizeF size = QSizeF());
  ~WebDialogMarkupVersionList();
  QString GetSelectedVersionTitle();

signals:
  void UploadVersion(int id);

protected slots:
  void Accept();

protected:
  void SetupModels();
  void SetupUi();

protected:
  //// Models
  QPointer<TreeModelDynamicMarkupVersions> Versions;

  //// Ui
  QPointer<QWidget>       Body;
  QPointer<QVBoxLayout>   BodyLayout;

  //// Input
  QPointer<QTableView>    View;

  //// Data
  QString                 SelectedVersionTitle;
};
