#include "plot_settings_dialog.h"
#include "ui_plot_settings_dialog.h"

#include <graphicsplot/settings_widgets/font_settings_widget.h>
#include <graphicsplot/graphicsplot_extended.h>

#ifdef CODEBASE_FEATURE_LICENSE_EMBEDDMENT_API
#include <licensing/licensing.h>
#endif // ~CODEBASE_FEATURE_LICENSE_EMBEDDMENT_API

PlotSettingsDialog::PlotSettingsDialog(GraphicsPlotExtended* plot, QWidget *parent, uint tabFlags)
  : QDialog(parent)
  , Ui(new Ui::PlotSettingsDialog)
  , Plot(plot)
  , PreviewCurve(nullptr)
  , CurrentAxis(nullptr)
  , CurrentCurve(nullptr)
  , CurrentLegend(nullptr)
  , CaptionItem(nullptr)
  , Flags(tabFlags)
{
  Ui->setupUi(this);
  if(!(Flags & GraphicsPlotExtended::EnableResetButton))
    Ui->buttonBox->setStandardButtons(Ui->buttonBox->standardButtons() & ~QDialogButtonBox::Reset);
  setModal(true);

#if defined(USE_CUSTOM_WINDOWS)
  setWindowTitle(tr("Plot settings"));
  Ui->titleWidget->hide();
#else
  setWindowTitle("");
  setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
#endif

  SetupUi();
  PreloadSettings();
  UpdatePreview();
  MakeConnects();
}

PlotSettingsDialog::~PlotSettingsDialog()
{
  delete Ui;
}

void PlotSettingsDialog::ApplySettings()
{
  for (const auto& key : AxisMap.keys())
  {
    if (auto axis = qobject_cast<GPAxis*>(key))
      AxisMap[axis].Apply(axis);
  }
  for (const auto& curve : CurveMap.keys())
  {
    if (curve)
      CurveMap[curve].Apply(curve);
  }

  auto inset = Plot->axisRect()->insetLayout();
  for (int i = 0; i < inset->elementCount(); i++)
    if (auto legend = qobject_cast<GPLegend*>(inset->elementAt(i)))
      LegendMap[legend].Apply(legend);

  Plot->setTitleText(Ui->titleText->text());
  Ui->titleSettings->GetSettings().Apply(Plot->getTitle());

  Plot->Style.CaptionsColor = Ui->previewPlot->Style.CaptionsColor;
  Plot->Style.CaptionsFont = Ui->previewPlot->Style.CaptionsFont;
  for (const auto& caption : Plot->Captions)
  {
    caption->setTextColor(Plot->Style.CaptionsColor);
    caption->setFont(Plot->Style.CaptionsFont);
  }
  Plot->CurvesUpdated();

  Plot->replot();
}

void PlotSettingsDialog::accept()
{
  ApplySettings();
  QDialog::accept();
}

void PlotSettingsDialog::SetupUi()
{
#ifdef CODEBASE_FEATURE_LICENSE_EMBEDDMENT_API
  CET_LICENSE_CHECK_OR_INVALIDATE_PTR(ui);
#endif // ~CODEBASE_FEATURE_LICENSE_EMBEDDMENT_API

  Ui->tabWidget->setCurrentIndex(1);

  Ui->cbAxis->addItem(tr("All axis"), (long long)nullptr);
  for (const auto& axis : Plot->Axes)
    if (!axis->label().isEmpty())
      Ui->cbAxis->addItem(axis->label(), (long long)axis);

  Ui->cbCurves->addItem(tr("All curves"), (long long)nullptr);
  for (const auto& curve : Plot->mCurves)
  {
    if (!curve->name().isEmpty())
      if (curve->data() && curve->data()->size() > 1)
        Ui->cbCurves->addItem(curve->name(), (long long)curve);
  }

  Ui->cbLegends->addItem(tr("All legends"), (long long)nullptr);
  bool hasCustomLegend = false;
  if (Plot->axisRect() && Plot->axisRect()->insetLayout())
  {
    for (int i = 0; i < Plot->axisRect()->insetLayout()->elementCount(); ++i)
    {
      if (auto legend = qobject_cast<GPLegend*>(Plot->axisRect()->insetLayout()->elementAt(i)))
      {
        if (!legend->property("Name").isNull())
        {
          Ui->cbLegends->addItem(legend->property("Name").toString(), (long long)legend);
          hasCustomLegend = true;
        }
        else if (legend == Plot->legend)
          Ui->cbLegends->addItem(tr("Default"), (long long)legend);
      }
    }
  }
  if (!hasCustomLegend)
    Ui->cbLegends->hide();

  Ui->cbAxis->setCurrentIndex(0);
  Ui->cbCurves->setCurrentIndex(0);
  Ui->cbLegends->setCurrentIndex(0);

  CurrentCurve = (GPCurve*)Ui->cbCurves->currentData().toLongLong();
  CurrentAxis = (GPAxis*)Ui->cbAxis->currentData().toLongLong();
  CurrentLegend = (GPLegend*)Ui->cbLegends->currentData().toLongLong();

  Ui->previewPlot->xAxis->setLabel(Ui->cbAxis->currentText());
  Ui->previewPlot->yAxis->setLabel(Ui->cbAxis->currentText());
  Ui->previewPlot->setFlag(GP::AASettingsEdit, false);
  Ui->previewPlot->setFlag(GP::AARulers, false);
  Ui->previewPlot->legend->setVisible(true);
  Ui->previewPlot->setTitleText(Plot->getTitleText());

  PreviewCurve = Ui->previewPlot->addCurve();
  QVector<double> k, v;
  for (double i = 0; i <= 18.9; i += 0.1)
  {
    k.append(i);
    v.append(sin(i));
  }
  PreviewCurve->setData(k, v);
  PreviewCurve->addToLegend();
  PreviewCurve->setName(Ui->cbCurves->currentText());

  Ui->previewPlot->viewAll(true);

  CaptionItem = Ui->previewPlot->addCaptionItem(PreviewCurve, tr("Preview"), false, QPoint(0, 0));
  CaptionItem->setTextPos(QPointF(10, 0));
}

void PlotSettingsDialog::MakeConnects()
{
  connect(Ui->buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(OnButtonBoxClicked(QAbstractButton*)));

  connect(Ui->curveSettings, &CurveSettingsWidget::SettingsChanged, this, &PlotSettingsDialog::OnCurveSettingsChanged);
  connect(Ui->legendSettings, &FontSettingsWidget::SettingsChanged, this, &PlotSettingsDialog::OnLegendSettingsChanged);
  connect(Ui->axisSettings, &FontSettingsWidget::SettingsChanged, this, &PlotSettingsDialog::OnAxisSettingsChanged);
  connect(Ui->titleSettings, &FontSettingsWidget::SettingsChanged, this, &PlotSettingsDialog::UpdatePreview);
  connect(Ui->captionsSettings, &FontSettingsWidget::SettingsChanged, this, &PlotSettingsDialog::UpdatePreview);

  connect(Ui->cbAxis, SIGNAL(currentIndexChanged(int)), this, SLOT(OnAxisChanged()));
  connect(Ui->cbCurves, SIGNAL(currentIndexChanged(int)), this, SLOT(OnCurveChanged()));
  connect(Ui->cbLegends, SIGNAL(currentIndexChanged(int)), this, SLOT(OnLegendChanged()));

  connect(Ui->titleText, SIGNAL(textChanged(QString)), this, SLOT(OnTitleChanged()));
}

void PlotSettingsDialog::PreloadSettings()
{
  for (const auto& axis : Plot->Axes)
    if (!axis->label().isEmpty())
      AxisMap.insert(axis, FontSettings(axis));
  if (!AxisMap.isEmpty() && (Flags & GraphicsPlotExtended::EnableAxesTab))
    AxisMap.insert(nullptr, AxisMap.first());
  else
    Ui->tabWidget->setTabEnabled(0, false);

  for (const auto& curve : Plot->mCurves)
    if (curve->data() && curve->data()->size() > 1 && !curve->name().isEmpty())
      CurveMap.insert(curve, CurveSettings(curve));
  if (!CurveMap.isEmpty() && (Flags & GraphicsPlotExtended::EnableCurvesTab))
    CurveMap.insert(nullptr, CurveMap.first());
  else
    Ui->tabWidget->setTabEnabled(1, false);

  if (Plot->axisRect() && Plot->axisRect()->insetLayout())
  {
    for (int i = 0; i < Plot->axisRect()->insetLayout()->elementCount(); ++i)
      if (auto legend = qobject_cast<GPLegend*>(Plot->axisRect()->insetLayout()->elementAt(i)))
        if (!legend->property("Name").isNull() || legend == Plot->legend)
          LegendMap.insert(legend, FontSettings(legend));
  }
  if (!LegendMap.isEmpty() && (Flags & GraphicsPlotExtended::EnableLegendTab))
    LegendMap.insert(nullptr, LegendMap.first());
  else
    Ui->tabWidget->setTabEnabled(2, false);

  for (int i = 0; i < Ui->tabWidget->count(); ++i)
  {
    if (Ui->tabWidget->isTabEnabled(i))
    {
      Ui->tabWidget->setCurrentIndex(i);
      break;
    }
  }

  Ui->titleText->setText(Plot->getTitleText());

  Ui->curveSettings->SetSettings(CurveMap.value(CurrentCurve));
  Ui->axisSettings->SetSettings(AxisMap.value(CurrentAxis));
  Ui->legendSettings->SetSettings(LegendMap.value(CurrentLegend));
  Ui->titleSettings->SetSettings(FontSettings(Plot->getTitle()));

  {
    CaptionItem->setVisible(Plot->getFlags() & GP::AACaptions);
    Ui->tabWidget->setTabEnabled(4, CaptionItem->visible());
    if (CaptionItem->visible())
    {
      FontSettings setts;
      QFont font = Plot->Style.CaptionsFont;
      setts.font = font;
      Ui->previewPlot->Style.CaptionsFont = font;
      QColor color = Plot->Style.CaptionsColor;
      setts.color = color;
      Ui->previewPlot->Style.CaptionsColor = color;
      Ui->captionsSettings->SetSettings(setts);
    }
  }
}

void PlotSettingsDialog::keyPressEvent(QKeyEvent *event)
{
  if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Escape || event->key() == Qt::Key_Enter)
    return;
  QDialog::keyPressEvent(event);
}

void PlotSettingsDialog::UpdatePreview()
{
  GraphicsPlot::blockReplots(true);

  auto curveSetts = CurveMap.value(CurrentCurve);
  curveSetts.Apply(PreviewCurve);

  auto axisSetts = AxisMap.value(CurrentAxis);
  axisSetts.Apply(Ui->previewPlot->xAxis);
  axisSetts.Apply(Ui->previewPlot->yAxis);

  auto legendSetts = LegendMap.value(CurrentLegend);
  legendSetts.Apply(Ui->previewPlot->legend);

  auto titleSetts = Ui->titleSettings->GetSettings();
  titleSetts.Apply(Ui->previewPlot->getTitle());

  auto captionSetts = Ui->captionsSettings->GetSettings();
  CaptionItem->setFont(captionSetts.font);
  CaptionItem->setTextColor(captionSetts.color);
  Ui->previewPlot->Style.CaptionsFont = captionSetts.font;
  Ui->previewPlot->Style.CaptionsColor = captionSetts.color;
  Ui->previewPlot->CurvesUpdated();

  GraphicsPlot::blockReplots(false);
  Ui->previewPlot->replot();
}

void PlotSettingsDialog::OnAxisChanged()
{
  if (CurrentAxis)
    AxisMap.insert(CurrentAxis, Ui->axisSettings->GetSettings());
  CurrentAxis = (GPAxis*)Ui->cbAxis->currentData().toLongLong();
  Ui->axisSettings->SetSettings(AxisMap.value(CurrentAxis));

  Ui->previewPlot->xAxis->setLabel(Ui->cbAxis->currentText());
  Ui->previewPlot->yAxis->setLabel(Ui->cbAxis->currentText());

  UpdatePreview();
}

void PlotSettingsDialog::OnCurveChanged()
{
  if (CurrentCurve)
    CurveMap.insert(CurrentCurve, Ui->curveSettings->GetSettings());
  CurrentCurve = (GPCurve*)Ui->cbCurves->currentData().toLongLong();
  Ui->curveSettings->SetSettings(CurveMap.value(CurrentCurve));

  if (CurrentCurve)
    PreviewCurve->setName(Ui->cbCurves->currentText());
  UpdatePreview();
}

void PlotSettingsDialog::OnLegendChanged()
{
  if (CurrentLegend)
    LegendMap.insert(CurrentLegend, Ui->legendSettings->GetSettings());
  CurrentLegend = (GPLegend*)Ui->cbLegends->currentData().toLongLong();
  Ui->legendSettings->SetSettings(LegendMap.value(CurrentLegend));

  UpdatePreview();
}

void PlotSettingsDialog::OnTitleChanged()
{
  Ui->previewPlot->setTitleText(Ui->titleText->text());
  UpdatePreview();
}

void PlotSettingsDialog::OnAxisSettingsChanged(const FontSettings& settings)
{
  if (!AxisMap.isEmpty())
  {
    if (CurrentAxis)
      AxisMap[CurrentAxis] = settings;
    else
      AxisMap[CurrentAxis].ApplyDiff(settings, AxisMap);
  }
  UpdatePreview();
}

void PlotSettingsDialog::OnCurveSettingsChanged(const CurveSettings& settings)
{
  if (!CurveMap.isEmpty())
  {
    if (CurrentCurve)
      CurveMap[CurrentCurve] = settings;
    else
      CurveMap[CurrentCurve].ApplyDiff(settings, CurveMap);
  }
  UpdatePreview();
}

void PlotSettingsDialog::OnLegendSettingsChanged(const FontSettings& settings)
{
  if (!LegendMap.isEmpty())
  {
    if (CurrentLegend)
      LegendMap[CurrentLegend] = settings;
    else
      LegendMap[CurrentLegend].ApplyDiff(settings, LegendMap);
  }
  UpdatePreview();
}

void PlotSettingsDialog::OnButtonBoxClicked(QAbstractButton *button)
{
  switch (Ui->buttonBox->buttonRole(button))
  {
    case QDialogButtonBox::ResetRole:
      Plot->ResetSettings();
      Plot->LoadSettings();
      Plot->replot();
      QDialog::reject();
      break;
    case QDialogButtonBox::ApplyRole:
      ApplySettings();
      break;
    default:
      break;
  };
}
