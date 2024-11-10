#ifndef STEPBACKDIALOG_H
#define STEPBACKDIALOG_H

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


class WebDialogStepBack : public WebDialog
{
    Q_OBJECT
public:
    explicit WebDialogStepBack(QWidget* parent=nullptr, int step = 1);
    virtual void Accept() override;
    virtual void Reject() override;

protected:
  void SetupUi();

private:
  QWidget *parent_widget;

  int step_back;
  QPointer<QWidget>         Body;
  QPointer<QVBoxLayout>     BodyLayout;
};

#endif // STEPBACKDIALOG_H
