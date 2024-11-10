#include "font_settings_widget.h"
#include "ui_font_settings_widget.h"

#if defined(USE_CUSTOM_WINDOWS)
  #include <custom_windows/custom_windows.h>
#endif

#include <graphicsplot/graphicsplot_extended.h>



FontSettingsWidget::FontSettingsWidget(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::FontSettingsWidget)
{
  ui->setupUi(this);

  ui->style->addItem(tr("Normal"), (int)QFont::StyleNormal);
  ui->style->addItem(tr("Italic"), (int)QFont::StyleItalic);
  ui->style->addItem(tr("Oblique"), (int)QFont::StyleOblique);

  ui->weight->addItem(tr("Light"), (int)QFont::Light);
  ui->weight->addItem(tr("Normal"), (int)QFont::Normal);
  ui->weight->addItem(tr("DemiBold"), (int)QFont::DemiBold);
  ui->weight->addItem(tr("Bold"), (int)QFont::Bold);
  ui->weight->addItem(tr("Black"), (int)QFont::Black);

  auto colorMap = GraphicsPlotExtendedStyle::GetDefaultColorMap();
  for (auto it = colorMap.begin(); it != colorMap.end(); ++it)
    QColorDialog::setCustomColor(it.key(), it.value());

  connect(ui->font, SIGNAL(currentFontChanged(QFont)), this, SLOT(OnSettingsChanged()));
  connect(ui->size, SIGNAL(valueChanged(int)), this, SLOT(OnSettingsChanged()));
  connect(ui->style, SIGNAL(currentIndexChanged(int)), this, SLOT(OnSettingsChanged()));
  connect(ui->weight, SIGNAL(currentIndexChanged(int)), this, SLOT(OnSettingsChanged()));
  connect(ui->color, SIGNAL(ColorChanged()), this, SLOT(OnSettingsChanged()));
}

FontSettingsWidget::~FontSettingsWidget()
{
  delete ui;
}

void FontSettingsWidget::SetSettings(const FontSettings& settings)
{
  QFont font = settings.font;
  {
    QSignalBlocker l(ui->size);
    ui->size->setValue(font.pointSize());
  }
  {
    QSignalBlocker l(ui->font);
    ui->font->setCurrentFont(font);
  }
  {
    QSignalBlocker l(ui->style);
    ui->style->setCurrentIndex(ui->style->findData((int)font.style()));
  }
  {
    QSignalBlocker l(ui->weight);
    ui->weight->setCurrentIndex(ui->weight->findData((int)font.weight()));
  }

  ui->color->SetColor(settings.color);
}

FontSettings FontSettingsWidget::GetSettings() const
{
  FontSettings settings;

  QFont font = ui->font->currentFont();
  font.setPointSize(ui->size->value());
  font.setWeight((QFont::Weight)ui->weight->currentData().toInt());
  font.setStyle((QFont::Style)ui->style->currentData().toInt());
  settings.font = font;
  settings.color = ui->color->GetColor();

  return settings;
}

void FontSettingsWidget::OnSettingsChanged()
{
  emit SettingsChanged(GetSettings());
}
