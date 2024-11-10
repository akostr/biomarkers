#pragma once

#include "view.h"

#include "logic/structures/common_structures.h"


using namespace Structures;

namespace Ui {
class ViewPageCreateIdentificationPlot;
}

namespace Models
{
class AnalysisDataTableModel;
}

namespace Views
{

class ViewPageCreateIdentificationPlot : public View
{
  Q_OBJECT

public:
  enum
  {
    SelectedPage,
    CreatedPlotsPage
  };
  explicit ViewPageCreateIdentificationPlot(QWidget *parent = nullptr);
  ~ViewPageCreateIdentificationPlot();
  virtual void ApplyContextCreationPlot(const QString& /*dataId*/, const QVariant& /*data*/) override;
  virtual void SwitchToPage(const QString& pageId) override;

private:
  void setupUi();
  void setupConnections();
  void applyFilter();
  void hideClearAction();
  void showPreviousPage();
  void createAndShowPlots();
  void createPlots();
  void clear();

  void setPage(int pageIndex);

private:
  Ui::ViewPageCreateIdentificationPlot *ui;
  int mTableId;
  int mParameterId;
  Models::AnalysisDataTableModel *mModel;
  QAction* mNameSearchAction;
  QAction* mNameSearchClearAction;
};

}
