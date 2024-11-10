#ifndef COUNTSPLOTMANUALHOTTELING_H
#define COUNTSPLOTMANUALHOTTELING_H

#include <QObject>
#include "counts_plot.h"
class QWidget;

struct ManualGroup
{
    QString nameGroup;
    GPShapeItem::DefaultColor colorGroup;
    QList<GPShapeWithLabelItem*> groupItems;
};

class CountsPlotManualHotteling : public CountsPlot
{
    Q_OBJECT

public:
    explicit CountsPlotManualHotteling(QWidget *parent = nullptr);

    void FillContextMenu(QContextMenuEvent* event, QMenu& menu, std::map<QAction*, std::function<void(void)>>& actions) override;

    void clearData() override;
    void clearGroups();
    void SetGroupContext(bool isGroup);

    std::map<int, ManualGroup> GetGroups();

protected:
    int findItemGroup(GPShapeWithLabelItem* findItem);
    int createGroup(const QString &nameGroup = QString());
    void addToGroup(int groupId, GPShapeWithLabelItem* item);
    void deleteFromGroup(GPShapeWithLabelItem* item);

    void greyAll();

    GPShapeItem::DefaultColor RandomColor() const;

protected:
    std::map<int, ManualGroup> Groups;
    bool IsGroupContext = false;
};

#endif // COUNTSPLOTMANUALHOTTELING_H
