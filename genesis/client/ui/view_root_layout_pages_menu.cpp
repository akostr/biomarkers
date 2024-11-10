#include "genesis_window.h"

#include "view_root_layout_pages_menu.h"
#include <logic/known_context_names.h>
#include <logic/known_context_tag_names.h>
#include <genesis_style/style.h>
#include <ui/known_view_names.h>
#include "ui_view_root_layout_pages_menu.h"
#include "logic/context_root.h"

#include <QPushButton>
#include <QScrollBar>
#include <QStyledItemDelegate>
#include <QFrame>
#include <QLabel>
#include <QRadioButton>
#include <QPainter>

const QString kMenuStyle = "QTreeWidget {font: @RegularBoldFont; color: @fontColorPrimary; show-decoration-selected: 1; icon-size: 24px;}"
                           "QTreeWidget::item {border-radius: 8px; padding: 12px;}"
                           "QTreeWidget::item:hover {background-color: rgb(245, 247, 249);}"
                           "QTreeWidget::item:selected:active {background-color: rgb(237, 242, 245);}"
                           "QTreeWidget::item:selected:!active {background-color: rgb(237, 242, 245);}"
                           "QTreeWidget::item:hover:selected {background-color: rgb(237, 242, 245);}"
                           "QTreeWidget::branch {background-color: white; border: none;}"
                           "QTreeWidget::branch:hover {background-color: rgb(245, 247, 249);}"
                           "QTreeWidget::branch:selected {background-color: rgb(237, 242, 245); border: none;}"
                           "QTreeWidget::branch:hover:selected {background-color: rgb(237, 242, 245);}"
                           "QTreeWidget::item:disabled {background-color: rgb(235, 235, 235); border-radius: 0px;}";

class CustomSelectedItemBordersDelegate : public QStyledItemDelegate
{
public:
  CustomSelectedItemBordersDelegate(QObject* parent = nullptr) : QStyledItemDelegate(parent){};

  // QAbstractItemDelegate interface
public:
  void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override
  {
    auto op = option;
    initStyleOption(&op, index);
    if(op.state & QStyle::State_Selected)
    {
      painter->save();
      painter->setClipRect(op.rect);
      painter->setPen(Qt::NoPen);
      painter->setBrush(QColor(237, 242, 245));
      auto rect = op.rect;
      if(index.column() == 0)
        rect.setWidth(rect.width() + 8);
      else
        rect.moveLeft(rect.left() - 8);
      painter->drawRoundedRect(rect, 8, 8);
      painter->restore();
    }
    else if(op.state & QStyle::State_MouseOver)
    {
      painter->save();
      painter->setClipRect(op.rect);
      painter->setPen(Qt::NoPen);
      painter->setBrush(QColor(245, 247, 249));
      auto rect = op.rect;
      if(index.column() == 0)
        rect.setWidth(rect.width() + 8);
      else
        rect.moveLeft(rect.left() - 8);
      painter->drawRoundedRect(rect, 8, 8);
      painter->restore();
    }
    QStyledItemDelegate::paint(painter, op, index);
  }
};

using namespace Core;
using namespace Names;
using namespace ViewPageNames;

int initialTreeViewWidth = 0;
////////////////////////////////////////////////////
//// Root Layout Pages Menu view class
namespace Views
{
ViewRootLayoutPagesMenu::ViewRootLayoutPagesMenu(QWidget* parent)
  : View(parent),
    ui(new Ui::ViewRootLayoutPagesMenuUi),
    mChromasAndFragmentsItem(nullptr),
    mIndexationItem(nullptr),
    mIdentificationItem(nullptr),
    mInterpretationItem(nullptr),
    mEventLogItem(nullptr),
    mCollapseDwnIcon(QIcon(":/resource/controls/collpse_dn.png")),
    mCollapseUpIcon(QIcon(":/resource/controls/collpse_up.png"))

{
  setupUi();
}

ViewRootLayoutPagesMenu::~ViewRootLayoutPagesMenu()
{
  delete ui;
}

void ViewRootLayoutPagesMenu::setupUi()
{
  setStyleSheet(Style::Genesis::GetPageNavigationStyle());
  mLayout = new QHBoxLayout(this);
  mLayout->setContentsMargins(0, 0, 0, 0);
  mLayout->setSpacing(0);

  mRoot = new QWidget(this);
  mLayout->addWidget(mRoot);

  ui->setupUi(mRoot);

  ui->menuTree->setItemDelegate(new CustomSelectedItemBordersDelegate(ui->menuTree));
  ui->commonMenuTree->setItemDelegate(new CustomSelectedItemBordersDelegate(ui->menuTree));

  QString projectsButtonStyle =
    ".QRadioButton                      { font: @RegularBoldFont; color: @fontColorPrimary;} \n"
    ".QRadioButton::indicator           { width: 24px; height: 24px;} \n"
    ".QRadioButton::indicator:unchecked { image: url(:/resource/icons/icon_folders.png); } \n"
    ".QRadioButton::indicator:checked   { image: url(:/resource/icons/icon_folders_active.png); } \n";

  ui->RootLayout->setContentsMargins(Style::Scale(32), Style::Scale(27), Style::Scale(32), Style::Scale(27));
  ui->hline->setStyleSheet(Style::GetStyleSheet(":/resource/styles/genesis_ui_qframe.qss"));

  ui->menuTree->setStyleSheet(Style::ApplySASS(kMenuStyle));
  ui->commonMenuTree->setStyleSheet(Style::ApplySASS(kMenuStyle));
  ui->commonMenuTree->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
  ui->commonMenuTree->setIndentation(0);
  ui->commonMenuTree->setColumnCount(2);
  ui->commonMenuTree->setTextElideMode(Qt::ElideNone);
  ui->commonMenuTree->setVerticalScrollMode(QTreeWidget::ScrollPerPixel);
  ui->commonMenuTree->verticalScrollBar()->setSingleStep(5);

  auto header = ui->commonMenuTree->header();
  header->setSectionResizeMode(0, QHeaderView::ResizeMode::Stretch);
  header->setSectionResizeMode(1, QHeaderView::ResizeMode::Fixed);

  header = ui->menuTree->header();
  header->setSectionResizeMode(0, QHeaderView::ResizeMode::Fixed);
  header->setSectionResizeMode(1, QHeaderView::ResizeMode::Fixed);

  ui->menuTree->setVerticalScrollMode(QTreeWidget::ScrollPerPixel);
  ui->menuTree->verticalScrollBar()->setSingleStep(5);
  ui->menuTree->setTextElideMode(Qt::ElideNone);
  ui->menuTree->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
  ui->menuTree->setIndentation(0);
  //// Toolbar
  {
    mUserToolbar = View::Create(ViewRootToolbarPageName, nullptr);
    ui->RootLayout->addWidget(mUserToolbar);
  }
  ui->menuContainer->setVisible(false);
  connect(ui->menuTree, &QTreeWidget::itemClicked, this, [this](QTreeWidgetItem* item, int column)
          {
            if(item->childCount() != 0)
            {//handle top-lvl items
              if(!item->isExpanded())
              {
                item->setExpanded(true);
                item->setData(1, Qt::DecorationRole, mCollapseUpIcon);
              }
              else
              {
                item->setExpanded(false);
                item->setData(1, Qt::DecorationRole, mCollapseDwnIcon);
              }
              return;
            }

            auto pageNameData = item->data(0, ViewPageNameRole);
            if(pageNameData.isValid() && !pageNameData.isNull())
            {
              QString pageName = pageNameData.toString();
              if(!pageName.isEmpty())
              {
                GenesisWindow::Get()->ShowPage(pageNameData.toString());
              }
            }
          });
  connect(ui->commonMenuTree, &QTreeWidget::itemClicked, this, [this](QTreeWidgetItem* item, int column)
          {
            if(item->childCount() != 0)
            {//handle top-lvl items
              if(!item->isExpanded())
              {
                item->setExpanded(true);
                item->setData(1, Qt::DecorationRole, mCollapseUpIcon);
              }
              else
              {
                item->setExpanded(false);
                item->setData(1, Qt::DecorationRole, mCollapseDwnIcon);
              }
              return;
            }

            auto pageNameData = item->data(0, ViewPageNameRole);
            if(pageNameData.isValid() && !pageNameData.isNull())
            {
              QString pageName = pageNameData.toString();
              if(!pageName.isEmpty())
              {
                GenesisWindow::Get()->ShowPage(pageNameData.toString());
                GenesisContextRoot::Get()->GetContextLibrary()->SetData(Names::LibraryContextTags::kCurrentTabInd, item->data(0, ViewPageLibTabIndRole));
              }
            }
          });

  connect(ui->projectInfoWidget, &ProjectInfoMenuWidget::checkStateChanged, this,
          [this](bool isChecked)
          {
            if(isChecked)
              GenesisWindow::Get()->ShowPage(ViewProjectInfoPageName);
          });
  topLevelItemsInit();
  loadCommonMenu();
}

void ViewRootLayoutPagesMenu::loadReservoirMenu()
{
  clearMenu();

  QFont subItemFont(Style::GetSASSValue("fontFaceNormal"));
  subItemFont.setPixelSize(Style::GetSASSValue("fontSizeRegularTextScalableFont").toInt());
  subItemFont.setWeight(QFont::Normal);

  ui->menuTree->insertTopLevelItem(ui->menuTree->indexOfTopLevelItem(mChromasAndFragmentsItem) + 1, mIndexationItem);

  auto versionsItem = new QTreeWidgetItem(mIndexationItem);
  versionsItem->setFont(0, subItemFont);
  versionsItem->setText(0, tr("Markup versions"));
  versionsItem->setData(0, ViewPageNameRole, ViewPageProjectMarkupVersionPageName);
  versionsItem->setData(0, Qt::DecorationRole, ViewRootLayoutPagesMenu::nullIcon());
  mItemsMap[ViewPageProjectMarkupVersionPageName] = versionsItem;

  auto digitalDataItem = new QTreeWidgetItem(mIndexationItem);
  digitalDataItem->setFont(0, subItemFont);
  digitalDataItem->setText(0, tr("Digital data"));
  digitalDataItem->setData(0, ViewPageNameRole, ViewNumericData);
  digitalDataItem->setData(0, Qt::DecorationRole, ViewRootLayoutPagesMenu::nullIcon());
  mItemsMap[ViewNumericData] = digitalDataItem;

  auto MGKModelsItem = new QTreeWidgetItem(mInterpretationItem);
  MGKModelsItem->setFont(0, subItemFont);
  MGKModelsItem->setText(0, tr("MGK models"));
  MGKModelsItem->setData(0, ViewPageNameRole, ViewProjectModelsListPageName);
  MGKModelsItem->setData(0, Qt::DecorationRole, ViewRootLayoutPagesMenu::nullIcon());
  mItemsMap[ViewProjectModelsListPageName] = MGKModelsItem;
  auto MRKModelsItem = new QTreeWidgetItem(mInterpretationItem);
  MRKModelsItem->setFont(0, subItemFont);
  MRKModelsItem->setText(0, tr("MRK models"));
  MRKModelsItem->setData(0, ViewPageNameRole, ViewPageMcrModelPageName);
  MRKModelsItem->setData(0, Qt::DecorationRole, ViewRootLayoutPagesMenu::nullIcon());
  mItemsMap[ViewPageMcrModelPageName] = MRKModelsItem;
  auto PLCModelsItem = new QTreeWidgetItem(mInterpretationItem);
  PLCModelsItem->setFont(0, subItemFont);
  PLCModelsItem->setText(0, tr("PLC models and predicts"));
  PLCModelsItem->setData(0, ViewPageNameRole, ViewPagePlsModelPageName);
  PLCModelsItem->setData(0, Qt::DecorationRole, ViewRootLayoutPagesMenu::nullIcon());
  mItemsMap[ViewPagePlsModelPageName] = PLCModelsItem;

  mIndexationItem->setExpanded(true);
  mIndexationItem->setData(1, Qt::DecorationRole, mCollapseUpIcon);
}

void ViewRootLayoutPagesMenu::loadBiomarkersMenu()
{
  clearMenu();

  QFont subItemFont(Style::GetSASSValue("fontFaceNormal"));
  subItemFont.setPixelSize(Style::GetSASSValue("fontSizeRegularTextScalableFont").toInt());
  subItemFont.setWeight(QFont::Normal);

  QFont disableItemFont(Style::GetSASSValue("fontFaceNormal"));
  disableItemFont.setPixelSize(Style::GetSASSValue("fontSizeRegularTextScalableFont").toInt());
  disableItemFont.setWeight(QFont::Weight::Thin);

  ui->menuTree->insertTopLevelItem(ui->menuTree->indexOfTopLevelItem(mChromasAndFragmentsItem) + 1, mIdentificationItem);

  auto versionsItem = new QTreeWidgetItem(mIdentificationItem);
  versionsItem->setFont(0, subItemFont);
  versionsItem->setText(0, tr("Markup versions"));
  versionsItem->setData(0, ViewPageNameRole, ViewPageProjectMarkupVersionPageName);
  versionsItem->setData(0, Qt::DecorationRole, ViewRootLayoutPagesMenu::nullIcon());
  mItemsMap[ViewPageProjectMarkupVersionPageName] = versionsItem;

  auto digitalDataItem = new QTreeWidgetItem(mIdentificationItem);
  digitalDataItem->setFont(0, subItemFont);
  digitalDataItem->setText(0, tr("Digital data"));
  digitalDataItem->setData(0, ViewPageNameRole, ViewNumericData);
  digitalDataItem->setData(0, Qt::DecorationRole, ViewRootLayoutPagesMenu::nullIcon());
  mItemsMap[ViewNumericData] = digitalDataItem;

  auto plotsItem = new QTreeWidgetItem(mIdentificationItem);
  plotsItem->setFont(0, subItemFont);
  plotsItem->setText(0, tr("Plots"));
  plotsItem->setData(0, Qt::DecorationRole, ViewRootLayoutPagesMenu::nullIcon());
  plotsItem->setData(0, ViewPageNameRole, ViewPageIdentificationPlotListName);
  mItemsMap[ViewPageIdentificationPlotListName] = plotsItem;

  auto MGKModelsItem = new QTreeWidgetItem(mInterpretationItem);
  MGKModelsItem->setFont(0, subItemFont);
  MGKModelsItem->setText(0, tr("MGK models"));
  MGKModelsItem->setData(0, ViewPageNameRole, ViewProjectModelsListPageName);
  mItemsMap[ViewProjectModelsListPageName] = MGKModelsItem;
  MGKModelsItem->setData(0, Qt::DecorationRole, ViewRootLayoutPagesMenu::nullIcon());

  mIdentificationItem->setExpanded(true);
  mIdentificationItem->setData(1, Qt::DecorationRole, mCollapseUpIcon);
}

void ViewRootLayoutPagesMenu::loadPlotsMenu()
{
  clearMenu();
}

void ViewRootLayoutPagesMenu::clearMenu()
{
  auto clearChildren = [](QTreeWidgetItem* item)
  {
    if(!item)
      return;
    while(item->childCount() > 0)
      item->removeChild(item->child(0));
  };
  clearChildren(mChromasAndFragmentsItem);
  clearChildren(mIndexationItem);
  clearChildren(mIdentificationItem);
  clearChildren(mInterpretationItem);
  clearChildren(mEventLogItem);
  ui->menuTree->takeTopLevelItem(ui->menuTree->indexOfTopLevelItem(mIndexationItem));
  ui->menuTree->takeTopLevelItem(ui->menuTree->indexOfTopLevelItem(mIdentificationItem));

}

void ViewRootLayoutPagesMenu::topLevelItemsInit()
{
  mChromasAndFragmentsItem = new QTreeWidgetItem({tr("Chromatogramms and framgents")});
  mChromasAndFragmentsItem->setData(0, ViewPageNameRole, ViewChromatogrammsFragmentsPageName);
  mChromasAndFragmentsItem->setData(0, Qt::DecorationRole, QIcon(":/resource/icons/mainMenu/CDF@2x.png"));
  ui->menuTree->addTopLevelItem(mChromasAndFragmentsItem);
  mItemsMap[ViewChromatogrammsFragmentsPageName] = mChromasAndFragmentsItem;

  mIndexationItem = new QTreeWidgetItem({tr("Indexation")});
  mIndexationItem->setData(1, Qt::DecorationRole, mCollapseUpIcon);
  mIndexationItem->setData(0, Qt::DecorationRole, QIcon(":/resource/icons/mainMenu/Indexing@2x.png"));
  ui->menuTree->addTopLevelItem(mIndexationItem);

  mIdentificationItem = new QTreeWidgetItem({tr("Identification")});
  mIdentificationItem->setData(1, Qt::DecorationRole, mCollapseUpIcon);
  mIdentificationItem->setData(0, Qt::DecorationRole, QIcon(":/resource/icons/mainMenu/Identification@2x.png"));
  ui->menuTree->addTopLevelItem(mIdentificationItem);

  mInterpretationItem = new QTreeWidgetItem({tr("Interpretation")});
  mInterpretationItem->setData(1, Qt::DecorationRole, mCollapseUpIcon);
  mInterpretationItem->setData(0, Qt::DecorationRole, QIcon(":/resource/icons/mainMenu/Interpretation@2x.png"));
  ui->menuTree->addTopLevelItem(mInterpretationItem);

  mEventLogItem = new QTreeWidgetItem({tr("Events log")});
  mEventLogItem->setData(0, Qt::DecorationRole, QIcon(":/resource/icons/mainMenu/Log@2x.png"));

  ui->menuTree->addTopLevelItem(mEventLogItem);
}

void ViewRootLayoutPagesMenu::loadCommonMenu()
{
  mProjectsItem = new QTreeWidgetItem({tr("Projects")});
  mProjectsItem->setData(0, Qt::DecorationRole, QIcon(":/resource/icons/icon_folders.png"));
  mProjectsItem->setData(0, ViewPageNameRole, ViewProjectsPageName);
  ui->commonMenuTree->addTopLevelItem(mProjectsItem);
  mCommonItemsMap[ViewProjectsPageName] = mProjectsItem;

  mLibraryItem = new QTreeWidgetItem({tr("Library")});
  mLibraryItem->setData(1, Qt::DecorationRole, mCollapseUpIcon);
  mLibraryItem->setData(0, Qt::DecorationRole, QIcon(":/resource/icons/mainMenu/icon_library@2x.png"));
  mLibraryItem->setData(0, ViewPageNameRole, ViewLibrary);
  ui->commonMenuTree->addTopLevelItem(mLibraryItem);
  mCommonItemsMap[ViewLibrary] = mProjectsItem;

  //after adding second column icon, we able to calculate right sizehint for this column based on icon size;
  auto header = ui->commonMenuTree->header();
  header->resizeSection(1, header->sectionSizeHint(1));

  mCompoundsItem = new QTreeWidgetItem({tr("Compounds")});
  mCompoundsItem->setData(0, Qt::DecorationRole, QIcon(":/resource/icons/mainMenu/icon_compounds@2x.png"));
  mCompoundsItem->setData(0, ViewPageNameRole, ViewLibrary);
  mCompoundsItem->setData(0, ViewPageLibTabIndRole, LibraryContextTags::kCompoundsTab);
  mLibraryItem->addChild(mCompoundsItem);

  mCoefficientsItem = new QTreeWidgetItem({tr("Coefficients")});
  mCoefficientsItem->setData(0, Qt::DecorationRole, QIcon(":/resource/icons/mainMenu/icon_coefficient@2x.png"));
  mCoefficientsItem->setData(0, ViewPageNameRole, ViewLibrary);
  mCoefficientsItem->setData(0, ViewPageLibTabIndRole, LibraryContextTags::kCoefficientsTab);
  mLibraryItem->addChild(mCoefficientsItem);

  mTemplatesItem = new QTreeWidgetItem({tr("Templates")});
  mTemplatesItem->setData(0, Qt::DecorationRole, QIcon(":/resource/icons/mainMenu/icon_chart_templates@2x.png"));
  mTemplatesItem->setData(0, ViewPageNameRole, ViewLibrary);
  mTemplatesItem->setData(0, ViewPageLibTabIndRole, LibraryContextTags::kTemplatesTab);
  mLibraryItem->addChild(mTemplatesItem);

  ui->commonMenuTree->expand(ui->commonMenuTree->indexFromItem(mLibraryItem));
}

void ViewRootLayoutPagesMenu::ApplyContextUi(const QString& dataId, const QVariant& data)
{
  bool isReset = isDataReset(dataId, data);
  if(!isReset && dataId != Names::ContextTagNames::Page)
    return;

  QString currentPageId;

  if(isReset)
    currentPageId = View::CurrentPageId();
  else if(dataId == Names::ContextTagNames::Page)
    currentPageId = data.toString();

  QSignalBlocker(ui->projectInfoWidget);
  ui->projectInfoWidget->setCheckState(false);
  ui->menuTree->selectionModel()->clearSelection();
  ui->commonMenuTree->selectionModel()->clearSelection();

  if(currentPageId.isEmpty())
    return;
  if(mItemsMap.contains(currentPageId))
    ui->menuTree->selectionModel()->select(ui->menuTree->indexFromItem(mItemsMap[currentPageId]), QItemSelectionModel::SelectionFlag::ClearAndSelect | QItemSelectionModel::SelectionFlag::Rows);
  else if(mCommonItemsMap.contains(currentPageId))
  {
    if(currentPageId == ViewLibrary)
      return;
    else
      ui->commonMenuTree->selectionModel()->select(ui->commonMenuTree->indexFromItem(mCommonItemsMap[currentPageId]), QItemSelectionModel::SelectionFlag::ClearAndSelect | QItemSelectionModel::SelectionFlag::Rows);
  }
}

void ViewRootLayoutPagesMenu::ApplyContextProject(const QString& dataId, const QVariant& data)
{
  bool isReset = isDataReset(dataId, data);
  if(isReset)
  {
    auto context = Core::GenesisContextRoot::Get()->GetContextProject();
    int projectId = context->GetData(Names::ContextTagNames::ProjectId).toInt();
    bool isMenuVisible = projectId != 0;
    ui->menuContainer->setVisible(isMenuVisible);
    ui->hline->setVisible(isMenuVisible);
    if(projectId)
    {
      ui->menuTree->expandAll();
      int minW = 0;
      for(int i = 0; i < ui->menuTree->model()->columnCount(); i++)
      {
        ui->menuTree->resizeColumnToContents(i);
        minW += ui->menuTree->columnWidth(i);
      }
      ui->menuTree->setMinimumWidth(minW + ui->menuTree->verticalScrollBar()->sizeHint().width());
    }
    ui->projectInfoWidget->setTitle(context->GetData("title").toString());
    ui->projectInfoWidget->setGroup(context->GetData("group").toString());
    auto size = 16;
    auto pixmap = context->GetData(ContextTagNames::ProjectStatusIcon).value<QIcon>().pixmap(size, size);
    ui->projectInfoWidget->setPixmap(pixmap);
  }
  else if(dataId ==  Names::ContextTagNames::ProjectId)
  {
    int projectId = data.toInt();
    bool isMenuVisible = projectId != 0;
    ui->menuContainer->setVisible(isMenuVisible);
    ui->hline->setVisible(isMenuVisible);
    if(projectId)
    {
      ui->menuTree->expandAll();
      int minW = 0;
      for(int i = 0; i < ui->menuTree->model()->columnCount(); i++)
      {
        ui->menuTree->resizeColumnToContents(i);
        minW += ui->menuTree->columnWidth(i);
      }
      ui->menuTree->setMinimumWidth(minW + ui->menuTree->verticalScrollBar()->sizeHint().width());
    }
  }
  else if(dataId == "title")
  {
    ui->projectInfoWidget->setTitle(data.toString());
  }
  else if(dataId == "group")
  {
    ui->projectInfoWidget->setGroup(data.toString());
  }
  else if(dataId == ContextTagNames::ProjectStatusIcon)
  {
    auto size = 16;
    auto pixmap = data.value<QIcon>().pixmap(size, size);
    ui->projectInfoWidget->setPixmap(pixmap);
  }
}
}//namespace Views


void Views::ViewRootLayoutPagesMenu::ApplyContextUser(const QString &dataId, const QVariant &data)
{
  bool isReset = isDataReset(dataId, data);
  if(isReset)
  {
    auto context = GenesisContextRoot::Get()->GetContextUser();
    auto modulesList = context->GetData(Names::UserContextTags::kAvailableModules).value<QList<int>>();
    ui->menuHeaderWidget->setAvailableModulesList(modulesList);
  }
  else if(dataId == Names::UserContextTags::kAvailableModules)
  {
    auto modulesList = data.value<QList<int>>();
    ui->menuHeaderWidget->setAvailableModulesList(modulesList);
  }
}

void Views::ViewRootLayoutPagesMenu::ApplyContextModules(const QString &dataId, const QVariant &data)
{
  bool isReset = isDataReset(dataId, data);
  if(isReset)
  {
    auto context = GenesisContextRoot::Get()->GetContextModules();
    setCurrentModule(context->GetData(Names::ModulesContextTags::kModule).toInt());
  }
  else if(dataId == Names::ModulesContextTags::kModule)
  {
    setCurrentModule(data.toInt());
  }
}

void ViewRootLayoutPagesMenu::setCurrentModule(int module)
{
  ui->menuHeaderWidget->setCurrentModule(module);
  switch(Names::ModulesContextTags::Module(module))
  {
  case ModulesContextTags::MReservoir:
    loadReservoirMenu();
    mLibraryItem->setHidden(true);
    break;
  case ModulesContextTags::MBiomarkers:
    loadBiomarkersMenu();
    mLibraryItem->setHidden(false);
    break;
  case ModulesContextTags::MPlots:
    mLibraryItem->setHidden(true);
    loadPlotsMenu();
    break;
  case ModulesContextTags::MNoModule:
  case ModulesContextTags::MLast:
    clearMenu();
    break;
  }
}

QIcon ViewRootLayoutPagesMenu::nullIcon()
{
  QPixmap pm(48,48);
  pm.setDevicePixelRatio(2);
  pm.fill(QColor(0,0,0,0));

  return QIcon(pm);
}

void Views::ViewRootLayoutPagesMenu::ApplyContextLibrary(const QString &dataId, const QVariant &data)
{
  bool isReset = isDataReset(dataId, data);
  int currentTabInd = -1;
  if(isReset)
  {
    auto vind = GenesisContextRoot::Get()->GetContextLibrary()->GetData(Names::LibraryContextTags::kCurrentTabInd);
    if(vind.isValid() && !vind.isNull())
    {
      currentTabInd = vind.toInt();
    }
  }
  else if(dataId == Names::LibraryContextTags::kCurrentTabInd)
  {
    if(data.isValid() && !data.isNull())
      currentTabInd = data.toInt();
  }
  if(currentTabInd != -1)
  {
    for(int r = 0; r < mLibraryItem->childCount(); r++)
    {
      auto child = mLibraryItem->child(r);
      if(child->data(0, ViewPageLibTabIndRole).toInt() == currentTabInd)
      {
        ui->commonMenuTree->selectionModel()->select(ui->commonMenuTree->indexFromItem(child), QItemSelectionModel::SelectionFlag::ClearAndSelect | QItemSelectionModel::SelectionFlag::Rows);
        return;
      }
    }
  }
}
