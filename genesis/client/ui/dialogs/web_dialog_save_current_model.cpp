#include "web_dialog_save_current_model.h"

#include <qlayout.h>
#include <qlabel.h>
#include <qpushbutton.h>

namespace Dialogs
{
  WebDialogSaveCurrentModel::WebDialogSaveCurrentModel(const QString& text, QWidget* parent)
    : WebDialogBox(tr("Save model?"), text, QDialogButtonBox::Save | QDialogButtonBox::Ignore | QDialogButtonBox::Cancel)
  {
  }
}
