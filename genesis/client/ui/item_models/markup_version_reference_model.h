#pragma once
#ifndef MARKUP_VERSION_REFERENCE_MODEL_H
#define MARKUP_VERSION_REFERENCE_MODEL_H

#include <QAbstractTableModel>
#include "logic/structures/common_structures.h"

namespace Models
{
  class MarkupVersionReferenceModel : public QAbstractTableModel
  {
    Q_OBJECT

    using ReferenceList = Structures::ReferenceList;
    using Reference = Structures::Reference;
  public:
    MarkupVersionReferenceModel(QObject* parent);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    void SetReferenceList(const ReferenceList& list);

    Qt::ItemFlags flags(const QModelIndex& index) const override;
    void setIconPath(const QString& icon_path);

    Reference at(int index) const;

  private:
    std::vector<QString> HorizontalHeaders;
    ReferenceList Data;
    QString IconPath;
  };
}
#endif
