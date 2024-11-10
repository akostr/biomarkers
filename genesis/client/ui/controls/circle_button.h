#ifndef CIRCLEBUTTON_H
#define CIRCLEBUTTON_H

#include <QVector2D>
#include <QWidget>
#include <QMap>

class QParallelAnimationGroup;
class QVariantAnimation;
class QGraphicsDropShadowEffect;
class CircleButton : public QWidget
{
  Q_OBJECT
public:
  struct AnimationsParameters;
  explicit CircleButton(QWidget *parent = nullptr);

  void collapse();
  void expand();
  bool isCollapsed();
  void toggleCollapsing();
  void setupAnimations(const AnimationsParameters& parameters);
  void setCenterOn(QPoint pos);

  //QWidget interface:
public:
  QSize sizeHint() const override;
  QSize minimumSizeHint() const override;
  bool IsOnCollapseTransition() const;


protected:
  void mousePressEvent(QMouseEvent *event) override;
  void enterEvent(QEnterEvent *event) override;
  void leaveEvent(QEvent *event) override;
  void paintEvent(QPaintEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;


public:
  //properties getters
  qreal borderPenWidth() const;
  QColor borderPenColor() const;
  QColor backgroundColor() const;
  QPixmap pixmap() const;
  int collapseDurationMs() const;
  QPointF pixmapShift() const;
  bool IsCheckable() const;
  qreal radius() const;

  //properties setters
  void setBorderPenWidth(qreal newBorderPenWidth);
  void setBorderPenColor(const QColor &newBorderPenColor);
  void setBackgroundColor(const QColor &newBackgroundColor);
  void setPixmap(const QPixmap &newPixmap);
  void setCollapseDurationMs(int newCollapseDurationMs);
  void setPixmapShift(QPointF newPixmapShift);
  void setRadius(qreal newRadius);
  void setDetectionRadius(double radius);
  void setCheckable(bool newIsCheckable);
  bool IsChecked() const;
  void setIsChecked(bool newIsChecked);

signals:

  void clicked();
  void toggled(bool checked);

  //properties notifiers
  void borderPenWidthChanged();
  void borderPenColorChanged();
  void backgroundColorChanged();
  void pixmapChanged();
  void collapseDurationChanged();
  void pixmapShiftChanged();
  void radiusChanged();
  void IsCheckableChanged();

private:
  //properties
  qreal mBorderPenWidth = 0;
  double mCollapseDetectionRadius = 0;
  QColor mBorderPenColor;
  QColor mBackgroundColor;
  QPixmap mPixmap;
  int mCollapseDurationMs = 0;
  QPointF mPixmapShift;
  QPointF mCenter;
  QColor mBorderPenHighlightedColor;
  QColor mBackgroundHighlightedColor;
  qreal mRadius = 0;
  bool mIsCheckable = false;
  bool mIsChecked = false;

  Q_PROPERTY(qreal borderPenWidth READ borderPenWidth WRITE setBorderPenWidth NOTIFY borderPenWidthChanged FINAL)
  Q_PROPERTY(QColor borderPenColor READ borderPenColor WRITE setBorderPenColor NOTIFY borderPenColorChanged FINAL)
  Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor NOTIFY backgroundColorChanged FINAL)
  Q_PROPERTY(QPixmap pixmap READ pixmap WRITE setPixmap NOTIFY pixmapChanged FINAL)
  Q_PROPERTY(int collapseDurationMs READ collapseDurationMs WRITE setCollapseDurationMs NOTIFY collapseDurationChanged FINAL)
  Q_PROPERTY(QPointF pixmapShift READ pixmapShift WRITE setPixmapShift NOTIFY pixmapShiftChanged FINAL)
  Q_PROPERTY(qreal radius READ radius WRITE setRadius NOTIFY radiusChanged FINAL)
  Q_PROPERTY(bool IsCheckable READ IsCheckable WRITE setCheckable NOTIFY IsCheckableChanged FINAL)

  //members
  QParallelAnimationGroup* mCollapseAnimation;
  QParallelAnimationGroup* mHighlightAnimation;
  QParallelAnimationGroup* mPressAnimation;
  QGraphicsDropShadowEffect* mShadow;
  QMap<QString, QVariantAnimation*> mAnimations;
  bool mHighlighted;
  bool mInHighlightTransition;
  bool mIsCollapsed;
  bool mIsOnCollapseTransition;

private:
  void recalcMask();
  void initializeAnimations();
  void onWindowMouseMove(QPointF pos);
  QSize getActualSize() const;
};

struct CircleButton::AnimationsParameters
{
  qreal expandedRadius = 20;
  qreal expandedBorderWidth = 0;
  qreal collapsedRadius = 0;
  qreal collapsedBorderWidth = 0;

  QColor highlightedBackgroundColor = Qt::lightGray;
  QColor highlightedBorderColor = QColor();
  QColor initialBackgroundColor = Qt::white;
  QColor initialBorderColor = QColor();

  QPointF clickedPixmapOffset = {2,2};
  QPointF initialPixmapOffset = QPointF();

  int collapseAnimationDurationMs = 500;
  int highlightAnimationDurationMs = 100;
  int clickAnimationDurationMs = 100;
};

#endif // CIRCLEBUTTON_H
