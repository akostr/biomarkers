#include "tableview_headerview_filter_popup_editor.h"

#include <genesis_style/style.h>

#include <QMouseEvent>
#include <QApplication>

#include <set>

namespace Details
{
  const int IconSize = 24;
  const int Pad = 10;
}

//// Filter popup editor
FilterPopupEditor::FilterPopupEditor(QWidget* parent, int column, QPointer<TreeModelPresentation> presentation, QPointer<TreeModel> model)
  : QWidget(parent)
  , Column(column)
  , Presentation(presentation)
  , Model(model)
{
}

void FilterPopupEditor::mousePressEvent(QMouseEvent *event)
{
  event->accept();
}

void FilterPopupEditor::mouseReleaseEvent(QMouseEvent *event)
{
  event->accept();
}

//// Filter popup editor for list of values
FilterPopupEditorValuesList::FilterPopupEditorValuesList(QWidget* parent, int column, QPointer<TreeModelPresentation> presentation, QPointer<TreeModel> model)
  : FilterPopupEditor(parent, column, presentation, model)
{
  //// Models
  auto columnModel = Model->GetSingleColumnProxyModel(column);
  QSet<QString> values;
  for (int r = 0; r < columnModel->rowCount(); r++)
  {
    values << columnModel->data(columnModel->index(r, 0), presentation->filterRole()).toString();
  }
  QStringList valuesList({values.begin(), values.end()});
  FilterModel = new FilterStringListModel(this);
  FilterModel->setStringList(valuesList);
  
  //// Update select / deselect handler
  auto updateSelectionCommands = [this]()
  {
    int checked = 0;
    for (int r = 0; r < FilterModelPresentation->rowCount(); r++)
    {
      if (FilterModelPresentation->data(FilterModelPresentation->index(r, 0), Qt::CheckStateRole) == Qt::Checked)
      {
        ++checked;
      }
    }
    
    if (checked != FilterModelPresentation->rowCount())
      SelectAll->setText(QString("<a href='#'><span style=\"text-decoration: none;\">%1</span></a>").arg(tr("Select all")));
    else
      SelectAll->setText(Style::ApplySASS(QString("<a href='#'><span style=\"text-decoration: none; color: @textColorPale;\">%1</span></a>")).arg(tr("Select all")));
    
    if (checked != 0)
      DeselectAll->setText(QString("<a href='#'><span style=\"text-decoration: none;\">%1</span></a>").arg(tr("Reset")));
    else
      DeselectAll->setText(Style::ApplySASS(QString("<a href='#'><span style=\"text-decoration: none; color: @textColorPale;\">%1</span></a>")).arg(tr("Reset")));
    
    SelectAll->setEnabled(checked != FilterModelPresentation->rowCount());
    DeselectAll->setEnabled(checked != 0);
  };
  
  //// Pfresentation model
  FilterModelPresentation = new QSortFilterProxyModel(this);
  FilterModelPresentation->setSourceModel(FilterModel);
  FilterModelPresentation->setFilterCaseSensitivity(Qt::CaseInsensitive);
  connect(FilterModelPresentation, &QAbstractItemModel::dataChanged, updateSelectionCommands);
  
  //// Ui
  QVBoxLayout* layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(Style::Scale(Details::Pad));
  
  //// Context search
  ContextSearch = new QLineEdit(this);
  ContextSearch->setPlaceholderText(tr("Search in list"));
  layout->addWidget(ContextSearch);
  connect(ContextSearch, &QLineEdit::textChanged, [this, updateSelectionCommands]()
  {
    FilterModelPresentation->setFilterWildcard(QString("*%1*").arg(ContextSearch->text()));
    updateSelectionCommands();
  });
  
  //// Select / deselect all
  {
    QHBoxLayout* toolLayout = new QHBoxLayout;
    toolLayout->setContentsMargins(0, Style::Scale(Details::Pad), 0, 0);
    toolLayout->setSpacing(Style::Scale(Details::Pad)*2);
    layout->addLayout(toolLayout);
    
    SelectAll = new QLabel(QString("<a href='#'><span style=\"text-decoration: none;\">%1</span></a>").arg(tr("Select all")), this);
    toolLayout->addWidget(SelectAll);
    connect(SelectAll, &QLabel::linkActivated, [this, updateSelectionCommands]
    {
      for (auto r = 0; r < FilterModelPresentation->rowCount(); r++)
      {
        FilterModelPresentation->setData(FilterModelPresentation->index(r, 0), Qt::Checked, Qt::CheckStateRole);
      }
      updateSelectionCommands();
    });
    
    DeselectAll = new QLabel(QString("<a href='#'><span style=\"text-decoration: none;\">%1</span></a>").arg(tr("Reset")), this);
    toolLayout->addWidget(DeselectAll);
    toolLayout->addStretch();
    connect(DeselectAll, &QLabel::linkActivated, [this, updateSelectionCommands]
    {
      for (auto r = 0; r < FilterModelPresentation->rowCount(); r++)
      {
        FilterModelPresentation->setData(FilterModelPresentation->index(r, 0), Qt::Unchecked, Qt::CheckStateRole);
      }
      updateSelectionCommands();
    });
  }
  
  //// Tbale
  Table = new QTableView(this);
  Table->horizontalHeader()->setStretchLastSection(true);
  Table->horizontalHeader()->hide();
  Table->verticalHeader()->hide();
  Table->setShowGrid(false);
  Table->setStyleSheet(Style::ApplySASS("QTableView { border: 1px solid @uiInputElementColorFrameOpaque; } QTableView::item { border: 1px solid transparent; }"));
  connect(Table, &QTableView::clicked, [this](const QModelIndex &index)
  {
    auto pos = Table->mapFromGlobal(QCursor::pos());
    if (pos.x() > 24) //// <- otherwise handled by delegate or whatever
    {
      if (FilterModelPresentation->data(index, Qt::CheckStateRole).toInt() == Qt::Checked)
        FilterModelPresentation->setData(index, Qt::Unchecked, Qt::CheckStateRole);
      else
        FilterModelPresentation->setData(index, Qt::Checked, Qt::CheckStateRole);
    }
  });
  
  Table->setModel(FilterModelPresentation);
  layout->addWidget(Table);
  
  updateSelectionCommands();
}

QVariant FilterPopupEditorValuesList::GetData() const
{
  QVariantList vl;
  QSet<QString> checked = FilterModel->GetChecked();
  for (auto& c : checked)
    vl << c;
  return vl;
}

void FilterPopupEditorValuesList::SetData(const QVariant& data)
{
  QVariantList vl = data.toList();
  QSet<QString> checked;
  for (auto& v : vl)
    checked << v.toString();
  FilterModel->SetChecked(checked);
}

//// Filter popup editor for date range
FilterPopupEditorDateRange::FilterPopupEditorDateRange(QWidget* parent, int column, QPointer<TreeModelPresentation> presentation, QPointer<TreeModel> model)
  : FilterPopupEditor(parent, column, presentation, model)
{
  //// Layouts
  QVBoxLayout* layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  
  QHBoxLayout* dates = new QHBoxLayout;
  dates->setSpacing(Style::Scale(Details::Pad));
  layout->addLayout(dates);
  
  //// Editors
  DateStart = new QDateEdit(this);
  DateStart->setFocus();
  dates->addWidget(DateStart);
  
  DateEnd = new QDateEdit(this);
  dates->addWidget(DateEnd);
  
  Calendars = new QStackedWidget(this);
  Calendars->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(Calendars);
  
  CalendarStart = new QCalendarWidget(this);
  Calendars->addWidget(CalendarStart);
  
  CalendarEnd = new QCalendarWidget(this);
  Calendars->addWidget(CalendarEnd);
  
  Calendars->setCurrentWidget(CalendarStart);
  
  //// Fill
  QDate dateStart;
  QDate dateEnd;
  
  for (int r = 0; r < Model->rowCount(); r++)
  {
    QDate date;
    QVariant d = Model->data(Model->index(r, Column), Qt::EditRole);
    if (d.isValid())
    {
      if (d
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        .typeId()
#else
        .type()
#endif
                == QMetaType::QDate)
        date = d.toDate();
      else if (d
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        .typeId()
#else
        .type()
#endif
               == QMetaType::QDateTime)
        date = d.toDateTime().date();
      else if (d
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        .typeId()
#else
        .type()
#endif
               == QMetaType::QString)
        date = QDate::fromString(d.toString());
    }
    if (date.isValid())
    {
      if (!dateStart.isValid() || date < dateStart)
        dateStart = date;
      if (!dateEnd.isValid() || date > dateEnd)
        dateEnd = date;
    }
  }
  
  //// Apply limits from available
  if (dateStart.isValid() && dateEnd.isValid())
  {
    DateStart->setDateRange(dateStart, dateEnd);
    DateEnd->setDateRange(dateStart, dateEnd);
    
    CalendarStart->setDateRange(dateStart, dateEnd);
    CalendarEnd->setDateRange(dateStart, dateEnd);
  }
  
  //// Apply existing filter data
  QVariantList filter = Presentation->GetColumnFilter(Column).toList();
  if (filter.size() != 2)
  {
    filter.clear();
    filter << dateStart;
    filter << dateEnd;
  }
  SetData(filter);
  
  connect(DateStart,      &QDateEdit::dateChanged,    [this] (const QDate& date) { CalendarStart->setSelectedDate(date); });
  connect(CalendarStart,  &QCalendarWidget::clicked,  [this] (const QDate& date) { DateStart->setDate(date); } );
  connect(DateEnd,        &QDateEdit::dateChanged,    [this] (const QDate& date) { CalendarEnd->setSelectedDate(date); });
  connect(CalendarEnd,    &QCalendarWidget::clicked,  [this] (const QDate& date) { DateEnd->setDate(date); } );
  
  connect(qApp, &QApplication::focusChanged, this, &FilterPopupEditorDateRange::ApplicationFocusChanged);
}

void FilterPopupEditorDateRange::ApplicationFocusChanged(QWidget* /*old*/, QWidget* w)
{
  if (w == DateStart)
    Calendars->setCurrentWidget(CalendarStart);
  else if (w == DateEnd)
    Calendars->setCurrentWidget(CalendarEnd);
}

QVariant FilterPopupEditorDateRange::GetData() const
{
  std::set<QDate> dates;
  dates.insert(DateStart->date());
  dates.insert(DateEnd->date());
  
  QVariantList result;
  result << *dates.begin();
  result << *dates.rbegin();
  return result;
}

void FilterPopupEditorDateRange::SetData(const QVariant& filter)
{
  QDate dateStart;
  QDate dateEnd;
  if (filter.isValid())
  {
    QVariantList filterList = filter.toList();
    if (filterList.size() == 2)
    {
      QVariant first = filterList.first();
      QVariant last = filterList.last();
      
      if (first.isValid() && first
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        .typeId()
#else
        .type()
#endif
          == QMetaType::QDate)
        dateStart = first.toDate();
      
      if (last.isValid() && last
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        .typeId()
#else
        .type()
#endif
          == QMetaType::QDate)
        dateEnd = last.toDate();
    }
  }
  
  //// Apply dates
  if (dateStart.isValid())
  {
    DateStart->setDate(dateStart);
    CalendarStart->setSelectedDate(dateStart);
  }
  if (dateEnd.isValid())
  {
    DateEnd->setDate(dateEnd);
    CalendarEnd->setSelectedDate(dateEnd);
  }
}

QSize FilterPopupEditorDateRange::minimumSizeHint() const
{
  QSize size = FilterPopupEditor::minimumSizeHint();
  size.setWidth(size.width() * 3 / 2);
  return size;
}

///// Filter string list model
FilterStringListModel::FilterStringListModel(QObject* parent)
  : QStringListModel(parent)
{
}

Qt::ItemFlags FilterStringListModel::flags(const QModelIndex &index) const
{
  return Qt::ItemIsEnabled|Qt::ItemIsUserCheckable;
}

QVariant FilterStringListModel::data(const QModelIndex &index, int role) const
{
  if (role == Qt::CheckStateRole)
  {
    if (Checked.contains(data(index, Qt::EditRole).toString()))
      return Qt::Checked;
    else
      return Qt::Unchecked;
  }
  if (role == Qt::DisplayRole)
  {
    QVariant result = QStringListModel::data(index, role);
    if (result.toString().isEmpty())
    {
      result = tr("[empty]");
    }
    return result;
  }
  return QStringListModel::data(index, role);
}

bool FilterStringListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
  if (role == Qt::CheckStateRole)
  {
    if (value.toInt() == Qt::Checked)
    {
      Checked << data(index, Qt::EditRole).toString();
    }
    else
    {
      Checked.remove(data(index, Qt::EditRole).toString());
    }
    emit dataChanged(index, index);
    return true;
  }
  return setData(index, value, role);
}

QSet<QString> FilterStringListModel::GetChecked() const
{
  return Checked;
}

void FilterStringListModel::SetChecked(const QSet<QString> checked)
{
  beginResetModel();
  Checked = checked;
  endResetModel();
}

//// Filter popup editor for substring
FilterPopupEditorSubstring::FilterPopupEditorSubstring(QWidget* parent, int column, QPointer<TreeModelPresentation> presentation, QPointer<TreeModel> model)
  : FilterPopupEditor(parent, column, presentation, model)
{
  //// Layouts
  QVBoxLayout* layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);

  //// Editor
  Edit = new QLineEdit(this);
  layout->addWidget(Edit);
}

QVariant FilterPopupEditorSubstring::GetData() const
{
  return Edit->text();
}

void FilterPopupEditorSubstring::SetData(const QVariant& data)
{
  Edit->setText(data.toString());
}

//// Filter popup editor for integer range
FilterPopupEditorIntegerRange::FilterPopupEditorIntegerRange(QWidget* parent, int column, QPointer<TreeModelPresentation> presentation, QPointer<TreeModel> model)
  : FilterPopupEditor(parent, column, presentation, model)
{
  //// Layout
  QHBoxLayout* layout = new QHBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(Style::Scale(Details::Pad));
  
  QLabel* from = new QLabel(tr("From"), this);
  layout->addWidget(from);
  
  Minimum = new QSpinBox(this);
  Minimum->setSingleStep(1);
  layout->addWidget(Minimum, 1);
  
  QLabel* to = new QLabel(tr("To"), this);
  layout->addWidget(to);
  
  Maximum = new QSpinBox(this);
  Maximum->setSingleStep(1);
  layout->addWidget(Maximum, 1);
  
  //// Fill
  int mn = 0;
  int mx = 0;
  
  for (int r = 0; r < Model->rowCount(); r++)
  {
    int value = 0;
    QVariant d = Model->data(Model->index(r, Column), Qt::EditRole);
    if (d.isValid())
    {
      QSet<int> integerConvertable = {
        QMetaType::Int,
        QMetaType::UInt,
        QMetaType::Double,
        QMetaType::Long,
        QMetaType::LongLong,
        QMetaType::Short,
        QMetaType::ULong,
        QMetaType::ULongLong
      };
      if (integerConvertable.contains(d
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        .typeId()
#else
        .type()
#endif
          ))
        value = d.toInt();
      else if (d
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        .typeId()
#else
        .type()
#endif
               == QMetaType::QString)
        value = d.toString().toInt();
    }

    if (value < mn)
      mn = value;
    if (value > mx)
      mx = value;
  }
  
  //// Apply limits from available
  Minimum->setRange(mn, mx);
  Minimum->setValue(mn);
  Maximum->setRange(mn, mx);
  Maximum->setValue(mx);
  
  //// Apply existing filter data
  QVariantList filter = Presentation->GetColumnFilter(Column).toList();
  if (filter.size() != 2)
  {
    filter.clear();
    filter << mn;
    filter << mx;
  }
  SetData(filter);
}

QVariant FilterPopupEditorIntegerRange::GetData() const
{
  QVariantList filter;
  filter << Minimum->value();
  filter << Maximum->value();
  return filter;
}

void FilterPopupEditorIntegerRange::SetData(const QVariant& filter)
{
  int mn = 0;
  int mx = 0;
  if (filter.isValid())
  {
    QVariantList filterList = filter.toList();
    if (filterList.size() == 2)
    {
      QVariant first = filterList.first();
      QVariant last = filterList.last();
      
      if (first.isValid() && first
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        .typeId()
#else
        .type()
#endif
          == QMetaType::Int)
        mn = first.toInt();
      
      if (last.isValid() && last
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        .typeId()
#else
        .type()
#endif
          == QMetaType::Int)
        mx = last.toInt();
    }
  }
  else
  {
    return;
  }
  
  //// Apply dates
  Minimum->setValue(mn);
  Maximum->setValue(mx);
}
