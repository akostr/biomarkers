#pragma once
#ifndef GP_HIGHLIGHT_CURVE_H
#define GP_HIGHLIGHT_CURVE_H

#include "graphicsplot/graphicsplot.h"

class GPHighlightCurve final : public GPCurve
{
  Q_OBJECT
public:
  explicit GPHighlightCurve(GPAxis* keyAxis, GPAxis* valueAxis, bool registerInPLot = true);
  ~GPHighlightCurve() override = default;

  void ExcludeSelectedRange();
  void CancelExcludeSelectedRange();
  QList<GPDataRange> GetExcludedRanges() const;
  QList<GPDataRange> GetSelectedSegments() const;
  QList<GPDataRange> GetUnSelectedSegments() const;
  void AddRangeForExclude(const GPDataRange& range);
  void CancelExcludeSelectedRange(const GPDataRange& range);

  void ClearExcluded();
  void SetExcludedColor(const QColor& color);
  void SetSelectionColor(const QColor& color);
  void SetSelectionWidth(int width);
  bool IsSelectedAreExcluded() const;

protected:
  int SelectionWidth = 6;
  QColor SelectionColor = QColor(187, 227, 250);
  QColor ExcludedColor = QColor(232, 235, 237);
  void draw(GPPainter* painter) override;
  QList<GPDataRange> ExcludedRanges;
};
#endif