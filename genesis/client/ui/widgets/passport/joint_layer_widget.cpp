#include "joint_layer_widget.h"
#include <logic/models/passport_combo_models.h>
#include <QStandardItemModel>
#include "ui_joint_layer_widget.h"
#include <genesis_style/style.h>

JointLayerWidget::JointLayerWidget(QStandardItemModel* layersModel, QWidget* parent, const QString& labelText)
  : iTabOrderWidget(parent),
    ui(new Ui::JointLayerWidget())
{
  ui->setupUi(this);
  SetupUi(layersModel);
  ui->layerLabel->setText(labelText + Style::GetInputAlert());
}

JointLayerWidget::~JointLayerWidget()
{
  delete ui;
}

void JointLayerWidget::SetLayer(const QPair<int, QString> &layer)
{
  if(layer.first == -1)
  {
    ui->layerComboBox->setCurrentIndex(-1);
  }
  else
  {
    int ind = ui->layerComboBox->findData(layer.first, PassportModelsManager::idRole);
    if(ind == -1)
    {
      if(auto model = qobject_cast<QStandardItemModel*>(ui->layerComboBox->model()))
      {
        auto item = new QStandardItem(layer.second);
        item->setData(layer.first, PassportModelsManager::idRole);
        model->appendRow(item);
        ui->layerComboBox->setCurrentIndex(ui->layerComboBox->count()-1);
      }
    }
    else
    {
      ui->layerComboBox->setCurrentIndex(ind);
    }
  }
}

QPair<int, QString> JointLayerWidget::SelectedLayer() const
{
  if(ui->layerComboBox->currentIndex() == -1 || ui->layerComboBox->currentText().isEmpty())
    return {-1, ""};
  return {ui->layerComboBox->currentData(PassportModelsManager::idRole).toInt(), ui->layerComboBox->currentText()};
}

void JointLayerWidget::SetLabelText(const QString& text)
{
  ui->layerLabel->setText(text + Style::GetInputAlert());
}

void JointLayerWidget::Highlight()
{
  mHighlighted = true;
  ui->layerComboBox->setStyleSheet(Style::ApplySASS("QComboBox {border-color: @errorColor;}"));
  ui->layerComboBox->style()->unpolish(ui->layerComboBox);
  ui->layerComboBox->style()->polish(ui->layerComboBox);
}

void JointLayerWidget::showRemoveButton(bool show)
{
  ui->removeButton->setVisible(show);
}

void JointLayerWidget::SetupUi(QStandardItemModel* layersModel)
{
  Q_ASSERT(layersModel);
  ui->layerComboBox->setModel(layersModel);
  ui->layerComboBox->setCurrentIndex(-1);
  setTabOrder(nullptr, ui->layerComboBox);
  setTabOrder(ui->layerComboBox, nullptr);
  connect(ui->layerComboBox, &QComboBox::currentIndexChanged, this, [this](int newInd)
  {
    if(newInd != -1)
    {
      if(mHighlighted)
      {
        mHighlighted = false;
        ui->layerComboBox->setStyleSheet("");
        ui->layerComboBox->style()->unpolish(ui->layerComboBox);
        ui->layerComboBox->style()->polish(ui->layerComboBox);
      }
    }
  });
}

void JointLayerWidget::on_removeButton_clicked()
{
    emit removeRequest();
}

QWidgetList JointLayerWidget::tabOrderWidgets()
{
  return {ui->layerComboBox, ui->removeButton};
}
