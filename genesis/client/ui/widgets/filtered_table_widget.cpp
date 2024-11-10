#include "filtered_table_widget.h"

#include "ui_filtered_table_widget.h"
#include "logic/tree_model_presentation.h"

FilteredTableWidget::FilteredTableWidget(QWidget* parent)
  : QWidget(parent)
{
  SetupUi();
  ConnectSignals();
}

CommonTableView* FilteredTableWidget::table() const
{
  return ui->tableView;
}

void FilteredTableWidget::SetupUi()
{
  ui = new Ui::FilteredTableWidget();
  ui->setupUi(this);
  ui->filterLineEdit->setVisible(false);
}

void FilteredTableWidget::ConnectSignals()
{
  connect(ui->filterLineEdit, &QLineEdit::textChanged, this, &FilteredTableWidget::FilterEditingFinished);
}

void FilteredTableWidget::FilterEditingFinished()
{
  const auto text = ui->filterLineEdit->text();
  if (auto presentation = qobject_cast<TreeModelPresentation*>(ui->tableView->model()))
  {
    presentation->SetRowFilter(text);
  }
}
