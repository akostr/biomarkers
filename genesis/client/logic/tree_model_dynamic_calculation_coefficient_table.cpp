#include "tree_model_dynamic_calculation_coefficient_table.h"

#include "logic/tree_model_item.h"
#include "logic/tree_model_presentation_library_compounds.h"
#include "api/api_rest.h"
#include "notification.h"

#include <QPixmap>
#include <QRegExp>

namespace
{
  QString FavoriteIconPath(":/resource/icons/compounds/icon_favorite.png");
  QString FavoriteIconCheckedPath(":/resource/icons/compounds/icon_favorite_yellow.png");
  QString CompoundTypePath(":/resource/icons/compounds/icon_users.png");
  QString CheckIconPath(":/resource/icons/compounds/icon_check.png");
  const QRegExp RegShortTitle("#\\[id_\\d+\\]|#\\d+|@|#id_\\d+");
}

TreeModelDynamicCalculationCoefficientTable::TreeModelDynamicCalculationCoefficientTable(int tableId, QObject* parent)
  : TreeModelDynamic(parent), TableId(tableId)
{
  //// Setup columns
  Columns[static_cast<int>(Column::CHECKED)].HeaderData[Qt::DisplayRole] = "";

  Columns[static_cast<int>(Column::LIBRARY_GROUP_ID)].Ids[Qt::EditRole] = "library_group_id";
  Columns[static_cast<int>(Column::LIBRARY_GROUP_ID)].Ids[Qt::DisplayRole] = "library_group_id";
  Columns[static_cast<int>(Column::LIBRARY_GROUP_ID)].HeaderData[Qt::DisplayRole] = tr("Library group id");

  Columns[Column::GROUP].Ids[Qt::EditRole] = "group";
  Columns[Column::GROUP].Ids[Qt::DisplayRole] = "group";
  Columns[Column::GROUP].HeaderData[Qt::DisplayRole] = tr("Group");

  Columns[Column::LIBRARY_ELEMENT_ID].Ids[Qt::EditRole] = "library_element_id";
  Columns[Column::LIBRARY_ELEMENT_ID].Ids[Qt::DisplayRole] = "library_element_id";
  Columns[Column::LIBRARY_ELEMENT_ID].HeaderData[Qt::DisplayRole] = tr("Library element id");

  Columns[Column::SHORT_TITLE].Ids[Qt::EditRole] = "short_title";
  Columns[Column::SHORT_TITLE].Ids[Qt::DisplayRole] = "short_title";
  Columns[Column::SHORT_TITLE].HeaderData[Qt::DisplayRole] = tr("Short title");

  Columns[Column::FULL_TITLE].Ids[Qt::EditRole] = "full_title";
  Columns[Column::FULL_TITLE].Ids[Qt::DisplayRole] = "full_title";
  Columns[Column::FULL_TITLE].HeaderData[Qt::DisplayRole] = tr("Full title");

  Columns[Column::CLASSIFIER_ID].Ids[Qt::EditRole] = "classifier_id";
  Columns[Column::CLASSIFIER_ID].Ids[Qt::DisplayRole] = "classifier_id";
  Columns[Column::CLASSIFIER_ID].HeaderData[Qt::DisplayRole] = tr("Classifier id");

  Columns[Column::SPECIFIC_ID].Ids[Qt::EditRole] = "specific_id";
  Columns[Column::SPECIFIC_ID].Ids[Qt::DisplayRole] = "specific_id";
  Columns[Column::SPECIFIC_ID].HeaderData[Qt::DisplayRole] = tr("Specific id");

  Columns[Column::SPECIFIC].Ids[Qt::EditRole] = "specific_name";
  Columns[Column::SPECIFIC].Ids[Qt::DisplayRole] = "specific_name";
  Columns[Column::SPECIFIC].HeaderData[Qt::DisplayRole] = tr("Specific");

  Columns[Column::FORMULA].Ids[Qt::EditRole] = "coefficient_formula";
  Columns[Column::FORMULA].Ids[Qt::DisplayRole] = "coefficient_formula";
  Columns[Column::FORMULA].HeaderData[Qt::DisplayRole] = tr("Formula");

  Columns[Column::CLASSIFIER_TITLE].Ids[Qt::EditRole] = "classifier_title";
  Columns[Column::CLASSIFIER_TITLE].Ids[Qt::DisplayRole] = "classifier_title";
  Columns[Column::CLASSIFIER_TITLE].HeaderData[Qt::DisplayRole] = tr("Classifier title");

  Columns[Column::COMPOUND_CLASS_TYPE_ID].Ids[Qt::EditRole] = "compound_class_type_id";
  Columns[Column::COMPOUND_CLASS_TYPE_ID].Ids[Qt::DisplayRole] = "compound_class_type_id";
  Columns[Column::COMPOUND_CLASS_TYPE_ID].HeaderData[Qt::DisplayRole] = tr("Compound class type id");

  Columns[Column::COMPOUND_CLASS_TITLE].Ids[Qt::EditRole] = "compound_class_title";
  Columns[Column::COMPOUND_CLASS_TITLE].Ids[Qt::DisplayRole] = "compound_class_title";
  Columns[Column::COMPOUND_CLASS_TITLE].HeaderData[Qt::DisplayRole] = tr("Compound class title");

  Columns[Column::TIC_SIM].Ids[Qt::EditRole] = "tic_sim";
  Columns[Column::TIC_SIM].Ids[Qt::DisplayRole] = "tic_sim";
  Columns[Column::TIC_SIM].HeaderData[Qt::DisplayRole] = tr("Tic sim");

  Columns[Column::MZ].Ids[Qt::EditRole] = "mz";
  Columns[Column::MZ].Ids[Qt::DisplayRole] = "mz";
  Columns[Column::MZ].HeaderData[Qt::DisplayRole] = tr("mz");

  Columns[Column::CHEMICAL_FORMULA].Ids[Qt::EditRole] = "chemical_formula";
  Columns[Column::CHEMICAL_FORMULA].Ids[Qt::DisplayRole] = "chemical_formula";
  Columns[Column::CHEMICAL_FORMULA].HeaderData[Qt::DisplayRole] = tr("Chemical formula");

  Columns[Column::DIMENSION].Ids[Qt::EditRole] = "dimension";
  Columns[Column::DIMENSION].Ids[Qt::DisplayRole] = "dimension";
  Columns[Column::DIMENSION].HeaderData[Qt::DisplayRole] = tr("Dimension");

  Columns[Column::ANALYSIS_METHODOLOGY].Ids[Qt::EditRole] = "analysis_methodology";
  Columns[Column::ANALYSIS_METHODOLOGY].Ids[Qt::DisplayRole] = "analysis_methodology";
  Columns[Column::ANALYSIS_METHODOLOGY].HeaderData[Qt::DisplayRole] = tr("Analysis methodology");

  Columns[Column::LITERATURE_SOURCE].Ids[Qt::EditRole] = "literature_source";
  Columns[Column::LITERATURE_SOURCE].Ids[Qt::DisplayRole] = "literature_source";
  Columns[Column::LITERATURE_SOURCE].HeaderData[Qt::DisplayRole] = tr("Literature source");

  Columns[Column::BIBLIOGRAPHICAL_REFERENCE].Ids[Qt::EditRole] = "bibliographical_reference";
  Columns[Column::BIBLIOGRAPHICAL_REFERENCE].Ids[Qt::DisplayRole] = "bibliographical_reference";
  Columns[Column::BIBLIOGRAPHICAL_REFERENCE].HeaderData[Qt::DisplayRole] = tr("Bibliographical reference");

  Columns[Column::MASS_SPECTRUM].Ids[Qt::EditRole] = "mass_spectrum";
  Columns[Column::MASS_SPECTRUM].Ids[Qt::DisplayRole] = "mass_spectrum";
  Columns[Column::MASS_SPECTRUM].HeaderData[Qt::DisplayRole] = tr("Mass spectrum");

  Columns[Column::INTENSITY_SPECTRUM].Ids[Qt::EditRole] = "intensity_spectrum";
  Columns[Column::INTENSITY_SPECTRUM].Ids[Qt::DisplayRole] = "intensity_spectrum";
  Columns[Column::INTENSITY_SPECTRUM].HeaderData[Qt::DisplayRole] = tr("Intensity spectrum");

  Columns[Column::FAVOURITES].Ids[Qt::EditRole] = "favourites";
  Columns[Column::FAVOURITES].Ids[Qt::DisplayRole] = "favourites";

  Columns[Column::PDF_TITLE].Ids[Qt::EditRole] = "pdf_title";
  Columns[Column::PDF_TITLE].Ids[Qt::DisplayRole] = "pdf_title";
  Columns[Column::PDF_TITLE].HeaderData[Qt::DisplayRole] = tr("PDF title");

  Columns[Column::PDF_SIZE].Ids[Qt::EditRole] = "pdf_size";
  Columns[Column::PDF_SIZE].Ids[Qt::DisplayRole] = "pdf_size";
  Columns[Column::PDF_SIZE].HeaderData[Qt::DisplayRole] = tr("PDF size");

  Columns[Column::PDF_LOAD_DATE].Ids[Qt::EditRole] = "pdf_load_date";
  Columns[Column::PDF_LOAD_DATE].Ids[Qt::DisplayRole] = "pdf_load_date";
  Columns[Column::PDF_LOAD_DATE].HeaderData[Qt::DisplayRole] = tr("PDF load date");

  Columns[Column::XLSX_TITLE].Ids[Qt::EditRole] = "xlsx_title";
  Columns[Column::XLSX_TITLE].Ids[Qt::DisplayRole] = "xlsx_title";
  Columns[Column::XLSX_TITLE].HeaderData[Qt::DisplayRole] = tr("XLSX title");

  Columns[Column::XLSX_SIZE].Ids[Qt::EditRole] = "xlsx_size";
  Columns[Column::XLSX_SIZE].Ids[Qt::DisplayRole] = "xlsx_size";
  Columns[Column::XLSX_SIZE].HeaderData[Qt::DisplayRole] = tr("XLSX size");

  Columns[Column::XLSX_LOAD_DATE].Ids[Qt::EditRole] = "xlsx_load_date";
  Columns[Column::XLSX_LOAD_DATE].Ids[Qt::DisplayRole] = "xlsx_load_date";
  Columns[Column::XLSX_LOAD_DATE].HeaderData[Qt::DisplayRole] = tr("XLSX load date");

  Columns[Column::PDF_ATTACHED].Ids[Qt::EditRole] = "pdf_attached";
  Columns[Column::PDF_ATTACHED].Ids[Qt::DisplayRole] = "pdf_attached";
  Columns[Column::PDF_ATTACHED].HeaderData[Qt::DisplayRole] = tr("PDF attached");

  Columns[Column::XLSX_ATTACHED].Ids[Qt::EditRole] = "xlsx_attached";
  Columns[Column::XLSX_ATTACHED].Ids[Qt::DisplayRole] = "xlsx_attached";
  Columns[Column::XLSX_ATTACHED].HeaderData[Qt::DisplayRole] = tr("XLSX attached");

  Columns[Column::ALL_GROUPS_TITLES].Ids[Qt::EditRole] = "all_groups_titles";
  Columns[Column::ALL_GROUPS_TITLES].Ids[Qt::DisplayRole] = "all_groups_titles";
  Columns[Column::ALL_GROUPS_TITLES].HeaderData[Qt::DisplayRole] = tr("All groups titles");
  PresentationModel = new TreeModelPresentationLibraryCompounds(this);
  Reset();
}

QVariant TreeModelDynamicCalculationCoefficientTable::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (role == Qt::CheckStateRole)
  {
    if (orientation == Qt::Horizontal)
    {
      if (section != Column::CHECKED)
        return QVariant();
      return OverallCheckState;
    }
  }

  if (orientation == Qt::Horizontal)
  {
    if (section > static_cast<int>(columnCount()))
      return QVariant();

    if (role == Qt::DecorationRole)
      return section == Column::CHECKED;
  }

  return TreeModelDynamic::headerData(section, orientation, role);
}

bool TreeModelDynamicCalculationCoefficientTable::setHeaderData(int section, Qt::Orientation orientation, const QVariant& value, int role)
{
  if (role == Qt::CheckStateRole)
  {
    if (orientation == Qt::Horizontal)
    {
      if (section != Column::CHECKED)
        return false;
      OverallCheckState = value.value<Qt::CheckState>();
      if (!blockCheckStateEvents)
        emit HeaderCheckStateChanged(orientation, OverallCheckState);
      return true;
    }
  }

  return TreeModelDynamic::setHeaderData(section, orientation, value, role);
}

QVariant TreeModelDynamicCalculationCoefficientTable::data(const QModelIndex& index, int role) const
{
  if (!index.isValid() || index.row() > CheckedStates.size())
    return QVariant();
  if (role == Qt::ToolTipRole)
  {
    QString toolTip = ("library_element_id: %1;\nlibrary_group_id: %2\nsystem=%3");
    auto item = GetItem(index);
    return toolTip.arg(item->GetData("library_element_id").toInt())
      .arg(item->GetData("library_group_id").toInt())
      .arg(item->GetData("system").toBool());
  }
  switch (static_cast<Column>(index.column()))
  {
  case Column::CHECKED:
  {
    if (role == Qt::DisplayRole || CheckedStates.isEmpty())
      return QVariant();
    return CheckedStates[index.row()] ? Qt::Checked : Qt::Unchecked;
  }
  case Column::FAVOURITES:
  {
    if (role == Qt::DecorationRole)
      return QPixmap(TreeModelDynamic::data(index, Qt::DisplayRole).toBool() ? FavoriteIconCheckedPath : FavoriteIconPath);
    else if (role == Qt::DisplayRole)
      return QVariant();
    break;
  }
  case Column::GROUP:
  {
    bool isSystem = GetItem(index)->GetData("system").toBool();
    if (role == Qt::DecorationRole
      && !isSystem)
      return QPixmap(CompoundTypePath);
    else
      return QVariant();
    break;
  }
  case Column::XLSX_ATTACHED:
  case Column::PDF_ATTACHED:
  {
    if (role == Qt::DecorationRole
      && TreeModelDynamic::data(index, Qt::DisplayRole).toBool())
      return QPixmap(CheckIconPath);
    break;
  }
  case Column::FORMULA:
  {
    if (role == Qt::DisplayRole)
      return RegShortTitle.splitString(TreeModelDynamic::data(index, Qt::DisplayRole).toString()).join("");
  }
  default:
  {
    return TreeModelDynamic::data(index, role);
  }
  }
  return QVariant();
}

bool TreeModelDynamicCalculationCoefficientTable::setData(const QModelIndex& index, const QVariant& data, int role)
{
  if (role != Qt::CheckStateRole
    || index.column() != static_cast<int>(Column::CHECKED))
    return false;
  CheckedStates[index.row()] = data.toBool();
  if (!blockCheckStateEvents)
    emit dataChanged(index, index, { Qt::CheckStateRole });
  return true;
}

void TreeModelDynamicCalculationCoefficientTable::setOverallCheckState(Qt::CheckState state)
{
  if (state == Qt::PartiallyChecked)
    state = Qt::Checked;
  QList<QPair<int, int>> changedIntervals;
  for (int r = 0; r < rowCount(); r++)
  {
    const auto libraryElementId = data(index(r, Column::LIBRARY_ELEMENT_ID), Qt::EditRole).toInt();
    CheckedStates[r] = state;
    if (changedIntervals.isEmpty() || changedIntervals.last().second + 1 < r)
      changedIntervals.append({ r,r });
    else
      changedIntervals.last().second = r;
  }
  if (!blockCheckStateEvents)
    for (auto& i : changedIntervals)
      emit dataChanged(index(i.first, Column::CHECKED), index(i.second, Column::CHECKED), { Qt::CheckStateRole });
}

bool TreeModelDynamicCalculationCoefficientTable::canFetchMore(const QModelIndex& parent) const
{
  QPointer<TreeModelItem> parentItem;
  if (parent.column() > 0)
    return 0;

  if (!parent.isValid())
    parentItem = Root;
  else
    parentItem = static_cast<TreeModelItem*>(parent.internalPointer());

  return parentItem->CanFetchMore();
}

void TreeModelDynamicCalculationCoefficientTable::fetchMore(const QModelIndex& parent)
{
  QPointer<TreeModelItem> parentItem;
  if (parent.column() > 0)
    return;

  if (!parent.isValid())
    parentItem = Root;
  else
    parentItem = static_cast<TreeModelItem*>(parent.internalPointer());

  if (!parentItem->CanFetchMore())
    return;

  FetchMore(parentItem);
}

void TreeModelDynamicCalculationCoefficientTable::FetchMore(QPointer<TreeModelItem> item)
{
  if (item->CanFetchMore())
  {
    item->SetFetched(true);

    API::REST::Tables::GetCoefficientsByTableId(TableId,
      [&, item](QNetworkReply*, QVariantMap result)
      {
        if (!item)
          return;
        beginResetModel();
        item->Load(result, false);
        postProcessFetch(item);
        endResetModel();
      },
      [item](QNetworkReply*, QNetworkReply::NetworkError err)
      {
        if (!item)
          return;

        item->Clear();
        Notification::NotifyError(tr("Failed to get compounds from database"), err);
      }
    );
  }
}

void TreeModelDynamicCalculationCoefficientTable::postProcessFetch(QPointer<TreeModelItem> item)
{
  CheckedStates.fill(false, item->GetRowCount());
}

void TreeModelDynamicCalculationCoefficientTable::setBlockingCheckStateEvents(bool blocked)
{
  blockCheckStateEvents = blocked;
}

int TreeModelDynamicCalculationCoefficientTable::checkedCount() const
{
  return std::count_if(CheckedStates.begin(), CheckedStates.end(), [](bool value) { return value; });;
}

int TreeModelDynamicCalculationCoefficientTable::getTableId() const
{
    return TableId;
}

QList<int> TreeModelDynamicCalculationCoefficientTable::getChecked() const
{
  QList<int> result;
  for (int row = 0; row < rowCount(); row++)
  {
    if (CheckedStates[row])
    {
      auto item = GetItem(index(row, 0));
      auto id = item->GetData("library_group_id").toInt();
      result.append(id);
    }
  }
  return result;
}