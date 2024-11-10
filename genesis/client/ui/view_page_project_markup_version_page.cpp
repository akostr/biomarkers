#include "view_page_project_markup_version_page.h"

#include "ui/widgets/tab_widget.h"
#include "ui/widgets/markup_version_reference_widget.h"
#include "ui/view_page_project_markups_list.h"
#include "genesis_style/style.h"
#include <ui/known_view_names.h>

#include <QVBoxLayout>
#include <QLabel>

using namespace Widgets;

namespace Views
{
  ViewPageProjectMarkupVersionPage::ViewPageProjectMarkupVersionPage(QWidget* parent)
    : View(parent)
  {
    SetupUi();
  }

  void ViewPageProjectMarkupVersionPage::SetupUi()
  {

    //// Self
    setStyleSheet(Style::Genesis::GetUiStyle());

    //// Layout
    const auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(Style::Scale(40), Style::Scale(32), Style::Scale(40), Style::Scale(32));
    layout->setSpacing(0);
    {
      QLabel* catpion = new QLabel(tr("Markup Versions"), this);
      layout->addWidget(catpion);

      catpion->setStyleSheet(Style::Genesis::GetH1());
    }

    auto tabContainer = new QWidget(this);
    tabContainer->setProperty("style", "white_base");
    layout->addWidget(tabContainer);
    auto containerLayout = new QVBoxLayout();
    tabContainer->setLayout(containerLayout);
    containerLayout->setContentsMargins(26,26,26,26);

    const auto tabWidget = new TabWidget(tabContainer);
    containerLayout->addWidget(tabWidget);

    tabWidget->AddTabWidget(View::Create(ViewPageNames::ViewProjectMarkupsListPageName, this), tr("Markups"));
    tabWidget->AddTabWidget(new MarkupVersionReferenceWidget(this), tr("References"));
  }
}
