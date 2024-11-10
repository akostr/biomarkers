#include "pls_setup_view_context.h"

#include <genesis_style/style.h>
#include <extern/common_gui/standard_widgets/tumbler.h>

#include <QWidgetAction>

namespace Widgets
{
  PlsSetupViewContext::PlsSetupViewContext(QWidget* parent)
    :QMenu(parent)
  {
    setup();
  }

  PlsSetupViewContext::~PlsSetupViewContext()
  {
    delete mShowNamesCheckBox;
    delete mShowPassportInfoCheckBox;
    delete mSelectUserStyleCheckBox;
    delete mTabBarLayout;
    // delete mResetUserStyleBtn;
    // delete mSetParamGroup;
    // delete mResetAllStylesBtn;
  }

  void PlsSetupViewContext::setup()
  {
    setAttribute(Qt::WA_TranslucentBackground, true);
    setStyleSheet(Style::Genesis::GetUiStyle());
    const auto wa = new QWidgetAction(this);
    const auto wgt = new QWidget;
    const auto wgtL = new QVBoxLayout;
    wgt->setLayout(wgtL);

    mShowNamesCheckBox = new Tumbler(tr("Show samples names"));
    mShowNamesCheckBox->setChecked(true);
    wgtL->addWidget(mShowNamesCheckBox);

    mShowPassportInfoCheckBox = new Tumbler(tr("Show passport info"));
    mShowPassportInfoCheckBox->setChecked(true);
    wgtL->addWidget(mShowPassportInfoCheckBox);

    mSelectUserStyleCheckBox = new Tumbler(tr("Show user style points"));
    mSelectUserStyleCheckBox->setDisabled(true);
    wgtL->addWidget(mSelectUserStyleCheckBox);

    // mSetParamGroup = new QPushButton(tr("Set param group"));
    // mSetParamGroup->setStyleSheet(Style::ApplySASS(".QPushButton { padding: 0px; background-color: transparent;}"
    //   "QPushButton:pressed {background-color: @dialogBoxButtonSecondaryBg}"));
    // mSetParamGroup->setFlat(true);
    // wgtL->addWidget(mSetParamGroup);
    // wgtL->setAlignment(mSetParamGroup, Qt::AlignLeft);

    // mResetUserStyleBtn = new QPushButton(tr("Reset user styles"));
    // mResetUserStyleBtn->setStyleSheet(Style::ApplySASS(".QPushButton { color: rgb(235, 51, 51);padding: 0px; background-color: transparent;}"
    //   "QPushButton:pressed {background-color: @dialogBoxButtonSecondaryBg}"));
    // mResetUserStyleBtn->setFlat(true);
    // wgtL->addWidget(mResetUserStyleBtn);
    // wgtL->setAlignment(mResetUserStyleBtn, Qt::AlignLeft);

    // mResetAllStylesBtn = new QPushButton(tr("Reset all styles"));
    // mResetAllStylesBtn->setFlat(true);
    // mResetAllStylesBtn->setStyleSheet(Style::ApplySASS(".QPushButton { color: rgb(235, 51, 51);padding: 0px; background-color: transparent;}"
    //   "QPushButton:pressed {background-color: @dialogBoxButtonSecondaryBg}"));
    // wgtL->addWidget(mResetAllStylesBtn);
    // wgtL->setAlignment(mResetAllStylesBtn, Qt::AlignLeft);

    wa->setDefaultWidget(wgt);
    addAction(wa);
  }
}
