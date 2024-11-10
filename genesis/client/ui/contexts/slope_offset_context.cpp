#include "slope_offset_context.h"

namespace Widgets
{
  SlopeOffsetContext::SlopeOffsetContext(QWidget* parent)
    : QMenu(parent)
  {
    SetupUi();
  }

  void SlopeOffsetContext::SetTitleActionText(const QString& text)
  {
    TitleAction->setText(text);
  }

  void SlopeOffsetContext::SetupUi()
  {
    TitleAction = addAction(tr("PICKED %n PEAKS", "", 0));
    TitleAction->setDisabled(true);
    addSeparator();
    ExcludePointsAction = addAction(tr("Pick for excluding from model"));
    connect(ExcludePointsAction, &QAction::triggered, this, &SlopeOffsetContext::ExcludePointTriggered);

    CancelExcludePointsAction = addAction(tr("Cancel excluding from model"));
    connect(CancelExcludePointsAction, &QAction::triggered, this, &SlopeOffsetContext::CancelExludePointTriggered);

    PickForNewModel = addAction(tr("Pick for new model"));
    connect(PickForNewModel, &QAction::triggered, this, &SlopeOffsetContext::PickForNewModelTriggered);
  }
}