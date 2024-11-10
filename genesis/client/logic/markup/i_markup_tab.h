#ifndef IMARKUPTAB_H
#define IMARKUPTAB_H

#include <QWidget>
#include <logic/markup/genesis_markup_forward_declare.h>
#include <logic/known_context_tag_names.h>

class QUndoCommand;
class IMarkupTab : public QWidget
{
  Q_OBJECT
public:
  IMarkupTab(QWidget* parent = nullptr);
  virtual void setMarkupModel(GenesisMarkup::MarkupModelPtr markupModelPtr) = 0;
  virtual bool isVisibleOnStep(GenesisMarkup::StepInfo step) = 0;
  virtual void setModule(Names::ModulesContextTags::Module module){};

signals:
  void newCommand(QUndoCommand* cmd);
};

#endif // IMARKUPTAB_H
