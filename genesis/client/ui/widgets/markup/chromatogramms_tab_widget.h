#ifndef CHROMATOGRAMMSTABWIDGET_H
#define CHROMATOGRAMMSTABWIDGET_H

#include <QTabWidget>
#include <QMap>

class ChromatogrammsTabWidget : public QTabWidget
{
  Q_OBJECT

public:
  enum TabType
  {
    markersTable,
    chromatogramms,
    numericTable,
    none
  };
  explicit ChromatogrammsTabWidget(QWidget* parent = nullptr);
  ~ChromatogrammsTabWidget();

  void AddTabWidget(QWidget* tabItem, const QString& tabName, TabType type);
  void HandleGuiInteractions(int guiInteractions);

private:
  QMap<TabType, int> mWidgetIndexes;
};

#endif // CHROMATOGRAMMSTABWIDGET_H
