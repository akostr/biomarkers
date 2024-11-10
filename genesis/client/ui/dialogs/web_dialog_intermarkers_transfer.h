#ifndef WebDialogIntermarkersTransfer_H
#define WebDialogIntermarkersTransfer_H

#include "web_dialog.h"
//#include <logic/match_state_machine.h>
#include <logic/markup/genesis_markup_forward_declare.h>

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
#include <QStandardItemModel>
#include <QTableView>
#include <QVector>
#include <QDoubleSpinBox>
#include <QScrollArea>


struct IntermarkerWidget
{
    QDoubleSpinBox * start;
    QDoubleSpinBox * end;
    QDoubleSpinBox *window;
};


enum IntermarkerParameter
{
    Index,
    Value,

    Last
};

class WebDialogIntermarkersTransfer : public WebDialog
{
    Q_OBJECT
public:
    explicit WebDialogIntermarkersTransfer(QWidget* parent_widget, int intervals_count);
    virtual void Accept() override;
    virtual void Reject() override;

    int getParameter();
    QVector <GenesisMarkup::IntermarkerInterval> getIntevals();

protected:
  void SetupUi();

private:
    QWidget *_parent_widget;
    int intervals;
    QPointer<QWidget>         Body;
    QPointer<QVBoxLayout>     BodyLayout;

    int lay_intervals;

    int parameter;

    QList <IntermarkerWidget> spinboxes;
};

#endif // WebDialogIntermarkersTransfer_H
