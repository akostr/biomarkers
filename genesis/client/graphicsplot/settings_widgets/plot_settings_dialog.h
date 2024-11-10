#pragma once

#include <QDialog>
#include "settings_structs.h"

namespace Ui {
  class PlotSettingsDialog;
}

class GraphicsPlotExtended;
class GPCurve;
class GPAxis;
class GPLegend;
class GPCaptionItem;
class QAbstractButton;

class PlotSettingsDialog : public QDialog
{
  Q_OBJECT

public:

  explicit PlotSettingsDialog(GraphicsPlotExtended* Plot, QWidget *parent = nullptr, uint tabFlags = GraphicsPlotExtended::EnableAllTabs);
  ~PlotSettingsDialog();

public slots:
  void ApplySettings();
  void accept();

protected slots:
  void UpdatePreview();

  void OnAxisChanged();
  void OnCurveChanged();
  void OnLegendChanged();
  void OnTitleChanged();

  void OnAxisSettingsChanged(const FontSettings& settings);
  void OnLegendSettingsChanged(const FontSettings& settings);
  void OnCurveSettingsChanged(const CurveSettings& settings);

  void OnButtonBoxClicked(QAbstractButton *button);

protected:
  void SetupUi();
  void MakeConnects();
  void PreloadSettings();

  void keyPressEvent(QKeyEvent *event);

protected:
  Ui::PlotSettingsDialog* Ui;

  GraphicsPlotExtended* Plot;
  GPCurve* PreviewCurve;

  GPAxis* CurrentAxis;
  GPCurve* CurrentCurve;
  GPLegend* CurrentLegend;
  GPCaptionItem* CaptionItem;

  QMap<QObject*, FontSettings> AxisMap;
  QMap<GPCurve*, CurveSettings> CurveMap;
  QMap<QObject*, FontSettings> LegendMap;
  uint Flags;
};
