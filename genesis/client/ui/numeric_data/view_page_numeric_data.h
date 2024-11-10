#ifndef VIEWPAGENUMERICDATA_H
#define VIEWPAGENUMERICDATA_H

#include <QObject>
#include <QPointer>
#include <ui/view.h>

class QVBoxLayout;
class QFrame;
class QTabWidget;
class TreeWidgetSelectNumericDataTable;
class TreeWidgetHeightRatioTable;
class TreeWidgetImportedTables;
class TreeWidgetJoinedTables;
class TreeWidgetCoefficientTables;
class TreeWidgetIdentifiedTables;
class TreeWidgetImportedIdentificationTables;
class TreeWidgetMergedTables;

class ViewPageNumericData: public Views::View
{
  Q_OBJECT
  public:
    explicit ViewPageNumericData(QWidget *parent= nullptr);
    void ApplyContextProject(const QString& dataId, const QVariant& data) override;
    void ApplyContextModules(const QString& dataId, const QVariant& data) override;

  protected:
    //// Ui
    QPointer<QVBoxLayout>       Layout;
    QPointer<QFrame>            Content;

    QPointer<QVBoxLayout>       mContentLayout;
    QPointer<QTabWidget>        mTabWidget;
    QPointer<TreeWidgetSelectNumericDataTable> mNumericTreeWidget;
    QPointer<TreeWidgetHeightRatioTable>       mRatioTreeWidget;
    QPointer<TreeWidgetImportedTables>         mImportedTablesWidget;
    QPointer<TreeWidgetJoinedTables>           mJoinedTablesWidget;
    QPointer<TreeWidgetCoefficientTables>      mCoefficientTablesWidget;
    QPointer<TreeWidgetImportedIdentificationTables>      mImportedIdentificationWidget;
    QPointer<TreeWidgetIdentifiedTables>       mIdentifiedTablesWidget;
    QPointer<TreeWidgetMergedTables>           mMergedTablesWidget;
    int mProjectId;

};

#endif // VIEWPAGENUMERICDATA_H
