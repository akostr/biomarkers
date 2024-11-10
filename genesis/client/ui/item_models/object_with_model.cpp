#include "object_with_model.h"

#include <genesis_style/style.h>
#include <QBoxLayout>

namespace Objects
{
  ObjectWithModel::ObjectWithModel(QObject* parent)
    : QObject(parent)
  {
  }

  ObjectWithModel::~ObjectWithModel()
  {
    WidgetModel = nullptr;
  }

  void ObjectWithModel::SetModel(IdentityModelPtr model)
  {
    WidgetModel = model;
    emit ModelChanged();
  }

  IdentityModelPtr ObjectWithModel::GetModel() const
  {
    return WidgetModel;
  }
}