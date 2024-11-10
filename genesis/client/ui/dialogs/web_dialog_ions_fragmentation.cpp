#include "web_dialog_ions_fragmentation.h"
#include "ui_web_dialog_ions_fragmentation.h"
#include <ui/flow_layout.h>
#include <ui/controls/range_widget.h>
#include <genesis_style/style.h>

#include <QFormLayout>
namespace Dialogs
{
WebDialogIonsFragmentation::WebDialogIonsFragmentation(QWidget *parent) :
  Templates::Dialog(parent, Btns::Ok | Btns::Cancel),
  ui(new Ui::WebDialogIonsFragmentation),
  m_ionsModel(new IonsListModel()),
  m_ionsRange(0,0)
{
  Size = QSizeF(0.4,0);
  setupUi();
  auto s = Templates::Dialog::Settings();
  s.dialogHeader = tr("Ions fragmentation");
  applySettings(s);
  checkDataValidity();
}

WebDialogIonsFragmentation::~WebDialogIonsFragmentation()
{
  delete ui;
}

void WebDialogIonsFragmentation::setIons(const QList<int> &ions)
{
  if(m_ionsModel)
    m_ionsModel->setIons(ions);
  int min = ions.first();
  int max = min;
  for(auto& ion : ions)
  {
    if(ion > max) max = ion;
    if(ion < min) min = ion;
  }
  m_ionsRange = {min, max};
}

QList<int> WebDialogIonsFragmentation::getIons()
{
  auto checkedIons = m_ionsModel->getCheckedIons();
  for(auto& range : m_ranges)
  {
    for(int ionValue = range.first; ionValue <= range.second; ionValue++)
      checkedIons << ionValue;
  }
  return checkedIons;
}

bool WebDialogIonsFragmentation::getSum()
{
  if(ui->sumGenerationCheckBox->isEnabled())
    return ui->sumGenerationCheckBox->isChecked();
  else
    return false;
}

void WebDialogIonsFragmentation::addRange()
{
  ui->noRangesLabel->setVisible(false);
  ui->rangesFormFrame->setVisible(true);
  auto newRangeWidget = new RangeWidget();
  newRangeWidget->setBounds(m_ionsRange.first, m_ionsRange.second);
  connect(newRangeWidget, &RangeWidget::closed, this, &WebDialogIonsFragmentation::onRangeClosed);

  auto count = m_rangesFormLayout->count();
  m_ranges[newRangeWidget] = {newRangeWidget->lowerBound(), newRangeWidget->upperBound()};
  connect(newRangeWidget, &RangeWidget::lowerBoundChanged, this, [this, newRangeWidget](int v){m_ranges[newRangeWidget].first = v;});
  connect(newRangeWidget, &RangeWidget::upperBoundChanged, this, [this, newRangeWidget](int v){m_ranges[newRangeWidget].second = v;});

  if(count != 0)
  {
    auto existedWidget = static_cast<RangeWidget*>(m_rangesFormLayout->itemAt(count - 1)->widget());
    if(existedWidget)
    {
      newRangeWidget->setBounds(existedWidget->upperBound() + 1, m_ionsRange.second);
      connect(existedWidget, &RangeWidget::upperBoundChanged, newRangeWidget, [newRangeWidget, this](int newUpperBound)
      {
        newRangeWidget->setBounds(newUpperBound + 1, m_ionsRange.second);
      });
    }
  }
  m_rangesFormLayout->addRow(tr("Range") + QString(" %1").arg(count + 1), newRangeWidget);
  checkDataValidity();
}

void WebDialogIonsFragmentation::onRangeClosed(RangeWidget *selfPointer)
{
  m_rangesFormLayout->removeRow(selfPointer);
  m_ranges.remove(selfPointer);
  if(m_rangesFormLayout->count() == 0)
  {
    ui->rangesFormFrame->setVisible(false);
    ui->noRangesLabel->setVisible(true);
  }
  checkDataValidity();
}

void WebDialogIonsFragmentation::invalidateFlowItems(bool toModel)
{
//  qDebug() << "invalidate(" << toModel << ")";
  if(toModel)
  {//from layout to model data invalidation
    QList<int> checkedIons;
    for(auto& ionPlateWgt : m_flowItemsMap)
      checkedIons << ionPlateWgt->property("ion").toInt();

    m_ionsModel->setIonsChecked(checkedIons);
  }
  else
  {//from model to layout data invalidation
    auto checkedIons = m_ionsModel->getCheckedIons();
    QWidgetList toRemove;
    QList<int> keysToRemove;
    for(auto& ionPlateWgt : m_flowItemsMap)
    {
      int ionValue = ionPlateWgt->property("ion").toInt();
      if(!checkedIons.contains(ionValue))
      {
        toRemove << ionPlateWgt;
        keysToRemove << ionValue;
        continue;
      }
    }

    for(auto& key : keysToRemove)
      m_flowItemsMap.remove(key);
    for(auto& wgt : toRemove)
      wgt->deleteLater();

    for(auto& ion : checkedIons)
    {
      if(!m_flowItemsMap.contains(ion))
      {
        m_flowItemsMap[ion] = createPlate(ion);
        m_flowLayout->addWidget(m_flowItemsMap[ion]);
      }
    }
  }
  checkDataValidity();
}

void WebDialogIonsFragmentation::checkDataValidity()
{
  if(m_ranges.size() > 0 || m_flowItemsMap.size() > 0)
    ButtonBox->button(Btns::Ok)->setEnabled(true);
  else
    ButtonBox->button(Btns::Ok)->setEnabled(false);
  if(m_ranges.empty() && m_flowItemsMap.size() == 1)
    ui->sumGenerationCheckBox->setEnabled(false);
  else if(!ui->sumGenerationCheckBox->isEnabled())
    ui->sumGenerationCheckBox->setEnabled(true);
}

void WebDialogIonsFragmentation::setupUi()
{
  auto content = getContent();
  auto body = new QWidget(content);
  body->setStyleSheet(Style::Genesis::GetUiStyle());
  ui->setupUi(body);
  content->layout()->addWidget(body);

  ui->ionsRangeCaption->setStyleSheet(Style::Genesis::GetH3());
  ui->parametersTabCaption->setStyleSheet(Style::Genesis::GetH3());
  ui->pickingTabCaption->setStyleSheet(Style::Genesis::GetH3());
  ui->rangesScrollArea->setSizeAdjustPolicy(QScrollArea::AdjustToContents);
  m_rangesFormLayout = static_cast<QFormLayout*>(ui->rangesFormFrame->layout());
//  ui->rangesFormFrame->setLayout(m_rangesFormLayout);
  ui->rangesFormFrame->setVisible(false);
  m_flowLayout = new FlowLayout();
  ui->flowLayoutFrame->setLayout(m_flowLayout);
  ui->ionsListView->setModel(m_ionsModel);
  connect(ui->addRangeButton, &QPushButton::clicked, this, &WebDialogIonsFragmentation::addRange);
  connect(m_ionsModel, &IonsListModel::dataChanged, this,
  [this](const QModelIndex &topLeft,
         const QModelIndex &bottomRight,
         const QList<int> &roles = QList<int>())
  {
    if(roles.contains(Qt::CheckStateRole))
      invalidateFlowItems(false);
  });
  ButtonBox->button(Btns::Ok)->setText(tr("Extract"));
}

QWidget *WebDialogIonsFragmentation::createPlate(int ion)
{
  auto plate = new QWidget();
  plate->setProperty("ion", ion);
  auto layout = new QHBoxLayout();
  plate->setLayout(layout);
  layout->addWidget(new QLabel(QString::number(ion)));
  auto closeButton = new QPushButton();
  layout->addWidget(closeButton);
  closeButton->setIcon(QIcon("://resource/icons/icon_action_cross.png"));
  closeButton->setMinimumSize(21,21);
  closeButton->setMaximumSize(21,21);
  closeButton->setFlat(true);
  connect(closeButton, &QPushButton::clicked, this, [this, plate]()
  {
    m_flowItemsMap.remove(plate->property("ion").toInt());
    plate->deleteLater();
    plate->close();
    invalidateFlowItems(true);
  });
  return plate;
}
}//namespace Dialogs

IonsListModel::IonsListModel(QObject *parent) : QStandardItemModel(parent)
{
  setColumnCount(1);
}


Qt::ItemFlags IonsListModel::flags(const QModelIndex &index) const
{
  Qt::ItemFlags f = QStandardItemModel::flags(index);
  f.setFlag(Qt::ItemIsUserCheckable, true);
  return f;
}


QVariant IonsListModel::data(const QModelIndex &index, int role) const
{
  if(role == Qt::CheckStateRole)
  {
    if(QStandardItemModel::data(index, role).isValid())
      return QStandardItemModel::data(index, role);
    else
      return Qt::Unchecked;
  }
  else if(role == Qt::SizeHintRole)
  {
    return QSize(200, 21);
  }
  return QStandardItemModel::data(index, role);
}

void IonsListModel::setIons(const QList<int> &ions)
{
  beginResetModel();
  m_ionsItemMap.clear();
  clear();
  for(auto& ion : ions)
  {
    auto item = new QStandardItem();
    item->setData(QString::number(ion), Qt::DisplayRole);
    item->setData(ion, Qt::EditRole);
    item->setData(false, Qt::CheckStateRole);
    m_ionsItemMap[ion] = item;
    appendRow(item);
  }
  endResetModel();
}

QList<int> IonsListModel::getCheckedIons() const
{
  QList<int> checked;
  for(int r = 0; r < rowCount(); r++)
  {
    auto ind = index(r, 0);
    if(ind.data(Qt::CheckStateRole).value<Qt::CheckState>() == Qt::Checked)
      checked << ind.data(Qt::EditRole).toInt();
  }
  return checked;
}

void IonsListModel::setIonsChecked(const QList<int> &ions)
{
  beginResetModel();
  for(auto& ion : ions)
  {
    if(m_ionsItemMap.contains(ion))
      m_ionsItemMap[ion]->setData(Qt::Checked, Qt::CheckStateRole);
  }
  for(auto& ion : m_ionsItemMap.keys())
  {
    if(!ions.contains(ion))
      m_ionsItemMap[ion]->setData(Qt::Unchecked, Qt::CheckStateRole);
  }
  endResetModel();
}
