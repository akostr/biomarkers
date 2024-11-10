#pragma once

#include <QDialog>
#include <functional>
#include <QPointer>

namespace Ui {
  class ZonesAmountDialog;
}

class GraphicsPlotExtended;
class GraphicsPlot;
class GPCurve;
class GPZonesRect;

typedef std::function<double(double, double)> amountFunction;
typedef std::function<bool(void)> testFunction;

class ZonesAmountDialog : public QDialog
{
  Q_OBJECT

public:
  explicit ZonesAmountDialog(GraphicsPlotExtended* parentPlot);
  ~ZonesAmountDialog();

  QVariantMap GetAmountSettings();
  void SetAmountSettings(const QVariantMap& settings);

  amountFunction GetAmountFunction();
  testFunction GetTestFunction();

  static amountFunction GetAmountFunctionFromSettings(GraphicsPlot *parentPlot, const QVariantMap &settings);
  static testFunction GetTestFunctionFromSettings(GraphicsPlot* parentPlot, const QVariantMap &settings);
  static QString GetLabelDescriptonFromSettings(const QVariantMap &settings);
  static QString GetValueDimensionFromSettings(const QVariantMap &settings);

protected:
  void SetupUi();
  void UpdateUi();
  void MakeConnect();

  static amountFunction CreateAmountVolumeFunc(GPCurve* rate);
  static amountFunction CreateAmountFuncFromProppant(GPCurve* rateProp, GPCurve* concProp, double densProp);
  static amountFunction CreateAmountFuncFromProppantAndFluid(GPCurve* rateFluid, GPCurve* concProp);

protected:
  Ui::ZonesAmountDialog* Ui;

  QPointer<GraphicsPlotExtended> Plot;
  QPointer<GPZonesRect> Zones;
};
