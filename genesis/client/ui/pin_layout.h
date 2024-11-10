#pragma once

#include <QVBoxLayout>
#include <QWidget>
#include <QScrollArea>
#include <QPointer>

class PinWidget : public QWidget
{
  Q_OBJECT
public:
  PinWidget(QWidget* parent = nullptr);
  
  void Pin(QWidget* widget);
  QPointer<QWidget> Unpin();
  QWidget* GetPinned();
  
public slots:
  void Update();
  
private:
//  QPointer<QWidget>     Container;
  QPointer<QVBoxLayout> ContainerLayout;
  
  QPointer<QWidget>     Pinned;
  QPointer<QWidget>     PrevParent;

  // QWidget interface
public:
  QSize sizeHint() const override;
  QSize minimumSizeHint() const override;
};

