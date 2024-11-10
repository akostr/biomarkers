#pragma once
#ifndef PLOTS_WIDGET_H
#define PLOTS_WIDGET_H

#include <qwidget.h>

namespace Ui
{
  class TabWidget;
}

namespace Widgets
{
  class TabWidget : public QWidget
  {
    Q_OBJECT

  public:
    explicit TabWidget(QWidget* parent = nullptr);
    ~TabWidget();

    void AddTabWidget(QWidget* tabItem, const QString& tabName);

    QWidget* GetCurrentWidget() const;

  signals :
    void IndexChanged();

  private:
    Ui::TabWidget* WidgetUi = nullptr;
  };
}
#endif