#pragma once
#ifndef VIEW_PAGE_PLS_MODEL_PAGE_H
#define VIEW_PAGE_PLS_MODEL_PAGE_H

#include "view.h"

class QLabel;
class QTableView;
class TreeModelDynamicProjectModelsList;
class TreeModelItem;

namespace Views
{
  class ViewPagePlsModelPage : public View
  {
    Q_OBJECT

  public:
    ViewPagePlsModelPage(QWidget* parent = nullptr);
    ~ViewPagePlsModelPage() = default;

  protected:
    void ApplyContextMarkupVersionAnalysisPLS(const QString& dataId, const QVariant& data) override;

    void DeleteItemFromTable(TreeModelItem* item, bool isPredict);
    void EditItemFromTable(TreeModelItem* item);

  private:
    QTableView* PlsModelTableView = nullptr;
    QTableView* PlsPredictionTableView = nullptr;
    TreeModelDynamicProjectModelsList* PlsModelList = nullptr;
    TreeModelDynamicProjectModelsList* PlsPredictModelList = nullptr;
    QLabel* StatusBarLabel = nullptr;

    void SetupUi();
    void DeleteAnalysisFromSeriver(int id);
    void UpdateTitleAndCommentOnServer(int id, const QString& title,
      const QString& comment);

    void OpenPlsModel(TreeModelItem* item);
    void OpenPlsPredictModel(TreeModelItem* item);

    QString GetStatusBarText() const;
  };
}
#endif