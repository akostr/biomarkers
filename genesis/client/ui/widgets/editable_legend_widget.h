#ifndef EDITABLE_LEGEND_WIDGET_H
#define EDITABLE_LEGEND_WIDGET_H

#include <QWidget>
#include <QPointer>

class LegendWidget;
class QPushButton;
class EditableLegendWidget : public QWidget
{
  Q_OBJECT
public:
  explicit EditableLegendWidget(QWidget *parent = nullptr);
  void setShapeBrush(const QBrush& brush);
  void setShapePen(const QPen& pen);
  void setShape(int shape);
  void setShape(const QPainterPath& shape);
  void setText(const QString& text);
  void setFont(const QFont& font);
  void setSpacing(int spacing);

signals:
  void editingRequested();

private:
  QPointer<LegendWidget>  mLegendWidget;
  QPointer<QPushButton> mEditButton;
};

#endif // EDITABLE_LEGEND_WIDGET_H
