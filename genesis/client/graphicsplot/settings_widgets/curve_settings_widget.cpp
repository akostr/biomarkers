#include "curve_settings_widget.h"
#include "ui_curve_settings_widget.h"

#if defined(USE_CUSTOM_WINDOWS)
  #include <custom_windows/custom_windows.h>
#endif

CurveSettingsWidget::CurveSettingsWidget(QWidget *parent) :
  QWidget(parent),
  Ui(new Ui::CurveSettingsWidget)
{
  Ui->setupUi(this);
  Ui->scrollAreaWidgetContents->setStyleSheet("background-color: white;");

  Ui->lineStyle->addItem(tr("None"), (int)Qt::NoPen);
  Ui->lineStyle->addItem(tr("Solid"), (int)Qt::SolidLine);
  Ui->lineStyle->addItem(tr("Dash"), (int)Qt::DashLine);
  Ui->lineStyle->addItem(tr("Dot"), (int)Qt::DotLine);
  Ui->lineStyle->addItem(tr("Dash-dot"), (int)Qt::DashDotLine);
  Ui->lineStyle->addItem(tr("Dash-dot-dot"), (int)Qt::DashDotDotLine);

  Ui->pointStyle->addItem(tr("None"), GPScatterStyle::ssNone);
  Ui->pointStyle->addItem(tr("Dot"), GPScatterStyle::ssDot);
  Ui->pointStyle->addItem(tr("Cross"), GPScatterStyle::ssCross);
  Ui->pointStyle->addItem(tr("Plus"), GPScatterStyle::ssPlus);
  Ui->pointStyle->addItem(tr("Circle"), GPScatterStyle::ssCircle);
  Ui->pointStyle->addItem(tr("Disc"), GPScatterStyle::ssDisc);
  Ui->pointStyle->addItem(tr("Square"), GPScatterStyle::ssSquare);
  Ui->pointStyle->addItem(tr("Diamond"), GPScatterStyle::ssDiamond);
  Ui->pointStyle->addItem(tr("Star"), GPScatterStyle::ssStar);
  Ui->pointStyle->addItem(tr("Triangle"), GPScatterStyle::ssTriangle);
  Ui->pointStyle->addItem(tr("Triangle Inverted"), GPScatterStyle::ssTriangleInverted);
  Ui->pointStyle->addItem(tr("Cross Square"), GPScatterStyle::ssCrossSquare);
  Ui->pointStyle->addItem(tr("Plus Square"), GPScatterStyle::ssPlusSquare);
  Ui->pointStyle->addItem(tr("Cross Circle"), GPScatterStyle::ssCrossCircle);
  Ui->pointStyle->addItem(tr("Plus Circle"), GPScatterStyle::ssPlusCircle);
  Ui->pointStyle->addItem(tr("Peace"), GPScatterStyle::ssPeace);

  Ui->brushStyle->addItem(tr("None"), (int)Qt::NoBrush);
  Ui->brushStyle->addItem(tr("Solid"), (int)Qt::SolidPattern);
  Ui->brushStyle->addItem(tr("Dense 1"), (int)Qt::Dense1Pattern);
  Ui->brushStyle->addItem(tr("Dense 2"), (int)Qt::Dense2Pattern);
  Ui->brushStyle->addItem(tr("Dense 3"), (int)Qt::Dense3Pattern);
  Ui->brushStyle->addItem(tr("Dense 4"), (int)Qt::Dense4Pattern);
  Ui->brushStyle->addItem(tr("Dense 5"), (int)Qt::Dense5Pattern);
  Ui->brushStyle->addItem(tr("Dense 6"), (int)Qt::Dense6Pattern);
  Ui->brushStyle->addItem(tr("Dense 7"), (int)Qt::Dense7Pattern);
  Ui->brushStyle->addItem(tr("Horizontal"), (int)Qt::HorPattern);
  Ui->brushStyle->addItem(tr("Vertical"), (int)Qt::VerPattern);
  Ui->brushStyle->addItem(tr("Cross"), (int)Qt::CrossPattern);
  Ui->brushStyle->addItem(tr("Diagonally left"), (int)Qt::BDiagPattern);
  Ui->brushStyle->addItem(tr("Diagonally right"), (int)Qt::FDiagPattern);
  Ui->brushStyle->addItem(tr("Diagonally cross"), (int)Qt::DiagCrossPattern);

  auto colorMap = GraphicsPlotExtendedStyle::GetDefaultColorMap();
  for (auto it = colorMap.begin(); it != colorMap.end(); ++it)
    QColorDialog::setCustomColor(it.key(), it.value());

  connect(Ui->lineWidth, SIGNAL(valueChanged(double)), this, SLOT(OnSettingsChanged()));
  connect(Ui->lineStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(OnSettingsChanged()));
  connect(Ui->lineColor, SIGNAL(ColorChanged()), this, SLOT(OnSettingsChanged()));

  connect(Ui->pointSize, SIGNAL(valueChanged(double)), this, SLOT(OnSettingsChanged()));
  connect(Ui->pointStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(OnSettingsChanged()));
  connect(Ui->pointColor, SIGNAL(ColorChanged()), this, SLOT(OnSettingsChanged()));

  connect(Ui->brushStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(OnSettingsChanged()));
  connect(Ui->brushColor, SIGNAL(ColorChanged()), this, SLOT(OnSettingsChanged()));
}

CurveSettingsWidget::~CurveSettingsWidget()
{
  delete Ui;
}

void CurveSettingsWidget::SetSettings(const CurveSettings& settings)
{
  {
    QSignalBlocker l(Ui->lineWidth);
    Ui->lineWidth->setValue(settings.linePen.widthF());
  }
  {
    QSignalBlocker l(Ui->lineStyle);
    Ui->lineStyle->setCurrentIndex(settings.linePen.style());
  }
  Ui->lineColor->SetColor(settings.linePen.color());

  {
    QSignalBlocker l(Ui->pointStyle);
    Ui->pointStyle->setCurrentIndex(settings.pointStyle.shape());
  }
  {
    QSignalBlocker l(Ui->pointSize);
    Ui->pointSize->setValue(settings.pointStyle.size());
  }

  {
    QSignalBlocker l(Ui->brushStyle);
    Ui->brushStyle->setCurrentIndex(settings.curveBrush.style());
  }
  Ui->pointColor->SetColor(settings.pointStyle.pen().color());
  Ui->brushColor->SetColor(settings.curveBrush.color());
}

CurveSettings CurveSettingsWidget::GetSettings() const
{
  CurveSettings settings;

  settings.linePen.setColor(Ui->lineColor->GetColor());
  settings.linePen.setWidthF(Ui->lineWidth->value());
  settings.linePen.setStyle((Qt::PenStyle)Ui->lineStyle->currentData().toInt());

  settings.pointStyle.setPen(QPen(Ui->pointColor->GetColor()));
  settings.pointStyle.setBrush(QBrush(Ui->pointColor->GetColor()));
  settings.pointStyle.setSize(Ui->pointSize->value());
  settings.pointStyle.setShape((GPScatterStyle::ScatterShape)Ui->pointStyle->currentData().toInt());

  settings.curveBrush.setStyle((Qt::BrushStyle)Ui->brushStyle->currentData().toInt());
  settings.curveBrush.setColor(Ui->brushColor->GetColor());

  return settings;
}

void CurveSettingsWidget::OnSettingsChanged()
{
  emit SettingsChanged(GetSettings());
}
