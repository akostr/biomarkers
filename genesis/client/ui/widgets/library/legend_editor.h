#pragma once
#ifndef LEGEND_EDITOR_H
#define LEGEND_EDITOR_H

#include <ui/dialogs/templates/dialog.h>
#include <QPointer>

#include "logic/models/shape_color_grouped_entity_plot_data_model.h"

class AnalysisEntityModel;
class RowFilterProxy;
class QStandardItemModel;
class LegendParameterTableModel;

namespace Ui
{
  class LegendEditor;
}
namespace Dialogs
{
class LegendEditor : public Templates::Dialog
{
  Q_OBJECT
public:
  explicit LegendEditor(QPointer<AnalysisEntityModel> model, QWidget* parent = nullptr,
                        std::optional<TPassportFilter> colorFilter = std::optional<TPassportFilter>(),
                        std::optional<TPassportFilter> shapeFilter = std::optional<TPassportFilter>());
  virtual ~LegendEditor();

  std::optional<TPassportFilter> colorFilter();
  std::optional<TPassportFilter> shapeFilter();
  QList<TLegendGroup> getGroups();

private:
  void setModel(QPointer<AnalysisEntityModel> model);
  void setupUi();
  void connectSignals();

  void updateLegendItems();
  void updateLegendItemsFromModel();
  void removeItemFromColor();
  void removeItemFromShape();
  bool eventFilter(QObject* object, QEvent* event);

  TPassportFilter makeFilter(const QString &val, const QString& key);

private:
  Ui::LegendEditor* ui = nullptr;
  QWidget* mContent;
  QPointer<LegendParameterTableModel> mTableModel = nullptr;
  QPointer<AnalysisEntityModel> mModel = nullptr;
  QPointer<QStandardItemModel> mParamsModel = nullptr;
  QPointer<RowFilterProxy> mColorsProxy = nullptr;
  QPointer<RowFilterProxy> mShapesProxy = nullptr;
  std::optional<TPassportFilter> mInitialColorFilter;
  std::optional<TPassportFilter> mInitialShapeFilter;
};
}//namespace Dialogs
#endif
