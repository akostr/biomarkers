#include "height_ratio_info.h"
#include "ui_height_ratio_info.h"
#include "genesis_style/style.h"

#include <QJsonObject>
#include <QFormLayout>
#include <QLabel>

namespace Dialogs
{
using Btns = QDialogButtonBox::StandardButton;
HeightRatioInfo::HeightRatioInfo(const QJsonObject &matrixData, QWidget *parent)
  : Templates::Info(parent, Btns::Close),
    ui(new Ui::HeightRatioInfo),
    mContent(new QWidget(this)),
    mData(matrixData)
{
  auto s = Templates::Info::Settings();
  s.buttonsProperties[Btns::Close]["blue"] = true;
  s.dialogHeader = tr("Matrix parameters");
  s.contentHeader = mData.tableTitle;
  applySettings(s);
  getContent()->layout()->addWidget(mContent);
  ui->setupUi(mContent);
  setupUi();
}

HeightRatioInfo::~HeightRatioInfo()
{
  delete ui;
}

void HeightRatioInfo::setupUi()
{
  mContent->setContentsMargins(0,0,0,0);
  auto formL = new QFormLayout(mContent);
  formL->setContentsMargins(0,0,0,0);
  formL->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
  formL->setLabelAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  formL->setFormAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  QString labelFontStyle = Style::ApplySASS("QLabel {font: @RegularTextFont; color: @fontColorSecondary;}");
  QString formFontStyle = Style::ApplySASS("QLabel {font: @RegularTextFont; color: @fontColorPrimary;}");
  {
    auto addRow = [&labelFontStyle, &formFontStyle, formL](const QString& label, const QString& value)
    {
      auto l = new QLabel(label);
      l->setStyleSheet(labelFontStyle);
      auto f = new QLabel(value);
      f->setStyleSheet(formFontStyle);
      formL->addRow(l, f);
    };
    addRow(tr("Window size:"), QString::number(mData.windowSize));
    addRow(tr("Minimum:"), QString::number(mData.minValue));
    addRow(tr("Maximum:"), QString::number(mData.maxValue));
    addRow(tr("Markers:"), mData.includeMarkers ? tr("enabled") : tr("disabled"));
    addRow(tr("Markup:"), mData.markupTitle);
  }
}

HeightRatioInfo::MatrixData::MatrixData(const QJsonObject &matrixData)
{
  auto jdata = matrixData["data_table_info"].toObject();
  includeMarkers = jdata["include_markers"].toBool();
  maxValue = jdata["max_value"].toDouble();
  minValue = jdata["min_value"].toDouble();
  windowSize = jdata["window_size"].toDouble();
  markupTitle = jdata["markup_title"].toString();
  tableTitle = jdata["table_title"].toString();
}
}//namespace Dialogs
