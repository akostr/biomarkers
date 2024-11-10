#include "intermarkers_transfer_dialog.h"
#include "ui_intermarkers_transfer_dialog.h"
#include <QPushButton>
#include <QComboBox>
#include <QScrollBar>
#include <QDoubleSpinBox>
#include <genesis_style/style.h>
#include <logic/markup/chromatogram_data_model.h>
#include <ui/genesis_window.h>

IntermarkersTransferDialog::IntermarkersTransferDialog(GenesisMarkup::ChromatogramDataModel *model, QWidget *parent) :
  Dialogs::Templates::Dialog(parent, QDialogButtonBox::Ok | QDialogButtonBox::Cancel),
  ui(new Ui::IntermarkersTransferDialog),
  mContent(new QWidget())
{
  Dialogs::Templates::Dialog::Settings s;
  s.dialogHeader = tr("Intermarkers transfer");
  s.buttonsNames = {{QDialogButtonBox::Ok, tr("Transfer intermarkers peaks")}};
  applySettings(s);
  ui->setupUi(mContent);
  getContent()->layout()->addWidget(mContent);
  mMarkersSortedList.clear();
  auto markersList = model->getEntities({{AbstractEntityDataModel::RoleEntityType, GenesisMarkup::TypePeak},
                                         {GenesisMarkup::PeakType, GenesisMarkup::PTMarker}});
  std::sort(markersList.begin(), markersList.end(), [](const AbstractEntityDataModel::ConstDataPtr& a, const AbstractEntityDataModel::ConstDataPtr& b)->bool
  {
    return a->getData(GenesisMarkup::PeakCovatsIndex).toDouble() < b->getData(GenesisMarkup::PeakCovatsIndex).toDouble();
  });
  for(auto& m : markersList)
  {
    mMarkersSortedList.append({m->getData(GenesisMarkup::ChromatogrammEntityDataRoles::PeakTitle).toString(),
                               m->getData(GenesisMarkup::ChromatogrammEntityDataRoles::PeakCovatsIndex).toDouble()});
  }
  setupUi();
}

IntermarkersTransferDialog::~IntermarkersTransferDialog()
{
  delete ui;
}

int IntermarkersTransferDialog::getParameter()
{
  if(ui->byClosenessRadioButton->isChecked())
    return IntermarkerParameter::Value;
  else if(ui->byWindowRadioButton->isChecked())
    return IntermarkerParameter::Index;
}

QVector<GenesisMarkup::IntermarkerInterval> IntermarkersTransferDialog::getIntervals()
{
  QVector<GenesisMarkup::IntermarkerInterval> intervals;
  auto t = ui->intervalsTableWidget;
  for(int row = 0; row < t->rowCount() - 1; row++)
  {
    auto start = getStartCombo(row);
    auto end = getEndCombo(row);
    auto spin = getSpinbox(row);
    if(start->currentIndex() == end->currentIndex())
      continue;
    GenesisMarkup::IntermarkerInterval interval;
    interval.start = start->currentData().toDouble();
    interval.end = end->currentData().toDouble();
    interval.window = spin->value();
    intervals << interval;
  }
  return intervals;
}

void IntermarkersTransferDialog::setupUi()
{
  ui->label->setStyleSheet(Style::ApplySASS("QLabel {font: @defaultFontBold;}"));
  ui->label_2->setStyleSheet(Style::ApplySASS("QLabel {font: @defaultFontBold;}"));

  auto hhv = ui->intervalsTableWidget->horizontalHeader();
  hhv->setStyleSheet(Style::ApplySASS("QHeaderView::section {font: @defaultFont;}"));
  hhv->setDefaultAlignment(Qt::AlignLeft);
  hhv->setMaximumHeight(hhv->fontMetrics().height());
  hhv->setSectionResizeMode(QHeaderView::ResizeMode::Stretch);
  hhv->setSectionResizeMode(hhv->count()-1, QHeaderView::ResizeMode::Fixed);
  ui->intervalsTableWidget->setColumnWidth(hhv->count()-1, 11 + 10);
  hhv->setSectionResizeMode(0, QHeaderView::ResizeMode::ResizeToContents);
  auto vhv = ui->intervalsTableWidget->verticalHeader();
  vhv->setSectionResizeMode(QHeaderView::ResizeMode::ResizeToContents);
  auto mo = ui->intervalsTableWidget->viewport()->metaObject();

  auto insertCombo = [this](int row, int column)->QComboBox*
  {
    auto combo = new QComboBox(nullptr);
    for(auto m : mMarkersSortedList)
      combo->addItem(m.first, m.second);

    combo->setMinimumWidth(50);
    ui->intervalsTableWidget->setCellWidget(row, column, combo);
    return combo;
  };
  auto insertSpinBox = [this](int row, int column)
  {
    auto spinbox = new QDoubleSpinBox(nullptr);
    spinbox->setMinimum(0.1);
    spinbox->setMaximum(1000);
    spinbox->setValue(5);
    spinbox->setSingleStep(0.1);
    spinbox->setMinimumWidth(50);
    ui->intervalsTableWidget->setCellWidget(row, column, spinbox);
  };
  auto getNearestFreeIndex = [this]()->int
  {
    int index = 1;
    for(int row = 0; row < ui->intervalsTableWidget->rowCount()-1; row++)
    {
      auto tind = ui->intervalsTableWidget->item(row, 0)->data(Qt::UserRole).toInt();
      if(tind - index > 0)
        return index;
      index++;
    }
    return index;
  };
  auto appendRow = [getNearestFreeIndex, insertCombo, insertSpinBox, this]()
  {
    auto row = ui->intervalsTableWidget->rowCount()-1;
    int index = getNearestFreeIndex();
    ui->intervalsTableWidget->insertRow(row);
    auto startCombo = insertCombo(row, 1);
    auto endCombo = insertCombo(row, 2);
    if(row == 0)
    {
      startCombo->setCurrentIndex(0);
      endCombo->setCurrentIndex(endCombo->count()-1);
    }
    else
    {
      auto prevEndCombo = getEndCombo(row-1);
      Q_ASSERT(prevEndCombo); //WTF?!
      startCombo->setCurrentIndex(prevEndCombo->currentIndex());
      endCombo->setCurrentIndex(endCombo->count()-1);
    }
    connect(startCombo, &QComboBox::currentIndexChanged, this, [this, startCombo](int index)
    {
      int row = findRow(startCombo, 1);
      handleCombos(row, true);
      startCombo->setToolTip(QString::number(startCombo->currentData().toDouble()));
    });
    connect(endCombo, &QComboBox::currentIndexChanged, this, [this, endCombo](int index)
    {
      int row = findRow(endCombo, 2);
      handleCombos(row, false);
      endCombo->setToolTip(QString::number(endCombo->currentData().toDouble()));
    });

    insertSpinBox(row, 3);

    auto rowHeaderItem = new QTableWidgetItem(tr("Interval ") + QString::number(index));
    rowHeaderItem->setData(Qt::UserRole, index);
    ui->intervalsTableWidget->setItem(row, 0, rowHeaderItem);
    auto removeButton = new QPushButton(QIcon(":/resource/icons/icon_cross.png"), "");
    removeButton->setFlat(true);
    removeButton->setIconSize({11,11});
    removeButton->setFixedWidth(11);
    removeButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);
    connect(removeButton, &QPushButton::clicked, this, [this, removeButton]
    {
      auto row = findRow(removeButton);
      if(row != -1)
      {
        ui->intervalsTableWidget->removeRow(row);
        if(ui->intervalsTableWidget->rowCount() < 2)
        {
          emit valitationRequested();
        }
        else
        {
          if(row >= ui->intervalsTableWidget->rowCount() - 1)
          {
            row = ui->intervalsTableWidget->rowCount() - 2;
            handleCombos(row, true);
          }
          else
          {
            handleCombos(row, false);
          }
        }
      }
    });
    ui->intervalsTableWidget->setCellWidget(row, 4, removeButton);
    ui->intervalsTableWidget->resizeColumnToContents(ui->intervalsTableWidget->columnCount()-1);
  };

  auto addIntervalBtn = new QPushButton(QIcon(":/resource/icons/icon_action_add.png"), tr("Add interval"));
  addIntervalBtn->setProperty("secondary", true);
  ui->intervalsTableWidget->insertRow(0);
  ui->intervalsTableWidget->setCellWidget(0,0,addIntervalBtn);
  ui->intervalsTableWidget->setSpan(0,0,1,4);
  connect(addIntervalBtn, &QPushButton::clicked, this, [appendRow, this]()
  {
    appendRow();
    auto s = ui->intervalsTableWidget->verticalScrollBar();
    if(s)
      s->setValue(s->maximum());
    emit valitationRequested();
  });

  for(int i = 0; i < 1; i++)
  {
    appendRow();
  }
  ui->intervalsTableWidget->resizeColumnToContents(ui->intervalsTableWidget->columnCount()-1);
  connect(this, &IntermarkersTransferDialog::valitationRequested, this, &IntermarkersTransferDialog::validateInput, Qt::QueuedConnection);
}
void IntermarkersTransferDialog::handleCombos(int changedRow, bool start)
{
  auto startCombo = getStartCombo(changedRow);
  auto endCombo = getEndCombo(changedRow);
  auto prevEndCombo = getEndCombo(changedRow-1);
  auto nextStartCombo = getStartCombo(changedRow + 1);
  if(startCombo->currentIndex() > endCombo->currentIndex())
  {
    if(start)
      endCombo->setCurrentIndex(startCombo->currentIndex());
    else
      startCombo->setCurrentIndex(endCombo->currentIndex());
  }
  if(prevEndCombo)
  {
    if(prevEndCombo->currentIndex() != startCombo->currentIndex())
    {
      prevEndCombo->setCurrentIndex(startCombo->currentIndex());
      handleCombos(changedRow - 1, false);
    }
  }
  else if(startCombo->currentIndex() != 0)
  {
    startCombo->blockSignals(true);
    startCombo->setCurrentIndex(0);
    startCombo->blockSignals(false);
  }
  if(nextStartCombo)
  {
    if(endCombo->currentIndex() != nextStartCombo->currentIndex())
    {
      nextStartCombo->setCurrentIndex(endCombo->currentIndex());
      handleCombos(changedRow + 1, true);
    }
  }
  else if(endCombo->currentIndex() != endCombo->count()-1)
  {
    endCombo->blockSignals(true);
    endCombo->setCurrentIndex(endCombo->count()-1);
    endCombo->blockSignals(false);
  }
  if(!validationRequested)
  {
    validationRequested = true;
    emit valitationRequested();
  }
}

QComboBox *IntermarkersTransferDialog::getStartCombo(int row)
{
  if(row < 0 && row >= ui->intervalsTableWidget->rowCount()-1)
    return nullptr;
  return qobject_cast<QComboBox*>(ui->intervalsTableWidget->cellWidget(row, 1));
}

QComboBox *IntermarkersTransferDialog::getEndCombo(int row)
{
  if(row < 0 && row >= ui->intervalsTableWidget->rowCount()-1)
    return nullptr;
  return qobject_cast<QComboBox*>(ui->intervalsTableWidget->cellWidget(row, 2));
}

QDoubleSpinBox *IntermarkersTransferDialog::getSpinbox(int row)
{
  if(row < 0 && row >= ui->intervalsTableWidget->rowCount()-1)
    return nullptr;
  return qobject_cast<QDoubleSpinBox*>(ui->intervalsTableWidget->cellWidget(row, 3));
}

int IntermarkersTransferDialog::findRow(QPushButton *closeBtn)
{
  for(int row = 0; row < ui->intervalsTableWidget->rowCount(); row++)
  {
    if(qobject_cast<QPushButton*>(ui->intervalsTableWidget->cellWidget(row, 4)) == closeBtn)
      return row;
  }
  return -1;
}

int IntermarkersTransferDialog::findRow(QComboBox *combo, int column)
{
  for(int row = 0; row < ui->intervalsTableWidget->rowCount(); row++)
  {
    if(qobject_cast<QComboBox*>(ui->intervalsTableWidget->cellWidget(row, column)) == combo)
      return row;
  }
  return -1;
}

bool IntermarkersTransferDialog::isIntervalsValid()
{
  if(ui->intervalsTableWidget->rowCount() <= 1)
    return false;
  return true;
}

void IntermarkersTransferDialog::validateInput()
{
  validationRequested = false;
  auto b = ButtonBox->button(QDialogButtonBox::Ok);
  if(!b)
    return;
  if(!b->isEnabled() && isIntervalsValid())
    b->setEnabled(true);
  else if(b->isEnabled() && !isIntervalsValid())
    b->setEnabled(false);
}
