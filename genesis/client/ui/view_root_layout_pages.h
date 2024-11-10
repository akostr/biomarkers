#pragma once

#include "view.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QStackedWidget>
#include <QSplitter>

////////////////////////////////////////////////////
//// Root Layout Pages view class
class CircleButton;
namespace Views
{
class ViewRootLayoutPages : public View
{
  Q_OBJECT

public:
  explicit ViewRootLayoutPages(QWidget* parent = 0);
  ~ViewRootLayoutPages();

  void SetupUi();
  QPointer<View> GetCurrentPage();
  QString GetCurrentPageId() const;

  //// Handle ui context
  virtual void ApplyContextUi(const QString& dataId, const QVariant& data) override;

  int getCollapseButtonYShift() const;
  void setCollapseButtonYShift(int newCollapseButtonYShift);
signals:
  void CollapseButtonYShiftChanged();


private:
  //// Ui
  QPointer<QHBoxLayout>         RootLayout;
  QPointer<QSplitter>           RootSplitter;
  QPointer<QWidget>             PagesArea;
  QPointer<QVBoxLayout>         PagesAreaLayout;
  QPointer<CircleButton>        CollapseButton;
  QPointer<View>                Menu;

  //// Toolbar
  QPointer<QWidget>             Toolbar;

  //// Pages
  QPointer<QStackedWidget>      PagesContainer;
  QMap<QString, QPointer<View>> Pages;

  int CollapseButtonYShift = 0;
  Q_PROPERTY(int CollapseButtonYShift READ getCollapseButtonYShift WRITE setCollapseButtonYShift NOTIFY CollapseButtonYShiftChanged FINAL)

protected:
  void showEvent(QShowEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void UpdateCirclePosition(QPointF pos);

  // QObject interface
public:
  bool eventFilter(QObject *watched, QEvent *event) override;
};
}//namespace Views
