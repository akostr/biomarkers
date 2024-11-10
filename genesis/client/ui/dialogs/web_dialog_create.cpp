#include "web_dialog_create.h"

#include <QAbstractItemModel>
#include <QPushButton>

WebDialogCreate::WebDialogCreate(QWidget* parent, QSizeF size)
  : WebDialog(parent, nullptr, size, QDialogButtonBox::Apply | QDialogButtonBox::Cancel)
  , KeyColumn(-1)
{
}

WebDialogCreate::~WebDialogCreate()
{
}

void WebDialogCreate::SetModel(QAbstractItemModel* model, int keyColumn)
{
  Model = model;
  KeyColumn = keyColumn;
}

void WebDialogCreate::SetNamesList(const QStringList& names)
{
  Names = QSet<QString>(names.begin(), names.end());
}

void WebDialogCreate::UpdateButtons()
{
  if (auto button = ButtonBox->button(QDialogButtonBox::Apply))
  {
    button->setEnabled(CanAccept());
  }
}

bool WebDialogCreate::CanAccept() const
{
  auto newName = GetCurrentName().simplified();
  return !newName.isEmpty() && IsUnique(newName);
}

bool WebDialogCreate::IsUnique(const QString& name) const
{
  if (Model && KeyColumn != -1)
  {
    for (int r = 0; r < Model->rowCount(); ++r)
    {
      if (Model->data(Model->index(r, KeyColumn)).toString() == name)
        return false;
    }
  }
  else if (!Names.isEmpty())
  {
    return Names.contains(name);
  }
  return true;
}
