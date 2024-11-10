#pragma once
#ifndef GENESISCONTEXTSTEPBASEDMARKUP_H
#define GENESISCONTEXTSTEPBASEDMARKUP_H
#include "context.h"

class QUndoStack;

class GenesisContextStepBasedMarkup : public GenesisContext
{
  Q_OBJECT
public:
  GenesisContextStepBasedMarkup(GenesisContext* parent);
  ~GenesisContextStepBasedMarkup();

  QPointer<QUndoStack> undoStack();

protected:
  friend class ViewPageProjectMarkupConcept;
  void resetUndoStack();

private:
  QPointer<QUndoStack> mUndoStack;
};

using GenesisContextStepBasedMarkupPtr = QPointer<GenesisContextStepBasedMarkup>;

#endif // GENESISCONTEXTSTEPBASEDMARKUP_H
