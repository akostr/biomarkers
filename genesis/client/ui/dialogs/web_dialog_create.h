#pragma once

#include "web_dialog.h"
#include <QSet>

class QAbstractItemModel;

class WebDialogCreate : public WebDialog
{
  Q_OBJECT

public:
  explicit WebDialogCreate(QWidget* parent, QSizeF size = QSizeF());
  ~WebDialogCreate();

  void SetModel(QAbstractItemModel* model, int keyColumn);
  void SetNamesList(const QStringList& names);

protected slots:
  virtual void UpdateButtons();

protected:
  virtual bool CanAccept() const;
  virtual QString GetCurrentName() const = 0;
  virtual bool IsUnique(const QString& name) const;

protected:
  QPointer<QAbstractItemModel> Model;
  int KeyColumn;

  QSet<QString> Names;
};
