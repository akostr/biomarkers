#include "identification_plot_widget.h"
#include "ui_identification_plot_widget.h"

#include "genesis_style/style.h"

#include "logic/tree_model_dynamic_library_templates.h"
#include "logic/tree_model_item.h"
#include "logic/service/igraphics_plot_cortesian_transform.h"
#include "ui/controls/dialog_line_edit_ex.h"
#include "ui/dialogs/templates/dialog.h"
#include "ui/dialogs/web_dialog_legend_settings.h"
#include "ui/item_models/analysis_data_table_model.h"
#include "ui/plots/gp_items/gpshape_item.h"
#include "ui/widgets/plot_legend_widget_refactor.h"
#include "ui/widgets/legend/legend_group_editor.h"
#include "logic/models/reference_analysis_data_table_model.h"
#include "logic/service/igraphics_plot_axis_modificator.h"
#include "logic/service/service_locator.h"
#include "logic/serializers/entity_model_json_serializer.h"
#include "legend/legend_editor.h"
#include "ui/plots/triplot/triplot.h"

#include "api/api_rest.h"
#include "logic/notification.h"

#include <common_gui/standard_widgets/tumbler.h>
#include <QSpinBox>

using namespace TemplatePlot;
using namespace Service;

IdentificationPlotWidget::IdentificationPlotWidget(QWidget *parent)
  : QWidget(parent)
  , ui(new Ui::IdentificationPlotWidget)
{
  mModel = new PlotTemplateModel(this);

  mEntityModel = new AnalysisEntityModel(this);

  setupUi();
  setupConnections();
}

IdentificationPlotWidget::~IdentificationPlotWidget()
{
  delete ui;
}

void IdentificationPlotWidget::setupUi()
{
  ui->setupUi(this);
  ui->plotWidget->setModel(mModel);
  ui->plotWidget->setEditMode(false);
  ui->selectCheckBox->setVisible(false);
  mMenu = new QMenu(ui->customizeButton);
  ui->customizeButton->setMenu(mMenu);
  ui->customizeButton->style()->polish(ui->customizeButton);
  ui->customizeButton->setStyleSheet(Style::Genesis::Fonts::H1());
  mEditHeader = new QAction(tr("Edit header"), mMenu);
  mEditLegendPosition = new QAction(tr("Edit legend positions"), mMenu);
  mEditHeight = new QAction(tr("Edit height plot"), mMenu);
  mEditLegend = new QAction(tr("Edit legend"), mMenu);
  mMenu->addAction(mEditHeader);
  mMenu->addAction(mEditLegendPosition);
  mMenu->addAction(mEditHeight);
  mMenu->addAction(mEditLegend);

  auto wa = new QWidgetAction(mMenu);
  auto wgt = new QWidget;
  auto wgtL = new QVBoxLayout;
  wgt->setLayout(wgtL);
  mShowHeaderTumbler = new Tumbler(tr("Show header"), wgt);
  wgtL->addWidget(mShowHeaderTumbler);
  mShowLegendTumbler = new Tumbler(tr("Show Legend"), wgt);
  wgtL->addWidget(mShowLegendTumbler);
  mShowGridTumbler = new Tumbler(tr("Show grid"), wgt);
  wgtL->addWidget(mShowGridTumbler);
  wa->setDefaultWidget(wgt);
  mMenu->addAction(wa);

  mShowGridTumbler->setChecked(true);
  mShowLegendTumbler->setChecked(true);
  mShowHeaderTumbler->setChecked(true);

  ui->headerLabel->setStyleSheet(Style::Genesis::Fonts::H2());
  showLegendWidgets(true);
}

void IdentificationPlotWidget::setupConnections()
{
  connect(mEditHeader, &QAction::triggered, this, &IdentificationPlotWidget::openHeaderDialog);
  connect(mEditLegendPosition, &QAction::triggered, this, &IdentificationPlotWidget::openLegendPositionDialog);
  connect(mEditLegend, &QAction::triggered, this, &IdentificationPlotWidget::openLegendDialog);
  connect(mEditHeight, &QAction::triggered, this, &IdentificationPlotWidget::openHeigtDialog);

  connect(mShowHeaderTumbler, &Tumbler::toggled, this, [this](bool show) {
    ui->headerLabel->setVisible(show);
  });
  connect(mShowLegendTumbler, &Tumbler::toggled, this, &IdentificationPlotWidget::showLegendWidgets);
  connect(mShowGridTumbler, &Tumbler::toggled, this, [this](bool show) {
    ui->plotWidget->SetAxisListGridVisible(ui->plotWidget->axesList(), show);
  });
  connect(ui->selectCheckBox, &QCheckBox::stateChanged ,this ,&IdentificationPlotWidget::selectChanged);

  connect(ui->leftLegendWidget, &PlotLegendWidgetRefactor::itemEditingRequested, this, &IdentificationPlotWidget::onGroupEditRequested);
  connect(ui->topLegendWidget, &PlotLegendWidgetRefactor::itemEditingRequested, this, &IdentificationPlotWidget::onGroupEditRequested);
  connect(ui->bottomLegendWidget, &PlotLegendWidgetRefactor::itemEditingRequested, this, &IdentificationPlotWidget::onGroupEditRequested);
  connect(ui->rightLegendWidget, &PlotLegendWidgetRefactor::itemEditingRequested, this, &IdentificationPlotWidget::onGroupEditRequested);
  connect(mModel->getRootItem(), &RootItem::sizeChanged, this, &IdentificationPlotWidget::updatePlotSize);
}

void IdentificationPlotWidget::setData(int templateId,
                                  QPointer<TreeModelDynamicLibraryTemplates> templateListModel,
                                  int tableId,
                                  QString dataType)
{
  mGroupElementId = templateId;
  QVariantMap data = templateListModel->GetItemByLibraryGroupId(templateId)->GetData();
  const auto isTriplot = data["template_type_id"].toInt() == static_cast<int>(PlotType::Triplot);
  auto modificator = ServiceLocator::Instance().Resolve<Triplot, IGraphicsPlotAxisModificator>();
  ui->plotWidget->xAxis->setVisible(!isTriplot);
  ui->plotWidget->yAxis->setVisible(!isTriplot);
  ui->plotWidget->setLockAxesInteraction(isTriplot);
  if (isTriplot)
    modificator->ApplyModificator(ui->plotWidget);
  else
    modificator->UndoModificator(ui->plotWidget);
  ui->plotWidget->SetAxisListGridVisible(ui->plotWidget->axesList(), mShowGridTumbler->isChecked());
  if (!data.contains("template_objects"))
  {
    templateListModel->getTemplateObjectsFromServer(templateId);
    connect (templateListModel, &TreeModelDynamicLibraryTemplates::templateObjectsUploaded, this, [this, templateListModel](int groupId, bool isOk){
      if (isOk && groupId == mGroupElementId)
      {
        QVariantMap data = templateListModel->GetItemByLibraryGroupId(mGroupElementId)->GetData();
        setTemplate(templateListModel);
        disconnect(templateListModel, &TreeModelDynamicLibraryTemplates::templateObjectsUploaded, this, nullptr);
      }
    });
  }
  else
  {
    setTemplate(templateListModel);
  }

  int libraryGroupIdForX = data["template_x_axis_library_element_id"].toInt();
  int libraryGroupIdForY = data["template_y_axis_library_element_id"].toInt();
  int libraryGroupIdForZ = data["template_z_axis_library_element_id"].toInt();

  API::REST::uploadEntityForIdentificationPlot(mGroupElementId, tableId, dataType, {libraryGroupIdForX, libraryGroupIdForY, libraryGroupIdForZ },
    [libraryGroupIdForX, libraryGroupIdForY, this](QNetworkReply*, QJsonDocument result)
    {
      auto obj = result.object();
      setEntity(obj, libraryGroupIdForX, libraryGroupIdForY);
    },
    [this](QNetworkReply*, QNetworkReply::NetworkError err)
    {
      Notification::NotifyError(tr("Failed to load plot"), err);
    });
}

void IdentificationPlotWidget::setData(const QJsonObject &obj)
{
  mPlotId = obj["plot_id"].toInt();
  mTitle = obj["title"].toString();
  mComment = obj["comment"].toString();
  QJsonObject settings = obj["plot_settings"].toObject();
  mModel->loadFromJson(obj["plot_objects"].toObject());

  mElementId = obj["template_library_element_id"].toInt();

  if (obj.contains("plot_settings") && !obj["plot_settings"].toObject().empty())
  {
    QJsonObject plotSettings = obj["plot_settings"].toObject();
    ui->headerLabel->setText(plotSettings["header"].toString());
    mShowGridTumbler->setChecked(plotSettings["show_grid"].toBool());
    mShowLegendTumbler->setChecked(plotSettings["show_legend"].toBool());
    mShowHeaderTumbler->setChecked(plotSettings["show_grid"].toBool());
    if (plotSettings.contains("height_fixed"))
    {
      int height = plotSettings["height_fixed"].toInt();
      setFixedHeight(height);
    }

    Service::EntityModelJsonSerializer serializer;
    auto identityModel = serializer.ToModel(plotSettings["entity_model"].toObject());
    if (mEntityModel)
    {
      disconnect(mEntityModel, nullptr, this, nullptr);
    }
    identityModel->Exec<QPointer<AnalysisEntityModel>>
      (Service::AnalysisIdentityModel::EntityModel,
       [this](QPointer<AnalysisEntityModel> model) {
         mEntityModel = model;
       });
    if (mEntityModel)
    {
      ui->topLegendWidget->setModel(mEntityModel);
      ui->leftLegendWidget->setModel(mEntityModel);
      ui->rightLegendWidget->setModel(mEntityModel);
      ui->bottomLegendWidget->setModel(mEntityModel);
      connect(mEntityModel, &ShapeColorGroupedEntityPlotDataModel::groupColorChanged, this, &IdentificationPlotWidget::onGroupColorChanged);
      connect(mEntityModel, &ShapeColorGroupedEntityPlotDataModel::groupShapeChanged, this, &IdentificationPlotWidget::onGroupShapeChanged);
      connect(mEntityModel, &AbstractEntityDataModel::entityChanged, this, &IdentificationPlotWidget::onEntityChanged);
      connect(mEntityModel, &ShapeColorGroupedEntityPlotDataModel::modelReset, this, &IdentificationPlotWidget::onModelReset);
      onModelReset();
    }
  }
  else
  {
    ui->headerLabel->setText(mTitle);
    mShowGridTumbler->setChecked(true);
    mShowLegendTumbler->setChecked(true);
    mShowHeaderTumbler->setChecked(true);
  }
}

QJsonObject IdentificationPlotWidget::saveToJson(bool ignorePlotId)
{
  QJsonObject obj;
  obj.insert("title", mTitle);
  obj.insert("comment", mComment);
  obj.insert("plot_type_id", 1);
  obj.insert("plot_objects", mModel->saveToJson());
  obj.insert("template_library_element_id", mElementId);
  if (!ignorePlotId && mPlotId > 0)
    obj.insert("plot_id", mPlotId);

  QJsonObject plotSettings;
  plotSettings.insert("show_header", ui->headerLabel->isVisible());
  switch (mLegendPosition)
  {
  case QInternal::LeftDock:
    plotSettings.insert("show_grid", ui->leftLegendWidget->isVisible());
    plotSettings.insert("show_legend", ui->leftLegendWidget->isVisible());
    break;
  case QInternal::TopDock:
    plotSettings.insert("show_grid", ui->topLegendWidget->isVisible());
    plotSettings.insert("show_legend", ui->topLegendWidget->isVisible());
    break;
  case QInternal::RightDock:
    plotSettings.insert("show_grid", ui->rightLegendWidget->isVisible());
    plotSettings.insert("show_legend", ui->rightLegendWidget->isVisible());
    break;
  case QInternal::BottomDock:
    plotSettings.insert("show_grid", ui->bottomLegendWidget->isVisible());
    plotSettings.insert("show_legend", ui->bottomLegendWidget->isVisible());
    break;
  default:
    break;
  }
  plotSettings.insert("header", ui->headerLabel->text());
  if (maximumHeight() == minimumHeight())
  {
    plotSettings.insert("height_fixed", height());
  }
  if (mEntityModel)
  {
    EntityModelJsonSerializer serializer;
    Model::IdentityModelPtr identityModel = std::make_shared<Model::IdentityModel>();
    identityModel->Set<QPointer<AnalysisEntityModel>>(AnalysisIdentityModel::EntityModel, mEntityModel);
    plotSettings.insert("entity_model", serializer.ToJson(identityModel));
  }
  obj.insert("plot_settings", plotSettings);
  return obj;
}

bool IdentificationPlotWidget::saveToFileSystem(const QString &path)
{
  QList <QWidget*> hideControlWidgets;
  QList <QWidget*> controlWidget = {ui->selectCheckBox, ui->customizeButton};
  for (auto widget: controlWidget)
  {
    if (widget->isVisible())
    {
      widget->setVisible(false);
      hideControlWidgets.append(widget);
    }
  }
  QPixmap pixmap(ui->savedWidget->size());
  ui->savedWidget->render(&pixmap);
  QString filename = path + "//" + mTitle + ".png";
  QFile file(filename);
  file.open(QIODevice::WriteOnly);
  bool isOk = pixmap.save(&file, "PNG");

  for (auto widget: hideControlWidgets)
  {
    widget->setVisible(true);
  }

  return isOk;
}

void IdentificationPlotWidget::setTemplate(QPointer<TreeModelDynamicLibraryTemplates> templateListModel)
{
  QVariantMap data = templateListModel->GetItemByLibraryGroupId(mGroupElementId)->GetData();
  mModel->loadFromJson(data["template_objects"].toJsonObject());

  QString title = data["short_title"].toString();
  mElementId = data ["library_element_id"].toInt();
  ui->headerLabel->setText(title);
  mTitle = title;
}

void IdentificationPlotWidget::setEntity(QJsonObject obj, int libraryGroupIdForX, int libraryGroubIdForY)
{
  EntityModelJsonSerializer serializer;
  auto identityModel = serializer.GenerateFromRequest(obj, libraryGroupIdForX, libraryGroubIdForY);
  if (mEntityModel)
  {
    disconnect(mEntityModel, nullptr, this, nullptr);
  }

  identityModel->Exec<QPointer<AnalysisEntityModel>>
    (AnalysisIdentityModel::EntityModel,
     [this](QPointer<AnalysisEntityModel> model) {
       mEntityModel = model;
     });

  if (!mEntityModel)
    return;

  ui->leftLegendWidget->setModel(mEntityModel);
  ui->rightLegendWidget->setModel(mEntityModel);
  ui->topLegendWidget->setModel(mEntityModel);
  ui->bottomLegendWidget->setModel(mEntityModel);
  connect(mEntityModel, &ShapeColorGroupedEntityPlotDataModel::groupColorChanged, this, &IdentificationPlotWidget::onGroupColorChanged);
  connect(mEntityModel, &ShapeColorGroupedEntityPlotDataModel::groupShapeChanged, this, &IdentificationPlotWidget::onGroupShapeChanged);
  connect(mEntityModel, &AbstractEntityDataModel::entityChanged, this, &IdentificationPlotWidget::onEntityChanged);
  connect(mEntityModel, &ShapeColorGroupedEntityPlotDataModel::modelReset, this, &IdentificationPlotWidget::onModelReset);
  auto uids = mEntityModel->getUIdListOfEntities(AnalysisEntity::TypeSample);
  auto transform = ServiceLocator::Instance().Resolve<Triplot, IGraphicsPlotCortesianTransform>();
  const auto type = mModel->getRootItem()->plotType();
  double triplotAxisRange = 1;
  if (const auto rect = ui->plotWidget->axisRect())
  {
    if(const auto axis = rect->axis(GPAxis::atBottom, 1))
    {
      triplotAxisRange = axis->range().size();
    }
  }
  for (auto uid: uids)
  {
    auto shape = mModel->addShape();
    shape->setUuid(uid);
    auto pos = mEntityModel->currentEntityCoord(uid);
    // add triplot coord transform
    const auto xComp = mEntityModel->currentXComponent();
    const auto yComp = mEntityModel->currentYComponent();
    const auto data = mEntityModel->getEntity(uid);
    const auto triplotCoords = data->getData(AnalysisEntity::RoleNormedComponentData).value<TNormedComponentsData>();
    const auto orthoCoord = transform->toCortesian({ QPointF(triplotCoords.value(xComp) * triplotAxisRange, triplotCoords.value(yComp) * triplotAxisRange) });
    if (type == TemplatePlot::PlotType::Triplot)
      shape->setCoordinate(orthoCoord.front());
    else
      shape->setCoordinate(pos);
    shape->setName(data->getData(AnalysisEntity::RoleTitle).toString());
  }
  onModelReset();
}

void IdentificationPlotWidget::openHeaderDialog()
{
  using Btns = QDialogButtonBox;
  auto s = Dialogs::Templates::Dialog::Settings();
  s.buttonsNames[Btns::Ok] = tr("Apply");
  s.dialogHeader = tr("Edit header");
  auto w = new QWidget();
  auto l = new QVBoxLayout(w);

  auto ls = DialogLineEditEx::Settings();
  ls.textHeader = tr("Plot header") + Style::GetInputAlert();
  ls.textMaxLen = 50;
  ls.textPlaceholder = tr("Type new plot header");
  ls.defaultText = ui->headerLabel->text();
  ls.textRequired = true;
  ls.textTooltipsSet.empty = tr("Header should be non-empty");
  ls.textTooltipsSet.notChecked = tr("Not checked");
  ls.textTooltipsSet.valid = tr("Header is valid");
  auto headerLineEdit = new DialogLineEditEx(w, ls);
  l->addWidget(headerLineEdit);
  l->addStretch(1);

  auto dial = new Dialogs::Templates::Dialog(this, s, Btns::Ok | Btns::Cancel, w);
  connect(headerLineEdit, &DialogLineEditEx::validityChanged, dial, [dial](bool valid)
          {
            dial->SetButtonEnabled(Btns::Ok, valid);
          });
  dial->SetButtonEnabled(Btns::Ok, true);

  dial->Open();

  connect(dial, &WebDialog::Accepted, this, [this, dial, headerLineEdit]() {
      ui->headerLabel->setText( headerLineEdit->text());
  });
}

void IdentificationPlotWidget::openLegendDialog()
{
  std::optional<TPassportFilter> colorFilter;
  if(mEntityModel->modelData().contains(AnalysisEntity::ModelRoleColorFilter))
    colorFilter = mEntityModel->modelData(AnalysisEntity::ModelRoleColorFilter).value<TPassportFilter>();
  std::optional<TPassportFilter> shapeFilter;
  if(mEntityModel->modelData().contains(AnalysisEntity::ModelRoleShapeFilter))
    shapeFilter = mEntityModel->modelData(AnalysisEntity::ModelRoleShapeFilter).value<TPassportFilter>();

  auto dial = new Dialogs::LegendEditor(mEntityModel, this, colorFilter, shapeFilter);
  connect(dial, &Dialogs::LegendEditor::Accepted, this, [this, dial]()
          {
            auto colorFilter = dial->colorFilter();
            auto shapeFilter = dial->shapeFilter();
            auto groupInfo = dial->getGroups();
            {
              QSignalBlocker block(mEntityModel);
              mEntityModel->regroup(colorFilter.value_or(TPassportFilter()),
                                    shapeFilter.value_or(TPassportFilter()), groupInfo);
            }
            mEntityModel->setData(mEntityModel->getData());
          });
  dial->Open();
}

void IdentificationPlotWidget::openHeigtDialog()
{
  using Btns = QDialogButtonBox;
  auto s = Dialogs::Templates::Dialog::Settings();
  s.buttonsNames[Btns::Ok] = tr("Apply");
  s.dialogHeader = tr("Edit height plot");
  auto w = new QWidget();
  auto l = new QVBoxLayout(w);

  QLabel *title = new QLabel(w);
  title->setText(tr("Plot height"));
  l->addWidget(title);
  QSpinBox *spinbox = new QSpinBox(w);
  spinbox->setMinimum(0);
  spinbox->setMaximum(2048);
  spinbox->setValue(ui->savedWidget->size().height());
  l->addWidget(spinbox);
  QCheckBox *applyForAll = new QCheckBox(w);
  applyForAll->setText(tr("Apply to all plots"));
  l->addWidget(applyForAll);

  auto dial = new Dialogs::Templates::Dialog(this, s, Btns::Ok | Btns::Cancel, w);
  dial->SetButtonEnabled(Btns::Ok, true);

  dial->Open();

  connect(dial, &WebDialog::Accepted, this, [this, dial, spinbox, applyForAll]() {
    int anotherElements = this->size().height() - ui->savedWidget->size().height() - ui->bottomVerticalSpacer->geometry().height();
    int newHeight = spinbox->value() + anotherElements;
    emit needToUpdateHeight(newHeight, applyForAll->isChecked());
  });
}

void IdentificationPlotWidget::openLegendPositionDialog()
{
  auto dialog = new Dialogs::WebDialogLegendSettings(this);
  dialog->setLegendPosition(mLegendPosition);
  dialog->setLegendAlign(mLegendAlign);
  dialog->setApplyToAll(mApplyToAllPlots);
  connect(dialog, &WebDialog::Accepted, this,
    [&, dialog]()
    {
      showLegendBySettings(dialog->applyToAll(), dialog->legendAlign(), dialog->legendPosition());
    });
  dialog->Open();
}

bool IdentificationPlotWidget::isSelectable()
{
  return ui->selectCheckBox->isVisible();
}

void IdentificationPlotWidget::setSelectable(bool isSelected)
{
  ui->selectCheckBox->setVisible(isSelected);
}

bool IdentificationPlotWidget::isSelected()
{
  return ui->selectCheckBox->isChecked();
}

void IdentificationPlotWidget::setSelected(bool isChecked)
{
  ui->selectCheckBox->setCheckState(isChecked ? Qt::Checked : Qt::Unchecked);
}

QString IdentificationPlotWidget::title()
{
  return mTitle;
}

void IdentificationPlotWidget::setTitle(const QString& title)
{
  mTitle = title;
  emit titleChanged(mTitle);
}

QString IdentificationPlotWidget::header()
{
  return ui->headerLabel->text();
}

void IdentificationPlotWidget::setHeader(const QString& header)
{
  ui->headerLabel->setText(header);
}

QString IdentificationPlotWidget::comment()
{
  return mComment;
}

void IdentificationPlotWidget::setComment(const QString& comment)
{
  mComment = comment;
}

void IdentificationPlotWidget::onEntityChanged(EntityType type,
              TEntityUid eId,
              DataRoleType role,
              const QVariant& value)
{

  for (auto& item : mModel->getRootItem()->items())
  {
    if (item->type() == PlotTemplateItem::ShapeType &&
        eId == item->uuid())
    {
      if (ShapeItem *shape = qobject_cast<ShapeItem *>(item))
      {
        shape->setColor(mEntityModel->getActualColor(shape->uuid()));

        if (role == AnalysisEntity::RoleColor)
        {
          shape->setColor(mEntityModel->getActualColor(eId));
        }
        else if (role == AnalysisEntity::RoleShape)
        {
          shape->setShape(mEntityModel->getActualShape(eId));
        }
        else if (role == AnalysisEntity::RoleShapePixelSize)
        {
          auto size = mEntityModel->getActualShapeSize(eId);
          shape->setSize(size);
        }
      }
      break;
    }
  }
}

void IdentificationPlotWidget::onGroupColorChanged(QSet<TEntityUid> affectedEntities)
{
  for (auto& item : mModel->getRootItem()->items())
  {
    if (item->type() == PlotTemplateItem::ShapeType &&
        affectedEntities.contains(item->uuid()))
    {
      if (ShapeItem *shape = qobject_cast<ShapeItem *>(item))
      {
        shape->setColor(mEntityModel->getActualColor(shape->uuid()));
      }
    }
  }
}

void IdentificationPlotWidget::onGroupShapeChanged(QSet<TEntityUid> affectedEntities)
{
  for (auto& item : mModel->getRootItem()->items())
  {
    if (item->type() == PlotTemplateItem::ShapeType &&
        affectedEntities.contains(item->uuid()))
    {
      if (ShapeItem *shape = qobject_cast<ShapeItem *>(item))
      {
        auto shapeDescriptor = mEntityModel->getShapeDescriptor(shape->uuid());
        shape->setShape(shapeDescriptor.path);
        shape->setSize(shapeDescriptor.pixelSize);
        shape->setColor(mEntityModel->getActualColor(shape->uuid()));
      }
    }
  }
}

void IdentificationPlotWidget::onGroupEditRequested(const QUuid& uid)
{
  if(mEntityModel->getEntity(uid))
  {
    auto dial = new Dialogs::LegendGroupEditor(uid, mEntityModel, this);
    connect(dial, &Dialogs::LegendGroupEditor::removeGroupAndRegroup, this, &IdentificationPlotWidget::removeGroupAndRegroup);
    dial->Open();
  }
}

void IdentificationPlotWidget::removeGroupAndRegroup(const TEntityUid& groupUid)
{
  using namespace AnalysisEntity;
  auto ent = mEntityModel->getEntity(groupUid);
  auto groups = mEntityModel->getEntities(ent->getType());
  auto affectedEntities = mEntityModel->getEntities([groupUid](AbstractEntityDataModel::ConstDataPtr ent)->bool
                                                    {
                                                      auto gset = ent->getData(RoleGroupsUidSet).value<TGroupsUidSet>();
                                                      return gset.contains(groupUid);
                                                    });
  if(ent->getType() == TypeGroup)
  {
    for(auto& ent : affectedEntities)
    {
      mEntityModel->resetEntityGroup(ent->getUid(), groupUid);
      for(auto& g : groups)
      {
        if(!g->hasDataAndItsValid(RolePassportFilter))
          continue;
        auto filter = g->getData(RolePassportFilter).value<TPassportFilter>();
        auto passport = ent->getData(RolePassport).value<TPassport>();
        if(filter.match(passport))
        {
          mEntityModel->setEntityGroup(ent->getUid(), g->getUid());
          break;
        }
      }
    }
  }
  else if(ent->getType() == TypeHottelingGroup)
  {
    auto defGrps = mEntityModel->getEntitiesUidsSet([](AbstractEntityDataModel::ConstDataPtr ent)
                                                    {
                                                      return ent->getType() == TypeHottelingGroup && !ent->getData(RoleGroupUserCreated).toBool();
                                                    });
    for(auto& ent : affectedEntities)
    {
      mEntityModel->resetEntityGroup(ent->getUid(), groupUid);
      if(!defGrps.isEmpty())
        mEntityModel->setEntityGroup(ent->getUid(), *defGrps.begin());
    }
  }
  mEntityModel->removeEntity(groupUid);
}

void IdentificationPlotWidget::onModelReset()
{
  auto entities = mEntityModel->getUIdListOfEntities(AnalysisEntity::TypeSample);
  for (auto& item : mModel->getRootItem()->items())
  {
    if (item->type() == PlotTemplateItem::ShapeType &&
        entities.contains(item->uuid()))
    {
      if (ShapeItem *shape = qobject_cast<ShapeItem *>(item))
      {
        auto shapeDescriptor = mEntityModel->getShapeDescriptor(shape->uuid());
        shape->setShape(shapeDescriptor.path);
        shape->setSize(shapeDescriptor.pixelSize);
        shape->setColor(mEntityModel->getActualColor(shape->uuid()));
      }
    }
  }
}

void IdentificationPlotWidget::resizeEvent(QResizeEvent *event)
{
  updatePlotSize();
  QWidget::resizeEvent(event);
}

void IdentificationPlotWidget::updatePlotSize()
{
  QSize size = mModel->getRootItem()->size();
  if (!size.isValid() || size.height() < 0 || size.width() < 0)
    return;

  ui->plotWidget->setToDefaultAxisX();
  ui->plotWidget->setToDefaultAxisY();

  QSize maximalSize = ui->resizeWidget->size();
  double coefficent = (double)size.width() / size.height();
  size.setHeight(maximalSize.height());
  size.setWidth(maximalSize.height() * coefficent);

  if (maximalSize.width() < size.width())
  {
    size.setWidth(maximalSize.width());
    size.setHeight(maximalSize.width() / coefficent);
  }
  QGridLayout *grid = static_cast<QGridLayout *>(ui->resizeWidget->layout());
  if (grid)
  {
    grid->setColumnStretch(0, (maximalSize.width() - size.width())/2);
    grid->setColumnStretch(1, size.width());
    grid->setColumnStretch(2, (maximalSize.width() - size.width())/2);

    grid->setRowStretch(0, size.height());
    grid->setRowStretch(1, maximalSize.height() - size.height());
  }
}

void IdentificationPlotWidget::showLegendBySettings(bool applyAll, Qt::Alignment align, QInternal::DockPosition position)
{
  mApplyToAllPlots = applyAll;
  mLegendPosition = position;
  mLegendAlign = align;
  ui->leftLegendWidget->setVisible(mLegendPosition == QInternal::LeftDock);
  ui->topLegendWidget->setVisible(mLegendPosition == QInternal::TopDock);
  ui->bottomLegendWidget->setVisible(mLegendPosition == QInternal::BottomDock);
  ui->rightLegendWidget->setVisible(mLegendPosition == QInternal::RightDock);
  const auto plotSize = ui->plotWidget->size();
  switch (mLegendAlign)
  {
  case Qt::AlignLeft:
    ui->topLegendHorizontalSpacer->changeSize(0, 0, QSizePolicy::Fixed, QSizePolicy::Fixed);
    ui->bottomLegendHorizontalSpacer->changeSize(0, 0, QSizePolicy::Fixed, QSizePolicy::Fixed);
    break;
  case Qt::AlignHCenter:
    ui->topLegendHorizontalSpacer->changeSize(plotSize.width() / 2, 0, QSizePolicy::Expanding, QSizePolicy::Fixed);
    ui->bottomLegendHorizontalSpacer->changeSize(plotSize.width() / 2, 0, QSizePolicy::Expanding, QSizePolicy::Fixed);
    break;
  case Qt::AlignRight:
    ui->topLegendHorizontalSpacer->changeSize(plotSize.width() * 3 / 4, 0, QSizePolicy::Expanding, QSizePolicy::Fixed);
    ui->bottomLegendHorizontalSpacer->changeSize(plotSize.width() * 3 / 2, 0, QSizePolicy::Expanding, QSizePolicy::Fixed);
    break;

  case Qt::AlignTop:
    ui->leftLegendVerticalSpacer->changeSize(0, 0, QSizePolicy::Fixed, QSizePolicy::Fixed);
    ui->rightLegendVerticalSpacer->changeSize(0, 0, QSizePolicy::Fixed, QSizePolicy::Fixed);
    break;
  case Qt::AlignVCenter:
    ui->leftLegendVerticalSpacer->changeSize(0, plotSize.height() / 2, QSizePolicy::Fixed, QSizePolicy::Fixed);
    ui->rightLegendVerticalSpacer->changeSize(0, plotSize.height() / 2, QSizePolicy::Fixed, QSizePolicy::Fixed);
    break;
  case Qt::AlignBottom:
    ui->leftLegendVerticalSpacer->changeSize(0, plotSize.height() * 3 / 4, QSizePolicy::Fixed, QSizePolicy::Fixed);
    ui->rightLegendVerticalSpacer->changeSize(0, plotSize.height() * 3 / 4, QSizePolicy::Fixed, QSizePolicy::Fixed);
    break;
  default:
    break;
  }
  mEntityModel->setData(mEntityModel->getData());
}

void IdentificationPlotWidget::showLegendWidgets(bool show)
{
  ui->bottomLegendWidget->setVisible(mLegendPosition == QInternal::BottomDock && show);
  ui->topLegendWidget->setVisible(mLegendPosition == QInternal::TopDock && show);
  ui->leftLegendWidget->setVisible(mLegendPosition == QInternal::LeftDock && show);
  ui->rightLegendWidget->setVisible(mLegendPosition == QInternal::RightDock && show);
}