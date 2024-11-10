#pragma once
#ifndef WEB_DIALOG_CONCENTRATION_TABLE_H
#define WEB_DIALOG_CONCENTRATION_TABLE_H

#include "web_dialog.h"
#include "logic/structures/common_structures.h"

namespace Dialogs
{
  class WebDialogConcentrationTable : public WebDialog
  {
    Q_OBJECT

  public:
    WebDialogConcentrationTable(QWidget* parent = nullptr);

    void SetConcentrationTable(Structures::ConcetrantionTable&& table);
  };
}
#endif