#include "view_page_numeric_data.h"

#include "tree_widget_select_numeric_data_table.h"
#include "tree_widget_height_ratio_table.h"
#include "tree_widget_imported_tables.h"
#include "tree_widget_imported_identification_tables.h"
#include "tree_widget_coefficient_tables.h"
#include "tree_widget_identified_tables.h"
#include "tree_widget_merged_tables.h"
#include "tree_widget_joined_tables.h"
#include <ui/dialogs/web_dialog_import_xls_file.h>
#include <ui/dialogs/web_dialog_xls_file_import_type.h>
#include <logic/known_context_tag_names.h>
#include <logic/context_root.h>

#include <QDebug>
#include <QLabel>
#include <QVBoxLayout>
#include <genesis_style/style.h>
#include <QTabWidget>
#include <QPushButton>


ViewPageNumericData::ViewPageNumericData(QWidget* parent)
	:View(parent)
{
	//// Self
	setStyleSheet(Style::Genesis::GetUiStyle());

	//// Layout
	Layout = new QVBoxLayout(this);
	Layout->setContentsMargins(Style::Scale(40), Style::Scale(32), Style::Scale(40), Style::Scale(32));
	Layout->setSpacing(0);

	auto* headerLayout = new QHBoxLayout;
	headerLayout->setContentsMargins(0, 0, 0, 0);
	headerLayout->setSpacing(Style::Scale(40));
	Layout->addLayout(headerLayout);

	//// Caption
	{
		QLabel* catpion = new QLabel(tr("Numeric data"), this);
		headerLayout->addWidget(catpion);

		catpion->setStyleSheet(Style::Genesis::GetH1());
	}

	auto captionToolBar = new QWidget(this);
	auto captionToolBarLayout = new QHBoxLayout(captionToolBar);
	captionToolBarLayout->setContentsMargins(0, 0, 0, 0);
	captionToolBarLayout->setSpacing(Style::Scale(16));
	captionToolBarLayout->addStretch(1);

	headerLayout->addWidget(captionToolBar);

	//// Space
	Layout->addSpacing(Style::Scale(24));

	//// Content
	{
		Content = new QFrame(this);
		Layout->addWidget(Content, 1);

		Content->setFrameShape(QFrame::Box);
		Content->setObjectName("rounded");
		Content->setStyleSheet("QWidget { background-color: white } ");
	}

	mContentLayout = new QVBoxLayout(Content);

	mTabWidget = new QTabWidget(Content);
	mNumericTreeWidget = new TreeWidgetSelectNumericDataTable(mTabWidget);
	mTabWidget->addTab(mNumericTreeWidget, tr("Data Tables"));

	mRatioTreeWidget = new TreeWidgetHeightRatioTable(mTabWidget);
	mTabWidget->addTab(mRatioTreeWidget, tr("Height Ratio Matrixes"));

	mImportedTablesWidget = new TreeWidgetImportedTables(mTabWidget);
	mTabWidget->addTab(mImportedTablesWidget, tr("Imported tables"));

	mJoinedTablesWidget = new TreeWidgetJoinedTables(mTabWidget);
	mTabWidget->addTab(mJoinedTablesWidget, tr("Joined tables"));

	mIdentifiedTablesWidget = new TreeWidgetIdentifiedTables(mTabWidget);
	mTabWidget->addTab(mIdentifiedTablesWidget, tr("Data Tables"));

	mCoefficientTablesWidget = new TreeWidgetCoefficientTables(mTabWidget);
	mTabWidget->addTab(mCoefficientTablesWidget, tr("Coefficient tables"));

	mImportedIdentificationWidget = new TreeWidgetImportedIdentificationTables(mTabWidget);
	mTabWidget->addTab(mImportedIdentificationWidget, tr("Imported tables"));

	mMergedTablesWidget = new TreeWidgetMergedTables(mTabWidget);
	mTabWidget->addTab(mMergedTablesWidget, tr("Merged tables"));

	mContentLayout->addWidget(mTabWidget);

}

void ViewPageNumericData::ApplyContextProject(const QString& dataId, const QVariant& data)
{
	bool isReset = isDataReset(dataId, data);
    if (!isReset && dataId != Names::ContextTagNames::ProjectId)
		return;

    auto module = Core::GenesisContextRoot::Get()->GetContextModules()->GetData(Names::ModulesContextTags::kModule).value<Names::ModulesContextTags::Module>();
    switch(module)
    {
    case Names::ModulesContextTags::MReservoir:
      mTabWidget->setCurrentWidget(mNumericTreeWidget);
      break;
    case Names::ModulesContextTags::MBiomarkers:
      mTabWidget->setCurrentWidget(mIdentifiedTablesWidget);
      break;
    case Names::ModulesContextTags::MPlots:
    case Names::ModulesContextTags::MNoModule:
    case Names::ModulesContextTags::MLast:
    default:
      break;
    }


	if (isReset)
	{
		auto projectContext = Core::GenesisContextRoot::Get()->GetContextProject();
        mProjectId = projectContext->GetData(Names::ContextTagNames::ProjectId).toInt();
	}
	else
	{
		mProjectId = data.toInt();
	}
}

void ViewPageNumericData::ApplyContextModules(const QString& dataId, const QVariant& data)
{
	const bool isReset = isDataReset(dataId, data);

	using namespace Names::ModulesContextTags;

	if (!isReset && dataId != kModule)
		return;

	Module module = MNoModule;

	if (isReset)
	{
		auto context = Core::GenesisContextRoot::Get()->GetContextModules();
		module = Module(context->GetData(kModule).toInt());
	}
	else if (dataId == kModule)
	{
		module = Module(data.toInt());
	}

	const auto isBiomarkers = module == Names::ModulesContextTags::MBiomarkers;
	const auto isReservoir = module == Names::ModulesContextTags::MReservoir;
	mTabWidget->setTabVisible(mTabWidget->indexOf(mNumericTreeWidget), isReservoir);
	mTabWidget->setTabVisible(mTabWidget->indexOf(mRatioTreeWidget), isReservoir);
	mTabWidget->setTabVisible(mTabWidget->indexOf(mImportedTablesWidget), isReservoir);
	mTabWidget->setTabVisible(mTabWidget->indexOf(mJoinedTablesWidget), isReservoir);
	mTabWidget->setTabVisible(mTabWidget->indexOf(mIdentifiedTablesWidget), isBiomarkers);
	mTabWidget->setTabVisible(mTabWidget->indexOf(mCoefficientTablesWidget), isBiomarkers);
	mTabWidget->setTabVisible(mTabWidget->indexOf(mImportedIdentificationWidget), isBiomarkers);
	mTabWidget->setTabVisible(mTabWidget->indexOf(mMergedTablesWidget), isBiomarkers);
	if (isBiomarkers)
		mTabWidget->setCurrentWidget(mIdentifiedTablesWidget);
	else if (isReservoir)
		mTabWidget->setCurrentWidget(mNumericTreeWidget);
}
