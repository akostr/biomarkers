#ifndef WEBDIALOGNEWREFERENCE_H
#define WEBDIALOGNEWREFERENCE_H

#pragma once

#include "web_dialog.h"
#include <QObject>
#include <QWidget>

#include <QPushButton>
#include <QLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QSpacerItem>
#include <QCheckBox>


class WebDialogNewReference : public WebDialog
{
    Q_OBJECT
public:
    explicit WebDialogNewReference(QWidget* parent=nullptr, QString new_name = QString());

    virtual void Accept() override;
    virtual void Reject() override;

protected:
  void SetupUi();

private:
  QWidget *parent_widget;
  QString reference_name;

  QPointer<QWidget>         Body;
  QPointer<QVBoxLayout>     BodyLayout;
};

#endif // WEBDIALOGNEWREFERENCE_H
