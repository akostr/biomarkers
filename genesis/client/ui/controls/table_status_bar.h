#pragma once

#include <QLabel>

#include "../../logic/tree_model.h"

////////////////////////////////////////////////////
//// Table status bar
class TableStatusBar : public QLabel
{
  Q_OBJECT
public:
  TableStatusBar(QWidget* parent, TreeModel* model, QSortFilterProxyModel *proxy = nullptr);
  ~TableStatusBar();

  void Setup();
  void Update();

private:
  QPointer<TreeModel> Model;
  QPointer<QSortFilterProxyModel> Proxy;
};
