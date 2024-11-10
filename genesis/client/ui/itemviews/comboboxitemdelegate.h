#ifndef COMBOBOXITEMDELEGATE_H
#define COMBOBOXITEMDELEGATE_H

#include <QObject>
#include <QWidget>
#include <QDebug>

#include <QComboBox>
#include <QStyledItemDelegate>
#include <standard_models/standard_item_model.h>
#include <QStandardItemModel>


class ComboBoxItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:

    explicit ComboBoxItemDelegate(StandardItemModel *model = nullptr, QWidget *parent = nullptr);


    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
    void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const;

//private slots:
//    void commit();

private:
    QStringList m_values;
    QAbstractItemModel *parent_model = nullptr;
    int m_min_width = 20;
};

#endif // COMBOBOXITEMDELEGATE_H
