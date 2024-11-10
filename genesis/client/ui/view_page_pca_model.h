#ifndef VIEW_PAGE_PCA_MODEL_H
#define VIEW_PAGE_PCA_MODEL_H

#include "view.h"
namespace Ui {
class ViewPagePcaModel;
}
class AnalysisPlotRefactor;
class QTreeView;
namespace Views
{
class ViewPagePcaModel : public View
{
  Q_OBJECT
public:
  ViewPagePcaModel(QWidget* parent = nullptr);
  ~ViewPagePcaModel();
  // View interface
public:
  void ApplyContextMarkupVersionAnalysisPCA(const QString &dataId, const QVariant &data) override;

private:
  void setupUi();
  void loadModels(int analysisId);

private:
  Ui::ViewPagePcaModel *ui;
  QPointer<AnalysisPlotRefactor> mCountPlot;
  QPointer<QTreeView> mCountModelTreeView;

};
}//namespace Views
#endif // VIEW_PAGE_PCA_MODEL_H
