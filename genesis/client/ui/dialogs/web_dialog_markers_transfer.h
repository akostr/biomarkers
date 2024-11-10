#ifndef WEBDIALOGMARKERSTRANSFER_H
#define WEBDIALOGMARKERSTRANSFER_H

#include "web_dialog.h"
#include <QObject>
#include <QWidget>
#include <QPushButton>
#include <QLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QCheckBox>
class QRadioButton;
enum TransferParameter
{
    Height,
    Time,
    NearestHeight
};

class WebDialogMarkersTransfer : public WebDialog
{
    Q_OBJECT



public:
    WebDialogMarkersTransfer(QWidget* parent_widget);
    virtual void Accept() override;
    virtual void Reject() override;

    int getParameter();

protected:
  void SetupUi();

private:
  QWidget *_parent_widget;
  QPointer<QWidget>         Body;
  QPointer<QRadioButton>    rb_height;
  QPointer<QRadioButton>    rb_time;
  QPointer<QRadioButton>    rb_nearest_height;
  QPointer<QVBoxLayout>     BodyLayout;

  int transfer_parameter;
};

#endif // WEBDIALOGMARKERSTRANSFER_H
