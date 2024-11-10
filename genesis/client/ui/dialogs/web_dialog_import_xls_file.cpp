#include "web_dialog_import_xls_file.h"
#include "logic/tree_model.h"
#include "ui_web_dialog_import_xls_file.h"

#include "ui/genesis_window.h"
#include "ui/item_models/analysis_data_table_model.h"
#include "ui/controls/dialog_line_edit_ex.h"
#include "genesis_style/style.h"
#include "api/api_rest.h"
#include "logic/known_context_tag_names.h"
#include "logic/known_json_tag_names.h"
#include "logic/service/service_locator.h"
#include "logic/models/reference_analysis_data_table_model.h"
#include "logic/service/ijson_serializer.h"
#include "logic/markup/genesis_markup_enums.h"
#include "logic/tree_model_dynamic.h"
#include "logic/context_root.h"
#include "ui/item_editors/expandable_default_item_editor_factory.h"
#include "ui/itemviews/datatable_item_delegate.h"
#include "ui/dialogs/web_overlay.h"
#include "ui/dialogs/models/import_xls_proxy_model.h"

#include <QFileDialog>
#include <QMetaEnum>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QStandardItemEditorCreator>
#include <QMenu>

int kBackupRole = Qt::UserRole + 1;

const QColor kTopRowHighlightColor = QColor(255,255,255);
const QColor kChangedValueHighlightColor = QColor(251,189,86);

class QueryGuard
{
private:
  struct QueryGuardedEntry
  {
    QueryGuardedEntry()
      :counter(-1),
      action{}{}
    QueryGuardedEntry(int count, std::function<void(void)> act)
      : counter(count),
        action(act){}
    int counter;
    std::function<void(void)> action;
  };
  static QMap<QUuid, QueryGuardedEntry> mEntries;
public:
  static bool registerGuard(const QUuid& uid, int counter, std::function<void(void)> action)
  {
    if(mEntries.contains(uid))
      return false;
    mEntries.insert(uid, {counter, action});
    return true;
  }
  static void reportReady(const QUuid& uid)
  {
    if(!mEntries.contains(uid))
      return;
    auto& guardEntry = mEntries[uid];
    if(guardEntry.counter == -1)//no entry
      return;
    guardEntry.counter--;
    if(guardEntry.counter <= 0)
    {
      guardEntry.action();
      mEntries.remove(uid);
    }
  }


};
QMap<QUuid, QueryGuard::QueryGuardedEntry> QueryGuard::mEntries = {};

namespace StringKeyContext
{
const QString kDataType("data_type");
const QString kPassportData("passport_data");
}
QMap<QString, QList<StringKey>> StringKeyEditor::mStringKeysContexts = {};
QList<LibElem> LibElemEditor::mCoefficients = {};
QList<LibElem> LibElemEditor::mCompounds = {};

namespace ImportDetails
{
extern QString GetFileFrameStyle();
extern QString GetButtonStyle();
}

using namespace Dialogs;

WebDialogImportXlsFile::WebDialogImportXlsFile(/*Import::XlsFileImportedType type, */int projectId, QWidget* parent) :
  Templates::Info(parent, QDialogButtonBox::Ok | QDialogButtonBox::Cancel)
  , ui(new Ui::WebDialogImportXlsFile)
  , mProjectId(projectId)
{
  {
    auto sktype = QMetaType::fromType<StringKey>();
    auto letype = QMetaType::fromType<LibElem>();
    auto stype = QMetaType::fromType<QString>();
    if(!QMetaType::hasRegisteredConverterFunction(sktype, stype))
      QMetaType::registerConverter<StringKey, QString>(&StringKey::toString);
    if(!QMetaType::hasRegisteredConverterFunction(letype, stype))
      QMetaType::registerConverter<LibElem, QString>(&LibElem::toString);
  }
  kStepsTips << tr("Step 1/4. Loading");
  kStepsTips << tr("Step 2/4. Table preview");
  kStepsTips << tr("Step 3/4. Title and comment");
  kStepsTips << tr("Step 4/4. Grouping");

  kInterstageOkButtonText = tr("Next step");
  kFinalOkButtonText = tr("Import");

  auto s = Templates::Info::Settings();
  s.contentHeader = kStepsTips[0];
  s.dialogHeader = tr("Table import");
  s.buttonsNames = { {QDialogButtonBox::Ok, kInterstageOkButtonText} };
  s.buttonsProperties = { {QDialogButtonBox::Ok, {{"blue", true}}} };

  auto overlay = new WebOverlay(tr("Loading"), this);
  StringKeyEditor::initStringKeyContextByModule(overlay);
  setupUi();
  applySettings(s);
}

WebDialogImportXlsFile::~WebDialogImportXlsFile()
{
  delete ui;
}

void WebDialogImportXlsFile::addFilesToList(FileInfoList infoList)
{
  if (!mFilePlates.isEmpty())
    return;

  for (const auto& info : infoList)
  {
    if (!mFilePlates.contains(info.absoluteFilePath()))
    {
      auto plate = new FilePlateWidget(info, ":/resource/icons/icon_xls_file.png");
      mFilePlates[info.absoluteFilePath()] = plate;
      ui->filesListLayout->addWidget(plate);
      connect(plate, &FilePlateWidget::closedByUser, this, &WebDialogImportXlsFile::onPlateClosed);
    }
  }
  inputValidityCheck();
}

void WebDialogImportXlsFile::Accept()
{
  stepForward();
}

void WebDialogImportXlsFile::Reject()
{
  Templates::Info::Reject();
}

void WebDialogImportXlsFile::onPlateClosed(QString absoluteFilePath)
{
  mFilePlates.remove(absoluteFilePath);
  inputValidityCheck();
}

void WebDialogImportXlsFile::stepForward()
{
  switch(mCurrentPage)
  {
  case PageFilesUpload:
  {
    mCurrentPage = PagePreviewTable;
    ui->stackedWidget->setCurrentIndex(mCurrentPage);
    auto s = getCurrentSettings();
    s.contentHeader = kStepsTips[ui->stackedWidget->currentIndex()];
    s.buttonsNames[QDialogButtonBox::Ok] = kInterstageOkButtonText;
    applySettings(s);
    mStepBackBtn->setVisible(true);
    loadTablePreview();
    break;
  }
  case PagePreviewTable:
  {
    mCurrentPage = PageTitleCommentSetup;
    ui->stackedWidget->setCurrentIndex(mCurrentPage);
    auto s = getCurrentSettings();
    s.contentHeader = kStepsTips[ui->stackedWidget->currentIndex()];
    s.buttonsNames[QDialogButtonBox::Ok] = kInterstageOkButtonText;
    applySettings(s);
    break;
  }
  case PageTitleCommentSetup:
  {
    mCurrentPage = PageChromatogrammsGroupSetup;
    ui->stackedWidget->setCurrentIndex(mCurrentPage);
    auto s = getCurrentSettings();
    s.contentHeader = kStepsTips[ui->stackedWidget->currentIndex()];
    s.buttonsNames[QDialogButtonBox::Ok] = kFinalOkButtonText;
    applySettings(s);
    break;
  }
  case PageChromatogrammsGroupSetup:
    uploadTable();
    break;
  case PageLast:
  default:
    break;
  }
}

void WebDialogImportXlsFile::stepBack()
{
  switch(mCurrentPage)
  {
  case PageFilesUpload:
    break;
  case PagePreviewTable:
  {
    mCurrentPage = PageFilesUpload;
    ui->stackedWidget->setCurrentIndex(mCurrentPage);
    auto s = getCurrentSettings();
    s.contentHeader = kStepsTips[ui->stackedWidget->currentIndex()];
    s.buttonsNames[QDialogButtonBox::Ok] = kInterstageOkButtonText;
    applySettings(s);
    mStepBackBtn->setVisible(false);
    break;
  }
  case PageTitleCommentSetup:
  {
    mCurrentPage = PagePreviewTable;
    ui->stackedWidget->setCurrentIndex(mCurrentPage);
    auto s = getCurrentSettings();
    s.contentHeader = kStepsTips[ui->stackedWidget->currentIndex()];
    s.buttonsNames[QDialogButtonBox::Ok] = kFinalOkButtonText;
    applySettings(s);
    break;
  }
  case PageChromatogrammsGroupSetup:
  {
    mCurrentPage = PageTitleCommentSetup;
    ui->stackedWidget->setCurrentIndex(mCurrentPage);
    auto s = getCurrentSettings();
    s.contentHeader = kStepsTips[ui->stackedWidget->currentIndex()];
    s.buttonsNames[QDialogButtonBox::Ok] = kFinalOkButtonText;
    applySettings(s);
    break;
  }
  case PageLast:
  default:
    break;
  }
}

void WebDialogImportXlsFile::inputValidityCheck(std::optional<bool> forceValidState)
{
  auto okBtn = ButtonBox->button(QDialogButtonBox::Ok);
  if(forceValidState.has_value())
  {
    okBtn->setEnabled(forceValidState.value());
    return;
  }
  switch(mCurrentPage)
  {
  case PageFilesUpload:
    ui->localImportButton->setEnabled(mFilePlates.isEmpty());
    okBtn->setEnabled(mFilePlates.size() == 1);
    break;
  case PagePreviewTable:
  case PageTitleCommentSetup:
  case PageChromatogrammsGroupSetup:
    if(ui->tableGroupingButtonsGroup->checkedId() == 1)
      okBtn->setEnabled(!ui->tablesGroupsCombo->currentText().isEmpty());
    else
      okBtn->setEnabled(true);
    break;
  case PageLast:
  default:
    okBtn->setEnabled(true);
    break;
  }
}

void WebDialogImportXlsFile::loadTablePreview()
{
  if (auto model = qobject_cast<QStandardItemModel*>(ui->previewTableView->model()))
    model->clear();
  if (mFilePlates.isEmpty())
    return;

  auto fileName = mFilePlates.firstKey();
  QFileInfo fi(fileName);
  QFile file(fileName);
  file.open(QIODevice::ReadOnly);
  auto byteArray = file.readAll();
  file.close();
  mTableName = fi.fileName();

  auto overlayPtr = new WebOverlay(tr("Loading table preview"), ui->previewTableView);
  API::REST::ParseXlsTable(byteArray, fi.fileName(), mProjectId,
    [this, overlayPtr](QNetworkReply*, QJsonDocument doc)
    {
      const int rowOffset = 2;
      const int colOffset = 1;

      auto obj = doc.object();
      if (obj.contains(JsonTagNames::Error) && obj.value(JsonTagNames::Error).toBool())
      {
        Notification::NotifyError(obj.value(JsonTagNames::Msg).toString(), tr("File upload error"));
        overlayPtr->deleteLater();
        return;
      }
      auto tableArray = obj["parsed_xlsx"].toArray();
      QStandardItemModel* model = qobject_cast<QStandardItemModel*>(ui->previewTableView->model());
      if (!model)
      {
        model = new QStandardItemModel(tableArray.size() + rowOffset, tableArray.first().toArray().size() + colOffset, ui->previewTableView);
        mColumnsTableConnection = QMetaObject::Connection();

      }
      if (!mTableProxy)
        mTableProxy = new ImportXlsProxyModel(this);
      mTableProxy->setUnhiddableColumns({ 0 });
      mTableProxy->setUnhiddableSourceRows({ 0,1/*,2*/ });
      mTableProxy->setSourceModel(model);
      ui->previewTableView->setModel(mTableProxy);

      if (!mColumnsTableConnection)
      {
        mColumnsTableConnection = connect(mTableProxy->sourceModel(), &QAbstractItemModel::dataChanged, this, &WebDialogImportXlsFile::onColumnsModelDataChanged);
      }

      {
        const auto dataItem = new QStandardItem(tr("Data type"));
        dataItem->setData(kTopRowHighlightColor, Qt::BackgroundRole);
        model->setItem(0, 0, dataItem);
        const auto importedItem = new QStandardItem(tr("Will be imported"));
        model->setItem(1, 0, importedItem);
      }
      for (int r = 0; r < tableArray.size(); r++)
      {
        const auto row = tableArray[r].toArray();
        for (auto c = 0; c < row.size(); c++)
        {
          auto cell = row[c].toVariant();
          auto item = new QStandardItem();
          item->setData(cell, Qt::DisplayRole);
          item->setData(cell, kBackupRole);
          model->setItem(r + rowOffset, c + colOffset, item);
        }
      }
      auto keyContexts = StringKeyEditor::keysContexts();
      auto passportFields = keyContexts.value(StringKeyContext::kPassportData);
      int passportStartColumn = model->columnCount() - passportFields.size();

      {
        StringKey topkey = { tr("Sample title"), "sample_title", StringKeyContext::kDataType };
        auto item = new QStandardItem();
        item->setData(QVariant::fromValue(topkey), Qt::DisplayRole);
        item->setData(kTopRowHighlightColor, Qt::BackgroundRole);
        model->setItem(0, 1, item);
      }

      // fill data values
      for (int c = rowOffset; c < passportStartColumn; c++)
      {
        using namespace Names::ModulesContextTags;
        using namespace Core;
        QString headerValue = mTableProxy->index(2, c).data().toString();
        auto findCompound = [](QString value, LibElem& elem)->bool
          {
            for (auto& e : LibElemEditor::compounds())
            {
              if (QString::compare(e.shortTitle, value, Qt::CaseInsensitive) == 0
                || QString::compare(e.fullTitle, value, Qt::CaseInsensitive) == 0)
              {
                elem = e;
                return true;
              }
            }
            return false;
          };
        auto findCoeff = [](QString value, LibElem& elem)->bool
          {
            for (auto& e : LibElemEditor::coefficients())
            {
              if (QString::compare(e.shortTitle, value, Qt::CaseInsensitive) == 0
                || QString::compare(e.fullTitle, value, Qt::CaseInsensitive) == 0)
              {
                elem = e;
                return true;
              }
            }
            return false;
          };

        auto currentModule = GenesisContextRoot::Get()->GetContextModules()->GetData(kModule).value<Module>();
        if (currentModule == MBiomarkers)
        {
          LibElem elem;
          StringKey topkey;
          if (findCompound(headerValue, elem))
          {
            topkey = { tr("Compounds"), "compounds", StringKeyContext::kDataType };
            auto item = new QStandardItem();
            item->setData(QVariant::fromValue(elem), Qt::DisplayRole);
            item->setData(kTopRowHighlightColor, Qt::BackgroundRole);
            model->setItem(1, c, item);
          }
          else if (findCoeff(headerValue, elem))
          {
            topkey = { tr("Coefficients"), "coefficients", StringKeyContext::kDataType };
            auto item = new QStandardItem();
            item->setData(QVariant::fromValue(elem), Qt::DisplayRole);
            item->setData(kTopRowHighlightColor, Qt::BackgroundRole);
            model->setItem(1, c, item);
          }
          else
          {
            topkey = { tr("Compounds"), "compounds", StringKeyContext::kDataType };
            auto item = new QStandardItem();
            item->setData(QVariant::fromValue(topkey), Qt::DisplayRole);
            item->setData(kTopRowHighlightColor, Qt::BackgroundRole);
            model->setItem(0, c, item);
          }
        }
        else if (currentModule == MReservoir)
        {
          StringKey topkey = { tr("Sample data"), "sample_data", StringKeyContext::kDataType };
          auto item = new QStandardItem();
          item->setData(QVariant::fromValue(topkey), Qt::DisplayRole);
          item->setData(kTopRowHighlightColor, Qt::BackgroundRole);
          model->setItem(0, c, item);
        }
      }
      // fill passport data
      for (int c = passportStartColumn; c < model->columnCount(); c++)
      {
          auto item = new QStandardItem();
          StringKey topkey = { tr("Geo data"), "geo_data", StringKeyContext::kDataType };
          item->setData(QVariant::fromValue(topkey), Qt::DisplayRole);
          item->setData(kTopRowHighlightColor, Qt::BackgroundRole);
          model->setItem(0, c, item);
      }
      ui->previewTableView->resizeColumnsToContents();
      ui->previewTableView->resizeRowsToContents();
      overlayPtr->deleteLater();
    },
    [overlayPtr](QNetworkReply*, QNetworkReply::NetworkError err)
    {
      qDebug() << err;
      overlayPtr->deleteLater();
    });
}

void WebDialogImportXlsFile::importFromLocal()
{
  auto file = QFileDialog::getOpenFileName(this, tr("Pick file to import"), "", "Excel file (*.xlsx)");
  if (!file.isEmpty())
  {
    FileInfoList infoList;
    infoList << QFileInfo(file);
    addFilesToList(infoList);
  }
}

void WebDialogImportXlsFile::onColumnsModelDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QList<int> &roles)
{
  auto modelPtr = qobject_cast<QStandardItemModel*>(sender());
  auto inRange = [](int lower, int upper, int value)
  {
    return value <= upper && value >= lower;
  };
  using namespace StringKeyContext;
  if(inRange(topLeft.row(), bottomRight.row(), 0)
      && (roles.contains(Qt::DisplayRole) || roles.isEmpty()))
    //if roles is empty than there was whole item insertion
  {
    auto keyContexts = StringKeyEditor::keysContexts();
    auto passportFields = keyContexts.value(StringKeyContext::kPassportData);
    int passportStartColumn = modelPtr->columnCount() - passportFields.size();

    for(int c = topLeft.column(); c <= bottomRight.column(); c++)
    {
      if(c == 0)
        continue;
      auto topItem = modelPtr->item(0, c);
      auto vdata = topItem->data(Qt::DisplayRole);
      if(vdata.typeId() != QMetaType::fromType<StringKey>().id())
        continue;
      StringKey data = vdata.value<StringKey>();
      if(data.context != kDataType)
        continue;
      auto item = new QStandardItem();
      item->setData(kTopRowHighlightColor, Qt::BackgroundRole);
      if(data.key == "dont_import")
      {
        item->setData("-", Qt::DisplayRole);
        item->setFlags(Qt::ItemIsSelectable);
      }
      else if(data.key == "sample_title")
      {
        item->setData(tr("Sample title"), Qt::DisplayRole);
        item->setFlags(Qt::ItemIsSelectable);
      }
      else if(data.key == "sample_data")
      {
        item->setData(tr("Sample data"), Qt::DisplayRole);
        item->setFlags(Qt::ItemIsSelectable);
      }
      else if(data.key == "geo_data")
      {
        auto passportFieldIndex = c - passportStartColumn;
        if(passportFieldIndex < 0)
          passportFieldIndex = 0;
        auto bottomkey = passportFields[passportFieldIndex];
        item->setData(QVariant::fromValue(bottomkey), Qt::DisplayRole);
      }
      else if(data.key == "compounds")
      {
        auto title = modelPtr->index(2, c).data().toString();
        bool found = false;
        for(auto& elem : LibElemEditor::compounds())
        {
          if(QString::compare(elem.shortTitle, title, Qt::CaseInsensitive) == 0
              || QString::compare(elem.fullTitle, title, Qt::CaseInsensitive) == 0)
          {
            item->setData(QVariant::fromValue(elem), Qt::DisplayRole);
            found = true;
            break;
          }
        }
        if(!found)
        {
          item->setData(QVariant::fromValue(LibElem{tr("Unknown compound"), tr("Unknown"), -1, -1, 1}), Qt::DisplayRole);
        }
      }
      else if(data.key == "coefficients")
      {
        auto title = modelPtr->index(2, c).data().toString();
        bool found = false;
        for(auto& elem : LibElemEditor::coefficients())
        {
          if(QString::compare(elem.shortTitle, title, Qt::CaseInsensitive) == 0
              || QString::compare(elem.fullTitle, title, Qt::CaseInsensitive) == 0)
          {
            item->setData(QVariant::fromValue(elem), Qt::DisplayRole);
            found = true;
            break;
          }
        }
        if(!found)
        {
          item->setData(QVariant::fromValue(LibElem{tr("Unknown coefficient"), tr("Unknown"), -1, -1, 2}), Qt::DisplayRole);
        }
      }
      modelPtr->setItem(1, c, item);
    }
  }
  if((roles.isEmpty() || roles.contains(Qt::DisplayRole)) &&
      (topLeft.column() > 0 && topLeft.row() > 1))
  {
    for(int r = topLeft.row(); r <= bottomRight.row(); r++)
    {
      for(int c = topLeft.column(); c <= bottomRight.column(); c++)
      {
        auto ind = modelPtr->index(r, c);
        if(ind.data() != ind.data(kBackupRole))
          modelPtr->setData(ind, kChangedValueHighlightColor, Qt::BackgroundRole);
        else
          modelPtr->setData(ind, QVariant(), Qt::BackgroundRole);
      }
    }
  }
}

void WebDialogImportXlsFile::onTableContextMenuRequested(const QPoint &pt)
{
  auto index = ui->previewTableView->indexAt(pt);
  if(!index.isValid())
    return;
  setProperty("context_menu_model_index", mTableProxy->mapToSource(index));
  mTableMenu->popup(ui->previewTableView->viewport()->mapToGlobal(pt));

  if(index.column() > 0 && index.row() > 1)
  {
    if(index.data() == index.data(kBackupRole))
      mRestoreValueAction->setVisible(false);
    else
      mRestoreValueAction->setVisible(true);
  }
  else
    mRestoreValueAction->setVisible(false);

  if(index.column() == 0)
    mRemoveColumnAction->setVisible(false);
  else
    mRemoveColumnAction->setVisible(true);
  if(index.row() < 2)
    mRemoveRowAction->setVisible(false);
  else
    mRemoveRowAction->setVisible(true);

  auto hc = mTableProxy->hiddenColumns();
  auto hr = mTableProxy->hiddenRows();
  const bool kDisableRestoring = true;
  if(hr.isEmpty() || kDisableRestoring)
    mRestoreRowAction->setVisible(false);
  else
  {
    mRestoreRowAction->setVisible(true);
    auto menu = mRestoreRowAction->menu();
    menu->clear();
    for(auto& row : hr)
    {
      auto act = menu->addAction(QString::number(row));
      connect(act, &QAction::triggered, this,
              [this, row]()
              {
                mTableProxy->setSourceRowHidden(row, false);
              });
    }
  }
  if(hc.isEmpty() || kDisableRestoring)
    mRestoreColumnAction->setVisible(false);
  else
  {
    mRestoreColumnAction->setVisible(true);
    auto menu = mRestoreColumnAction->menu();
    menu->clear();
    for(auto& column : hc)
    {
      auto act = menu->addAction(QString::number(column));
      connect(act, &QAction::triggered, this,
              [this, column]()
              {
                mTableProxy->setSourceColumnHidden(column, false);
              });
    }
  }
}

void WebDialogImportXlsFile::hideRow(int row)
{
  if(row == -1)
  {
    auto index = property("context_menu_model_index").toModelIndex();
    row = index.row();
  }
  mTableProxy->setSourceRowHidden(row, true);
}

void WebDialogImportXlsFile::hideColumn(int column)
{
  if(column == -1)
  {
    auto index = property("context_menu_model_index").toModelIndex();
    column = index.column();
  }
  mTableProxy->setSourceColumnHidden(column, true);
}

void WebDialogImportXlsFile::restoreValue()
{
  auto index = property("context_menu_model_index").toModelIndex();
  if(index.row() < 2 || index.column() < 1)
    return;
  mTableProxy->sourceModel()->setData(index, index.data(kBackupRole), Qt::DisplayRole);
}

void WebDialogImportXlsFile::setupUi()
{
  auto body = new QWidget(this);
  body->setStyleSheet(Style::Genesis::GetUiStyle());
  ui->setupUi(body);

  mTableMenu = new QMenu(body);
  mVHeaderMenu = new QMenu(body);
  mHHeaderMenu = new QMenu(body);
  mRemoveColumnAction = new QAction(tr("Remove column"));
  mRemoveRowAction = new QAction(tr("Remove row"));
  mRestoreRowAction = new QAction(tr("Restore row"));
  mRestoreColumnAction = new QAction(tr("Restore column"));
  mRestoreValueAction = new QAction(tr("Restore value"));
  mTableMenu->addAction(mRestoreValueAction);
  mTableMenu->addAction(mRemoveColumnAction);
  mTableMenu->addAction(mRemoveRowAction);
  mVHeaderMenu->addAction(mRemoveColumnAction);
  mHHeaderMenu->addAction(mRemoveRowAction);

  auto rowRestoreMenu = new QMenu(body);
  auto columnRestoreMenu = new QMenu(body);
  mRestoreRowAction->setMenu(rowRestoreMenu);
  mRestoreColumnAction->setMenu(columnRestoreMenu);
  mTableMenu->addAction(mRestoreRowAction);
  mTableMenu->addAction(mRestoreColumnAction);


  setProperty("context_menu_model_index", QModelIndex());

  connect(mRemoveRowAction, &QAction::triggered, this, [this](){hideRow(-1);});
  connect(mRemoveColumnAction, &QAction::triggered, this, [this](){hideColumn(-1);});
  connect(mRestoreValueAction, &QAction::triggered, this, &WebDialogImportXlsFile::restoreValue);

  auto tableStyle = Style::ApplySASS(
      "QTableView::item {border: none; padding: 2px; margin: 0px;}"
      "QHeaderView {background-color: @uiInputElementColorBgSecondary;}"
      "QHeaderView::section {background-color: @uiInputElementColorBgSecondary;"
      "                      font: @SecondaryTextFont;"
      "                      padding: 2px;}"
      "QTableView QTableCornerButton::section {background-color: @uiInputElementColorBgSecondary; border: 1px solid transparent;}"
      "QTableView QComboBox {padding: 1px; font: @SecondaryTextFont;}");
  ui->previewTableView->setStyleSheet(tableStyle);
  ui->previewTableView->verticalHeader()->setDefaultAlignment(Qt::AlignVCenter | Qt::AlignLeft); // this line isn't working
  ui->previewTableView->horizontalHeader()->setDefaultAlignment(Qt::AlignVCenter | Qt::AlignRight); // this line isn't working
  getContent()->layout()->addWidget(body);
  ui->dndFrame->setStyleSheet(ImportDetails::GetFileFrameStyle());
  ui->localImportButton->setStyleSheet(ImportDetails::GetButtonStyle());
  connect(ui->localImportButton, &QPushButton::clicked, this, &WebDialogImportXlsFile::importFromLocal);
  connect(ui->dndFrame, &DnDZoneFrame::filesDropped, this, &WebDialogImportXlsFile::addFilesToList);
  ui->tablePreviewCaption->setStyleSheet(Style::Genesis::GetH2());
  ui->dndFrame->setAcceptableFileExtensions({"xlsx"});
  ui->stackedWidget->setCurrentIndex(0);
  mStepBackBtn = new QPushButton(QIcon("://resource/icons/icon_button_lt.png"), tr("Previous step"), this);
  mStepBackBtn->setProperty("secondary", true);
  mStepBackBtn->setStyleSheet(Style::GetStyleSheet(":/resource/styles/genesis_ui_qpushbutton.qss"));
  connect(mStepBackBtn, &QPushButton::clicked, this, &WebDialogImportXlsFile::stepBack);
  mStepBackBtn->setVisible(false);

  auto factory = new ExpandableDefaultItemEditorFactory();
  factory->registerEditor(QMetaType::fromType<StringKey>().id(), new QStandardItemEditorCreator<StringKeyEditor>());
  factory->registerEditor(QMetaType::fromType<LibElem>().id(), new QStandardItemEditorCreator<LibElemEditor>());
  ui->previewTableView->setProperty("dataDrivenColors", true);
  auto delegate = new DataTableItemDelegate(ui->previewTableView);
  ui->previewTableView->setItemDelegate(delegate);
  if(auto delegate = qobject_cast<QStyledItemDelegate*>(ui->previewTableView->itemDelegate()))
  {
    delegate->setItemEditorFactory(factory);
  }
  else
  {
    QItemEditorFactory::setDefaultFactory(factory);//unwanted behaviour
    qDebug() << "something wrong with delegates";
  }

  ButtonLayout->insertWidget(0, mStepBackBtn);

  {
    auto s = DialogLineEditEx::Settings();
    s.defaultText = tr("Table name");
    s.textHeader = tr("Table name");
    s.textMaxLen = 50;
    s.textPlaceholder = tr("Enter table name");
    s.textRequired = true;
    s.controlSymbols = true;
    s.textTooltipsSet = {
                         tr("Name is empty"),
                         tr("Name alredy in use"),
                         tr("Not checked"),
                         tr("Name is valid"),
                         tr("Name must not contain the characters '\', '/', '\"', '*', '<', '|', '>'")
                         };
    ui->titleEdit->applySettings(s);
  }

  {
    auto s = DialogTextEditEx::Settings();
    s.textHeader = tr("Table comment");
    s.textMaxLen = 70;
    s.textPlaceholder = tr("Enter table comment");
    s.textRequired = false;
    ui->commentEdit->applySettings(s);
  }

  connect(ui->titleEdit, &DialogLineEditEx::validityChanged, this, [&](bool isValid)
  {
    inputValidityCheck(isValid);
  });
  ui->tableGroupingButtonsGroup->setId(ui->commonTablesListRadio, 0);
  ui->tableGroupingButtonsGroup->setId(ui->tablesGroupRadio, 1);
  connect(ui->tableGroupingButtonsGroup, &QButtonGroup::idToggled, this,
          [this](int id, bool checked)
          {
            if(id == 0 && checked)
              ui->tablesGroupsCombo->setVisible(false);
            else if(id == 1 && checked)
              ui->tablesGroupsCombo->setVisible(true);
          });
  connect(ui->tablesGroupsCombo, &QComboBox::currentTextChanged, this,
          [this]()
          {
            inputValidityCheck();
          });
  ui->tablesGroupsCombo->setVisible(false);
  ui->commonTablesListRadio->setChecked(true);

  Size = QSizeF(0.75, 0.5);
  UpdateGeometry();

  inputValidityCheck();
  API::REST::GetTableGroups(Core::GenesisContextRoot::ProjectId(), Names::Group::ImportedTable,
                            [this](QNetworkReply* r, QJsonDocument doc)
                            {
                              auto jdata = doc.object()["data"].toArray();
                              for (int i = 0; i < jdata.size(); i++)
                              {
                                auto jitem = jdata[i].toArray();
                                ui->tablesGroupsCombo->addItem(jitem[1].toString(), jitem[0].toInt());
                              }
                            },
                            [&](QNetworkReply* r, QNetworkReply::NetworkError err)
                            {
                              Notification::NotifyError(tr("Failed to load tables groups information"), err);
                            });
  ui->previewTableView->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(ui->previewTableView, &QTableView::customContextMenuRequested, this, &WebDialogImportXlsFile::onTableContextMenuRequested);
}

void WebDialogImportXlsFile::uploadTable()
{
  QJsonObject jroot;
  jroot["project_id"] = Core::GenesisContextRoot::ProjectId();
  jroot["title"] = ui->titleEdit->text();
  jroot["comment"] = ui->commentEdit->text();
  using namespace Names::ModulesContextTags;
  using namespace Core;
  auto module = GenesisContextRoot::Get()->GetContextModules()->GetData(kModule).value<Module>();
  QString tableType = "ImportedTable";
  if(module == MBiomarkers)
    tableType = "ImportedTableIdentified";
  jroot["table_type"] = tableType;
  bool newGroup = ui->tableGroupingButtonsGroup->checkedId() == 1
                  && (!ui->tablesGroupsCombo->currentData().isValid()
                      || ui->tablesGroupsCombo->currentText() != ui->tablesGroupsCombo->currentData(Qt::DisplayRole));
  jroot["create_group"] = newGroup;
  jroot["group_title"] = ui->tablesGroupsCombo->currentText();
  jroot["table_group_id"] = ui->tableGroupingButtonsGroup->checkedId() == 0 || newGroup ? QJsonValue() : QJsonValue(ui->tablesGroupsCombo->currentData().toInt());
  QJsonObject jtable;
  auto passportKeys = StringKeyEditor::keysContexts()[StringKeyContext::kPassportData];
  QJsonArray jpassportKeys;
  for(auto& key : passportKeys)
    jpassportKeys << key.key;

  jtable["passport_keys"] = jpassportKeys;
  QJsonArray jsampleData;
  QJsonArray jpassportData;
  QJsonArray jvaluesCustomData;
  QJsonArray jvaluesHeaders;
  QJsonArray jvaluesLibraryGroupIds;
  QList<QJsonArray> customDataRows(mTableProxy->rowCount() - 3);
  using PassportValue = QPair<QString, QString>;
  using PassportEntry = QList<PassportValue>;
  using PassportsArray = QList<PassportEntry>;

  PassportEntry commonEntry;
  for(auto& key : passportKeys)
    commonEntry << PassportValue{key.key, ""};
  PassportsArray passArr(mTableProxy->rowCount() - 3, commonEntry);

  int startColumn = 1;
  int topKeyRow = 0;
  int bottomKeyRow = 1;
  int headersRow = 2;
  int dataStartRow = 3;
  for(int column = startColumn; column < mTableProxy->columnCount(); column++)
  {
    auto topKey = mTableProxy->index(topKeyRow, column).data().value<StringKey>();
    if(topKey.key == "dont_import")
      continue;

    if(topKey.key == "sample_title")
    {
      for(int r = dataStartRow; r < mTableProxy->rowCount(); r++)
        jsampleData << mTableProxy->index(r, column).data().toString();
    }
    else if(topKey.key == "sample_data")
    {
      jvaluesHeaders << mTableProxy->index(headersRow, column).data().toString();
      for(int r = dataStartRow; r < mTableProxy->rowCount(); r++)
        customDataRows[r-dataStartRow] << mTableProxy->index(r, column).data().toDouble();
    }
    else if(topKey.key == "compounds")
    {
      jvaluesHeaders << mTableProxy->index(headersRow, column).data().toString();
      jvaluesLibraryGroupIds << mTableProxy->index(bottomKeyRow, column).data().value<LibElem>().libraryGroupId;
      for(int r = dataStartRow; r < mTableProxy->rowCount(); r++)
        customDataRows[r-dataStartRow] << mTableProxy->index(r, column).data().toDouble();
    }
    else if(topKey.key == "coefficients")
    {
      jvaluesHeaders << mTableProxy->index(headersRow, column).data().toString();
      jvaluesLibraryGroupIds << mTableProxy->index(bottomKeyRow, column).data().value<LibElem>().libraryGroupId;
      for(int r = dataStartRow; r < mTableProxy->rowCount(); r++)
        customDataRows[r-dataStartRow] << mTableProxy->index(r, column).data().toDouble();
    }
    else if(topKey.key == "geo_data")
    {
      auto subkey = mTableProxy->index(bottomKeyRow, column).data().value<StringKey>();
      auto it = std::find(passportKeys.begin(), passportKeys.end(), subkey);
      if(it == passportKeys.end())
        continue;
      auto ind = std::distance(passportKeys.begin(), it);
      for(int r = dataStartRow; r < mTableProxy->rowCount(); r++)
      {
        auto& pasEntry = passArr[r-dataStartRow];
        pasEntry[ind].second = mTableProxy->index(r, column).data().toString();
      }
    }
  }
  for(auto& row : customDataRows)
  {
    jvaluesCustomData << row;
  }
  for(auto& entry : passArr)
  {
    QJsonArray jpassport;
    for(auto& value : entry)
      jpassport << value.second;
    jpassportData << jpassport;
  }
  jtable["sample_data"] = jsampleData;
  jtable["passport_data"] = jpassportData;
  jtable["values_height_data"] = QJsonArray();
  jtable["values_area_data"] = QJsonArray();
  jtable["values_kovats_data"] = QJsonArray();
  jtable["values_ret_time_data"] = QJsonArray();
  jtable["values_headers"] = jvaluesHeaders;
  jtable["values_custom_data"] = jvaluesCustomData;
  jtable["values_library_group_ids"] = jvaluesLibraryGroupIds;
  jroot["table_data"] = jtable;

  // qDebug().noquote() << QJsonDocument(jroot).toJson();

  API::REST::UploadParsedImportTable(jroot,
                                     [this](QNetworkReply*, QJsonDocument doc)
                                     {
                                       auto jroot = doc.object();
                                       if(jroot.contains("error") && jroot["error"].toBool())
                                       {
                                         Notification::NotifyError(jroot["msg"].toString(), tr("Server replies error"));
                                         return;
                                       }
                                       Notification::NotifySuccess(tr("Table imported"));
                                       TreeModelDynamic::ResetInstances("TreeModelDynamicNumericTables");
                                       Done(QDialog::Accepted);
                                     },
                                     [](QNetworkReply*, QNetworkReply::NetworkError err)
                                     {
                                       Notification::NotifyError(tr("Network error"), err);
                                     });
}

void StringKeyEditor::initStringKeyContextByModule(WebOverlay* overlay)
{
  mStringKeysContexts.clear();
  auto parsePassportEntries = [](const QJsonObject& data)
  {
    QList<StringKey> passportEntries;
    auto jheaders = data["passport_headers"].toArray();
    auto jkeys = data["passport_keys"].toArray();
    for(int i = 0; i < std::min(jheaders.size(), jkeys.size()); i++)
    {
      StringKey entry;
      entry.context = StringKeyContext::kPassportData;
      entry.key = jkeys[i].toString();
      entry.translation = jheaders[i].toString();
      passportEntries << entry;
    }
    mStringKeysContexts[StringKeyContext::kPassportData] = passportEntries;
  };

  using namespace Names::ModulesContextTags;
  using namespace Core;
  auto module = (Module)GenesisContextRoot::Get()->GetContextModules()->GetData(kModule).toInt();
  switch(module)
  {
  case Names::ModulesContextTags::MReservoir:
    API::REST::GetXlsColumnsCombos("ImportedTable",
                                   [&parsePassportEntries, overlay](QNetworkReply*, QJsonDocument doc)
                                   {
                                     auto jroot = doc.object();
                                     auto jresult = jroot["result"].toObject();
                                     parsePassportEntries(jresult);
                                     mStringKeysContexts[StringKeyContext::kDataType] =
                                         {
                                          {tr("Don't import"), "dont_import", StringKeyContext::kDataType},
                                          {tr("Sample title"), "sample_title", StringKeyContext::kDataType},
                                          {tr("Geo data"), "geo_data", StringKeyContext::kDataType},
                                          {tr("Sample data"), "sample_data", StringKeyContext::kDataType},
                                          };
                                     if(overlay)
                                       overlay->deleteLater();
                                   },
                                   [overlay](QNetworkReply*, QNetworkReply::NetworkError err)
                                   {
                                     Notification::NotifyError(tr("Error"), err);
                                     if(overlay)
                                       overlay->deleteLater();
                                   });
    break;
  case Names::ModulesContextTags::MBiomarkers:
  {
    auto uid = QUuid::createUuid();
    QueryGuard::registerGuard(uid, 3, [overlay]()
                              {
                                overlay->deleteLater();
                              });
    API::REST::Tables::GetLibraryElements(GenesisContextRoot::ProjectId(), 1,
                                          [uid](QNetworkReply*, QVariantMap data)
                                          {
                                            auto array = data["children"].toList();
                                            QList<LibElem> compounds;
                                            for(int i = 0; i < array.size(); i++)
                                            {
                                              auto entry = array[i].toMap();
                                              LibElem elem;
                                              elem.type = 1;
                                              elem.libraryElementId = entry["library_element_id"].toInt();
                                              elem.libraryGroupId = entry["library_group_id"].toInt();
                                              elem.shortTitle = entry["short_title"].toString();
                                              elem.fullTitle = entry["full_title"].toString();
                                              compounds << elem;
                                            }
                                            compounds.prepend({tr("Unknown compound"), tr("Unknown"), -1, -1, 1});
                                            LibElemEditor::setCompounds(compounds);
                                            QueryGuard::reportReady(uid);
                                          },
                                          [uid](QNetworkReply*, QNetworkReply::NetworkError err)
                                          {
                                            QueryGuard::reportReady(uid);
                                          });
    API::REST::Tables::GetLibraryElements(GenesisContextRoot::ProjectId(), 2,
                                          [uid](QNetworkReply*, QVariantMap data)
                                          {
                                            auto array = data["children"].toList();
                                            QList<LibElem> coefficients;
                                            for(int i = 0; i < array.size(); i++)
                                            {
                                              auto entry = array[i].toMap();
                                              LibElem elem;
                                              elem.type = 2;
                                              elem.libraryElementId = entry["library_element_id"].toInt();
                                              elem.libraryGroupId = entry["library_group_id"].toInt();
                                              elem.shortTitle = entry["short_title"].toString();
                                              elem.fullTitle = entry["full_title"].toString();
                                              coefficients << elem;
                                            }
                                            coefficients.prepend({tr("Unknown coefficient"), tr("Unknown"), -1, -1, 2});
                                            LibElemEditor::setCoefficients(coefficients);
                                            QueryGuard::reportReady(uid);
                                          },
                                          [uid](QNetworkReply*, QNetworkReply::NetworkError err)
                                          {
                                            QueryGuard::reportReady(uid);
                                          });
    API::REST::GetXlsColumnsCombos("ImportedTableIdentified",
                                   [&parsePassportEntries, uid](QNetworkReply*, QJsonDocument doc)
                                   {
                                     auto jroot = doc.object();
                                     auto jresult = jroot["result"].toObject();
                                     parsePassportEntries(jresult);
                                     mStringKeysContexts[StringKeyContext::kDataType] =
                                         {
                                             {tr("Don't import"), "dont_import", StringKeyContext::kDataType},
                                             {tr("Sample title"), "sample_title", StringKeyContext::kDataType},
                                             {tr("Geo data"), "geo_data", StringKeyContext::kDataType},
                                             {tr("Compounds"), "compounds", StringKeyContext::kDataType},
                                             {tr("Coefficients"), "coefficients", StringKeyContext::kDataType}
                                         };
                                     QueryGuard::reportReady(uid);
                                   },
                                   [uid](QNetworkReply*, QNetworkReply::NetworkError err)
                                   {
                                     Notification::NotifyError(tr("Error"), err);
                                     QueryGuard::reportReady(uid);
                                   });
    break;
  }
  case Names::ModulesContextTags::MPlots:
  case Names::ModulesContextTags::MNoModule:
  case Names::ModulesContextTags::MLast:
  default:
    break;
  }
}
