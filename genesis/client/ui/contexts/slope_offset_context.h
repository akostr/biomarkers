#pragma once
#ifndef SLOPE_OFFSET_CONTEXT_H
#define SLOPE_OFFSET_CONTEXT_H

#include <QMenu>

namespace Widgets
{
  class SlopeOffsetContext : public QMenu
  {
    Q_OBJECT

  public:
    explicit SlopeOffsetContext(QWidget* parent = nullptr);
    ~SlopeOffsetContext() override = default;

    void SetTitleActionText(const QString& text);

    QAction* TitleAction = nullptr;
    QAction* ExcludePointsAction = nullptr;
    QAction* CancelExcludePointsAction = nullptr;
    QAction* PickForNewModel = nullptr;

  signals:
    void ExcludePointTriggered();
    void CancelExludePointTriggered();
    void PickForNewModelTriggered();

  private:
    void SetupUi();
  };
}
#endif