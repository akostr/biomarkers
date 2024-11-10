#pragma once

#include "tableview_headerview.h"

#include <QSpinBox>

//// Filter popup editor
class FilterPopupEditor : public QWidget
{
  Q_OBJECT
public:
  FilterPopupEditor(QWidget* parent, int column, QPointer<TreeModelPresentation> presentation, QPointer<TreeModel> model);
  
  virtual void mousePressEvent(QMouseEvent *event) override;
  virtual void mouseReleaseEvent(QMouseEvent *event) override;
  
  virtual QVariant GetData() const = 0;
  virtual void SetData(const QVariant& data) = 0;
  
protected:
  //// Logic
  int                             Column;
  QPointer<TreeModelPresentation> Presentation;
  QPointer<TreeModel>             Model;
};

///// Filter string list model
class FilterStringListModel : public QStringListModel
{
  Q_OBJECT
public:
  FilterStringListModel(QObject* parent);
  
  virtual Qt::ItemFlags flags(const QModelIndex &index) const override;
  virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
  virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
  
  QSet<QString> GetChecked() const;
  void SetChecked(const QSet<QString> checked);
  
private:
  QSet<QString> Checked;
};

//// Filter popup editor for list of values
class FilterPopupEditorValuesList : public FilterPopupEditor
{
  Q_OBJECT
public:
  FilterPopupEditorValuesList(QWidget* parent, int column, QPointer<TreeModelPresentation> presentation, QPointer<TreeModel> model);
  
  virtual QVariant GetData() const override;
  virtual void SetData(const QVariant& data) override;
  
private:
  //// Logic
  QPointer<FilterStringListModel>   FilterModel;
  QPointer<QSortFilterProxyModel>   FilterModelPresentation;
  
  //// Ui
  QPointer<QLineEdit>               ContextSearch;
  QPointer<QLabel>                  SelectAll;
  QPointer<QLabel>                  DeselectAll;
  QPointer<QTableView>              Table;
};

//// Filter popup editor for date range
class FilterPopupEditorDateRange : public FilterPopupEditor
{
  Q_OBJECT
public:
  FilterPopupEditorDateRange(QWidget* parent, int column, QPointer<TreeModelPresentation> presentation, QPointer<TreeModel> model);
  
  virtual QVariant GetData() const override;
  virtual void SetData(const QVariant& data) override;
  
  virtual QSize minimumSizeHint() const override;
  
public slots:
  void ApplicationFocusChanged(QWidget* /*old*/, QWidget* w);
  
private:
  QPointer<QDateEdit>       DateStart;
  QPointer<QDateEdit>       DateEnd;
  QPointer<QStackedWidget>  Calendars;
  QPointer<QCalendarWidget> CalendarStart;
  QPointer<QCalendarWidget> CalendarEnd;
};

//// Filter popup editor for substring
class FilterPopupEditorSubstring : public FilterPopupEditor
{
  Q_OBJECT
public:
  FilterPopupEditorSubstring(QWidget* parent, int column, QPointer<TreeModelPresentation> presentation, QPointer<TreeModel> model); 
  
  virtual QVariant GetData() const override;
  virtual void SetData(const QVariant& data) override;
  
private:
  QPointer<QLineEdit> Edit;
};

//// Filter popup editor for integer range
class FilterPopupEditorIntegerRange : public FilterPopupEditor
{
  Q_OBJECT
public:
  FilterPopupEditorIntegerRange(QWidget* parent, int column, QPointer<TreeModelPresentation> presentation, QPointer<TreeModel> model);

  virtual QVariant GetData() const override;
  virtual void SetData(const QVariant& data) override;

private:
  QPointer<QSpinBox> Minimum;
  QPointer<QSpinBox> Maximum;
};
