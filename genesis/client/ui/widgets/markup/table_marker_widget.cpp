#include "table_marker_widget.h"

#include "logic/markup/genesis_markup_enums.h"
#include "qsortfilterproxymodel.h"
#include "qspinbox.h"
#include "qstyleditemdelegate.h"
#include "ui_table_marker_widget.h"

#include "logic/markup/item_models/covats_index_model_table.h"
#include "logic/markup/markup_data_model.h"
#include <genesis_style/style.h>
#include <QItemEditorFactory>
#include <QUndoCommand>

using namespace Models;

namespace
{
  class MarkerWindowColumnDelegate : public QStyledItemDelegate
  {
  public:
    MarkerWindowColumnDelegate(QObject *parent = nullptr)
      : QStyledItemDelegate(parent){}

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
      auto spinBox = new QDoubleSpinBox(parent);
      spinBox->setMinimum(0.1);
      spinBox->setMaximum(100);
      return spinBox;
    }
    void setEditorData(QWidget *editor, const QModelIndex &index) const override
    {
      if(auto spinBox = qobject_cast<QDoubleSpinBox*>(editor))
        spinBox->setValue(index.data(Qt::EditRole).toDouble());
    }
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override
    {
      if(auto spinBox = qobject_cast<QDoubleSpinBox*>(editor))
        model->setData(index, spinBox->value());
    }
  };
}

namespace Widgets
{
  TableMarkerWidget::TableMarkerWidget(QWidget* parent)
    :IMarkupTab(parent)
  {
    ui = new Ui::TableMarkerWidget();
    ui->setupUi(this);
    setStyleSheet(Style::Genesis::GetUiStyle());
    ui->CalcIndicesButton->setProperty("secondary", true);
    mModel = new Models::CovatsIndexModelTable(this);
    connect(mModel, &Models::CovatsIndexModelTable::newCommand, this, &TableMarkerWidget::newCommand);
    mSortModel = new QSortFilterProxyModel(ui->MarkerTableView);
    mSortModel->setSourceModel(mModel);
    ui->MarkerTableView->setModel(mSortModel);
    ui->MarkerTableView->sortByColumn(1, Qt::SortOrder::AscendingOrder);
    ui->MarkerTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    connect(ui->CalcIndicesButton, &QPushButton::clicked, mModel, &Models::CovatsIndexModelTable::CalculateIndices);
    ui->MarkerTableView->setItemDelegateForColumn(3, new MarkerWindowColumnDelegate(ui->MarkerTableView));
  }

  void TableMarkerWidget::setMarkupModel(GenesisMarkup::MarkupModelPtr model)
  {
    mModel->SetModel(model);
  }

  bool TableMarkerWidget::isVisibleOnStep(GenesisMarkup::StepInfo step)
  {
    return step.guiInteractions.testFlag(GenesisMarkup::SIGMarkersTable);
  }


  void TableMarkerWidget::setModule(Names::ModulesContextTags::Module module)
  {
    switch(module)
    {
    case Names::ModulesContextTags::MNoModule:
    case Names::ModulesContextTags::MReservoir:
    case Names::ModulesContextTags::MPlots:
    case Names::ModulesContextTags::MLast:
    default:
      ui->CalcIndicesButton->show();
      ui->MarkerTableView->showColumn(2);
      break;
    case Names::ModulesContextTags::MBiomarkers:
      ui->CalcIndicesButton->hide();
      ui->MarkerTableView->hideColumn(2);
      break;
    }
  }
}

