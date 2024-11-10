#include "plot_setup_view_context.h"

#include <genesis_style/style.h>
#include <extern/common_gui/standard_widgets/tumbler.h>

#include <QWidgetAction>

namespace Widgets
{
  PlotSetupViewContext::PlotSetupViewContext(QWidget* parent)
    : QMenu(parent)
  {
    setup();
  }

  PlotSetupViewContext::~PlotSetupViewContext()
  {
    delete mShowNamesCheckBox;
    delete mShowPassportInfoCheckBox;
    delete mTabBarLayout;
    delete mSetParamGroup;
    delete mRemoveAllUserGroups;
  }

  void PlotSetupViewContext::setup()
  {
    setAttribute(Qt::WA_TranslucentBackground, true);
    setStyleSheet(Style::Genesis::GetUiStyle());
    const auto wa = new QWidgetAction(this);
    const auto wgt = new QWidget;
    const auto wgtL = new QVBoxLayout;
    wgt->setLayout(wgtL);

    mShowNamesCheckBox = new Tumbler(tr("Show samples names"));
    mShowNamesCheckBox->setToolTip(tr("The setting is not applied to kernels whose names include resources."));
    mShowNamesCheckBox->setChecked(true);
    wgtL->addWidget(mShowNamesCheckBox);

    mShowPassportInfoCheckBox = new Tumbler(tr("Show passport info"));
    mShowPassportInfoCheckBox->setToolTip(tr("The setting does not affect samples in which information from the passport is forcibly included."));
    mShowPassportInfoCheckBox->setChecked(true);
    wgtL->addWidget(mShowPassportInfoCheckBox);

    mSetParamGroup = new QPushButton(tr("Set param group"));
    mSetParamGroup->setStyleSheet(Style::ApplySASS(".QPushButton { color: rgb(0, 0, 0);padding: 0px; background-color: transparent;}"
      "QPushButton:pressed {background-color: @dialogBoxButtonSecondaryBg}"));
    mSetParamGroup->setFlat(true);
    wgtL->addWidget(mSetParamGroup);
    wgtL->setAlignment(mSetParamGroup, Qt::AlignLeft);

    mRemoveAllUserGroups = new QPushButton(tr("Remove all user group from legend"));
    mRemoveAllUserGroups->setFlat(true);
    mRemoveAllUserGroups->setStyleSheet(Style::ApplySASS(".QPushButton { color: rgb(235, 51, 51);padding: 0px; background-color: transparent;}"
      "QPushButton:pressed {background-color: @dialogBoxButtonSecondaryBg}"));
    wgtL->addWidget(mRemoveAllUserGroups);
    wgtL->setAlignment(mRemoveAllUserGroups, Qt::AlignLeft);

    wa->setDefaultWidget(wgt);
    addAction(wa);
  }
}
