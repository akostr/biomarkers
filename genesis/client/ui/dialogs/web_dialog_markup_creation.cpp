#include "web_dialog_markup_creation.h"
#include "qapplication.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QScrollArea>

#include <genesis_style/style.h>
#include <logic/notification.h>
#include <logic/context_root.h>
#include <logic/known_context_tag_names.h>
#include <ui/flow_layout.h>
#include <ui/dialogs/web_overlay.h>
#include <api/api_rest.h>

namespace Dialogs
{
WebDialogMarkupCreation::WebDialogMarkupCreation(QList<int> filesIds, QWidget* parent)
  : Templates::SaveEdit(parent)
  , FilesIds(filesIds)
{
  auto s = Templates::SaveEdit::Settings();
  s.header = tr("Markup creation");
  s.subHeader = tr("%n chromatogramm(s)", "", FilesIds.size());
  s.titleHeader = tr("Markup title");
  s.commentHeader = tr("Comment");
  s.titlePlaceholder = tr("Enter markup title");
  s.commentPlaceholder = tr("Enter comment");
  s.titleMaxLen = 50;
  s.commentMaxLen = 70;
  s.okBtnText = tr("Create markup");
  s.cancelBtnText = tr("Cancel");
  s.defaultTitle = "";
  s.defaultComment = "";
  s.titleTooltipsSet.empty = tr("Markup title can't be empty");
  s.titleTooltipsSet.forbidden = tr("This markup name alredy occupied");
  s.titleTooltipsSet.notChecked = tr("Not checked");
  s.titleTooltipsSet.valid = tr("Valid markup name");
  s.titleRequired = true;
  s.commentRequired = false;
  applySettings(s);
  auto overlay = new WebOverlay(tr("Loading"), this);
  // qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
  loadMarkupsList(overlay);
}

WebDialogMarkupCreation::~WebDialogMarkupCreation()
{
}

QString WebDialogMarkupCreation::GetMarkupName()
{
  return getTitle();
}

QString WebDialogMarkupCreation::GetComment()
{
  return getComment();
}

QList<int> WebDialogMarkupCreation::GetFilesIds()
{
  return FilesIds;
}

void WebDialogMarkupCreation::loadMarkupsList(WebOverlay *overlay)
{
  auto projectId = Core::GenesisContextRoot::Get()->ProjectId();
  Q_ASSERT(projectId > 0);
  API::REST::Tables::GetProjectMarkups(projectId,
      [this, overlay](QNetworkReply*, QVariantMap data)
      {
        auto children = data["children"].toList();
        QStringList forbiddenTitles;
        for(auto& ch : children)
        {
          auto item = ch.toMap();
          forbiddenTitles << item["title"].toString();
        }
        auto s = settings();
        s.forbiddenTitles = forbiddenTitles;
        applySettings(s);
        delete overlay;
      },
      [this, overlay](QNetworkReply*, QNetworkReply::NetworkError e)
      {
        Notification::NotifyError(tr("Network error"), e);
        delete overlay;
      });
}

}//namespace Dialogs
