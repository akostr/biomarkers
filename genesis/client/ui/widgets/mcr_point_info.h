#pragma once
#ifndef MCR_POINT_INFO
#define MCR_POINT_INFO

#include <QWidget>

namespace Ui
{
  class McrPointInfo;
}

class McrPointInfo : public QWidget
{
  Q_OBJECT
public:
  explicit McrPointInfo(QWidget* parent = nullptr);
  virtual ~McrPointInfo();

  void SetCaption(const QString& text);
  void SetSampleName(const QString& name);
  void SetComponentNumber(const QString& number);

private:
  Ui::McrPointInfo* WidgetUi = nullptr;
  void SetupUi();
};
#endif