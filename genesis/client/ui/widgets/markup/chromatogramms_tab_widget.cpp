#include "chromatogramms_tab_widget.h"
#include <logic/markup/markup_step_controller.h>

ChromatogrammsTabWidget::ChromatogrammsTabWidget(QWidget *parent)
  : QTabWidget{parent}
{

}

ChromatogrammsTabWidget::~ChromatogrammsTabWidget()
{
  mWidgetIndexes.clear();
}

void ChromatogrammsTabWidget::AddTabWidget(QWidget *tabItem, const QString &tabName, TabType type)
{
    auto ind = addTab(tabItem, tabName);
    if(mWidgetIndexes.contains(type))
    {
      widget(mWidgetIndexes[type])->deleteLater();
      removeTab(mWidgetIndexes[type]);
      mWidgetIndexes.remove(type);
    }
    mWidgetIndexes[type] = ind;
}


void ChromatogrammsTabWidget::HandleGuiInteractions(int guiInteractions)
{
  auto flags = GenesisMarkup::StepGuiInteractionsFlags::fromInt(guiInteractions);
  setTabVisible(mWidgetIndexes[markersTable], flags.testFlag(GenesisMarkup::SIGMarkersTable));
  setTabVisible(mWidgetIndexes[numericTable], flags.testFlag(GenesisMarkup::SIGDigitalDataTable));
}
