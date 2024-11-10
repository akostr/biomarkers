#include "../logic/context_root.h"

#include "view_factory_impls.h"
#include "view_factory_registry.h"

//// View impls
#include "view_login.h"
#include "view_login_create.h"
#include "view_root.h"
#include "view_root_layout_pages.h"
#include "view_root_layout_pages_menu.h"
#include "view_page_module_selection.h"
#include "view_root_layout_welcome.h"
#include "view_root_layout_welcome_banners.h"
#include "view_root_toolbar.h"
#include "view_root_toolbar_account_menu.h"

#include "view_page_project_children.h"
#include "view_page_project_data.h"
#include "view_page_project_imported_files.h"
#include "view_page_project_markups_list.h"
#include "view_page_project_models_list.h"
#include "view_page_project_pca_graphics_plot_page.h"
#include "view_page_project_info.h"
#include "view_page_project_log.h"
#include "view_page_projects.h"
#include "view_page_project_pls_graphics_plot_page.h"
#include "view_page_chromatogramms_fragments.h"
#include "view_page_library.h"
#include "view_page_statistic.h"

#include "known_view_names.h"
#include "view_page_identification.h"
#include "view_page_identification_plot_list.h"
#include "view_page_interpretation.h"
#include "view_page_login_edit.h"
#include "view_page_pls_model_page.h"
#include "view_page_project_pls_predict.h"
#include "view_page_project_mcr_graphics_plot_page.h"
#include "view_page_project_markup_version_page.h"
#include "view_page_mcr_model_page.h"
#include "ui/view_page_project_markup.h"
#include "view_page_analysis_data_table_page.h"
#include "view_page_analysis_data_table_page.h"
#include "view_page_profile.h"
#include "view_page_pca_model.h"
#include "view_page_create_identification_plot.h"
#include "ui/numeric_data/view_page_numeric_data.h"

using namespace Core;
using namespace Views;

namespace Details
{
  template<class V>
  class Factory : public ViewFactory
  {
  public:
    Factory(QObject* parent)
      : ViewFactory(parent)
    {
    }

    virtual View* CreateView(QWidget* parent) override
    {
      if (!ViewImpl)
      {
        ViewImpl = new V(parent);
        ViewImpl->SetContext(GenesisContextRoot::Get().get());
      }
      return ViewImpl;
    }

    virtual View* GetView() override
    {
      return ViewImpl;
    }

  private:
    QPointer<V> ViewImpl;
  };
}

namespace ViewFactoryImpls
{
  using namespace ViewPageNames;

  void InitializeViewFactories()
  {
    if (auto registry = ViewFactoryRegistry::Get())
    {
      std::map<QString, std::function<ViewFactory*()>> mapping
      {
        //// Login
        { ViewLoginPageName,                      [registry]() -> ViewFactory* { return new Details::Factory<ViewLogin>(registry); } },
        { ViewLoginCreatePageName,                [registry]() -> ViewFactory* { return new Details::Factory<ViewLoginCreate>(registry); } },

        //// Roots
        { ViewRootPageName,                       [registry]() -> ViewFactory* { return new Details::Factory<ViewRoot>(registry); } },
        { ViewRootLayoutPagesName,                [registry]() -> ViewFactory* { return new Details::Factory<ViewRootLayoutPages>(registry); } },
        { ViewRootLayoutPagesMenuPageName,        [registry]() -> ViewFactory* { return new Details::Factory<ViewRootLayoutPagesMenu>(registry); } },
        { ViewPageModuleSelection,                [registry]() -> ViewFactory* { return new Details::Factory<ViewPageModulesSelection>(registry); } },
        { ViewRootLayoutWelcomePageName,          [registry]() -> ViewFactory* { return new Details::Factory<ViewRootLayoutWelcome>(registry); } },
        { ViewRootLayoutWelcomeBannersPageName,   [registry]() -> ViewFactory* { return new Details::Factory<ViewRootLayoutWelcomeBanners>(registry); } },
        { ViewRootToolbarPageName,                [registry]() -> ViewFactory* { return new Details::Factory<ViewRootToolbar>(registry); } },
        { ViewRootToolbarAccountMenuPageName,     [registry]() -> ViewFactory* { return new Details::Factory<ViewRootToolbarAccountMenu>(registry); } },
        //// Pages
        { ViewProjectChildrenPageName,            [registry]() -> ViewFactory* { return new Details::Factory<ViewPageProjectChildren>(registry); } },
        { ViewProjectDataPageName,                [registry]() -> ViewFactory* { return new Details::Factory<ViewPageProjectData>(registry); } },
        { ViewProjectImportedFilesPageName,       [registry]() -> ViewFactory* { return new Details::Factory<ViewPageProjectImportedFiles>(registry); } },
        { ViewChromatogrammsFragmentsPageName,    [registry]() -> ViewFactory* { return new Details::Factory<ViewPageChromatogrammsFragments>(registry); } },
        { ViewLibrary,                            [registry]() -> ViewFactory* { return new Details::Factory<ViewPageLibrary>(registry); } },
        { ViewProjectMarkupPageName,              [registry]() -> ViewFactory* { return new Details::Factory<GenesisMarkup::ViewPageProjectMarkup>(registry); } },
        { ViewProjectMarkupsListPageName,         [registry]() -> ViewFactory* { return new Details::Factory<ViewPageProjectMarkupsList>(registry); } },
        { ViewProjectModelsListPageName,          [registry]() -> ViewFactory* { return new Details::Factory<ViewPageProjectModelsList>(registry); } },
        { ViewPageProjectPCAGraphicsPlotPageName, [registry]() -> ViewFactory* { return new Details::Factory<ViewPageProjectPcaGraphicsPlotPage>(registry); } },
        { ViewPagePcaModelName,                   [registry]() -> ViewFactory* { return new Details::Factory<ViewPagePcaModel>(registry); } },
        { ViewPagePlsModelPageName,               [registry]() -> ViewFactory* { return new Details::Factory<ViewPagePlsModelPage>(registry); } },
        { ViewPageMcrModelPageName,               [registry]() -> ViewFactory* { return new Details::Factory<ViewPageMcrModelPage>(registry); } },
        { ViewPageProjectMcrGraphicPlotPageName,  [registry]() -> ViewFactory* { return new Details::Factory<ViewPageProjectMCRGraphicsPlotPage>(registry); } },
        { ViewProjectInfoPageName,                [registry]() -> ViewFactory* { return new Details::Factory<ViewPageProjectInfo>(registry); } },
        { ViewProjectLogPageName,                 [registry]() -> ViewFactory* { return new Details::Factory<ViewPageProjectLog>(registry); } },
        { ViewNumericData,                        [registry]() -> ViewFactory* { return new Details::Factory<ViewPageNumericData>(registry); } },

        { ViewPageAnalysisDataTablePageName,          [registry]() -> ViewFactory* { return new Details::Factory<ViewPageAnalysisDataTablePage>(registry); }},

        { ViewProjectsPageName,                   [registry]() -> ViewFactory* { return new Details::Factory<ViewPageProjects>(registry); } },
        { ViewIdentificationPageName,             [registry]() -> ViewFactory* { return new Details::Factory<ViewPageIdentification>(registry); } },
        { ViewInterpretationPageName,             [registry]() -> ViewFactory* { return new Details::Factory<ViewPageInterpretation>(registry); } },
        { ViewLoginEditPageName,                  [registry]() -> ViewFactory* { return new Details::Factory<ViewPageLoginEdit>(registry); } },
        { ViewPageProjectPLSGraphicsPlotPageName, [registry]() -> ViewFactory* { return new Details::Factory<ViewPageProjectPlsGraphicsPlotPage>(registry); } },
        { ViewPageProjectPlsPredictName,          [registry]() -> ViewFactory* { return new Details::Factory<ViewPageProjectPlsPredict>(registry); } },
        { ViewPageProjectMarkupVersionPageName,   [registry]() -> ViewFactory* { return new Details::Factory<ViewPageProjectMarkupVersionPage>(registry); } },
        { ViewPageProfileName,                    [registry]() -> ViewFactory* { return new Details::Factory<ViewPageProfile>(registry); } },
        { ViewPageStatisticName,                    [registry]() -> ViewFactory* { return new Details::Factory<ViewPageStatistic>(registry); } },
        { ViewPageCreateIdentificationPlotName,                 [registry]() -> ViewFactory* { return new Details::Factory<ViewPageCreateIdentificationPlot>(registry); } },
        { ViewPageIdentificationPlotListName,     [registry]() -> ViewFactory* { return new Details::Factory<ViewPageIdentificationPlotList>(registry); } }
      };

      for (auto& [key, factory] : mapping)
      {
        registry->RegisterFactory(key, factory());
      }
    }
  }
}
