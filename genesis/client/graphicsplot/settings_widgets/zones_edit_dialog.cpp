#include "zones_edit_dialog.h"
#include "ui_zones_edit_dialog.h"

#include <graphicsplot/graphicsplot_extended.h>

#ifndef DISABLE_STANDARD_VIEWS
#include <standard_views/standard_item_model_controller.h>
#endif // ~!DISABLE_STANDARD_VIEWS

#if defined(USE_CUSTOM_WINDOWS)
#include <custom_windows/custom_windows.h>
#endif

#ifndef DISABLE_STANDARD_VIEWS
ZonesTableModel::ZonesTableModel(QObject* parent, const QString& dateTimeFormat)
  : QStandardItemModel(parent)
  , DateTimeFormat(dateTimeFormat)
{
  StandardItemModel::ModelInformation modelInformation;
  modelInformation.ColumnKeys = {
    { ColumnName        , "ColumnName" },
    { ColumnPosLeft     , "ColumnPosLeft" },
    { ColumnPosRight    , "ColumnPosRight" },
    { ColumnVisible     , "ColumnVisible" },
    { ColumnColor       , "ColumnColor" },
  };
  StandardItemModel::SetModelInformation(this, modelInformation);
}

QVariant ZonesTableModel::data(const QModelIndex &index, int role) const
{
  if (index.column() == ColumnPosLeft || index.column() == ColumnPosRight)
  {
    if (role == Qt::DisplayRole || role == Qt::EditRole)
    {
      double v = QStandardItemModel::data(index, role).toDouble();
      if (DateTimeFormat.isEmpty())
        return v;
      if (DateTimeFormat == GPAxisAwareTicker::MinutesFormat)
        return v / 1000.0 / 60.0;
      if (DateTimeFormat == GPAxisAwareTicker::SecondsFormat)
        return v / 1000.0;
      else
        return QDateTime::fromMSecsSinceEpoch(v);
    }
    else if (role == Qt::UserRole)
    {
      return QStandardItemModel::data(index, Qt::EditRole).toDouble();
    }
  }
  return QStandardItemModel::data(index, role);
}

bool ZonesTableModel::setData(const QModelIndex &idx, const QVariant &value, int role)
{
  QVariant v = value;
  if (idx.column() == ColumnPosLeft || idx.column() == ColumnPosRight)
  {
    if (role == Qt::DisplayRole || role == Qt::EditRole || role == Qt::UserRole)
    {
      if (role == Qt::UserRole)
      {
        QStandardItemModel::setData(idx, v, Qt::EditRole);
      }
      else
      {
        if (DateTimeFormat == GPAxisAwareTicker::MinutesFormat)
          v = v.toDouble() * 1000.0 * 60.0;
        else if (DateTimeFormat == GPAxisAwareTicker::SecondsFormat)
          v = v.toDouble() * 1000.0;
        else
          v = v.toDateTime().toMSecsSinceEpoch();
        QStandardItemModel::setData(idx, v, Qt::EditRole);
      }

      if (idx.column() == ColumnPosLeft && idx.row() != 0)
        QStandardItemModel::setData(index(idx.row() - 1, ColumnPosRight), v, Qt::EditRole);
      if (idx.column() == ColumnPosRight && idx.row() != rowCount() - 1)
        QStandardItemModel::setData(index(idx.row() + 1, ColumnPosLeft), v, Qt::EditRole);
      emit dataChanged(index(0, ColumnPosLeft), index(rowCount() - 1, ColumnPosRight));
      return true;
    }
  }
  return QStandardItemModel::setData(idx, v, role);
}



ZonesEditDialog::ZonesEditDialog(GraphicsPlotExtended *parent, QPointer<GPZonesRect> zonesRect)
  : QDialog(parent)
  , ParentPlot(parent)
  , Ui(new Ui::ZonesEditDialog)
  , ZonesRect(zonesRect)
{
  Ui->setupUi(this);
  setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint);
  setModal(true);

  QString dateTimeFormat;
  if (ZonesRect && ZonesRect->getKeyAxis())
    if (auto awt = qobject_cast<GPAxisAwareTicker*>(ZonesRect->getKeyAxis()->ticker().data()))
      dateTimeFormat = awt->dateTimeFormat();
  ZonesModel = new ZonesTableModel(this, dateTimeFormat);

  SetupUi();
  UpdateUi();
  MakeConnects();
}

ZonesEditDialog::~ZonesEditDialog()
{
  delete Ui;
}

void ZonesEditDialog::accept()
{
  Apply();
  QDialog::accept();
}

void ZonesEditDialog::SetupUi()
{
  Ui->tZones->setModel(ZonesModel);
  Ui->tZones->horizontalHeader()->setStretchLastSection(true);
  Ui->tZones->setSelectionBehavior(QAbstractItemView::SelectRows);

  auto controller = new StandardItemModelController(this, ZonesModel, Ui->tZones);
  controller->SetFeatureFlags(StandardItemModelController::FeatureSelection, controller->GetFeatureFlags(StandardItemModelController::FeatureSelection)
                              | StandardItemModelController::SelectionAddRemoveRows);

  ZonesModel->setHorizontalHeaderLabels({tr("Name"), tr("Left"), tr("Right"), tr("Visible"), tr("Color")});

  Ui->cbTextPos->addItem(tr("Center"), (int)Qt::AlignVCenter);
  Ui->cbTextPos->addItem(tr("Top"), (int)Qt::AlignTop);
  Ui->cbTextPos->addItem(tr("Bottom"), (int)Qt::AlignBottom);
  Ui->cbTextPos->setCurrentIndex(0);

  Ui->tabWidget->setCurrentIndex(0);
}

void ZonesEditDialog::UpdateUi()
{
  Ui->fontSettings->SetSettings(FontSettings(ZonesRect));
  Ui->cbTextPos->setCurrentIndex(Ui->cbTextPos->findData(ZonesRect->getStyle().textAlignment));

  if (auto data = ZonesRect->data())
  {
    auto lines = data->getLines();
    auto brushes = ZonesRect->getStyle().zonesBrush;
    for (int i = 0; i < data->size() - 1; ++i)
    {
      auto name = new QStandardItem(data->at(i).mName);

      auto visible = new QStandardItem();
      visible->setFlags(visible->flags() | Qt::ItemIsEditable | Qt::ItemIsUserCheckable);
      visible->setData(data->at(i).mVisible ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole);

      auto color = new QStandardItem();
      auto c = brushes.value(i).color();
      c.setAlpha(255);
      color->setBackground(c);

      ZonesModel->appendRow({name, new QStandardItem(), new QStandardItem(), visible, color});
    }
    SetZonesPos(lines);
  }
}

void ZonesEditDialog::MakeConnects()
{
  connect(Ui->buttonBox, &QDialogButtonBox::clicked, this, [this](QAbstractButton *button) {
    if (Ui->buttonBox->buttonRole(button) == QDialogButtonBox::ApplyRole)
      Apply();
  });
  connect(Ui->tZones, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(OnColorClicked(QModelIndex)));

  connect(ZonesModel, SIGNAL(rowsInserted(QModelIndex,int,int)), this, SLOT(OnInsertRows(QModelIndex,int,int)));
  connect(ZonesModel, SIGNAL(rowsRemoved(QModelIndex,int,int)), this, SLOT(OnRemoveRows(QModelIndex,int,int)));
}

QVector<double> ZonesEditDialog::UpdateZonesLines()
{
  QVector<double> lines;
  if (auto data = ZonesRect->data())
  {
    lines = data->getLines();
    if (lines.size() != ZonesModel->rowCount() + 1)
    {
      double start = lines.value(0, 1);
      double delta = (lines.value(lines.size() - 1, 1) - lines.value(0, 0)) / ZonesModel->rowCount();
      lines.clear();

      for (int i = 0; i < ZonesModel->rowCount(); ++i)
        lines.append(start + delta * i);
      lines.append(start + delta * ZonesModel->rowCount());
    }
  }
  return lines;
}

void ZonesEditDialog::Apply()
{
  QVector<double> lines;
  QVector<QString> names;
  QVector<bool> visibles;
  QList<QBrush> colors;

  for (int i = 0; i < ZonesModel->rowCount(); ++i)
  {
    names.append(ZonesModel->data(ZonesModel->index(i, ZonesTableModel::ColumnName), Qt::EditRole).toString());
    visibles.append(ZonesModel->item(i, ZonesTableModel::ColumnVisible)->checkState() == Qt::Checked);

    lines.append(ZonesModel->data(ZonesModel->index(i, ZonesTableModel::ColumnPosLeft), Qt::UserRole).toDouble());

    auto c = ZonesModel->item(i, ZonesTableModel::ColumnColor)->background().color();
    c.setAlpha(20);
    colors.append(c);
  }
  lines.append(ZonesModel->data(ZonesModel->index(ZonesModel->rowCount() - 1, ZonesTableModel::ColumnPosRight), Qt::UserRole).toDouble());

  qSort(lines);

  if (auto data = ZonesRect->data())
  {
    data->setLines(lines);
    data->setNames(names);
    data->setVisibility(visibles);
  }
  auto style = ZonesRect->getStyle();
  style.zonesBrush = colors;
  style.textAlignment = (Qt::AlignmentFlag)Ui->cbTextPos->currentData().toInt();
  ZonesRect->setStyle(style);

  Ui->fontSettings->GetSettings().Apply(ZonesRect);

  ParentPlot->SaveSettings();
  if (ParentPlot->ZonesRect)
    ParentPlot->ZonesRect->updateZonesAmount();
  ParentPlot->queuedReplot();
}

void ZonesEditDialog::OnColorClicked(QModelIndex index)
{
  if (index.isValid() && index.column() == ZonesTableModel::ColumnColor)
  {
    auto item = ZonesModel->item(index.row(), index.column());
    if (item)
    {
      QColor newColor;

      QColorDialog* dialog = new QColorDialog(item->background().color(), this);
      auto colorMap = GraphicsPlotExtendedStyle::GetDefaultColorMap();
      for (auto it = colorMap.begin(); it != colorMap.end(); ++it)
        dialog->setCustomColor(it.key(), it.value());

#if defined(USE_CUSTOM_WINDOWS)
      CustomWindows::CustomNativeQtDialog window(this, dialog);
      if (window.exec() == QDialog::Accepted)
#else
      if (dialog->exec() == QDialog::Accepted)
#endif
        newColor = dialog->currentColor();

#if !defined(USE_CUSTOM_WINDOWS)
      delete dialog;
#endif

      item->setBackground(newColor);
    }
  }
}

void ZonesEditDialog::OnInsertRows(const QModelIndex& /*parent*/, int first, int last)
{
  for (int r = first; r <= last; ++r)
  {
    ZonesModel->setItem(r, ZonesTableModel::ColumnName, new QStandardItem());

    ZonesModel->setItem(r, ZonesTableModel::ColumnPosLeft, new QStandardItem());
    ZonesModel->setItem(r, ZonesTableModel::ColumnPosRight, new QStandardItem());


    auto visible = new QStandardItem();
    visible->setFlags(visible->flags() | Qt::ItemIsEditable | Qt::ItemIsUserCheckable);
    visible->setData(Qt::Checked, Qt::CheckStateRole);
    ZonesModel->setItem(r, ZonesTableModel::ColumnVisible, visible);

    auto color = new QStandardItem();
    const auto &brushes = ZonesRect->getStyle().zonesBrush;
    auto c = brushes.value((ZonesModel->rowCount() + 1) % brushes.size()).color();
    c.setAlpha(255);
    color->setBackground(c);
    ZonesModel->setItem(r, ZonesTableModel::ColumnColor, color);
  }
  SetZonesPos(UpdateZonesLines());
}

void ZonesEditDialog::OnRemoveRows(const QModelIndex& /*parent*/, int /*first*/, int /*last*/)
{
  SetZonesPos(UpdateZonesLines());
}

void ZonesEditDialog::SetZonesPos(const QVector<double> &positions)
{
  for (int i = 0; i < positions.size() - 1; ++i)
  {
    ZonesModel->setData(ZonesModel->index(i, ZonesTableModel::ColumnPosLeft), positions[i], Qt::UserRole);
    ZonesModel->setData(ZonesModel->index(i, ZonesTableModel::ColumnPosRight), positions[i + 1], Qt::UserRole);
  }
}

#endif // ~!DISABLE_STANDARD_VIEWS