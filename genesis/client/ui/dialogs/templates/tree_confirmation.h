#ifndef TREECONFIRMATION_H
#define TREECONFIRMATION_H

#include <ui/dialogs/templates/confirm.h>
#include <logic/models/tree_confirmation_model.h>

namespace Ui {
  class TreeConfirmation;
}

namespace Dialogs
{
  namespace Templates
  {
    class TreeItem
    {
    public:
      QList<TreeItem*> children;
      QVariant displayRoleData;
      QVariant userRoleData;
      //root item will not displayed
    };

    class TreeConfirmation : public Confirm
    {
      Q_OBJECT
    public:

      struct Settings
      {
        Settings() {};
        Confirm::Settings dialogSettings = { tr("File removing"), {}, {} };
        TreeItem* rootTreeItem = nullptr;
        QString phrase = tr("Are you shure that you want to remove file and it's dependencies?");
        std::function<QString(int)> updateCounterLabelFunc;
      };
      TreeConfirmation(QWidget* parent = nullptr);
      ~TreeConfirmation();

      void applySettings(Settings s);
      Settings settings();
      void setupUi();
      QList<QVariant> getConfirmedUserDataList();

      //return alredy opened dialog with all proper api call on acception
      static TreeConfirmation* DeleteMarkupFilesWithDependencies(const QList<QPair<int, QString> >& projectFilesInfos, QWidget* parent);
      static TreeConfirmation* DeleteProjectWithDependencies(const QList<QPair<int, QString>>& projectInfos, QWidget* parent);
      static TreeConfirmation* DeleteTablesWithDependencies(const QList<QPair<int, QString> >& tablesInfos, QWidget* parent, QString tableKey = QString());
      static TreeConfirmation* DeleteCompoundsWithDependencies(const QList<QPair<int, QString> >& compoundInfo, QWidget* parent);
      static TreeConfirmation* DeleteCoefficientWithDependencies(const QList<QPair<int, QString> >& compoundInfo, QWidget* parent);
      static TreeConfirmation* DeletePlotTemplateWithDependencies(const QList<QPair<int, QString> >& compoundInfo, QWidget* parent);

    private:
      Ui::TreeConfirmation* ui;
      Settings mSettings;
      TreeConfirmationModel mTreeModel;
      void clearTreeItems();
      void updateCounterLabel(int newCount);
    };
  }
}

#endif // TREECONFIRMATION_H
