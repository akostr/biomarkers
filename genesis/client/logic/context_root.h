#pragma once

//#include "context.h"
#include "logic/genesis_context_step_based_markup.h"

#include <QJsonObject>
namespace Views
{
class View;
}

///////////////////////////////////////////////////////////////////
//// Genesis context root
namespace Core
{
  class GenesisContextRoot : public GenesisContext
  {
    Q_OBJECT

  public:
    //// Root app context
    explicit GenesisContextRoot(GenesisContext* parent);
    ~GenesisContextRoot();

    //// Instance
    static QPointer<GenesisContextRoot> Get();
    static int ProjectId();
  // protected:
  //   //// Get child context that describes current ui state

  public:
    GenesisContextPtr GetContextUi();
    //// Get child context that describes current user
    GenesisContextPtr GetContextUser();
    GenesisContextPtr GetContextModules();

    //// Get child context that describes current project
    GenesisContextPtr GetContextProject();

    //// Get child context that describes current file
    GenesisContextPtr GetContextFileCDF();

    //// Get child context that describes current project markup
    GenesisContextPtr GetContextMarkup();

    //// Get child context that describes current project markup version
    GenesisContextPtr GetContextMarkupVersion();

    //// Get child context that describes result of step-based markup
    GenesisContextStepBasedMarkupPtr GetContextStepBasedMarkup();

    //// Get child context that describes ratio matrix for markup
    GenesisContextPtr GetContextHeightRatioMatrix();

    //// Get child context that describes current project markup version analysis
    GenesisContextPtr GetContextMarkupVersionAnalisysPCA();
    GenesisContextPtr GetContextMarkupVersionAnalisysMCR();
    GenesisContextPtr GetContextMarkupVersionAnalisysPLS();
    GenesisContextPtr GetContextMarkupVersionAnalisysPLSPredict();

    //// Get child context that describes PCA counts and loads plots data
    GenesisContextPtr GetContextPCAPlots();

    //// Get child library context
    GenesisContextPtr GetContextLibrary();

    //// Get creation plot context
    GenesisContextPtr GetCreationPlot();

    //// Save settings
    void SaveSettings();

    //// Load setiings
    void LoadSettings();

  private:
    //// Setup initial structure
    void Setup();
    // friend class Views::View;
  };
  using GenesisContextRootPtr = QPointer<GenesisContextRoot>;
}
