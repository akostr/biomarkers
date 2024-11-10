#pragma once
#ifndef EDIT_DELETE_MENU_CONTEXT_H
#define EDIT_DELETE_MENU_CONTEXT_H

#include <QMenu>
#include <QPointer>

namespace Widgets
{
  class EditDeleteMenuContext : public QMenu
  {
    Q_OBJECT

  public:
    EditDeleteMenuContext(QWidget* parent = nullptr);
    ~EditDeleteMenuContext() = default;

  signals:
    void Edit();
    void Delete();

  private:
    void Setup();
  };

  using EditDeleteMenuContextPtr = QPointer<EditDeleteMenuContext>;
}
#endif