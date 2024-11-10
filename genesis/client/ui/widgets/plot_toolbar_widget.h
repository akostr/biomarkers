#pragma once
#ifndef WIDGET_WITH_PC_AXIS_H
#define WIDGET_WITH_PC_AXIS_H

#include <QWidget>
#include <QBoxLayout>
#include <QPushButton>
#include <QCheckBox>
#include <QFrame>
#include <QLabel>

class Tumbler;

namespace Widgets
{
  class PlotToolbarWidget : public QWidget
  {
    Q_OBJECT

  public:
    explicit PlotToolbarWidget(QWidget* parent = nullptr);
    virtual ~PlotToolbarWidget() = default;

    QLabel* Caption = nullptr;
    QPushButton* PCButton = nullptr;
    QPushButton* AlterPCButton = nullptr;
    QPushButton* PickOtherAxisButton = nullptr;
    QPushButton* CancelExcludingChoise = nullptr;

    Tumbler* SwitchScale = nullptr;
    QPushButton* DisplayMenuButton = nullptr;
    QMenu* mSetupViewMenu = nullptr;
    QHBoxLayout* TabBarLayout = nullptr;

    void SetNameForPCButtons(size_t pcCount);
    void SetMenu(QMenu* menu);

  private:
    void PickedComponentButtonClicked();
    void AlterComponentButtonClicked();
    void PickOtherButtonClicked();

    QPushButton* CreatePCButton() const;
    QPushButton* CreateAlterPCButton() const;
    QPushButton* CreatePickOtherAxisButton() const;
    Tumbler* CreateOpenPlotButton() const;
    QFrame* CreateFrameLine() const;

    void SetupToolbar();
  };
  using PlotToolbarWidgetPtr = QSharedPointer<PlotToolbarWidget>;
}
#endif
