#include "web_dialog_pick_axis_components.h"

#include "ui_web_dialog_pick_axis_components.h"

#include "genesis_style/style.h"

#include <QStandardItemModel>


enum DataRole
{
  FilterByVarianceRole = Qt::UserRole + 1,
  NumberRole = Qt::UserRole + 2
};

using Buttons = QDialogButtonBox::StandardButton;

WebDialogPickAxisComponents::WebDialogPickAxisComponents(QWidget* parent, const QList<double>& explPCsVariance)
  : WebDialog(parent, nullptr, QSize(0.2, 0.1), Buttons::Ok | Buttons::Cancel)
{
  SetupUi(explPCsVariance);
  connect(Ui->CloseButton, &QPushButton::clicked, this, &WebDialog::Reject);
}

QPair<int, int> WebDialogPickAxisComponents::GetComponents()
{
  return { Ui->HorizontalAxisComboBox->currentData(NumberRole).toInt(),
        Ui->VerticalAxisComboBox->currentData(NumberRole).toInt() };
}

void WebDialogPickAxisComponents::setCurrentXComponent(int index)
{
  Ui->HorizontalAxisComboBox->setCurrentIndex(index);
}

void WebDialogPickAxisComponents::setCurrentYComponent(int index)
{
  Ui->VerticalAxisComboBox->setCurrentIndex(index);
}

void WebDialogPickAxisComponents::SetupUi(const QList<double>& explPCsVariance)
{
  const auto body = new QWidget();
  body->setStyleSheet(Style::Genesis::GetUiStyle());
  Ui = new Ui::WebDialogPickAxisComponents();
  Ui->setupUi(body);
  Ui->CaptionLabel->setStyleSheet(Style::Genesis::Fonts::ModalHead());

  auto model = new QStandardItemModel(this);
  Ui->HorizontalAxisComboBox->setModel(model);
  Ui->VerticalAxisComboBox->setModel(model);

  for (int i = 0; i < explPCsVariance.size(); i++)
  {
    auto item = new QStandardItem();
    item->setData(tr("PC%1 (%2%)").arg(i + 1).arg(explPCsVariance[i]), Qt::DisplayRole);
    item->setData(i, NumberRole);
    model->appendRow(item);
  }
  ButtonBox->button(QDialogButtonBox::Ok)->setText(tr("Apply"));
  if (const auto layout = Content->layout())
  {
    layout->addWidget(body);
  }
}
