#ifndef PLOT_LEGEND_WIDGET_REFACTOR_H
#define PLOT_LEGEND_WIDGET_REFACTOR_H

#include <QWidget>
#include <QPainterPath>
#include <QUuid>
#include <QPointer>
class LegendShapeItem;
class FlowLayout;
class QVBoxLayout;
class QHBoxLayout;
class QPushButton;
struct TreeItem
{
  QString className;
  QList<QSharedPointer<TreeItem>> nodes;
};
class PlotLegendWidgetRefactor : public QWidget
{
  Q_OBJECT
public:
  struct LegendItem
  {
    QPainterPath shape;
    QColor color;
    QString title;
    QUuid itemId;
    QString tooltip;
  };

  explicit PlotLegendWidgetRefactor(QWidget *parent = nullptr);
  ~PlotLegendWidgetRefactor();

  void setupUi();
  int addLayout(const QList<LegendItem>& items);
  int insertLayout(int ind, const QList<LegendItem>& items);
  bool removeLayout(int ind);
  bool replaceLayout(int ind, const QList<LegendItem>& items);
  bool addItem(int ind, const LegendItem& item, bool prepend = false);
  bool removeItem(int ind, QUuid& uid);
  void setToolTip(QUuid& uid, const QString& tooltip);

  bool setShape(const QUuid& uid, QPainterPath shape);
  bool setColor(const QUuid& uid, QColor color);
  bool setTitle(const QUuid& uid, QString title);

  int minimumVisibleRows() const;
  void setMinimumVisibleRows(int newMinimumVisibleRows);
  void clear();

signals:
  void itemEditingRequested(const QUuid& uid);

protected:
  void collapse();
  void expand();
  void toggleCollapsing();

protected:
  QHash<QUuid, QList<QPointer<LegendShapeItem>>> mItemWidgets;

private:
  void clearLayout(QPointer<FlowLayout> layout);
  void clearLayout(QPointer<QVBoxLayout> layout);
  void clearLayout(QPointer<QHBoxLayout> layout);
  QPointer<QVBoxLayout> makeSubLayout(const QList<LegendItem>& items);
  void fillFlowLayout(const QList<LegendItem>& items, bool prepend = false);

protected:
  int mSpacing;
  QList<QPointer<QVBoxLayout>> mLayouts;
  QPointer<QVBoxLayout> mRootLayout;
  QPointer<QPushButton> mHideBtn;
  QPointer<FlowLayout> mFlowLayout;
  int mMinimumVisibleRows = 3;
  bool mIsCollapsed = false;
};





#endif // PLOT_LEGEND_WIDGET_REFACTOR_H
