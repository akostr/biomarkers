#ifndef WEBDIALOGANALYSISLIST_H
#define WEBDIALOGANALYSISLIST_H

#include "web_dialog.h"

class QTableView;
class TreeModelDynamicAnalyzes;
class WebDialogAnalysisList : public WebDialog
{
  Q_OBJECT
public:
  WebDialogAnalysisList(QWidget *parent);

signals:
  void UploadAnalysis(int id);

protected slots:
  void Accept();

protected:
  void SetupModels();
  void SetupUi();

protected:
  //// Models
  QPointer<TreeModelDynamicAnalyzes> Analyzes;

  //// Ui
  QPointer<QWidget>         Body;
  QPointer<QVBoxLayout>     BodyLayout;

  //// Input
  QPointer<QTableView>    View;
};

#endif // WEBDIALOGANALYSISLIST_H
