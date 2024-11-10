#include "plot_toolbar_widget.h"

#include <QLabel>
#include <QMenu>
#include <QWidgetAction>

#include <genesis_style/style.h>
#include <extern/common_gui/standard_widgets/tumbler.h>

namespace Widgets
{
  PlotToolbarWidget::PlotToolbarWidget(QWidget* parent)
    : QWidget(parent)
  {
    SetupToolbar();
  }

  void PlotToolbarWidget::SetupToolbar()
  {
    TabBarLayout = new QHBoxLayout(this);
    Caption = new QLabel(tr("Components for axis:"));
    Caption->setStyleSheet(Style::Genesis::Fonts::TabsControls());
    TabBarLayout->addWidget(Caption);
    PCButton = CreatePCButton();
    TabBarLayout->addWidget(PCButton);
    AlterPCButton = CreateAlterPCButton();
    TabBarLayout->addWidget(AlterPCButton);
    TabBarLayout->addWidget(CreateFrameLine());
    PickOtherAxisButton = CreatePickOtherAxisButton();
    TabBarLayout->addWidget(PickOtherAxisButton);
    SwitchScale = CreateOpenPlotButton();
    TabBarLayout->addWidget(SwitchScale);//attention here!!
    TabBarLayout->addStretch();
    CancelExcludingChoise = new QPushButton(tr("Cancel excluding choise"));
    TabBarLayout->addWidget(CancelExcludingChoise);
    TabBarLayout->addWidget(CreateFrameLine());
    DisplayMenuButton = new QPushButton(tr("Displaying settings"));
    DisplayMenuButton->setProperty("menu_white", true);
    TabBarLayout->addWidget(DisplayMenuButton);
  }

  void PlotToolbarWidget::SetNameForPCButtons(size_t pcCount)
  {
    enum class PCEnum : size_t
    {
      MIN_PC_COUNT_FOR_BUTTON = 1,
      MAX_PC_COUNT_FOR_BUTTON,
      MIN_PC_COUNT_FOR_2_BUTTONS
    };

    switch (static_cast<PCEnum>(pcCount))
    {
    case PCEnum::MIN_PC_COUNT_FOR_BUTTON:
    {
      PCButton->setText(tr("PC1 and PC1"));
      AlterPCButton->setHidden(true);
      break;
    }
    case PCEnum::MAX_PC_COUNT_FOR_BUTTON:
    {
      PCButton->setText(tr("PC1 and PC2"));
      AlterPCButton->setHidden(true);
      break;
    }
    case PCEnum::MIN_PC_COUNT_FOR_2_BUTTONS:
    {
      PCButton->setText(tr("PC1 and PC2"));
      AlterPCButton->setText(tr("PC2 and PC3"));
      AlterPCButton->setHidden(false);
      break;
    }
    default:
    {
      PCButton->setText(tr("PC1 and PC2"));
      AlterPCButton->setText(tr("PC3 and PC4"));
      AlterPCButton->setHidden(false);
      break;
    }
    };
  }

  void PlotToolbarWidget::PickedComponentButtonClicked()
  {
    AlterPCButton->setChecked(false);
  }

  void PlotToolbarWidget::AlterComponentButtonClicked()
  {
    PCButton->setChecked(false);
  }

  void PlotToolbarWidget::PickOtherButtonClicked()
  {
    PCButton->setChecked(false);
    AlterPCButton->setChecked(false);
  }

  QPushButton* PlotToolbarWidget::CreatePCButton() const
  {
    const auto countsPCbutton = new QPushButton;
    countsPCbutton->setCheckable(true);
    countsPCbutton->setChecked(false);
    countsPCbutton->setToolTip(tr("Components for axis"));
    countsPCbutton->setProperty("secondary", true);
    connect(countsPCbutton, &QPushButton::clicked, this, &PlotToolbarWidget::PickedComponentButtonClicked);
    return countsPCbutton;
  }

  QPushButton* PlotToolbarWidget::CreateAlterPCButton() const
  {
    const auto countsAlterPK_button = new QPushButton;
    countsAlterPK_button->setCheckable(true);
    countsAlterPK_button->setChecked(false);
    countsAlterPK_button->setToolTip(tr("Components for axis"));
    countsAlterPK_button->setProperty("lightweight_grey", true);
    connect(countsAlterPK_button, &QPushButton::clicked, this, &PlotToolbarWidget::AlterComponentButtonClicked);
    return countsAlterPK_button;
  }

  QPushButton* PlotToolbarWidget::CreatePickOtherAxisButton() const
  {
    const auto pickOtherAxisButton = new QPushButton(tr("Pick other axis"));
    pickOtherAxisButton->setProperty("secondary", true);
    connect(pickOtherAxisButton, &QPushButton::clicked, this, &PlotToolbarWidget::PickOtherButtonClicked);
    return pickOtherAxisButton;
  }

  Tumbler* PlotToolbarWidget::CreateOpenPlotButton() const
  {
    auto rescaleTumbler = new Tumbler(tr("Axes scale ratio 1:1"));
    rescaleTumbler->setProperty("lightweight_brand", true);
    return rescaleTumbler;
  }

  QFrame* PlotToolbarWidget::CreateFrameLine() const
  {
    const auto frameLine = new QFrame();
    frameLine->setFrameShape(QFrame::VLine);
    frameLine->setFrameShadow(QFrame::Plain);
    frameLine->setStyleSheet("QFrame { border-style: none; "
      "background-color: #ebedef; }");
    return frameLine;
  }

  void PlotToolbarWidget::SetMenu(QMenu* menu)
  {
    DisplayMenuButton->setMenu(menu);
  }
}
