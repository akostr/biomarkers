#ifndef READONLYSIMPLETREEMODEL_H
#define READONLYSIMPLETREEMODEL_H
#include <QStandardItemModel>

class ReadOnlySimpleTreeItem: public QStandardItem
{
//  Q_OBJECT
public:
  ReadOnlySimpleTreeItem(const QString &itemString, int id);


};

#endif // READONLYSIMPLETREEMODEL_H
