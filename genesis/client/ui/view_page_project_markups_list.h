#pragma once

#include "view.h"

class QVBoxLayout;
class QFrame;
class QTableView;
class TreeModelDynamicProjectMarkupsList;
class QLabel;

namespace Control
{
  class EventTableView;
}

namespace Views
{
class ViewPageProjectMarkupsList : public View
{
  Q_OBJECT

public:
  explicit ViewPageProjectMarkupsList(QWidget* parent = 0);
  ~ViewPageProjectMarkupsList() = default;

  void SetupUi();

protected slots:
  void UpdateMarkupsCountLabel();

private:
  //// Logic
  QPointer<TreeModelDynamicProjectMarkupsList> Markups;

  //// Ui
  QPointer<QVBoxLayout>        Layout;
  QPointer<QFrame>             Content;
  QPointer<QVBoxLayout>        ContentLayout;
  QPointer<Control::EventTableView>         Table;
  QPointer<QLabel>             MarkupsCountLabel;

  // View interface
public:
  void ApplyContextModules(const QString &dataId, const QVariant &data) override;
};
}//namespace Views
