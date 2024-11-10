#pragma once
#ifndef PLS_SETUP_VIEW_CONTEXT_H
#define PLS_SETUP_VIEW_CONTEXT_H

#include <QMenu>
#include <QWidget>
#include <QCheckBox>
#include <QPushButton>
#include <QHBoxLayout>

namespace Widgets
{
  class PlsSetupViewContext : public QMenu
  {
    Q_OBJECT
  public:
    explicit PlsSetupViewContext(QWidget* parent);
    virtual ~PlsSetupViewContext();

    QCheckBox* mShowNamesCheckBox = nullptr;
    QCheckBox* mShowPassportInfoCheckBox = nullptr;
    QCheckBox* mSelectUserStyleCheckBox = nullptr;
    QHBoxLayout* mTabBarLayout = nullptr;
    // QPushButton* mResetUserStyleBtn = nullptr;
    // QPushButton* mSetParamGroup = nullptr;
    // QPushButton* mResetAllStylesBtn = nullptr;

  private:
    void setup();
  };
}
#endif
