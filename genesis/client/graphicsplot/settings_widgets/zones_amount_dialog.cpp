#include "zones_amount_dialog.h"
#include "ui_zones_amount_dialog.h"

#include <QPointer>
#include <graphicsplot/graphicsplot_extended.h>

ZonesAmountDialog::ZonesAmountDialog(GraphicsPlotExtended *parentPlot)
  : QDialog(parentPlot)
  , Ui(new Ui::ZonesAmountDialog)
  , Plot(parentPlot)
{
  Ui->setupUi(this);
  setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint);
  setModal(true);

  SetupUi();
  UpdateUi();
  MakeConnect();
}

ZonesAmountDialog::~ZonesAmountDialog()
{
  delete Ui;
}

QVariantMap ZonesAmountDialog::GetAmountSettings()
{
  QVariantMap map;

  map["type"] = Ui->cbWorkmode->currentIndex();
  switch (Ui->cbWorkmode->currentIndex())
  {
    case 0:
      break;
    case 1:
      {
        map["desc"] = Ui->leVolume->text();
        map["dim"] = tr("cub m");
        map["rate_id"] = Ui->cbRate->currentData().toString();
      }
      break;
    case 2:
      {
        map["desc"] = Ui->leProp->text();
        map["dim"] = tr("kg");
        map["rate_id"] = Ui->cbPropRate->currentData().toString();
        map["conc_id"] = Ui->cbPropConc->currentData().toString();
        map["dens"] = Ui->sbPropDens->value();
      }
      break;
    case 3:
      {
        map["desc"] = Ui->leProp_fluid->text();
        map["dim"] = tr("kg");
        map["rate_id"] = Ui->cbFluidRate->currentData().toString();
        map["conc_id"] = Ui->cbPropConc_fluid->currentData().toString();
      }
      break;
    default:
      break;
  }

  return map;
}

void ZonesAmountDialog::SetAmountSettings(const QVariantMap& settings)
{
  if (settings.isEmpty())
    return;

  int index = settings["type"].toInt();
  Ui->cbWorkmode->setCurrentIndex(index);
  Ui->stackedWidget->setCurrentIndex(index);

  switch (index)
  {
    case 0:
      return;
    case 1:
      {
        Ui->leVolume->setText(settings["desc"].toString());
        Ui->cbRate->setCurrentIndex(Ui->cbRate->findData(settings["rate_id"]));
      }
      return;
    case 2:
      {
        Ui->leProp->setText(settings["desc"].toString());
        Ui->cbPropRate->setCurrentIndex(Ui->cbPropRate->findData(settings["rate_id"]));
        Ui->cbPropConc->setCurrentIndex(Ui->cbPropConc->findData(settings["conc_id"]));
        Ui->sbPropDens->setValue(settings["dens"].toDouble());
      }
      return;
    case 3:
      {
        Ui->leProp_fluid->setText(settings["desc"].toString());
        Ui->cbFluidRate->setCurrentIndex(Ui->cbFluidRate->findData(settings["rate_id"]));
        Ui->cbPropConc_fluid->setCurrentIndex(Ui->cbPropConc_fluid->findData(settings["conc_id"]));
      }
      return;
    default:
      break;
  }
}

amountFunction ZonesAmountDialog::GetAmountFunction()
{
  if (Plot)
    return GetAmountFunctionFromSettings(Plot, GetAmountSettings());
  return nullptr;
}

testFunction ZonesAmountDialog::GetTestFunction()
{
  if (Plot)
    return GetTestFunctionFromSettings(Plot, GetAmountSettings());
  return nullptr;
}

amountFunction ZonesAmountDialog::GetAmountFunctionFromSettings(GraphicsPlot* parentPlot, const QVariantMap &settings)
{
  if (settings.isEmpty())
    return nullptr;

  GPCurve* curveRate = nullptr;
  GPCurve* curveConc = nullptr;
  QString rateId = settings["rate_id"].toString();
  QString concId = settings["conc_id"].toString();

  for (int i = 0; i < parentPlot->curveCount(); ++i)
  {
    if (parentPlot->curve(i)->property(GPObjectsKeys::OKCurveId).toString() == rateId)
      curveRate = parentPlot->curve(i);
    if (parentPlot->curve(i)->property(GPObjectsKeys::OKCurveId).toString() == concId)
      curveConc = parentPlot->curve(i);
  }

  switch (settings["type"].toInt())
  {
    case 0:
      return nullptr;
    case 1:
      {
        if (!curveRate)
          return nullptr;
        return CreateAmountVolumeFunc(curveRate);
      }
      return nullptr;
    case 2:
      {
        if (!curveRate || !curveConc)
          return nullptr;
        double dens = settings["dens"].toDouble();
        return CreateAmountFuncFromProppant(curveRate, curveConc, dens);
      }
    case 3:
      {
        if (!curveRate || !curveConc)
          return nullptr;
        return CreateAmountFuncFromProppantAndFluid(curveRate, curveConc);
      }
    default:
      break;
  }
  return nullptr;
}

testFunction ZonesAmountDialog::GetTestFunctionFromSettings(GraphicsPlot *parentPlot, const QVariantMap &settings)
{
  if (settings.isEmpty())
    return nullptr;

  GPCurve* curveRate = nullptr;
  GPCurve* curveConc = nullptr;
  QString rateId = settings["rate_id"].toString();
  QString concId = settings["conc_id"].toString();

  for (int i = 0; i < parentPlot->curveCount(); ++i)
  {
    if (parentPlot->curve(i)->property(GPObjectsKeys::OKCurveId).toString() == rateId)
      curveRate = parentPlot->curve(i);
    if (parentPlot->curve(i)->property(GPObjectsKeys::OKCurveId).toString() == concId)
      curveConc = parentPlot->curve(i);
  }

  switch (settings["type"].toInt())
  {
    case 0:
      return nullptr;
    case 1:
      {
        QPointer<GPCurve> ptr(curveRate);
        return [ptr](){
          return !ptr.isNull() && ptr->realVisibility();
        };
      }
      return nullptr;
    case 2:
    case 3:
      {
        QPointer<GPCurve> ptr1(curveRate);
        QPointer<GPCurve> ptr2(curveConc);
        return [ptr1, ptr2](){
          return !ptr1.isNull() && !ptr2.isNull() && ptr1->realVisibility() && ptr2->realVisibility();
        };
      }
    default:
      break;
  }
  return nullptr;
}

QString ZonesAmountDialog::GetLabelDescriptonFromSettings(const QVariantMap &settings)
{
  if (settings.isEmpty())
    return QString();
  return settings["desc"].toString();
}

QString ZonesAmountDialog::GetValueDimensionFromSettings(const QVariantMap &settings)
{
  if (settings.isEmpty())
    return QString();
  return settings["dim"].toString();
}

void ZonesAmountDialog::SetupUi()
{
  Ui->cbWorkmode->setCurrentIndex(0);
  Ui->stackedWidget->setCurrentIndex(0);
}

void ZonesAmountDialog::UpdateUi()
{
  if (Plot)
  {
    for (int i = 0; i < Plot->curveCount(); ++i)
    {
      auto curve = Plot->curve(i);
      QString name = curve->name();
      auto curveId = curve->property(GPObjectsKeys::OKCurveId);
      if (!name.isEmpty() && !curveId.isNull())
      {
        Ui->cbRate->addItem(name, curveId);
        Ui->cbFluidRate->addItem(name, curveId);
        Ui->cbPropConc->addItem(name, curveId);
        Ui->cbPropConc_fluid->addItem(name, curveId);
        Ui->cbPropRate->addItem(name, curveId);
      }
    }
  }
}

void ZonesAmountDialog::MakeConnect()
{
  connect(Ui->cbWorkmode, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index){
    Ui->stackedWidget->setCurrentIndex(index);
  });
}

amountFunction ZonesAmountDialog::CreateAmountVolumeFunc(GPCurve *rate)
{
  if (rate)
  {
    auto data = rate->data();
    std::vector<double> t = GraphicsPlotExtendedUtils::getCurveKeyDataStd(rate);
    std::vector<double> v;
    v.reserve(data->size());

    for (int i = 0; i < data->size(); ++i)
    {
      v.push_back(data->at(i)->value >= 0.0 ? data->at(i)->value : 0.0);
    }
    auto interp = GraphicsPlotExtendedUtils::getAccumulatedDataInterpolation(v, t);

    auto calcVolume = [interp](double start, double end){
      return GraphicsPlotExtendedUtils::getRangeAmount(interp, start, end) / 1000.0 / 60.0;
    };
    return calcVolume;
  }
  return nullptr;
}

amountFunction ZonesAmountDialog::CreateAmountFuncFromProppant(GPCurve *rateProp, GPCurve *concProp, double densProp)
{
  if (rateProp && concProp)
  {
    auto dataRate = rateProp->data();
    auto dataConc = concProp->data();

    std::vector<double> t = GraphicsPlotExtendedUtils::getCurveKeyDataStd(rateProp);
    std::vector<double> r;
    std::vector<double> c;
    std::vector<double> m;
    r.reserve(dataRate->size());
    c.reserve(dataConc->size());
    m.reserve(dataConc->size());

    for (int i = 0; i < dataRate->size() && i < dataConc->size(); ++i)
    {
      r.push_back(dataRate->at(i)->value);
      c.push_back(dataConc->at(i)->value);
    }
    t.resize(r.size());
    m.push_back(0);
    for (int i = 1; i < dataRate->size(); ++i)
    {
      double v = r[i] * (t[i] - t[i - 1]);
      if (v < 0.0)
        v = 0.0;
      m.push_back((v * c[i]) / (1 + c[i] / densProp));
    }
    auto interpMass = GraphicsPlotExtendedUtils::Interpolation(t, GraphicsPlotExtendedUtils::accumulateVector(m));

    auto calcVolume = [interpMass](double start, double end){
      return (interpMass.interp1d(end) - interpMass.interp1d(start)) / 1000.0 / 60.0;
    };
    return calcVolume;
  }
  return nullptr;
}

amountFunction ZonesAmountDialog::CreateAmountFuncFromProppantAndFluid(GPCurve *rateFluid, GPCurve *concProp)
{
  if (rateFluid && concProp)
  {
    auto dataRate = rateFluid->data();
    auto dataConc = concProp->data();

    std::vector<double> t = GraphicsPlotExtendedUtils::getCurveKeyDataStd(rateFluid);
    std::vector<double> r;
    std::vector<double> c;
    std::vector<double> m;
    r.reserve(dataRate->size());
    c.reserve(dataConc->size());
    m.reserve(dataConc->size());

    for (int i = 0; i < dataRate->size() && i < dataConc->size(); ++i)
    {
      r.push_back(dataRate->at(i)->value);
      c.push_back(dataConc->at(i)->value);
    }
    t.resize(r.size());
    m.push_back(0);
    for (int i = 1; i < dataRate->size(); ++i)
    {
      double v = r[i] * (t[i] - t[i - 1]);
      if (v < 0.0)
        v = 0.0;
      m.push_back(v * c[i]);
    }
    auto interpMass = GraphicsPlotExtendedUtils::Interpolation(t, GraphicsPlotExtendedUtils::accumulateVector(m));

    auto calcVolume = [interpMass](double start, double end){
      return (interpMass.interp1d(end) - interpMass.interp1d(start)) / 1000.0 / 60.0;
    };
    return calcVolume;
  }
  return nullptr;
}
