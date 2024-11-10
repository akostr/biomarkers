#include "genesis_context_step_based_markup.h"
#include <QUndoStack>

GenesisContextStepBasedMarkup::GenesisContextStepBasedMarkup(GenesisContext* parent)
  : GenesisContext(parent)
  , mUndoStack(new QUndoStack(this))
{

}

GenesisContextStepBasedMarkup::~GenesisContextStepBasedMarkup()
{

}

void GenesisContextStepBasedMarkup::resetUndoStack()
{
  mUndoStack->clear();
}

QPointer<QUndoStack> GenesisContextStepBasedMarkup::undoStack()
{
  return mUndoStack;
}
