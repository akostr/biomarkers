#pragma once

#include <QFrame>
#include <QPushButton>
#include <QList>
#include <QPointer>

//// Fwds
class QSplitter;
class QVBoxLayout;

/////////////////////////////////////////////
//// Collapsed Widgets View Item
class CollapsedWidgetsViewItem : public QPushButton
{
  Q_OBJECT
public:
  explicit CollapsedWidgetsViewItem(const QString& title, QWidget* parent, QLayout* layout, QWidget* target, const QString& path, bool hideInSplitter);
  ~CollapsedWidgetsViewItem();

  QWidget* GetAssociatedWidget();
  QSplitter* GetParentSplitterForAssociatedWidget(int* indexOfAssiciatedWidget = 0, int* indexOfOppositeWidget = 0, QList<int>* sizes = 0);

  QString GetSettingsPath();

  bool IsAssociatedWidgetVisible();
  void SetAssociatedWidgetVisible(bool visible);

  void Load();
  void Save(bool* arg = 0);

  void SetActive(bool active);

  virtual void paintEvent(QPaintEvent* event);

  virtual QSize sizeHint() const;
  virtual QSize minimumSizeHint() const;

  void SetHideInSplitter(bool hidden);

public slots:
  void updateWidget();

protected:
  virtual bool eventFilter(QObject *watched, QEvent *event);

private:
  QPointer<QWidget>   AssociatedWidget;
  bool                HideInSplitter;
  bool                Active;
  QString             Path;
};

/////////////////////////////////////////////
//// Collapsed Widgets View
class CollapsedWidgetsView : public QFrame
{
  Q_OBJECT
public:
  explicit CollapsedWidgetsView(QWidget* parent);
  ~CollapsedWidgetsView();

  void SetHideInSplitter(bool hidden);

  CollapsedWidgetsViewItem* CreateItemForWidget(QWidget* target, const QString& name, const QString& path);
  CollapsedWidgetsViewItem* CreateItem(const QString& name);

  QList<CollapsedWidgetsViewItem*> Items();

  void AddStretch(int stretch);
  void AddSpacing(int spacing);
  
private:
  QList<CollapsedWidgetsViewItem*> Buttons;
  QVBoxLayout*                     Layout;
  bool                             HideInSplitter;
};
