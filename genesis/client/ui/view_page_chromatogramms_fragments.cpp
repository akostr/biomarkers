#include "view_page_chromatogramms_fragments.h"

#include <api/api_rest.h>
#include <genesis_style/style.h>
#include <logic/context_root.h>
#include <logic/notification.h>
#include <logic/enums.h>
#include <logic/tree_model_dynamic_file_types.h>
#include <logic/known_context_tag_names.h>
#include <ui/chromatograms_fragments/chromatogramms_table_widget.h>
#include <ui/chromatograms_fragments/fragments_table_widget.h>
#include <ui/dialogs/web_dialog_import_files.h>
#include <ui/dialogs/web_dialog_import_file_type.h>

#include "logic/known_context_tag_names.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTableView>
#include <QTabWidget>
#include <QMenu>

using namespace Names;

namespace Views
{
ViewPageChromatogrammsFragments::ViewPageChromatogrammsFragments(QWidget *parent)
  : View(parent)
{
  setupUi();
}

void ViewPageChromatogrammsFragments::ApplyContextProject(const QString& dataId, const QVariant& data)
{
  bool isReset = isDataReset(dataId, data);
  if(!isReset && dataId != Names::ContextTagNames::ProjectId)
    return;
  if(isReset)
    m_projectId = Core::GenesisContextRoot::Get()->ProjectId();
  else
    m_projectId = data.toInt();

  m_chromatogrammsWidget->setScrollPosition(0);
  m_fragmentsWidget->setScrollPosition(0);
}

void ViewPageChromatogrammsFragments::ApplyContextModules(const QString &dataId, const QVariant &data)
{
  if(m_chromatogrammsWidget)
    m_chromatogrammsWidget->ExitAction();
  if(m_fragmentsWidget)
    m_fragmentsWidget->ExitAction();
}

void ViewPageChromatogrammsFragments::setupUi()
{
  setStyleSheet(Style::Genesis::GetUiStyle());

  auto layout = new QVBoxLayout(this);
  setLayout(layout);
  layout->setContentsMargins(Style::Scale(40), Style::Scale(32), Style::Scale(40), Style::Scale(32));
  layout->setSpacing(10);

  auto* headerLayout = new QHBoxLayout;
  headerLayout->setContentsMargins(0, 0, 0, 0);
  headerLayout->setSpacing(Style::Scale(40));
  layout->addLayout(headerLayout);

  auto caption = new QLabel(tr("Chromatogramms and fragments"), this);
  caption->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  caption->setStyleSheet(Style::Genesis::GetH1());
  headerLayout->addWidget(caption);

  auto captionToolBar = new QWidget(this);
  auto captionToolBarLayout = new QHBoxLayout(captionToolBar);
  captionToolBarLayout->setContentsMargins(0, 0, 0, 0);
  captionToolBarLayout->setSpacing(Style::Scale(16));
  captionToolBarLayout->addStretch(1);

  m_importFilesButton = new QPushButton(tr("Import files"));
  m_importFilesButton->setIcon(QIcon(":/resource/icons/icon_action_import.png"));
  m_importFilesButton->setProperty("secondary", true);
  captionToolBarLayout->addWidget(m_importFilesButton);

  connect(m_importFilesButton, &QPushButton::clicked, this, &ViewPageChromatogrammsFragments::importFiles);

  headerLayout->addWidget(captionToolBar);
  auto tabContainer = new QWidget(this);
  tabContainer->setProperty("style", "white_base");
  layout->addWidget(tabContainer);
  auto containerLayout = new QVBoxLayout();
  tabContainer->setLayout(containerLayout);
  containerLayout->setContentsMargins(26,26,26,26);

  auto tabView = new QTabWidget(tabContainer);
  containerLayout->addWidget(tabView);
//  layout->addWidget(tabView);

  {//// Chromatogramms tab
    m_chromatogrammsWidget = new ChromatogrammsTableWidget(tabView);
    m_chromatogrammsWidget->setStyleSheet("ChromatogrammsTableWidget {background: white;}");
    tabView->addTab(m_chromatogrammsWidget, tr("Chromatogramms"));
  }

  {//// Fragments tab
    m_fragmentsWidget = new FragmentsTableWidget(tabView);
    m_chromatogrammsWidget->setStyleSheet("FragmentsTableWidget {background: white;}");
    tabView->addTab(m_fragmentsWidget, tr("Fragments"));
  }
}

void ViewPageChromatogrammsFragments::importFiles()
{
  using namespace Dialogs;
  auto typeDial = new WebDialogImportFileType(this);
  connect(typeDial, &WebDialog::Accepted, this, [this, typeDial]()
  {
    auto fileType = typeDial->fileType();
    auto dial = new WebDialogImportFiles(fileType, m_projectId, this);
    dial->Open();
  });
  typeDial->Open();
}
}//namespace Views
