#ifndef EXPANDABLE_DEFAULT_ITEM_EDITOR_FACTORY_H
#define EXPANDABLE_DEFAULT_ITEM_EDITOR_FACTORY_H

#include <QItemEditorFactory>

class ExpandableDefaultItemEditorFactory : public QItemEditorFactory
{
public:
  ExpandableDefaultItemEditorFactory();

  QWidget *createEditor(int userType, QWidget *parent) const override;
  QByteArray valuePropertyName(int userType) const override;
};

#endif // EXPANDABLE_DEFAULT_ITEM_EDITOR_FACTORY_H
