#include "comboboxitemdelegate.h"

ComboBoxItemDelegate::ComboBoxItemDelegate( StandardItemModel *model, QWidget *parent)
    : QStyledItemDelegate(parent)
    ,parent_model(model)
{
    m_values << "x1" << "x10" << "x100" << "x1000";
}


QWidget *ComboBoxItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);

    QComboBox *cb = new QComboBox(parent);
    cb->addItems(m_values);
    parent_model->setData(index, QString("x1"), Qt::DisplayRole);
    parent_model->setData(index, QString("x1"), Qt::EditRole);
    //cb->setCurrentIndex(0);

    return cb;
}

void ComboBoxItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QComboBox *cb = qobject_cast<QComboBox *>(editor);
    Q_ASSERT(cb);
    const QString currentText = index.data(Qt::EditRole).toString();
    const int cbIndex = cb->findText(currentText);
    if(currentText.isEmpty())
    {
        cb->setCurrentIndex(0);
    }
    else
    {
        if (cbIndex >= 0)
        {
           cb->setCurrentIndex(cbIndex);
        }
        else
        {
            cb->setCurrentIndex(0);
        }
    }
}


void ComboBoxItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QComboBox *cb = qobject_cast<QComboBox *>(editor);
    Q_ASSERT(cb);
    model->setData(index, cb->currentText(), Qt::DisplayRole);
    model->setData(index, cb->currentText(), Qt::EditRole);
    model->setData(index, cb->currentIndex(), Qt::EditRole+3);
}

void ComboBoxItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    editor->setGeometry(option.rect);
}

//void ComboBoxItemDelegate::commit()
//{
//    QComboBox *editor = qobject_cast<QComboBox *>(sender());
//    emit commitData(editor);
//}

