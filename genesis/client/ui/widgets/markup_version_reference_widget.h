#pragma once
#ifndef MARKUP_VERSION_REFERENCE_WIDGET_H
#define MARKUP_VERSION_REFERENCE_WIDGET_H

#include <QWidget>
#include <QTableView>
#include <QLabel>
#include <QPointer>

class TreeModelDynamicProjectMarkupEtalonList;
class TreeModelItem;

class CommonTableView;

namespace Widgets
{
  class MarkupVersionReferenceWidget : public QWidget
  {
    Q_OBJECT
  public:
    explicit MarkupVersionReferenceWidget(QWidget* parent = nullptr);

  protected:
    void DeleteItemFromTable(TreeModelItem* item);
    void EditItemFromTable(TreeModelItem* item);
    void UpdateReferencesCountLabel();

  private:
    CommonTableView* ReferenceTableView = nullptr;
    TreeModelDynamicProjectMarkupEtalonList* ModelEtalonList = nullptr;

    void SetupUi();

    void UpdateTitleAndCommentOnServer(int id, const QString& title,
      const QString& comment);

    void DeleteReference(int id);


    QPointer<QLabel> ReferencesCountLabel;
  };
}
#endif
