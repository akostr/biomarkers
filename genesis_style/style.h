#pragma once

#include "constants.h"

#include <QString>
#include <QColor>

// #define DISABLE_STYLES

namespace Style
{
  //// Scale padding or spacing value
  int Scale(double value);

  //// Scale font
  int ScaleFont(double value);

  //// Formatted hyperlynk
  QString GetHyperlink(const QString& text, const QString& url = "#");

  //// Formatted input alert
  QString GetInputAlert();

  //// Apply SASS
  QString ApplySASS(const QString& stylesheet);

  //// Get SASS value
  QString GetSASSValue(const QString& key);

  //// Get SASS color
  QColor GetSASSColor(const QString& key);

  //// Get style sheet
  QString GetStyleSheet(const QString& file);

  //// Genesis
  namespace Genesis
  {
    //// Get general ui style
    QString GetUiStyle();

    //// Get h1
    QString GetH1();

    //// Get dialog caption header font style
    QString GetH1_5();

    //// Get h2
    QString GetH2();
    
    //// Get h3
    QString GetH3();

    //// Get h4
    QString GetH4();

    //// Get vivid label
    QString GetVividLabel();

    //// Get page navigation style
    QString GetPageNavigationStyle();

    //// Get toolbar style
    QString GetToolbarStyle();

    //// Get TreeView style
    QString GetTreeViewStyle();

    //// Get TableView style
    QString GetTableViewStyle();

    //// Get switcher tab bar style
    QString GetSwitcherTabBarStyle();

    QString GetMainMenuHeaderStyle();

    namespace Fonts
    {
      QString H1();
      QString H2();
      QString H3();
      QString ModalHead();
      QString ModalHeadSteps();
      QString ModalHeadConfirmation();
      QString RegularBold();
      QString RegularText();
      QString RegularLight();
      QString RegularTextSelectList();
      QString SecondaryTextSelectList();
      QString SecondaryText();
      QString SecondaryBold();
      QString TabsControls();
      QString SmallText();
      QString AxisTicks();
      QString InterFont();

    }
  }

  //// Legacy
  namespace Legacy
  {
    //// Get Application StyleSheet
    QString GetApplicationStyleSheet();

    //// Get Custom Window Title StyleSheet
    QString GetCustomWindowTitleStyle();

    //// Get Custom Window Title Mainwindow StyleSheet
    QString GetCustomWindowTitleMainwindowStyle();

    //// Get Custom Window Title Menu StyleSheet
    QString GetCustomWindowTitleMenuStyle();

    //// Get Custom Window Menu Container Style
    QString GetCustomWindowMenuContainerStyle();

    //// Get Main toolbar stylesheet
    QString GetMainToolbarStyle();

    //// Get Collapsed Widgets View StyleSheet
    QString GetCollapsedWidgetsViewStyle();

    //// Get Splitter Style
    QString GetSplitterStyle(); 

    //// Get Status Bar Style
    QString GetStatusBarStyle(); 

    //// Get Brand Tabbar Style
    QString GetBrandTabbarStyle();

    //// Get Brand Large Tabbar Sytyle
    QString GetBrandLargeTabbarStyle();

    //// Get Tabbar Style
    QString GetTabbarStyle();

    //// Get Tabbar Style
    QString GetTabbarLargeStyle();

    //// Get Tabbar Vivid Style
    QString GetTabbarVividStyle();

    //// Get Brand Toolbar style
    QString GetBrandToolbarStyle();

    //// Get Toolbar style
    QString GetToolbarStyle();

    //// Get Toolbar Transparent style
    QString GetToolbarTransparentStyle();

    //// Get Navigation Toolbar Style
    QString GetNavigationToolbarStyle();

    /// Get TreeView Style
    QString GetTreeViewStyle();

    //// Get Project TreeView Style
    QString GetProjectTreeViewStyle();

    //// Get Page Navigation Brand Style
    QString GetPageNavigationBrandStyle();

    //// Get Page Navigation Style
    QString GetPageNavigationStyle();

    //// Get Page Navigation Section Label Style
    QString GetPageNavigationSectionLabelStyle();

    //// Get Page Navigation Section Label No Padding Style
    QString GetPageNavigationSectionLabelNopaddingStyle();

    //// Get Page Navigation Radiobutton Delegate Style
    QString GetPageNavigationRadiobuttonDelegateStyle();

    //// Get Page Navigation Radiobutton Delegate Brand Style
    QString GetPageNavigationRadiobuttonDelegateBrandStyle();

    /// Get Project Warning Label Style
    QString GetProjectWarningLabelStyle();

    //// Get Properties Style
    QString GetPropertiesStyle();

    //// Get Properties Brand Style
    QString GetPropertiesBrandStyle();

    //// Get Dock style
    QString GetDockStyle();

    //// Get general page style
    QString GetPageStyle();

    //// Get page caption style
    QString GetPageCaptionStyle();

    //// Get page section style
    QString GetPageSectionStyle();

    //// Get page caption brand style
    QString GetPageCaptionBrandStyle();

    //// Get general ui style
    QString GetUiStyle();

    //// Get ui specific widgets style
    QString GetUiQPushButtonStyle();
    QString GetUiQPushButtonPlainStyle();
    QString GetUiQAbstractSpinBoxLabelStyle();
    QString GetUiQTabWidgetStyle();
    QString GetUiQSliderStyle();

    //// Get dialog button style
    QString GetDialogButtonStyle();

    //// Get Home Page Style
    QString GetHomePageStyle();

    //// Get Input Error Warning Frame Style
    QString GetInputErrorWarningFrameStyle();

    //// Get progress style
    QString GetProgressStyle();

    //// Get Loading Static Style
    QString GetLoadingStaticStyle();

    //// Get White QWidget Style
    QString GetWhiteQWidgetStyle();

    //////////////////////////////////////////////////////////////////////////////////////
    //// Get Input Error Warning Icon Path
    QString GetInputErrorWarningIconPath();
  }
}
