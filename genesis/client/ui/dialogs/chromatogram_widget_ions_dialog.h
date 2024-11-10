//#pragma once
//
//#include "../dialogs/web_dialog.h"
//
//#include <standard_models/standard_item_model.h>
//#include <standard_views/standard_item_model_controller.h>
//
//#include <QTableView>
//#include <ui/itemviews/comboboxitemdelegate.h>
//
//class ChromatogramWidget;
//
//class QComboBox;
//class QCheckBox;
//class QSpinBox;
//
//
//class ChromatogramWidgetIonsDialog : public WebDialog
//{
//    Q_OBJECT
//public:
//    ChromatogramWidgetIonsDialog(ChromatogramWidget* parent, ChromatogramPlotOldLegacy* plot);
//
//    virtual void Accept() override;
//    virtual void Reject() override;
//
//  protected:
//    void SetupUi();
//
//  private:
//
//    ChromatogramWidget*       ParentWidget;
//    ChromatogramPlotOldLegacy*         Plot;
//
//    QList <int> ions;
//    QMap <int, int> selected_ions;
//    QMap <int, QCheckBox*> ions_checkboxes;
//    QMap <int, QComboBox*> ions_scales;
//
//    QPointer<QWidget>         Body;
//    QPointer<QVBoxLayout>     BodyLayout;
//
//    StandardItemModel *model;
//    QTableView *view;
//
//  };
