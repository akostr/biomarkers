#ifndef TABSWITCHER_H
#define TABSWITCHER_H

#include <QTabBar>

class TabSwitcher : public QTabBar
{
  Q_OBJECT
public:
  TabSwitcher(QWidget* parent = nullptr);
};

#endif // TABSWITCHER_H
