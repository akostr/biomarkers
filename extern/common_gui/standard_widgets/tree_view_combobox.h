#pragma once

#include <QComboBox>
#include <QTimer>
#include <QStyledItemDelegate>

class QTreeView;

class TreeViewComboBox : public QComboBox
{
  Q_OBJECT

  typedef std::function<bool(QModelIndex)> CanAcceptIndexFunc;

public:
  explicit TreeViewComboBox(QWidget* parent);
  ~TreeViewComboBox();

  void SetCanAcceptIndexFunction(CanAcceptIndexFunc func);

  void SetCurrentIndex(const QModelIndex& index);
  QModelIndex GetCurrentIndex() const;

  void setModel(QAbstractItemModel* m);

  void showPopup();

protected slots:
  void OnCurrentIndexChanged();
  void OnModelDataChanged();
  void UpdateCurrentText();

  void paintEvent(QPaintEvent* event);
  bool eventFilter(QObject* object, QEvent* event);

signals:
  void CurrentIndexChanged(QModelIndex index);

protected:
  CanAcceptIndexFunc CanAcceptIndex;
  QPersistentModelIndex CurrentModelIndex;
  QTimer OpenTimer;
};

class TreeViewComboBoxItemDlegate : public QStyledItemDelegate
{
  Q_OBJECT
public:
  explicit TreeViewComboBoxItemDlegate(QObject* parent = nullptr);
  ~TreeViewComboBoxItemDlegate();

  void NotifySizeHintChanged(QAbstractItemModel* model, const QModelIndex& index = QModelIndex());

  virtual void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
  virtual QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;

private:
  mutable int CachedComboWidth;
};