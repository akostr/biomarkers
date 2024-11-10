#include "mcr_point_info.h"

#include "ui_mcr_point_info.h"

#include "genesis_style/style.h"

McrPointInfo::McrPointInfo(QWidget* parent)
  : QWidget(parent)
{
  SetupUi();
}

McrPointInfo::~McrPointInfo()
{
  delete WidgetUi;
}

void McrPointInfo::SetCaption(const QString& text)
{
  WidgetUi->titleLabel->setText(text);
}

void McrPointInfo::SetSampleName(const QString& name)
{
  WidgetUi->sampleLabel->setText(name);
  adjustSize();
}

void McrPointInfo::SetComponentNumber(const QString& number)
{
  WidgetUi->componentLabel->setText(number);
}

void McrPointInfo::SetupUi()
{
  WidgetUi = new Ui::McrPointInfo();
  WidgetUi->setupUi(this);
  setStyleSheet(Style::ApplySASS(
    "QWidget {background-color: rgb(45, 50, 54); border-radius:2px;}"
    "QLabel{ font: @SecondaryTextFont; color: #FFFFFF; }"
  ));
}
