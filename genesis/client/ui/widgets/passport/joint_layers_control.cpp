#include "joint_layers_control.h"

#include "ui_joint_layers_control.h"
#include <QComboBox>

JointLayersControl::JointLayersControl(QWidget* parent)
  : iTabOrderWidget(parent)
{
  SetupUi();
}

//JointLayersControl::~JointLayersControl()
//{

//}

void JointLayersControl::SetLayers(const QList<QPair<int, QString>>& layers)
{
  auto l = layers;
  const auto& widgets = mWidgets;
  int i = 0;
  for(auto witer = widgets.begin(); witer != widgets.end(); witer++)
  {
    auto w = *witer;
    if(i >= l.size())
    {
      w->deleteLater();
      mWidgets.erase(witer--);
    }
    else
    {
      w->SetLayer(l[i]);
    }
    i++;
  }
  if(i < l.size())
  {
    for(i; i < l.size(); i++)
    {
      auto w = AddLayerWidget();
      w->SetLayer(l[i]);
    }
  }
  emit tabOrderChanged();
}

QList<QPair<int, QString>> JointLayersControl::GetSelectedLayers() const
{
  QList<QPair<int, QString>> layers;
  const auto& widgets = mWidgets;
  std::transform(widgets.begin(), widgets.end(), std::back_inserter(layers),
    [&](const JointLayerWidget* item) { return item->SelectedLayer(); });
  return layers;
}

void JointLayersControl::SetLayersComboModel(QStandardItemModel* layersModel)
{
  mLayersModel = layersModel;
  AddLayerWidget();
  AddLayerWidget();
}

void JointLayersControl::HighlightEmpty()
{
  for(auto w : mWidgets)
  {
    if(w->SelectedLayer().first == -1)
      w->Highlight();
  }
}

void JointLayersControl::updateRemoveButtons()
{
  auto showRemoveButton = mWidgets.size() > 2;

  for(auto w : mWidgets)
  {
    w->showRemoveButton(showRemoveButton);
  }
}

void JointLayersControl::SetupUi()
{
  WidgetUi = new Ui::JointLayersControl();
  WidgetUi->setupUi(this);
  WidgetUi->addLayerPushButton->setProperty("lightweight_brand", true);
  WidgetUi->addLayerPushButton->style()->polish(WidgetUi->addLayerPushButton);
  connect(WidgetUi->addLayerPushButton, &QPushButton::clicked, this, &JointLayersControl::AddLayerWidget);
  auto wlist = JointLayersControl::tabOrderWidgets();
  for(int i = 0; i < wlist.size(); i++)
  {
    auto w = wlist[i];
    QWidget* prev = nullptr;
    if(i != 0) prev = wlist[i-1];
    setTabOrder(prev, w);
  }
}

JointLayerWidget* JointLayersControl::AddLayerWidget()
{

  const auto count = mWidgets.count();
  Q_ASSERT(mLayersModel);
  auto widget = new JointLayerWidget(mLayersModel, this, tr("Object/Layer") + QString(" %1").arg(count + 1));
  widget->setProperty("number", count);
  connect(widget, &JointLayerWidget::removeRequest, this, [this]()
  {
    if(mWidgets.count() > 2)
    {
      mWidgets.removeOne(sender());
      sender()->deleteLater();
      ResortCombos();
      updateRemoveButtons();
    }
  });
  WidgetUi->layerGridLayout->addWidget(widget, count / 2, count % 2);
  mWidgets << widget;
  auto parent = parentWidget();
  while(parent && strcmp(parent->metaObject()->className(), "PassportMainForm") != 0)
  {
    parent = parent->parentWidget();
  }
  Q_ASSERT(parent);
  for(auto& c : widget->findChildren<QComboBox*>())
  {
    c->setFocusPolicy(Qt::StrongFocus);
    c->installEventFilter(parent);
  }
  emit tabOrderChanged();
  updateRemoveButtons();
  return widget;
}

void JointLayersControl::ResortCombos()
{
  for(const auto& w : mWidgets)
    WidgetUi->layerGridLayout->removeWidget(w);
  std::sort(mWidgets.begin(), mWidgets.end(), [](JointLayerWidget* first, JointLayerWidget* second)
  {
    return first->property("number").toInt() < second->property("number").toInt();
  });

  for(int i = 0; i < mWidgets.size(); i++)
  {
    auto& w = mWidgets[i];
    w->setProperty("number", i);
    w->SetLabelText(tr("Object/Layer") + QString(" %1").arg(i + 1));
    WidgetUi->layerGridLayout->addWidget(w, i / 2, i % 2);
  }
  emit tabOrderChanged();
}


QWidgetList JointLayersControl::tabOrderWidgets()
{
  QWidgetList widgets;
  for(auto& w : mWidgets)
    widgets.append(w->tabOrderWidgets());
  widgets.append(WidgetUi->addLayerPushButton);
  return widgets;
}
