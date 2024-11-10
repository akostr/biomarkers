#pragma once
#ifndef VIEW_PAGE_MCR_MODEL_PAGE_H
#define VIEW_PAGE_MCR_MODEL_PAGE_H

#include "view.h"

class QLabel;
class QTableView;
class TreeModelDynamicProjectModelsList;
class TreeModelItem;

namespace Views
{
  class ViewPageMcrModelPage : public View
  {
    Q_OBJECT

  public:
    explicit ViewPageMcrModelPage(QWidget* parent = nullptr);
    ~ViewPageMcrModelPage() = default;

  protected:
    void DeleteItemFromTable(TreeModelItem* item);
    void EditItemFromTable(TreeModelItem* item);

  private:
    QTableView* McrModelTableView = nullptr;
    QLabel* StatusBarLabel = nullptr;
    TreeModelDynamicProjectModelsList* McrModelList = nullptr;

    void SetupUi();
    void OpenMcrModel(TreeModelItem* item);

    void DeleteAnalysisFromSeriver(int id);
    void UpdateTitleAndCommentOnServer(int id, const QString& title,
      const QString& comment);

    QString GetStatusBarText() const;
  };
}
#endif