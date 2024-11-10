#ifndef STEPPERWIDGET_H
#define STEPPERWIDGET_H

#include "qstatictext.h"
#include <QPen>
#include <QWidget>
#include <QToolTip>
#include <logic/markup/genesis_markup_enums.h>

namespace GenesisMarkup
{
using StepsList = QList<std::tuple<int, QString, GenesisMarkup::Steps, QString>>;
class StepperWidget : public QWidget
{
  Q_OBJECT
public:
  explicit StepperWidget(QWidget *parent = nullptr);
  //setters
  void setSteps(const StepsList &steps);
  void setFont(const QFont &font);
  void setFilledBrush(const QBrush &brush);
  void setEmptyBrush(const QBrush &brush);
  void setHighlightedFilledBrush(const QBrush &brush);
  void setHighlightedEmptyBrush(const QBrush &brush);
  void setEmptyTextPen(const QPen &pen);
  void setFilledTextPen(const QPen &pen);
  void setLabelFilledTextPen(const QPen &pen);
  void setLabelEmptyTextPen(const QPen &pen);
  void setBorderPen(const QPen &pen);

  void setCircleRadius(int pixelRadius);
  void setMargin(int margin);
  void setLineWidth(int newLineWidth);

signals:
  void stepChanged(GenesisMarkup::Steps step);

public slots:
  void stepForward();
  void setStep(int stepIndex);


  // QWidget interface
public:
  QSize sizeHint() const override;
  QSize minimumSizeHint() const override;

protected:
  void mouseReleaseEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void paintEvent(QPaintEvent *event) override;
  void resizeEvent(QResizeEvent *event) override;
  bool event(QEvent *event) override;
private:
  StepsList mSteps;
  QList<QStaticText> mLabels;
  QList<QString> mTooltips;
  QList<QStaticText> mCircleNumbers;
  QList<QPointF> mCenters;

  //settings
  QFont mFont;
  QBrush mFilledBrush;
  QBrush mCompletedBrush;
  QBrush mEmptyBrush;
  QBrush mHighlightedFilledBrush;
  QBrush mHighlightedEmptyBrush;
  QPen mEmptyTextPen;
  QPen mFilledTextPen;
  QPen mLabelFilledTextPen;
  QPen mLabelEmptyTextPen;
  QPen mBorderPen;
  int mCircleRadius;
  int mMargin;
  double mLineWidth;
  int mCurrentStep;
  int mToolTipIndex;
  QString mCurrentToolTip;

  //variables
  int mHighlightedPosition;//setted when mouse move over circles
  double mStepWidth;//calculated on resize
  int mTextMaxHeight;//calculated for size hint on resize
  int mTextMinWidth;//calculated for size hint on resize

private:
  void recalcStaticText();
  QColor resolveCircleBrushColor(int ind);
  //QColor resolveLineBrushColor(int ind);
  QColor resolveCircleTextPenColor(int ind);
  QGradient resolveLineGradient(const QRect &line, const QColor &leftColor, const QColor &rightColor);
  QString itemAt(const QPoint &pos);
};
}//GenesisMarkup

#endif // STEPPERWIDGET_H
