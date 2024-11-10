#include "tree_model_dynamic_library_templates.h"

#include "logic/tree_model_item.h"
#include "api/api_rest.h"
#include "logic/notification.h"

TreeModelDynamicLibraryTemplates::TreeModelDynamicLibraryTemplates(QObject *parent)
    : TreeModelDynamicLibraryCompounds(ElementType::PLOT_TEMPLATE, parent)
    , m_tableId(-1)
{

}

void TreeModelDynamicLibraryTemplates::uploadTemplatesForTable(int tableId)
{
  if (m_tableId != tableId)
  {
    m_tableId = tableId;
    Reset();
  }
}

void TreeModelDynamicLibraryTemplates::FetchMore(QPointer<TreeModelItem> item)
{
  if (m_tableId <= 0)
  {
    TreeModelDynamicLibraryCompounds::FetchMore(item);
  }
  else
  {
    if (item->CanFetchMore())
    {
      item->SetFetched(true);

      API::REST::Tables::GetTemplatesByTableId(m_tableId,
          [&, item](QNetworkReply*, QVariantMap result)
          {
            if (!item)
              return;
            beginResetModel();
            item->Load(result, false);

            postProcessFetch(item);
            endResetModel();
          },
          [item](QNetworkReply*, QNetworkReply::NetworkError err)
          {
            if (!item)
              return;

            item->Clear();
            Notification::NotifyError(tr("Failed to get templates from database"), err);
          }
          );
    }
  }
}

