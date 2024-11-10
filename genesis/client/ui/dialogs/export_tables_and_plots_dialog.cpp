#include "export_tables_and_plots_dialog.h"
#include "ui_export_tables_and_plots_dialog.h"

#include <QLayout>
#include <QPushButton>

using Btns = QDialogButtonBox::StandardButton;

ExportTreeModel::ExportTreeModel(QObject* parent) : QStandardItemModel(parent)
{
}

ExportTreeModel::~ExportTreeModel()
{
}

QVariant ExportTreeModel::data(const QModelIndex& index, int role) const
{
  if (index.isValid())
  {
    switch (role)
    {
    case Qt::DisplayRole:
    {
      auto str = QStandardItemModel::data(index, Qt::DisplayRole).toString();
      size_t count = 0;
      size_t lastSpace = 0;
      for (size_t i = 0; i < str.size(); i++)
      {
        count++;
        if (str[i] == ' ')
        {
          lastSpace = i;
        }
        if (count == 40)
        {
          count = 0;
          str[lastSpace] = '\n';
        }
      }
      return str;
    }

    default:
      break;
    }
  }
  return QStandardItemModel::data(index, role);
}

Qt::ItemFlags ExportTreeModel::flags(const QModelIndex& index) const
{
  if (index.column() == 1)
    return Qt::NoItemFlags;
  Qt::ItemFlags f = QAbstractItemModel::flags(index);
  f |= Qt::ItemIsUserCheckable;
  return f;
}


ExportTablesAndPlotsDialog::ExportTablesAndPlotsDialog(const ItemSelected& tables,
  const ItemSelected& plots, QWidget* parent)
  : Dialogs::Templates::Dialog(parent, Btns::Ok | Btns::Cancel)
  , ui(new Ui::ExportTablesAndPlotsDialog)
  , mContent(new QWidget(nullptr))
  , mTables(tables)
  , mPlots(plots)
{
  setupUi();
}

ExportTablesAndPlotsDialog::~ExportTablesAndPlotsDialog()
{
}

void ExportTablesAndPlotsDialog::setupUi()
{
  Dialogs::Templates::Dialog::Settings s;
  s.dialogHeader = tr("Export tables and plots");
  s.buttonsNames = { {QDialogButtonBox::Ok, tr("Export")},
                   {QDialogButtonBox::Cancel, tr("Cancel")} };
  applySettings(s);

  ui->setupUi(mContent);
  getContent()->layout()->addWidget(mContent);
  ui->treeView->header()->setVisible(false);

  mModel = new ExportTreeModel(this);
  {
    auto item = new QStandardItem(tr("Tables"));
    item->setCheckable(true);
    item->setCheckState(Qt::CheckState::Checked);
    mModel->appendRow(item);

    for (const auto& [name, checked] : mTables)
    {
      auto child = new QStandardItem(name);
      child->setCheckable(true);
      child->setCheckState(checked);
      item->setChild(item->rowCount(), child);
    }
  }

  {
    auto item = new QStandardItem(tr("Plots"));
    item->setCheckable(true);
    item->setCheckState(Qt::CheckState::Checked);
    mModel->appendRow(item);

    for (const auto& [name, checked] : mPlots)
    {
      auto child = new QStandardItem(name);
      child->setCheckable(true);
      child->setCheckState(checked);
      item->setChild(item->rowCount(), child);
    }
  }

  ui->treeView->setModel(mModel);
  ui->treeView->expandAll();
  ui->treeView->setStyleSheet(
    "QTreeView::item {"
    " border: none;"
    " padding: 1px;"
    "}"
    "QTreeView::item:selected:active{"
    " background: transparent;"
    "}"
    "QTreeView::branch {"
    " border: none;"
    "}"
    "QTreeView {"
    " border: none;"
    "}");

  connect(mModel, &ExportTreeModel::itemChanged, [&](QStandardItem* item)
  {
    auto parent = item->parent();
    auto state = item->checkState();

    if (parent)
    {
      auto setStateParent = [&]
      {
        for (size_t i = 0; i < parent->rowCount(); i++)
        {
          if (parent->child(i)->checkState() != state)
          {
            state = Qt::PartiallyChecked;
            break;
          }
        }

        mModel->blockSignals(true);
        parent->setCheckState(state);
        mModel->blockSignals(false);
        mModel->layoutChanged();
      };

      if (parent->row() == 0) // tables
      {
        mTables[item->row()].second = state;
        setStateParent();
      }
      else // plots
      {
        mPlots[item->row()].second = state;
        setStateParent();
      }
    }
    else // parent, change all children
    {
      if (item->checkState() == Qt::Checked)
      {
        for (size_t i = 0; i < item->rowCount(); i++)
          item->child(i)->setCheckState(Qt::Checked);
      }
      else if (item->checkState() == Qt::Unchecked)
      {
        for (size_t i = 0; i < item->rowCount(); i++)
          item->child(i)->setCheckState(Qt::Unchecked);
      }
    }

    // set show export button
    bool showExportButton = false;
    for (const auto& [name, state] : mTables)
    {
      if (state == Qt::Checked)
      {
        showExportButton = true;
        break;
      }
    }

    if (!showExportButton)
    {
      for (const auto& [name, state] : mPlots)
      {
        if (state == Qt::Checked)
        {
          showExportButton = true;
          break;
        }
      }
    }

    if (auto ok = ButtonBox->button(QDialogButtonBox::Ok))
      ok->setEnabled(showExportButton);
  });
}

QVector<QPair<QString, Qt::CheckState>> ExportTablesAndPlotsDialog::Tables() const
{
  return mTables;
}

ItemSelected ExportTablesAndPlotsDialog::Plots() const
{
  return mPlots;
}
