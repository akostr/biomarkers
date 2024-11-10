#include "analysis_data_table_model.h"

#include "ui/itemviews/common_table_view.h"
#include "logic/models/reference_analysis_data_table_model.h"
#include "logic/markup/genesis_markup_enums.h"
#include "logic/known_json_tag_names.h"

#include <QVariant>


using namespace Model;
using namespace Structures;
using namespace GenesisMarkup;

namespace
{
	// for correct conversion double
	QLocale LOCALE;
}

namespace Models
{
	AnalysisDataTableModel::AnalysisDataTableModel(QObject* parent)
		: QAbstractTableModel(parent)
	{
	}

	int AnalysisDataTableModel::rowCount(const QModelIndex&) const
	{
		if (!DataTableModel)
			return 0;
		return static_cast<int>(DataTableModel->Get<size_t>(ReferenceAnalysisDataTableModel::RowCount));
	}

	int AnalysisDataTableModel::columnCount(const QModelIndex&) const
	{
		return static_cast<int>(HorizontalHeaders.size());
	}

	QVariant AnalysisDataTableModel::data(const QModelIndex& index, int role) const
	{
		const auto row = index.row();

		if (row > rowCount())
			return QVariant();

		const auto column = index.column();
		if (role == Qt::DisplayRole)
		{
			if (column == 0)
			{
				const auto& sampleData = DataTableModel->Get<StringList>(ReferenceAnalysisDataTableModel::SampleData);
				return QString::fromStdString(sampleData[row]);
			}
			if (column == YConcentrationColumn)
			{
				QString con;
				if (YConcentrations[row].has_value())
					con = LOCALE.toString(YConcentrations[row].value());
				return con;
			}

			// YConcentrationColumn is zero based index of y concentrations
			// Calculate display peaks
			// PeakZeroIndex is zero based indeox of first peak
			if (column > YConcentrationColumn && column < PeakCount + PeakZeroIndex)
			{
				const auto peakColumn = column - PeakZeroIndex;
				const auto pair = ParameterAccessName.find(Parameter);
				if (pair != ParameterAccessName.end())
				{
					const auto& peaks = DataTableModel->Get<Double2DVector>(pair->second);
					return peaks[row][peakColumn];
				}
			}
			if (column > PeakCount + YConcentrationColumn)
			{
				const auto& passportData = DataTableModel->Get<StringList2D>(ReferenceAnalysisDataTableModel::PassportData);
				const auto passportColumn = column - PeakCount - PeakZeroIndex;
				return QString::fromStdString(passportData[row][passportColumn]);
			}
		}

		if (role == Qt::BackgroundRole
			&& HorizontalCheckboxDecoration[column]
			&& VerticalCheckboxDecoration[row]
			&& CheckedPeaks[column]
			&& CheckedSamples[row]
			&& (ShowPeakCheckboxes
				|| ShowSampleCheckboxes))
		{
			return BackgroundColor;
		}

		return QVariant();
	}

	bool AnalysisDataTableModel::setData(const QModelIndex& index, const QVariant& value, int role)
	{
		if (role == Qt::EditRole && index.column() == YConcentrationColumn)
		{
			const auto row = index.row();
			YConcentrations[row] = value.toString().isEmpty()
				? std::nullopt
				: std::make_optional(LOCALE.toDouble(value.toString()));
			emit dataChanged(index, index, { role });
			emit headerDataChanged(Qt::Vertical, row, row);
			return true;
		}

		if (role == Qt::BackgroundRole && value.isValid())
		{
			const auto point = value.toPoint();
			const auto row = static_cast<size_t>(point.x());
			int top = 0;
			int left = 0;
			int bottom = rowCount() - 1;
			int right = columnCount() - 1;
			if (row >= 0 && row < CheckedSamples.size())
			{
				top = row;
				bottom = row;
			}
			const auto column = static_cast<size_t>(point.y());
			if (column >= 0 && column < CheckedPeaks.size())
			{
				left = column;
				right = column;
			}
			emit dataChanged(this->index(top, left), this->index(bottom, right), { role });
		}
		return QAbstractItemModel::setData(index, value, role);
	}

	QVariant AnalysisDataTableModel::headerData(int section, Qt::Orientation orientation, int role) const
	{
		if (role == Qt::CheckStateRole)
		{
			if (orientation == Qt::Horizontal)
			{
				if (section >= CheckedPeaks.size() || CheckedPeaks.empty())
					return QVariant();
				return static_cast<bool>(CheckedPeaks[section]) ? Qt::Checked : Qt::Unchecked;
			}

			if (orientation == Qt::Vertical)
			{
				if (section >= CheckedSamples.size() || CheckedSamples.empty())
					return QVariant();
				return static_cast<bool>(CheckedSamples[section]) ? Qt::Checked : Qt::Unchecked;
			}
		}

		if (orientation == Qt::Horizontal)
		{
			if (section > static_cast<int>(HorizontalHeaders.size()))
				return QVariant();

			if (role == Qt::DisplayRole)
				return HorizontalHeaders[section];

			if (role == Qt::DecorationRole && ShowPeakCheckboxes)
				return static_cast<bool>(HorizontalCheckboxDecoration[section]);
		}

		if (orientation == Qt::Vertical && role == Qt::DecorationRole && ShowSampleCheckboxes)
		{
			if (ShowYConcentration)
			{
				const auto value = data(index(section, YConcentrationColumn)).toString();
				VerticalCheckboxDecoration[section] = AllowEmptyConcentration || (!value.isNull() && !value.isEmpty());
			}
			return QVariant(VerticalCheckboxDecoration[section]);
		}

		return QVariant();
	}

	bool AnalysisDataTableModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant& value, int role)
	{
		if (role != Qt::CheckStateRole)
			return false;

		if (orientation == Qt::Horizontal
			&& HorizontalCheckboxDecoration[section]
			&& ShowPeakCheckboxes)
		{
			CheckedPeaks[section] = (value.value<Qt::CheckState>() == Qt::Unchecked) ? false : true;
			const auto it = std::find(CheckedPeaks.begin(), CheckedPeaks.end(), !CheckedPeaks.front());

			emit HeaderCheckStateChanged(orientation,
				it != CheckedPeaks.end() ? Qt::PartiallyChecked
				: CheckedPeaks.front() ? Qt::Checked : Qt::Unchecked);
			return true;
		}

		if (orientation == Qt::Vertical
			&& VerticalCheckboxDecoration[section]
			&& ShowSampleCheckboxes)
		{
			bool boolState = (value.value<Qt::CheckState>() == Qt::Unchecked) ? false : true;
			CheckedSamples[section] = boolState;
			const auto it = std::find(CheckedSamples.begin(), CheckedSamples.end(), !CheckedSamples.front());
			emit HeaderCheckStateChanged(orientation,
				it != CheckedSamples.end() ? Qt::PartiallyChecked
				: CheckedSamples.front() ? Qt::Checked : Qt::Unchecked);
			return true;
		}

		return false;
	}

	Qt::ItemFlags AnalysisDataTableModel::flags(const QModelIndex& index) const
	{
		if (index.column() == YConcentrationColumn) {
			return Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled;
		}

		return QAbstractTableModel::flags(index);
	}

	void AnalysisDataTableModel::SetModel(IdentityModelPtr model)
	{
		beginResetModel();
		DataTableModel = model;
		FillHorizontalHeaders();
		endResetModel();
	}

	void AnalysisDataTableModel::SetCurrentParameter(int param)
	{
		Parameter = param;
		emit dataChanged(index(0, 0), index(rowCount(), columnCount()));
	}

	void AnalysisDataTableModel::SetShowCheckboxes(bool isShow)
	{
		ShowPeakCheckboxes = isShow;
		ShowSampleCheckboxes = isShow;
		emit headerDataChanged(Qt::Horizontal, 0, columnCount() - 1);
		emit headerDataChanged(Qt::Vertical, 0, rowCount() - 1);
	}

	void AnalysisDataTableModel::SetShowPeakCheckboxes(bool isShow)
	{
		ShowPeakCheckboxes = isShow;
		emit headerDataChanged(Qt::Horizontal, 0, columnCount() - 1);
	}

	void AnalysisDataTableModel::FillPeaksChecked(bool checked)
	{
		CheckedPeaks.assign(CheckedPeaks.size(), checked);
		emit headerDataChanged(Qt::Horizontal, 0, columnCount() - 1);
	}

	void AnalysisDataTableModel::FillSamplesChecked(bool checked)
	{
		if (checked)
			CheckedSamples = VerticalCheckboxDecoration;
		else
			CheckedSamples.assign(CheckedSamples.size(), checked);
		emit headerDataChanged(Qt::Vertical, 0, CheckedSamples.size() - 1);
	}

	void AnalysisDataTableModel::SetCheckedPeaks(const std::vector<bool>& peaks)
	{
		if (CheckedPeaks.size() == peaks.size())
			CheckedPeaks = peaks;
		else
			std::copy(peaks.begin(), peaks.end(), CheckedPeaks.begin() + PeakZeroIndex);

		emit headerDataChanged(Qt::Horizontal, 0, columnCount() - 1);
	}

	void AnalysisDataTableModel::SetCheckedSamples(const std::vector<bool>& samples)
	{
		CheckedSamples = samples;
		emit headerDataChanged(Qt::Vertical, 0, rowCount() - 1);
	}

	void AnalysisDataTableModel::SetShowYConcentration(bool isShown)
	{
		ShowYConcentration = isShown;
		if (!isShown)
			VerticalCheckboxDecoration.assign(CheckedSamples.size(), true);
	}

	void AnalysisDataTableModel::SetAllowEmptyConcentrations(bool allow)
	{
		AllowEmptyConcentration = allow;
		if (allow)
			VerticalCheckboxDecoration.assign(CheckedSamples.size(), true);
	}

	Model::IdentityModelPtr AnalysisDataTableModel::GetModel() const
	{
		return DataTableModel;
	}

	std::vector<bool> AnalysisDataTableModel::GetCheckedPeaks() const
	{
		const auto firstIt = CheckedPeaks.begin() + PeakZeroIndex;
		return { firstIt, firstIt + PeakCount };
	}

	std::vector<bool> AnalysisDataTableModel::GetCheckedSamples() const
	{
		return CheckedSamples;
	}

	std::vector<std::optional<double>> AnalysisDataTableModel::GetConcentrations() const
	{
		return YConcentrations;
	}

	void AnalysisDataTableModel::SetConcentration(const std::vector<std::optional<double>>& yConcentrations)
	{
		YConcentrations = yConcentrations;
		emit dataChanged(index(0, YConcentrationColumn), index(rowCount(), YConcentrationColumn));
	}

	int AnalysisDataTableModel::GetYConcentrationColumn() const
	{
		return YConcentrationColumn;
	}

  int AnalysisDataTableModel::GetCurrentParameter()
  {
    return Parameter;
  }

  Double2DVector AnalysisDataTableModel::GetCurrentData()
  {
    return DataTableModel->Get<Double2DVector>(ParameterAccessName[Parameter]);
  }

  Structures::IntVector AnalysisDataTableModel::GetLibraryElementIds()
  {
    IntVector ids;
    DataTableModel->Exec<IntVector>(ReferenceAnalysisDataTableModel::ValuesLibraryElementIds,
                                    [&](IntVector a_ids) { ids = a_ids; });
    return ids;
  }

	void AnalysisDataTableModel::FillHorizontalHeaders()
	{
		DataTableModel->Exec<size_t>(ReferenceAnalysisDataTableModel::RowCount,
			[&](size_t rowCount) { CheckedSamples.resize(rowCount); });

		VerticalCheckboxDecoration.assign(CheckedSamples.size(), true);
		YConcentrations.resize(CheckedSamples.size());
		HorizontalHeaders.clear();

		const auto& sampleHeaders = DataTableModel->Get<StringList>(ReferenceAnalysisDataTableModel::SampleHeaders);
		const auto& peakHeaders = DataTableModel->Get<StringList>(ReferenceAnalysisDataTableModel::ValuesHeaders);
		const auto& passportHeaders = DataTableModel->Get<StringList>(ReferenceAnalysisDataTableModel::PassportHeader);

		// fill horizontal headers
		HorizontalHeaders.reserve(sampleHeaders.size() + peakHeaders.size() + passportHeaders.size() + 1);
		std::transform(sampleHeaders.begin(), sampleHeaders.end(), std::back_inserter(HorizontalHeaders),
			[](const std::string& name) { return QString::fromStdString(name); });
		YConcentrationColumn = HorizontalHeaders.size();

		DataTableModel->Exec<std::string>(ReferenceAnalysisDataTableModel::ConcentrationHeader,
			[&](const std::string& name) {HorizontalHeaders.push_back(QString::fromStdString(name)); });

		PeakZeroIndex = HorizontalHeaders.size(); // + concentration header

		std::transform(peakHeaders.begin(), peakHeaders.end(), std::back_inserter(HorizontalHeaders),
			[](const std::string& name) { return QString::fromStdString(name); });

		std::transform(passportHeaders.begin(), passportHeaders.end(), std::back_inserter(HorizontalHeaders),
			[](const std::string& name) { return QString::fromStdString(name); });

		PeakCount = static_cast<int>(peakHeaders.size());
		CheckedPeaks.resize(HorizontalHeaders.size(), false);

		// fill horizontal checkbox decoration
		HorizontalCheckboxDecoration.reserve(HorizontalHeaders.size());
		std::transform(sampleHeaders.begin(), sampleHeaders.end(), std::back_inserter(HorizontalCheckboxDecoration),
			[](const std::string&) {return false; });
		HorizontalCheckboxDecoration.push_back(false); // concentation
		std::transform(peakHeaders.begin(), peakHeaders.end(), std::back_inserter(HorizontalCheckboxDecoration),
			[](const std::string&) {return true; });
		std::transform(passportHeaders.begin(), passportHeaders.end(), std::back_inserter(HorizontalCheckboxDecoration),
			[](const std::string&) {return false; });

		std::unordered_map<int, std::string> possibleData
		{
			{ PeakHeight, ReferenceAnalysisDataTableModel::ValuesHeightData },
			{ PeakArea, ReferenceAnalysisDataTableModel::ValuesAreaData },
			{ PeakCovatsIndex, ReferenceAnalysisDataTableModel::ValuesCovatsData },
			{ PeakRetentionTime, ReferenceAnalysisDataTableModel::ValuesRetTimeData },
            { PeakMarkerWindow, ReferenceAnalysisDataTableModel::ValuesCustomData },
		};
		for (auto& [type, name] : possibleData)
		{
			DataTableModel->Exec<Double2DVector>(name,
				[&](const Double2DVector& data)
				{
					if (!data.empty())
						ParameterAccessName.try_emplace(type, name);
				});
		}

	}
}
