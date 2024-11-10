#include "tab_switcher.h"
#include <genesis_style/style.h>

TabSwitcher::TabSwitcher(QWidget* parent)
{
  setDrawBase(false);
  setStyleSheet(Style::Genesis::GetSwitcherTabBarStyle());
}
