#pragma once
#ifndef WIDGET_WITH_MODEL_H
#define WIDGET_WITH_MODEL_H

#include <QObject>

#include "logic/models/identity_model.h"

namespace Objects
{
  using namespace Model;

  class ObjectWithModel : public QObject
  {
    Q_OBJECT
  public:
    ObjectWithModel(QObject* parent = nullptr);
    virtual ~ObjectWithModel();

    void SetModel(IdentityModelPtr model);
    IdentityModelPtr GetModel() const;

  protected:
    IdentityModelPtr WidgetModel = nullptr;

  signals:
    void ModelChanged();
  };
  using ObjectWithModelPtr = QSharedPointer<ObjectWithModel>;
}
#endif