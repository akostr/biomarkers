#pragma once
#include "joint_layer_widget.h"
#ifndef JOINT_LAYERS_CONTROLS_H
#define JOINT_LAYERS_CONTROLS_H

#include "itab_order_list.h"


namespace Ui
{
	class JointLayersControl;
}

class QStandardItemModel;
class JointLayersControl : public iTabOrderWidget
{
	Q_OBJECT
public:
  explicit JointLayersControl(QWidget* parent = nullptr);
  virtual ~JointLayersControl() = default;

	void SetLayers(const QList<QPair<int, QString> > &layers);
  QList<QPair<int, QString> > GetSelectedLayers() const;
  void SetLayersComboModel(QStandardItemModel *layersModel);
  void HighlightEmpty();
  void updateRemoveButtons();

protected:
  Ui::JointLayersControl* WidgetUi = nullptr;

	void SetupUi();
  JointLayerWidget* AddLayerWidget();
  void ResortCombos();

  QList<JointLayerWidget*> mWidgets;


  // iTabOrderList interface
public:
  QWidgetList tabOrderWidgets() override;

private:
  QStandardItemModel* mLayersModel = nullptr;
 };
#endif
