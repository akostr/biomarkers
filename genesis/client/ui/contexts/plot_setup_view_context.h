#pragma once
#ifndef PLOT_SETUP_VIEW_CONTEXT_H
#define PLOT_SETUP_VIEW_CONTEXT_H

#include <QWidget>
#include <QMenu>
#include <QCheckBox>
#include <QPushButton>
#include <QHBoxLayout>

namespace Widgets
{
  class PlotSetupViewContext : public QMenu
  {
    Q_OBJECT
  public:
    explicit PlotSetupViewContext(QWidget* parent = nullptr);
    virtual ~PlotSetupViewContext();

    QCheckBox* mShowNamesCheckBox = nullptr;
    QCheckBox* mShowPassportInfoCheckBox = nullptr;
    QHBoxLayout* mTabBarLayout = nullptr;
    QPushButton* mSetParamGroup = nullptr;
    QPushButton* mRemoveAllUserGroups = nullptr;

  private:
    void setup();
  };
}

#endif