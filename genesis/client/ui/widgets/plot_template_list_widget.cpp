#include "plot_template_list_widget.h"
#include "ui_plot_template_list_widget.h"
#include "api/api_rest.h"
#include "genesis_style/style.h"
#include "logic/context_root.h"
#include "logic/known_context_tag_names.h"
#include "logic/models/plot_template_model.h"
#include "logic/notification.h"
#include "logic/tree_model_dynamic_library_compounds_additional.h"
#include "logic/tree_model_dynamic_library_templates.h"
#include "logic/tree_model_item.h"
#include "logic/tree_model_presentation.h"
#include "ui/dialogs/templates/confirm.h"
#include "ui/genesis_window.h"
#include "ui/view_page_library.h"
#include "logic/enums.h"

#include <QMenu>
#include <QAction>

using namespace TemplatePlot;

namespace
{
  const QString template_type_id = "template_type_id";
}
PlotTemplateListWidget::PlotTemplateListWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PlotTemplateListWidget)
    , m_selectedId(-1)
    , m_library(nullptr)
{
  setupUi();
  setupConnections();
}

PlotTemplateListWidget::~PlotTemplateListWidget()
{
  delete ui;
}

void PlotTemplateListWidget::setupUi()
{
  ui->setupUi(this);
  setMode(Mode::ViewMode);
  ui->lExistTemplateList->setStyleSheet(Style::Genesis::GetH2());
  ui->lAddTemplate->setStyleSheet(Style::Genesis::GetH2());
  ui->tvExistTemplateList->setStyleSheet(Style::ApplySASS("QTreeView::branch:selected{border: 3px solid; "
      "border-left-color: transparent; "
      "border-right-color: @brandColor;"
      "border-top-color: transparent;"
      "border-bottom-color: transparent;}"));
  ui->tvExistTemplateList->style()->polish(ui->tvExistTemplateList);
  ui->tvExistTemplateList->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
  ui->tvExistTemplateList->header()->setStretchLastSection(false);

  ui->pbCancel->setProperty("secondary", true);
  ui->pbNextStep->setProperty("blue", true);
  ui->pbAddExistTemplate->setProperty("blue", true);

  m_model = new TreeModelDynamicLibraryCompoundsAdditional(this, ElementType::PLOT_TEMPLATE);
  m_existTemplateProxyModel = m_model->GetPresentationModel();

  m_itemModel = new PlotTemplateModel(this);
  ui->wdtEditTemplate->setModel(m_itemModel);
  ui->wdtDetails->setModel(m_itemModel);

  {
    using CS = TreeModelDynamicLibraryCompounds::Column;
    QSet<int> FilteredColumns =
    {
      CS::CHECKED,
      CS::CLASSIFIER_ID,
      CS::COMPOUND_CLASS_TYPE_ID,
      CS::LIBRARY_ELEMENT_ID,
      CS::LIBRARY_GROUP_ID,
      CS::ALL_GROUPS_TITLES,
      CS::PDF_LOAD_DATE,
      CS::PDF_SIZE,
      CS::XLSX_SIZE,
      CS::XLSX_LOAD_DATE,
      CS::XLSX_TITLE,
      CS::PDF_TITLE,
      CS::LAST_COLUMN,
      CS::MASS_SPECTRUM,
      CS::INTENSITY_SPECTRUM,
      CS::TEMPLATE_TYPE_ID,
      CS::TEMPLATE_X_AXIS_ID,
      CS::TEMPLATE_Y_AXIS_ID,
      CS::TEMPLATE_Z_AXIS_ID,
      CS::TEMPLATE_OBJECTS,
      CS::TEMPLATE_X_AXIS_TITLE,
      CS::TEMPLATE_Y_AXIS_TITLE,
      CS::TEMPLATE_Z_AXIS_TITLE,
      CS::TEMPLATE_AXES_TITLES,
      CS::FULL_TITLE,
      CS::FORMULA,
      CS::CHEMICAL_FORMULA,
    };
    auto columns = m_model->getColumns().keys();
    QSet<int> acceptedColumns = QSet<int>(columns.begin(), columns.end()).subtract(FilteredColumns);
    m_existTemplateProxyModel->SetAcceptedColumns(acceptedColumns);
  }

  ui->tvExistTemplateList->setModel(m_existTemplateProxyModel);
  ui->tvExistTemplateList->header()->resizeSections(QHeaderView::ResizeToContents);
  setMode(Mode::ViewMode);
  ui->pbControlMenu->setProperty("menu_secondary", true);

  ui->wdtTemplateList->setCaptionLabel(tr("Template list"));

  auto controlMenu = new QMenu(ui->pbControlMenu);
  auto editTemplateAction = new QAction(tr("Edit"));
  controlMenu->addAction(editTemplateAction);
  m_shareAccessAction = new QAction(tr("Share access"));
  controlMenu->addAction(m_shareAccessAction);
  auto removeCompoundAction = new QAction(tr("Remove"));
  controlMenu->addAction(removeCompoundAction);
  m_makeSystemAction = new QAction(tr("Make system"));
  controlMenu->addAction(m_makeSystemAction);
  m_makeSystemAction->setVisible(Core::GenesisContextRoot::Get()->GetContextUser()->GetData(Names::UserContextTags::kSuperuser).toBool());

  connect(editTemplateAction, &QAction::triggered, this, &PlotTemplateListWidget::beginModifying);
  connect(m_shareAccessAction, &QAction::triggered, this, &PlotTemplateListWidget::beginSelectedSharing);
  connect(removeCompoundAction, &QAction::triggered, this, &PlotTemplateListWidget::beginSelectedRemoving);
  connect(m_makeSystemAction, &QAction::triggered, this, &PlotTemplateListWidget::makeSystemic);

  ui->pbControlMenu->setMenu(controlMenu);
  ui->pbControlMenu->style()->polish(ui->pbControlMenu);
  ui->pbControlMenu->setProperty("disabled_for_system", false);

}

void PlotTemplateListWidget::setPageLibrary(Views::ViewPageLibrary *library)
{
  m_library = library;
}

void PlotTemplateListWidget::setAxesList(QList<Structures::KeyString> keyData)
{
  ui->wdtEditTemplate->setAxesList(keyData);
}

void PlotTemplateListWidget::setMode(Mode mode)
{
  m_mode = mode;
  switch (m_mode)
  {
  case Mode::NoneMode:
    m_selectedId = -1;
  case Mode::ViewMode:
  {
    const auto selectedIdIsValid = m_selectedId > 0;
    ui->stwdtList->setCurrentWidget(ui->templateListViewPage);
    ui->lAddTemplate->setVisible(selectedIdIsValid);
    ui->pbControlMenu->setVisible(selectedIdIsValid);
    ui->lAddTemplate->setText(m_selectedName);
    ui->stwdtTemplate->setCurrentWidget(selectedIdIsValid ? ui->templateViewPage : ui->EmptyPage);
    ui->pbNextStep->setVisible(false);
    ui->pbCancel->setVisible(false);
    ui->pbAddExistTemplate->setVisible(false);
    emit disableFiltes(false);
    break;
  }
  case Mode::AddMode:
    m_selectedId = -1;
  case Mode::EditMode:
    ui->stwdtList->setCurrentWidget(ui->templateListEditPage);
    ui->stwdtTemplate->setCurrentWidget(ui->templateEditPage);
    ui->lAddTemplate->setText(m_selectedId > 0 ? tr("Edit plot template") : tr("Add plot template"));
    ui->lAddTemplate->setVisible(true);
    ui->pbNextStep->setVisible(true);
    ui->pbCancel->setVisible(true);
    ui->pbAddExistTemplate->setVisible(false);
    ui->pbControlMenu->setVisible(false);
    emit disableFiltes(true);
    break;
  case Mode::AddExistToGroup:
    ui->stwdtList->setCurrentWidget(ui->templateListEditPage);
    ui->stwdtTemplate->setCurrentWidget(ui->templateViewPage);
    ui->lAddTemplate->setText(m_selectedName);
    ui->lAddTemplate->setVisible(true);
    ui->pbNextStep->setVisible(false);
    ui->pbCancel->setVisible(true);
    ui->pbAddExistTemplate->setVisible(true);
    ui->pbControlMenu->setVisible(false);
    emit disableFiltes(true);
    break;

  case Mode::CreationPlotMode:
    ui->stwdtTemplate->setCurrentWidget(m_selectedId > 0 ? ui->templateViewPage : ui->EmptyPage);
    ui->lAddTemplate->setText(m_selectedName);
    ui->lAddTemplate->setVisible(true);
    ui->stwdtList->setCurrentWidget(ui->templateViewPage);
    ui->pbNextStep->setVisible(false);
    ui->pbCancel->setVisible(false);
    ui->pbAddExistTemplate->setVisible(false);
    ui->pbControlMenu->setVisible(false);
    ui->wdtTemplateList->setSelectedMode();
    emit disableFiltes(false);
  }
}

QPointer<TreeModelDynamicLibraryTemplates> PlotTemplateListWidget::getPlotListModel()
{
  return ui->wdtTemplateList->getModel();
}

int PlotTemplateListWidget::getPlotType() const
{
  return m_model->MainItem()->GetData(template_type_id).toInt();
}

void PlotTemplateListWidget::beginAdding()
{
  m_model->Clear();
  ui->wdtEditTemplate->clear();
  setMode(Mode::AddMode);
}

void PlotTemplateListWidget::beginModifying()
{
  m_model->Clear();
  ui->wdtEditTemplate->clear();
  auto item = ui->wdtTemplateList->getCurrentItem();
  if (!item)
  {
    Notification::NotifyWarning(tr("Pick item for modifying"));
    return;
  }
  ui->wdtEditTemplate->setData(item->GetData());
  setMode(Mode::EditMode);
}

void PlotTemplateListWidget::leaveModifying()
{
  //disableUiCombobox(false);
  setMode(Mode::ViewMode);
}

void PlotTemplateListWidget::saveEditableTemplate()
{
  if (!ui->wdtEditTemplate->hasChanges())
  {
    leaveModifying();
    return;
  }
  QString validityErrorMsg;

  QVariantMap data = ui->wdtEditTemplate->getData();
  if (!checkInputValidity(data, &validityErrorMsg))
  {
    Notification::NotifyError(validityErrorMsg, tr("Input error"));
    return;
  }

  auto loadingOverlayId = GenesisWindow::Get()->ShowOverlay(tr("Uploading data"));

  auto makeValue = [](const QString& str)->QJsonValue
    {
      return str.isEmpty() ? QJsonValue() : QJsonValue(str);
    };

  // for (auto key: data.keys())
  // {
  //   qDebug() << key << ": " << data[key];
  // }

  QJsonObject outdata;// = QJsonObject::fromVariantMap(data);//TODO: check it
  outdata["element_type_id"] = 3;
  if (m_selectedId > 0)
    outdata["library_element_id"] = data["library_element_id"].toInt();
  outdata["title"] = makeValue(data["short_title"].toString());
  outdata["classifier_name"] = makeValue(data["classifier_title"].toString());
  outdata["compound_class_name"] = makeValue(data["compound_class_title"].toString());
  outdata["specific_name"] = makeValue(data["specific_name"].toString());
  outdata["tic_sim"] = makeValue(data["tic_sim"].toString());
  outdata["mz"] = makeValue(data["mz"].toString());
  outdata["bibliographical_reference"] = makeValue(data["bibliographical_reference"].toString());
  outdata["literature_source"] = makeValue(data["literature_source"].toString());
  outdata[template_type_id] = data[template_type_id].toInt();
  outdata["template_x_axis_library_element_id"] = data["template_x_axis_library_element_id"].toInt();
  outdata["template_y_axis_library_element_id"] = data["template_y_axis_library_element_id"].toInt();
  outdata["template_z_axis_library_element_id"] = data["template_z_axis_library_element_id"].toInt();
  outdata["template_objects"] = data["template_objects"].toJsonObject();
  outdata["group_name"] = makeValue(m_groupName);
  int pdfFileId = data["pdf_id"].toInt();
  
  // for (auto key: outdata.keys())
  // {
  //   qDebug() << key << ": " << data[key];
  // }
  QString name = outdata["short_title"].toString();
  QString groupName = outdata["group_name"].toString();
  API::REST::AddNewOrModifyPlotTemplate(outdata,
      [name, groupName, this, pdfFileId, loadingOverlayId](QNetworkReply*, QJsonDocument doc)
      {
        auto obj = doc.object();
        if (obj["error"].toBool())
        {
          Notification::NotifyError(doc.object()["msg"].toString(), "Server error");
          GenesisWindow::Get()->RemoveOverlay(loadingOverlayId);
          return;
        }
        bool hasAnyAdditionalOperation = false;
        Notification::NotifySuccess(tr("Template %1 added to group %2").arg(name).arg(groupName));
        leaveModifying();
        int createdLibraryGroupId = obj["created_library_group_id"].toInt();
        auto pdfOp = ui->wdtEditTemplate->getPdfOperation();
        if (pdfOp == PlotTemplateEditWidget::OPRemove)
        {
          if (!m_library)
            m_library->removeAttachments(createdLibraryGroupId,
                                         pdfFileId,
                                         loadingOverlayId,
                                         pdfOp == PlotTemplateEditWidget::OPRemove, false);
          connect(m_library, &Views::ViewPageLibrary::coefficientUpdatingComplete, this,
              [this, createdLibraryGroupId]()
              {
                TreeModelDynamic::ResetInstances("TreeModelDynamicLibraryTemplates");
                ui->wdtTemplateList->setCurrentLibraryGroupId(createdLibraryGroupId);
              }, Qt::SingleShotConnection);
          hasAnyAdditionalOperation = true;
        }
        QString pdfPath;
        if (pdfOp == PlotTemplateEditWidget::OPModify)
          pdfPath = ui->wdtEditTemplate->getPdfPath();
        if (!pdfPath.isEmpty())
        {
          bool hasAnyUploading = m_library->loadAttachments(createdLibraryGroupId, loadingOverlayId, pdfPath, "");
          if (!hasAnyUploading)
          {
            TreeModelDynamic::ResetInstances("TreeModelDynamicLibraryTemplates");
            ui->wdtTemplateList->setCurrentLibraryGroupId(obj["created_library_group_id"].toInt());
          }
          else
          {
            connect(m_library, &Views::ViewPageLibrary::coefficientUpdatingComplete, this,
                [this, createdLibraryGroupId]()
                {
                  TreeModelDynamic::ResetInstances("TreeModelDynamicLibraryTemplates");
                  ui->wdtTemplateList->setCurrentLibraryGroupId(createdLibraryGroupId);
                }, Qt::SingleShotConnection);
          }
          hasAnyAdditionalOperation = true;
        }
        else
        {
          TreeModelDynamic::ResetInstances("TreeModelDynamicLibraryTemplates");
        }
        if (!hasAnyAdditionalOperation)
        {
          GenesisWindow::Get()->RemoveOverlay(loadingOverlayId);
        }

      },
      [loadingOverlayId](QNetworkReply*, QNetworkReply::NetworkError e)
      {
        Notification::NotifyError(tr("Network error"), e);
        GenesisWindow::Get()->RemoveOverlay(loadingOverlayId);
      });
}

void PlotTemplateListWidget::addExisted()
{
  auto selModel = ui->tvExistTemplateList->selectionModel();
  auto currentInd = selModel->currentIndex();
  auto currentItem = m_model->GetItem(m_existTemplateProxyModel->mapToSource(currentInd));
  int id = currentItem->GetData("library_group_id").toInt();
  QString name = currentItem->GetData("short_title").toString();
  QString groupName = m_groupName;
  auto loadingOverlayId = GenesisWindow::Get()->ShowOverlay(tr("Uploading data"));
  API::REST::AddExistedPlotTemplate(groupName, id,
      [name, groupName, this, loadingOverlayId](QNetworkReply*, QJsonDocument doc)
      {
        auto obj = doc.object();
        if (obj["error"].toBool())
        {
          Notification::NotifyError(doc.object()["msg"].toString(), "Server error");
          return;
        }
        Notification::NotifySuccess(tr("Template %1 added to group %2").arg(name).arg(groupName));
        leaveModifying();
        TreeModelDynamic::ResetInstances("TreeModelDynamicLibraryTemplates");
        ui->wdtTemplateList->setCurrentLibraryGroupId(obj["library_group_id"].toInt());
        GenesisWindow::Get()->RemoveOverlay(loadingOverlayId);
      },
      [loadingOverlayId](QNetworkReply*, QNetworkReply::NetworkError e)
      {
        Notification::NotifyError(tr("Network error"), e);
        GenesisWindow::Get()->RemoveOverlay(loadingOverlayId);
      });
}


void PlotTemplateListWidget::beginSelectedSharing()
{
  auto libraryGroupId = ui->wdtDetails->libraryGroupId();
  ui->wdtTemplateList->startItemSharing(libraryGroupId);

}

void PlotTemplateListWidget::beginSelectedRemoving()
{
  auto libraryGroupId = ui->wdtDetails->libraryGroupId();
  ui->wdtTemplateList->startItemRemoving(libraryGroupId);
}

void PlotTemplateListWidget::makeSystemic()
{
  auto libraryGroupId = ui->wdtDetails->libraryGroupId();
  API::REST::MakeCompoundSystemic(libraryGroupId,
                                  [](QNetworkReply*, QJsonDocument doc)
  {
    auto obj = doc.object();
    // qDebug().noquote() << doc.toJson();
    if (obj["error"].toBool())
    {
      Notification::NotifyError(obj["msg"].toString(), tr("Error while making template systemic"));
      TreeModelDynamic::ResetInstances("TreeModelDynamicLibraryTemplates");
      return;
    }
    Notification::NotifySuccess(obj["msg"].toString(), tr("Template made systemic"));
    TreeModelDynamic::ResetInstances("TreeModelDynamicLibraryTemplates");
  },
  [](QNetworkReply*, QNetworkReply::NetworkError err)
  {
    Notification::NotifyError(tr("Network error"), err);
    TreeModelDynamic::ResetInstances("TreeModelDynamicLibraryTemplates");
  });
}

void PlotTemplateListWidget::FilterByState(Structures::LibraryFilterState state)
{
  ui->wdtTemplateList->FilterByState(state);
  m_groupName = state.Group;
}

void PlotTemplateListWidget::disableUiForOpenShare(bool disable)
{
  ui->pbControlMenu->setDisabled(disable || ui->pbControlMenu->property("disabled_for_system").toBool());
}

void PlotTemplateListWidget::setupConnections()
{
  connect(ui->wdtTemplateList, &LibraryPlotTemplateWidget::PickedItem, this, &PlotTemplateListWidget::onPicked);
  connect(ui->wdtTemplateList, &LibraryPlotTemplateWidget::AddPlotTemplateActionTriggered, this, &PlotTemplateListWidget::beginAdding);

  connect(ui->wdtEditTemplate, &PlotTemplateEditWidget::dataChanged, this,
    [this](const QVariantMap& data)
    {
      auto item = m_model->MainItem();
      auto itemData = item->GetData();
      itemData.insert(data);
      item->SetData(itemData);
    });

  connect(ui->wdtEditTemplate, &PlotTemplateEditWidget::titlesChanged, m_model, [this](const QString & title)
          {
    m_model->FindRelated(title, title);
  });

  connect(m_model, &TreeModelDynamicLibraryCompoundsAdditional::subHeaderIndexChanged, this,
    [this](const QModelIndex& newSubHeaderIndex)
    {
      m_existTemplateProxyModel->invalidate();
      auto mappedIndex = m_existTemplateProxyModel->mapFromSource(newSubHeaderIndex);
      ui->tvExistTemplateList->setProperty("sub_header_index", mappedIndex);
      ui->tvExistTemplateList->setFirstColumnSpanned(mappedIndex.row(), mappedIndex.parent(), true);
    },
    Qt::QueuedConnection);

  connect(ui->tvExistTemplateList->selectionModel(), &QItemSelectionModel::currentRowChanged, this,
    [this](const QModelIndex& current, const QModelIndex& previous)
    {
      auto indexVal = ui->tvExistTemplateList->property("sub_header_index");
      if (!indexVal.isValid() || indexVal.isNull())
        return;
      auto subHeaderIndex = indexVal.value<QModelIndex>();
      if (subHeaderIndex.isValid() && current.row() == subHeaderIndex.row())
        return;
      if (current.row() == m_existTemplateProxyModel->mapFromSource(m_model->MainItem()->GetIndex(1)).row())
      {
        setMode(Mode::EditMode);
      }
      else
      {
        setMode(Mode::AddExistToGroup);
        auto currentItem = m_model->GetItem(m_existTemplateProxyModel->mapToSource(current));
        qlonglong groupId = currentItem->GetData().value("library_group_id").toInt();
        QVariantMap data = ui->wdtTemplateList->GetItemByLibraryGroupId(groupId)->GetData();
        if (!data.contains("template_objects"))
          sendRequestTemplateObject(groupId);
        ui->wdtDetails->setData(data);
      }
    });

  connect(ui->pbNextStep, &QPushButton::clicked, this, [this](){
    auto data = ui->wdtEditTemplate->getData();
    QString validityErrorMsg;
    if (checkInputValidity(data, &validityErrorMsg))
    {
      emit nextStep();
    }
    else
    {
      Notification::NotifyError(validityErrorMsg, tr("Input error"));
    }
  });

  connect(ui->pbControlMenu, &QPushButton::clicked, this, &PlotTemplateListWidget::beginModifying);
  connect(ui->pbCancel, &QPushButton::clicked, this, &PlotTemplateListWidget::leaveModifying);
  connect(ui->pbAddExistTemplate, &QPushButton::clicked, this, &PlotTemplateListWidget::addExisted);

  connect(this, &PlotTemplateListWidget::saveChanges, this, &PlotTemplateListWidget::saveEditableTemplate);


  connect(ui->wdtTemplateList, &LibraryPlotTemplateWidget::ActionModePlotTemplateTriggered,
          this, &PlotTemplateListWidget::ActionModeTriggered);
  connect(ui->wdtTemplateList, &LibraryPlotTemplateWidget::CancelAccessPlotTemplate,
          this, &PlotTemplateListWidget::CancelAccess);

  connect(ui->wdtTemplateList->getModel(), &TreeModelDynamicLibraryTemplates::templateObjectsUploaded, this, &PlotTemplateListWidget::updateTemplateObjects);
}

bool PlotTemplateListWidget::checkInputValidity(const QVariantMap& data, QString* msg)
{
  auto validAndNotNull = [](const QVariant& val)->bool
    {return val.isValid() && !val.isNull(); };
  auto checkByTag = [&validAndNotNull, &data](const QString& tag)->bool
    {return validAndNotNull(data[tag]); };
  bool dataValidity = checkByTag("short_title") &&
    checkByTag("template_x_axis_library_element_id") &&
    checkByTag("template_y_axis_library_element_id") &&
    checkByTag("classifier_title");

  if (data["template_type_id"].toInt() == static_cast<int>(PlotType::Triplot))
    dataValidity &= checkByTag("template_z_axis_library_element_id");

  if (!dataValidity && msg)
  {
    auto makeMsg = [msg](const QString fieldTag)
    {
      if (msg->isEmpty())
        *msg = tr("Field %1 is empty").arg(fieldTag);
      else
        msg->append(QString("\n") + tr("Field %1 is empty").arg(fieldTag));
    };
    if (!checkByTag("short_title"))
      makeMsg(tr("Title"));
    if(!checkByTag("classifier_title"))
      makeMsg(tr("Classifier"));
    if (!checkByTag("template_x_axis_library_element_id"))
      makeMsg(tr("X axis"));
    if (!checkByTag("template_y_axis_library_element_id"))
      makeMsg(tr("Y axis"));
    if (data["template_type_id"].toInt() == static_cast<int>(PlotType::Triplot)
      && !checkByTag("template_z_axis_library_element_id"))
      makeMsg(tr("Z axis"));
  }

  return dataValidity;
}

void PlotTemplateListWidget::onPicked(const QVariantMap& item)
{
  if (item.isEmpty())
  {
    m_selectedId = -1;
    setMode(Mode::ViewMode);
    return;
  }
  m_selectedId = item.value("library_group_id").toInt();
  m_selectedName = item.value("short_title").toString();
  if (!item.contains("template_objects"))
    sendRequestTemplateObject(m_selectedId);
  switch (m_mode)
  {
  case Mode::NoneMode:
    setMode(Mode::ViewMode);
  case Mode::ViewMode:
  {
    ui->wdtDetails->setData(item);
    const auto userContext = Core::GenesisContextRoot::Get()->GetContextUser();
    const bool isSystemItem = item["system"].toBool();
    const bool isSuperuser = userContext->GetData(Names::UserContextTags::kSuperuser).toBool();
    const bool isDisabled = (isSystemItem && !isSuperuser) || !item.contains("template_objects");
    ui->pbControlMenu->setDisabled(isDisabled);
    ui->pbControlMenu->setProperty("disabled_for_system", isDisabled);
    m_makeSystemAction->setVisible(!isSystemItem && isSuperuser);
    m_shareAccessAction->setVisible(!isSystemItem);
    setMode(Mode::ViewMode);
  }
  break;
  case Mode::EditMode:
    setMode(Mode::EditMode);
    if (ui->wdtEditTemplate->hasChanges())
    {
      auto confirmation = Dialogs::Templates::Confirm::confirmation(this, tr("Warning"),
        tr("You have unsaved changes, wich you will lose if continue"),
        QDialogButtonBox::Save | QDialogButtonBox::Ignore | QDialogButtonBox::Discard);
      auto settings = confirmation->getSettings();
      settings.buttonsProperties[QDialogButtonBox::Ignore]["red"] = true;
      confirmation->applySettings(settings);
      connect(confirmation, &Dialogs::Templates::Confirm::Clicked, this,
        [this, item, confirmation](QDialogButtonBox::StandardButton button)
        {
          if (button == QDialogButtonBox::Ignore)
            ui->wdtEditTemplate->setData(item);
          confirmation->deleteLater();
        });
      confirmation->Open();
    }
    else
    {
      ui->wdtEditTemplate->setData(item);
    }
    break;
  case Mode::CreationPlotMode:
  {
    ui->lAddTemplate->setText(item["short_title"].toString());
    ui->wdtDetails->setData(item);
    if (m_selectedId > 0)
    {
      ui->stwdtTemplate->setCurrentWidget(ui->templateViewPage);
    }
    else
    {
      ui->stwdtTemplate->setCurrentWidget(ui->EmptyPage);
    }
    break;
  }
  default:
    break;
  }
}

void PlotTemplateListWidget::sendRequestTemplateObject(int libraryGroupId)
{
  ui->wdtTemplateList->getModel()->getTemplateObjectsFromServer(libraryGroupId);
  ui->pbControlMenu->setDisabled(true);
  ui->pbControlMenu->setProperty("disabled_for_system", true);
  ui->wdtDetails->setDisablePlot(true);
}

void PlotTemplateListWidget::updateTemplateObjects(int libraryGroupId, bool isOk)
{
  if (m_selectedId == libraryGroupId)
  {
    TreeModelItem *item = ui->wdtTemplateList->getModel()->GetItemByLibraryGroupId(libraryGroupId);
    if (!item)
      return;
    auto json = item->GetData("template_objects").toJsonObject();
    m_itemModel->loadFromJson(json);
    ui->wdtDetails->setDisablePlot(!isOk);
    const auto userContext = Core::GenesisContextRoot::Get()->GetContextUser();
    const bool isSystemItem = item->GetData("system").toBool();
    const bool isSuperuser = userContext->GetData(Names::UserContextTags::kSuperuser).toBool();
    const bool isDisabled = (isSystemItem && !isSuperuser) || json.isEmpty();
    ui->pbControlMenu->setDisabled(isDisabled);
    ui->pbControlMenu->setProperty("disabled_for_system", isDisabled);
  }
}


void PlotTemplateListWidget::showTemplatesForTable(int tableId)
{
  m_tableId = tableId;
  m_selectedId = -1;
  ui->wdtTemplateList->uploadTemplatesForTable(m_tableId);
  setMode(Mode::CreationPlotMode);
  using CS = TreeModelDynamicLibraryCompounds::Column;
  QSet<int> FilteredColumns =
    {
      CS::CHECKED,
      CS::FAVOURITES,
      CS::GROUP,
      CS::SHORT_TITLE,
      CS::TEMPLATE_AXES_TITLES,
    };
  m_existTemplateProxyModel->SetAcceptedColumns(FilteredColumns);
}
