#include "web_dialog_concentration_table.h"

#include <QLayout>
#include <QLabel>
#include <QPushButton>

#include "ui/widgets/table_view_widget.h"
#include "ui/item_models/concentration_table_model.h"
#include "genesis_style/style.h"

using namespace Structures;
using namespace Models;
using namespace Widgets;

namespace Dialogs
{
  WebDialogConcentrationTable::WebDialogConcentrationTable(QWidget* parent)
    : WebDialog(parent, nullptr, QSizeF(0.5, 0.5))
  {
    const auto body = new QWidget(Content);
    Content->layout()->addWidget(body);
    body->setStyleSheet(Style::Genesis::GetUiStyle());

    const auto bodyLayout = new QVBoxLayout(body);
    bodyLayout->setContentsMargins(Style::Scale(40), Style::Scale(32), Style::Scale(40), 0);
    bodyLayout->setSpacing(0);

    const auto label = new QLabel(tr("Concentration table"), body);
    label->setStyleSheet(Style::Genesis::GetH2());
    bodyLayout->addWidget(label);
    const auto tableWidget = new TableViewWidget(body);
    const auto model = new ConcentrationTableModel(body);
    tableWidget->SetModel(model);
    bodyLayout->addWidget(tableWidget);
    Size = QSizeF(0.6, 0.7);
    if (const auto okButton = ButtonBox->button(QDialogButtonBox::Ok))
      okButton->setText(tr("Close"));
  }

  void WebDialogConcentrationTable::SetConcentrationTable(ConcetrantionTable&& table)
  {
    if (const auto tableWidget = findChild<TableViewWidget*>())
    {
      if (const auto model = qobject_cast<ConcentrationTableModel*>(tableWidget->Model()))
        model->SetConcentrationTable(std::forward<ConcetrantionTable>(table));
      tableWidget->SetResizeMode(QHeaderView::ResizeToContents);
      tableWidget->SetStretchLastSection(true);
    }
  }
}
