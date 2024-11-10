#pragma once
#ifndef JOINT_LAYER_WIDGET_H
#define JOINT_LAYER_WIDGET_H

#include <QWidget>
#include "itab_order_list.h"

namespace Ui
{
  class JointLayerWidget;
}
class QStandardItemModel;
class JointLayerWidget : public iTabOrderWidget
{
  Q_OBJECT
public:
  explicit JointLayerWidget(QStandardItemModel *layersModel, QWidget* parent = nullptr, const QString& labelText = tr("Object/Layer"));
  virtual ~JointLayerWidget();

//  void FillLayers(const QStringList& layers);
  void SetLayer(const QPair<int, QString> &layer);
  QPair<int, QString> SelectedLayer() const;

  void SetLabelText(const QString& text);
  void Highlight();
  void showRemoveButton(bool show);
private slots:
  void on_removeButton_clicked();

signals:
  void removeRequest();

private:
  Ui::JointLayerWidget* ui = nullptr;
  void SetupUi(QStandardItemModel *layersModel);
  bool mHighlighted;

  // iTabOrderList interface
public:
  QWidgetList tabOrderWidgets() override;
};
#endif
