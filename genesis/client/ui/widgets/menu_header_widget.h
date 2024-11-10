#ifndef MENU_HEADER_WIDGET_H
#define MENU_HEADER_WIDGET_H

#include <QWidget>
#include <QPointer>
#include <QSet>

namespace Ui {
class MenuHeaderWidget;
}

class QMenu;
class QAction;
class MenuHeaderWidget : public QWidget
{
  Q_OBJECT

public:
  explicit MenuHeaderWidget(QWidget *parent = nullptr);
  ~MenuHeaderWidget();

public:
  static const char* kReservoirModuleName;
  static const char* kBiomarkerModuleName;
  static const char* kPlotsModuleName;
  static const char* kModulePropertyName;

public:
  void setCurrentModule(int module);
  void setAvailableModulesList(const QList<int> & availableModules);

private:
  void setModuleStyle(const QString &newModule);
  QString moduleStyle() const;
  void formMenu();

signals:
  void moduleStyleChanged();

private:
  Ui::MenuHeaderWidget *ui;
  QString mModule;
  QSet<int> mAvailableModules;
  int mCurrentModule;
  QPointer<QMenu> mMenu;
  QPointer<QAction> mActionToReservoirModule;
  QPointer<QAction> mActionToBiomarkersModule;
  QPointer<QAction> mActionToPlotsModule;
  QPointer<QAction> mActionToModuleSelection;

  // QWidget interface
  Q_PROPERTY(QString module_style READ moduleStyle WRITE setModuleStyle NOTIFY moduleStyleChanged FINAL)

protected:
  void paintEvent(QPaintEvent *event) override;
};

#endif // MENU_HEADER_WIDGET_H
