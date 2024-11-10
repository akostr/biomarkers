#include "tab_widget.h"

#include "ui_tab_widget.h"

namespace Widgets
{
  TabWidget::TabWidget(QWidget* parent)
    : QWidget(parent)
  {
    WidgetUi = new Ui::TabWidget();
    WidgetUi->setupUi(this);
    connect(WidgetUi->tabWidget, &QTabWidget::currentChanged, this, &TabWidget::IndexChanged);
  }

  TabWidget::~TabWidget()
  {
    delete WidgetUi;
  }

  void TabWidget::AddTabWidget(QWidget* tabItem, const QString& tabName)
  {
    WidgetUi->tabWidget->addTab(tabItem, tabName);
    emit IndexChanged();
  }

  QWidget* TabWidget::GetCurrentWidget() const
  {
    return WidgetUi->tabWidget->currentWidget();
  }
}//Widgets
