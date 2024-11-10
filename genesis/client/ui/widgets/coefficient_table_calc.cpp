#include "coefficient_table_calc.h"

#include "ui_coefficient_table_calc.h"
#include "logic/enums.h"
#include "logic/tree_model_dynamic_calculation_coefficient_table.h"
#include "logic/tree_model_item.h"
#include "logic/tree_model_presentation.h"
#include "ui/itemviews/data_checkable_headerview.h"
#include "genesis_style/style.h"
#include "logic/structures/library/library_structures.h"
#include "api/api_rest.h"
#include "logic/notification.h"
#include "logic/service/service_locator.h"
#include "logic/service/ijson_serializer.h"
#include "logic/models/coefficient_calculation_model.h"
#include "logic/known_json_tag_names.h"
#include "ui/item_models/analysis_data_table_model.h"
#include "logic/context_root.h"
#include "logic/known_context_tag_names.h"
#include "logic/structures/common_structures.h"

using namespace LibraryFilter;
using namespace Control;
using namespace Structures;
using namespace Service;
using namespace Models;
using namespace Core;
using namespace Structures;

CoefficientTableCalc::CoefficientTableCalc(QWidget* parent)
	: QWidget(parent)
{
	SetupUi();
	ConnectSignals();
}

CoefficientTableCalc::~CoefficientTableCalc()
{
	delete ui;
}

int CoefficientTableCalc::CurrentStep() const
{
	return currentStep;
}

void CoefficientTableCalc::Next()
{
	ui->stackedWidget->setCurrentIndex(++currentStep);
	StepFunc[currentStep]();
}

void CoefficientTableCalc::Undo()
{
	ui->stackedWidget->setCurrentIndex(--currentStep);
	emit disableOkButton(false);
}

void CoefficientTableCalc::LoadCoefficient(int tableId)
{
	CoefficientsTreeModel = new TreeModelDynamicCalculationCoefficientTable(tableId);
	CoefficientsTreeProxyModel = CoefficientsTreeModel->GetPresentationModel();
	CoefficientsTreeProxyModel->setDynamicSortFilter(false);
	ui->treeView->setModel(CoefficientsTreeProxyModel);
	ui->treeView->setSortingEnabled(true);
	CoefficientsTreeModel->SetCheckable(true);
	using CS = TreeModelDynamicCalculationCoefficientTable::Column;
	QSet<int> FilteredColumns =
	{
	  CS::MASS_SPECTRUM,
	  CS::INTENSITY_SPECTRUM,
	  CS::CLASSIFIER_ID,
	  CS::COMPOUND_CLASS_TYPE_ID,
	  CS::LIBRARY_ELEMENT_ID,
	  CS::LIBRARY_GROUP_ID,
	  CS::ANALYSIS_METHODOLOGY,
	  CS::ALL_GROUPS_TITLES,
	  CS::PDF_LOAD_DATE,
	  CS::PDF_SIZE,
	  CS::XLSX_SIZE,
	  CS::XLSX_LOAD_DATE,
	  CS::XLSX_TITLE,
	  CS::PDF_TITLE,
	  CS::SPECIFIC_ID,
	  CS::LAST_COLUMN,
	  CS::XLSX_ATTACHED,
	  CS::LAST_COLUMN,
	};
	auto columns = CoefficientsTreeModel->getColumns().keys();
	QSet<int> acceptedColumns = QSet<int>(columns.begin(), columns.end()).subtract(FilteredColumns);
	CoefficientsTreeProxyModel->SetAcceptedColumns(acceptedColumns);

	connect(ui->treeView->selectionModel(), &QItemSelectionModel::currentRowChanged, this,
		[this](const QModelIndex& current, const QModelIndex& previous)
		{
			if (!current.isValid())
				ui->coefficientsDetailsWidget->setData(QVariantMap());

			const auto index = CoefficientsTreeProxyModel->mapToSource(current);
			if (auto item = CoefficientsTreeModel->GetItem(index))
			{
				auto data = item->GetData();
				ui->coefficientsDetailsWidget->setData(data);
				ui->CoefficientLabel->setText(data["full_title"].toString());
			}
		});

	connect(CoefficientsTreeModel, &TreeModelDynamicCalculationCoefficientTable::HeaderCheckStateChanged, this,
		[this](Qt::Orientation orientation, Qt::CheckState state)
		{
			CoefficientsTreeModel->setOverallCheckState(state);
		});

	Header = new DataCheckableHeaderView(Qt::Horizontal, ui->treeView);
	ui->treeView->setHeader(Header);
	Header->setSectionResizeMode(QHeaderView::ResizeToContents);
	Header->setStretchLastSection(false);
	Header->setSectionResizeMode(CS::CHECKED, QHeaderView::Interactive);
	connect(CoefficientsTreeModel, &TreeModelDynamicCalculationCoefficientTable::dataChanged, this,
		[this](const QModelIndex& tleft, const QModelIndex& tright, QList<int> roles)
		{
			if (!roles.contains(Qt::CheckStateRole) ||
				tright.column() < CS::CHECKED ||
				tleft.column() > CS::CHECKED)
				return;
			const auto count = CoefficientsTreeModel->checkedCount();
			emit disableOkButton(count == 0);
			const auto state = (count == 0)
				? Qt::Unchecked
				: (count < CoefficientsTreeModel->rowCount())
				? Qt::PartiallyChecked
				: Qt::Checked;
			CoefficientsTreeModel->setBlockingCheckStateEvents(true);
			CoefficientsTreeModel->setHeaderData(CS::CHECKED, Qt::Horizontal, state, Qt::CheckStateRole);
			CoefficientsTreeModel->setBlockingCheckStateEvents(false);
			ui->treeView->update();
		});
}

void CoefficientTableCalc::SetupUi()
{
	ui = new Ui::CoefficientTableCalc();
	ui->setupUi(this);
	ui->coefficientsDetailsWidget->setData(QVariantMap());
	ui->splitter->setSizes({ INT_MAX, INT_MAX });
	ui->searchLineEdit->setStyleSheet(
		"QLineEdit { border: solid; background: transparent; "
		"border-color : rgba(0, 66, 105, 72) ; border-width : 1px; "
		"border-style: outset; border-radius: 4px;"
		"placeholder-text-color: rgba(0, 32, 51, 35%);}"
	);
	ui->searchLineEdit->setPlaceholderText(tr("Search by name"));
	NameSearchAction = new QAction(QIcon(":/resource/icons/search.png"), "", ui->searchLineEdit);
	NameSearchAction->setToolTip(tr("Search by name"));
	NameSearchClearAction = new QAction(QIcon(":/resource/icons/icon_action_cross.png"), "", ui->searchLineEdit);
	NameSearchClearAction->setToolTip(tr("Clear search"));
	ui->searchLineEdit->addAction(NameSearchAction, QLineEdit::ActionPosition::LeadingPosition);
	ui->searchLineEdit->addAction(NameSearchClearAction, QLineEdit::ActionPosition::TrailingPosition);
	ui->filesFilterCombo->clear();
  ui->filesFilterCombo->addItem(tr("All files"), static_cast<int>(FileType::All));
	ui->filesFilterCombo->addItem(tr("System"), static_cast<int>(FileType::System));
	ui->filesFilterCombo->addItem(tr("User"), static_cast<int>(FileType::User));
	ui->filesFilterCombo->addItem(tr("Favorite"), static_cast<int>(FileType::Favorite));
	ui->classifiersFilterCombo->clear();
  ui->classifiersFilterCombo->addItem(tr("All classifiers"), static_cast<int>(Classifiers::All));
	ui->classifiersFilterCombo->addItem(tr("GC_PID"), static_cast<int>(Classifiers::GC_PID));
	ui->classifiersFilterCombo->addItem(tr("GC_MS"), static_cast<int>(Classifiers::GC_MS));
    // ui->classifiersFilterCombo->addItem(tr("Saturated"), static_cast<int>(Classifiers::Saturated));
    // ui->classifiersFilterCombo->addItem(tr("Aromatic"), static_cast<int>(Classifiers::Aromatic));
	ui->classifiersFilterCombo->addItem(tr("Pyrolysis"), static_cast<int>(Classifiers::Pyrolysis));
	ui->classifiersFilterCombo->addItem(tr("Gas composition"), static_cast<int>(Classifiers::GasComposition));
	ui->classifiersFilterCombo->addItem(tr("Isotopy"), static_cast<int>(Classifiers::Isotopy));
	ui->classifiersFilterCombo->addItem(tr("Water composition"), static_cast<int>(Classifiers::WaterComposition));
	ui->specificsFilterCombo->clear();
  ui->specificsFilterCombo->addItem(tr("All specifics"), static_cast<int>(Specifics::All));
	ui->specificsFilterCombo->addItem(tr("Genesis"), static_cast<int>(Specifics::Genesis));
	ui->specificsFilterCombo->addItem(tr("Maturity"), static_cast<int>(Specifics::Maturity));
	ui->specificsFilterCombo->addItem(tr("Biodegradation"), static_cast<int>(Specifics::Biodegradation));
	ui->specificsFilterCombo->addItem(tr("Sedimentation conditions"), static_cast<int>(Specifics::SedimentationConditions));
	ui->specificsFilterCombo->addItem(tr("Not specified"), static_cast<int>(Specifics::NotSpecified));
	ui->treeView->setProperty("dataDrivenColors", true);
    ui->treeView->setStyleSheet(Style::ApplySASS("QTreeView::branch:selected{border: 3px solid; "
		"border-left-color: transparent; "
		"border-right-color: @brandColor;"
		"border-top-color: transparent;"
		"border-bottom-color: transparent;}"));
	StepFunc =
	{
	  [&]() { if (CoefficientsTreeModel) CoefficientsTreeModel->Reset(); },
	  std::bind(&CoefficientTableCalc::RequestCalculationTable, this),
	  std::bind(&CoefficientTableCalc::SaveTitleAndComment, this),
	  std::bind(&CoefficientTableCalc::SelectStorage, this),
	  std::bind(&CoefficientTableCalc::SaveCoefficient, this)
	};
	HideClearAction(ui->searchLineEdit->text());
	ui->CoefficientLabel->setStyleSheet(Style::Genesis::Fonts::RegularBold());
}

void CoefficientTableCalc::ConnectSignals()
{
	connect(ui->filesFilterCombo, &QComboBox::currentIndexChanged, this, &CoefficientTableCalc::ApplyFilter);
	connect(ui->classifiersFilterCombo, &QComboBox::currentIndexChanged, this, &CoefficientTableCalc::ApplyFilter);
	connect(ui->specificsFilterCombo, &QComboBox::currentIndexChanged, this, &CoefficientTableCalc::ApplyFilter);
	connect(ui->searchLineEdit, &QLineEdit::textChanged, this, &CoefficientTableCalc::ApplyFilter);
	connect(ui->searchLineEdit, &QLineEdit::textChanged, this, &CoefficientTableCalc::HideClearAction);
	connect(NameSearchAction, &QAction::triggered, this, &CoefficientTableCalc::ApplyFilter);
	connect(NameSearchClearAction, &QAction::triggered, ui->searchLineEdit, &QLineEdit::clear);
	connect(ui->titleCommentWidget, &Widgets::TitleCommentWidget::titleTextChanged, this, [&]()
		{
			emit disableOkButton(!IsValidTitleAndComment());
		});
}

void CoefficientTableCalc::ApplyFilter()
{
	CoefficientsTreeProxyModel->SetRowFilter(QVariant::fromValue<LibraryFilterState>(
		{
		  ui->searchLineEdit->text(),
		  QString(),
		  static_cast<LibraryFilter::FileType>(ui->filesFilterCombo->currentIndex()),
		  static_cast<LibraryFilter::Classifiers>(ui->classifiersFilterCombo->currentIndex()),
		  static_cast<LibraryFilter::Specifics>(ui->specificsFilterCombo->currentIndex()),
		}));
}

void CoefficientTableCalc::HideClearAction(const QString& text)
{
	NameSearchClearAction->setVisible(!text.isEmpty());
}

void CoefficientTableCalc::RequestCalculationTable()
{
	const auto tableId = CoefficientsTreeModel->getTableId();
	const auto checked = CoefficientsTreeModel->getChecked();
	const auto projectId = GenesisContextRoot::Get()->ProjectId();
	API::REST::GetCalcCoefficientsByTableId(projectId, tableId, checked,
		[&](QNetworkReply*, QJsonDocument data)
		{
			auto object = data.object();
			if (object.value(JsonTagNames::Error).toBool())
			{
				Notification::NotifyError(tr("Error while data receiving : %1")
					.arg(object.value(JsonTagNames::Msg).toString()));
			}
			else
			{
				auto serializer = ServiceLocator::Instance().Resolve<CoefficientCalculationModel, IJsonSerializer>();
				auto model = serializer->ToModel(object.value(JsonTagNames::Columns).toObject());
				auto analysisModel = new AnalysisDataTableModel();
				analysisModel->SetModel(model);
				ui->tableView->SetModel(analysisModel);
				model->Exec<Double2DVector>(CoefficientCalculationModel::ValuesCustomData,
					[&](const Double2DVector& data)
					{
						if (data.empty())
							ui->tableView->FillParameterComboBoxShort();
						else
							ui->tableView->FillCustomParameterComboBox();
					});
				ui->tableView->SetHideYConcentration(true);
				ui->tableView->SetCheckBoxVisible(false);
			}
		},
		[](QNetworkReply*, QNetworkReply::NetworkError err)
		{
			Notification::NotifyError(tr("Error while data receiving : %1").arg(err));
		}
	);
}

// empty function all data stored in ui
void CoefficientTableCalc::SaveTitleAndComment()
{
	emit disableOkButton(!IsValidTitleAndComment());
}

void CoefficientTableCalc::SelectStorage()
{
	auto projectId = GenesisContextRoot::Get()->ProjectId();
    API::REST::GetTableGroups(projectId, Names::Group::CoefficientTableIdentified,
		[&](QNetworkReply* r, QJsonDocument doc)
		{
			auto obj = doc.object();
      // qDebug() << doc;
			auto groups = obj.value(JsonTagNames::Data).toArray();
			std::map<QString, int> idByNames;
			std::transform(groups.begin(), groups.end(), std::inserter(idByNames, idByNames.end()),
				[](const QJsonValueRef& val)
				{
					auto arr = val.toArray();
					return std::make_pair(arr[1].toString(), arr.first().toInt());
				});
			ui->editGroupWidget->SetGroupInfo(idByNames);
		},
		[](QNetworkReply*, QNetworkReply::NetworkError err)
		{ Notification::NotifyError(tr("Failed to load file groups information"), err); });
}

void CoefficientTableCalc::SaveCoefficient()
{
	if (auto abstractModel = ui->tableView->GetModel())
	{
		auto serializer = ServiceLocator::Instance().Resolve<CoefficientCalculationModel, IJsonSerializer>();
		auto modelObj = serializer->ToJson(abstractModel->GetModel());
		QJsonObject obj;
		obj.insert(JsonTagNames::TableContent, modelObj);
		obj.insert(JsonTagNames::Title, ui->titleCommentWidget->Title());
		obj.insert(JsonTagNames::Comment, ui->titleCommentWidget->Comment());
		auto projectId = GenesisContextRoot::Get()->ProjectId();
		obj.insert(JsonTagNames::project_id, projectId);
		if (ui->editGroupWidget->IsGroupChecked())
		{
			if (ui->editGroupWidget->IsCreateNewGroup())
			{
				obj.insert(JsonTagNames::CreateGroup, "true");
				obj.insert(JsonTagNames::GroupTitle, ui->editGroupWidget->GroupTitle());
			}
			else
				obj.insert(JsonTagNames::TableGroupId, ui->editGroupWidget->SelectedGroupId());
		}
		API::REST::SaveCoefficientTable(obj,
			[](QNetworkReply*, QJsonDocument doc)
			{
				auto obj = doc.object();
				if (obj.value(JsonTagNames::Error).toBool())
					Notification::NotifyError(tr("Error while save data : %1").arg(obj.value(JsonTagNames::Msg).toString()));
				else
					Notification::NotifySuccess(tr("Table saved"), tr("Successfull"));
			},
			[](QNetworkReply*, QNetworkReply::NetworkError err)
			{
				Notification::NotifyError(tr("Error while data receiving : %1").arg(err));
			});
	}
}

bool CoefficientTableCalc::IsValidTitleAndComment() const
{
	return ui->titleCommentWidget->IsValid();
}
