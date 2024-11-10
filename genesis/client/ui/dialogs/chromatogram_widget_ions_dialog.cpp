//#include "chromatogram_widget_ions_dialog.h"
//#include <ui/chromotogram_page/chromatogram_widget.h>
//
//#include <genesis_style/style.h>
//
//ChromatogramWidgetIonsDialog::ChromatogramWidgetIonsDialog
//    (ChromatogramWidget *parent, ChromatogramPlotOldLegacy *plot)
//    : WebDialog(parent, nullptr, QSizeF(0.8, 0.8), QDialogButtonBox::Ok|QDialogButtonBox::Cancel)
//  , ParentWidget(parent)
//  , Plot(plot)
//{
//    ions = parent->GetAllIons();
//    selected_ions = parent->GetSelectedIons();
//    SetupUi();
//}
//
//void ChromatogramWidgetIonsDialog::SetupUi()
//{
//    Body = new QWidget(Content);
//    Content->layout()->addWidget(Body);
//
//    BodyLayout = new QVBoxLayout(Body);
//    BodyLayout->setContentsMargins(Style::Scale(40), Style::Scale(32), Style::Scale(40), 0);
//    BodyLayout->setSpacing(0);
//
//    {
//      QLabel* caption = new QLabel(tr("Ions"), Body);
//      BodyLayout->addWidget(caption);
//
//      caption->setAlignment(Qt::AlignCenter);
//      caption->setStyleSheet(Style::Genesis::GetH2());
//    }
//
//    BodyLayout->addSpacing(Style::Scale(26));
//    auto form = new QWidget;
//    form->setStyleSheet(Style::Genesis::GetUiStyle());
//    BodyLayout->addWidget(form);
//
//    QString style =
//          "\nQWidget#scrollAreaWgt\n"
//          "{\n"
//          "  background-color: white;\n"
//          "}\n"
//          "\nQPushButton\n"
//          "{\n"
//          "  background-color: white\n"
//          "}\n";
//
//    model = new StandardItemModel(this);
//    view = new QTableView;
//    model->setHorizontalHeaderLabels(QStringList() << tr("Select") << tr("Scale"));
//    ComboBoxItemDelegate *cd_column = new ComboBoxItemDelegate(model);
//    view->setItemDelegateForColumn(1, cd_column);
//    int row = 0;
//    //std::sort(ions.begin(),ions.end());
//
//    QStringList scales;
//    scales << "x1" << "x10" << "x100" << "x1000";
//
//    foreach (int mass, ions)
//    {
//        StandardItem *item = new StandardItem("");
//        StandardItem *item_2 = new StandardItem("x1");
//        item->setCheckable(true);
//
//        if(selected_ions.contains(mass))
//        {
//            item->setCheckState(Qt::Checked);
//            item_2->setData(scales.at(selected_ions[mass]),Qt::DisplayRole);
//            item_2->setData(scales.at(selected_ions[mass]), Qt::EditRole);
//            item_2->setData(selected_ions[mass], Qt::EditRole+3);
//        }
//
//        model->appendRow(item);
//        model->setItem(row,1, item_2);
//        model->setHeaderData(row++, Qt::Vertical, QString::number(mass));
//
//    }
//    view->setModel(model);
//    view->setStyleSheet(style);
//
//    view->setColumnWidth(0,90);
//    view->setColumnWidth(1,90);
//
//    BodyLayout->addWidget(view, 1);
//}
//
//
//void ChromatogramWidgetIonsDialog::Accept()
//{
//    QMap <int,int> selected_mass;
//    for(auto i = 0; i < model->rowCount(); ++i)
//    {
//        StandardItem *item = model->item(i,0);
//        if(item->checkState()==Qt::Checked)
//        {
//            int mass = model->headerData(i,Qt::Vertical).toInt();
//            selected_mass[mass] = model->item(i,1)->data(Qt::EditRole+3).toInt();
//        }
//    }
//
//    ParentWidget->SelectIons(selected_mass);
//
//    //// Done
//    WebDialog::Accept();
//}
//
//void ChromatogramWidgetIonsDialog::Reject()
//{
//    WebDialog::Reject();
//}
//
