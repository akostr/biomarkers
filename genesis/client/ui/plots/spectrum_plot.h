#ifndef SPECTRUMPLOT_H
#define SPECTRUMPLOT_H

#include <graphicsplot/graphicsplot.h>

using Spectrum = QPair<QVector<double>, QVector<double>>;
using SpectrumMap = QMap<int, Spectrum>;

class BarsLabelsItem;
class SpectrumPlot : public GraphicsPlot
{
  Q_OBJECT
public:
  SpectrumPlot(QWidget *parent = nullptr);
  void SwitchMode(bool isBarsMode);
  void setChromaLabelsVisible(bool visible);
  void setCompoundLabelsVisible(bool visible);

  // QWidget interface
protected:
  void mouseMoveEvent(QMouseEvent *event) override;
  void mouseDoubleClickEvent(QMouseEvent *event) override;
  void assignColor(int row);
  void removeColor(int row);

signals:
  void barColorChanged(int row, QVariant color);

private:
  QList<QColor> kColorsDrum;
  QList<int> mColorsToRowsMap;
  QMap<int, GPBars*> mBarsMap;
  QMap<int, GPCurve*> mCurvesMap;
  QMap<int, BarsLabelsItem*> mLabelsMap;
  GPBars* mReferenceBars = nullptr;
  GPCurve* mReferenceCurve = nullptr;
  BarsLabelsItem* mReferenceLabels;
  GPBarsGroup* mGroup = nullptr;
  GPItemText* mTip = nullptr;
  GPItemLine* mTipLine = nullptr;
  GPItemLine* mTipLineEnd = nullptr;
  bool mIsBarsMode = true;

public:
  void UpdateBars();
  void setReferenceSpec(const Spectrum& spec);
  void addSpec(int row, const Spectrum& spec);
  void removeSpec(int row);
  void clearSpecs();
};

#endif // SPECTRUMPLOT_H
