#include "style.h"

#include <QFile>
#include <QMap>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>
#include <QDirIterator>

#include <QLibraryInfo>
#include <QVersionNumber>

void Q_INIT_RESOURCE_genesis_style()
{
  Q_INIT_RESOURCE(genesis_style);
}

namespace Style
{
  namespace Details
  {
    QMap<QString, QString>          SASS_Values__;
    QMap<QString, QColor>           SASS_Colors__;
    QList<QPair<QString, QString>>  SASS_Values_Sorted__;
    QMap<QString, QString>          Styles_Cache__;

    void InitializeSASS()
    {
      Q_INIT_RESOURCE_genesis_style();

      // Initialize
      if (Details::SASS_Values__.empty())
      {
        QFile sassFile(":/resource/styles/genesis_sass.json");
        if (sassFile.open(QFile::ReadOnly))
        {
          // Read
          QByteArray data = sassFile.readAll();
          QJsonDocument sassDocument = QJsonDocument::fromJson(data);
          QJsonObject sassRootObject = sassDocument.object();

          // Initialize dynamic values
          QVersionNumber qtVersion = QLibraryInfo::version();
          if (qtVersion <= QVersionNumber(5, 12, 1)
           || qtVersion >= QVersionNumber(5, 14, 2))
          {
            sassRootObject["qt_version_dependent_styled_menu_left_padding_workaround"] = "1px";
          }

          // Store
          QJsonObject::iterator sass = sassRootObject.begin();
          while (sass != sassRootObject.end())
          {
            Details::SASS_Values__[sass.key()] = sass.value().toString();
            ++sass;
          }

          // Apply scales
          for (auto sass = Details::SASS_Values__.begin(); sass != Details::SASS_Values__.end(); ++sass)
          {
            if (sass.key().endsWith("ScalableFont"))
            {
              sass.value() = QString::number(ScaleFont(sass.value().toDouble()));
            }
            else if (sass.key().endsWith("Scalable"))
            {
              sass.value() = QString::number(Scale(sass.value().toDouble()));
            }
          }

          // Sort
          for (auto sass = Details::SASS_Values__.begin(); sass != Details::SASS_Values__.end(); ++sass)
          {
            Details::SASS_Values_Sorted__.append({ sass.key(), sass.value() });
          }
          std::sort(Details::SASS_Values_Sorted__.begin(), Details::SASS_Values_Sorted__.end(), [&](QPair<QString, QString>& lt, QPair<QString, QString>& rt)
          {
            if (lt.first != rt.first)
            {
              if (lt.first.contains(rt.first))
              {
                return true;
              }
              if (rt.first.contains(lt.first))
              {
                return false;
              }
            }
            return lt < rt;
          });

          // Recurse
          for (auto sass = Details::SASS_Values_Sorted__.begin(); sass != Details::SASS_Values_Sorted__.end(); ++sass)
          {
            if (sass->second.startsWith("@"))
            {
              sass->second = ApplySASS(sass->second);
            }
          }
        }

        /////////////////////////////////////////////////////////////////////////
        // Preload files applicable as values
        QDirIterator it(":/resource/styles", { "*.qss" });
        while (it.hasNext())
        {
          QString f = it.next();
          QString k = it.fileName();
          QString v = GetStyleSheet(f);

          Details::SASS_Values__[k] = v;
        }

        // Resort
        Details::SASS_Values_Sorted__.clear();

        // Sort
        for (auto sass = Details::SASS_Values__.begin(); sass != Details::SASS_Values__.end(); ++sass)
        {
          Details::SASS_Values_Sorted__.append({ sass.key(), sass.value() });
        }
        std::sort(Details::SASS_Values_Sorted__.begin(), Details::SASS_Values_Sorted__.end(), [&](QPair<QString, QString>& lt, QPair<QString, QString>& rt)
        {
          if (lt.first != rt.first)
          {
            if (lt.first.contains(rt.first))
            {
              return true;
            }
            if (rt.first.contains(lt.first))
            {
              return false;
            }
          }
          return lt < rt;
        });

        // Apply in unsorted map, too
        for (auto sass = Details::SASS_Values__.begin(); sass != Details::SASS_Values__.end(); ++sass)
        {
          if (sass.value().startsWith("@"))
          {
            sass.value() = ApplySASS(sass.value());
          }
        }

        // Clear cache
        Styles_Cache__.clear();
      }
    }

    struct SASSInitializer
    {
      SASSInitializer()
      {
        InitializeSASS();
      }

    } SASSInitializer_Instance__;
  }

  //// Scale padding or spacing value
  int Scale(double value)
  {
//    return (double)value * 3.0 / 4.0 + 0.5;
    return (int)(value + 0.5);
  }

  //// Scale font
  int ScaleFont(double value)
  {
#if defined(Q_OS_MAC)
    return value + 1;
#endif
    return (int)((double)value + 0.5);
  }

  //// Formatted hyperlynk
  QString GetHyperlink(const QString& text, const QString& url)
  {
    return ApplySASS(QString("<a href=\"%2\"><span style=\"color: @textColorLink; text-decoration: none;\">%1</span></a>"))
      .arg(text)
      .arg(url);
  }

  //// Formatted input alert
  QString GetInputAlert()
  {
    return ApplySASS("<span style=\"color: @textColorInputAlert;\">*</span>");
  }

  //// Apply SASS
  QString ApplySASS(const QString& stylesheet)
  {
    if (!stylesheet.contains("@"))
      return stylesheet;

    QString result = stylesheet;
    for (auto sass = Details::SASS_Values_Sorted__.begin(); sass != Details::SASS_Values_Sorted__.end(); ++sass)
    {
      QString k = QString("@") + sass->first;
      result = result.replace(k, sass->second);
    }
    if (result != stylesheet)
    {
      // Reenter
      return ApplySASS(result);
    }
    return result;
  }

  QString GetSASSValue(const QString& key)
  {
    return Details::SASS_Values__.value(key);
  }

  QColor GetSASSColor(const QString& key)
  {
    auto cached = Details::SASS_Colors__.find(key);
    if (cached != Details::SASS_Colors__.end())
    {
      return cached.value();
    }

    QColor color(GetSASSValue(key));
    Details::SASS_Colors__[key] = color;
    return color;
  }

  //// Get style sheet
  QString GetStyleSheet(const QString& fileName)
  {
#ifdef DISABLE_STYLES
    return QString();
#endif // DISABLE_STYLES

    auto cached = Details::Styles_Cache__.find(fileName);
    if (cached != Details::Styles_Cache__.end())
      return cached.value();

    QFile stylesheetFile(fileName);
    if (stylesheetFile.open(QFile::ReadOnly))
    {
      QString raw = stylesheetFile.readAll();
      QString result = ApplySASS(raw);
      Details::Styles_Cache__[fileName] = result;
      return result;
    }
    return QString();
  }

  //// Genesis
  namespace Genesis
  {
    //// Get general ui style
    QString GetUiStyle()
    {
      return GetStyleSheet(":/resource/styles/genesis_ui.qss");
    }

    //// Get h1
    QString GetH1()
    {
      return Fonts::H1();
    }
    //// Get h1.5
    QString GetH1_5()
    {
      return GetStyleSheet(":/resource/styles/genesis_h1_5.qss");
    }
    
    //// Get h2
    QString GetH2()
    {
      return Fonts::H2();
    }
    
    //// Get h3
    QString GetH3()
    {
      return Fonts::H3();
    }

    //// Get h4
    QString GetH4()
    {
      return GetStyleSheet(":/resource/styles/genesis_h4.qss");
    }

    //// Get vivid label
    QString GetVividLabel()
    {
      return GetStyleSheet(":/resource/styles/genesis_label_vivid.qss");
    }

    //// Get page navigation style
    QString GetPageNavigationStyle()
    {
      return GetStyleSheet(":/resource/styles/genesis_navigation_menu.qss");
    }

    //// Get toolbar style
    QString GetToolbarStyle()
    {
      return GetStyleSheet(":/resource/styles/genesis_toolbar.qss");
    }

    QString GetTreeViewStyle()
    {
      return GetStyleSheet(":/resource/styles/genesis_ui_qtreeview.qss");
    }

    QString GetTableViewStyle()
    {
      return GetStyleSheet(":/resource/styles/genesis_ui_qtableview.qss");
    }

    QString GetSwitcherTabBarStyle()
    {
      return GetStyleSheet(":/resource/styles/genesis_ui_qtabbar_switcher.qss");
    }

    QString GetMainMenuHeaderStyle()
    {
      return GetStyleSheet(":/resource/styles/custom/genesis_ui_main_menu_header.qss");
    }

    namespace Fonts
    {

      QString H1()
      {
        return GetStyleSheet(":/resource/styles/genesis_font_h1.qss");
      }

      QString H2()
      {
        return GetStyleSheet(":/resource/styles/genesis_font_h2.qss");
      }

      QString H3()
      {
        return GetStyleSheet(":/resource/styles/genesis_font_h3.qss");
      }

      QString ModalHead()
      {
        return GetStyleSheet(":/resource/styles/genesis_font_modal_head.qss");
      }

      QString ModalHeadSteps()
      {
        return GetStyleSheet(":/resource/styles/genesis_font_modal_head_steps.qss");
      }

      QString ModalHeadConfirmation()
      {
        return GetStyleSheet(":/resource/styles/genesis_font_modal_head_confirmation.qss");
      }

      QString RegularBold()
      {
        return GetStyleSheet(":/resource/styles/genesis_font_regular_bold.qss");
      }

      QString RegularText()
      {
        return GetStyleSheet(":/resource/styles/genesis_font_regular_text.qss");
      }

      QString RegularLight()
      {
        return GetStyleSheet(":/resource/styles/genesis_font_regular_light.qss");
      }

      QString RegularTextSelectList()
      {
        return GetStyleSheet(":/resource/styles/genesis_font_regular_text_selectlist.qss");
      }

      QString SecondaryTextSelectList()
      {
        return GetStyleSheet(":/resource/styles/genesis_font_secondary_text_selectlist.qss");
      }

      QString SecondaryText()
      {
        return GetStyleSheet(":/resource/styles/genesis_font_secondary_text.qss");
      }

      QString SecondaryBold()
      {
        return GetStyleSheet(":/resource/styles/genesis_font_secondary_bold.qss");
      }

      QString TabsControls()
      {
        return GetStyleSheet(":/resource/styles/genesis_font_tabs_controls.qss");
      }

      QString SmallText()
      {
        return GetStyleSheet(":/resource/styles/genesis_font_small_text.qss");
      }

      QString AxisTicks()
      {
        return GetStyleSheet(":/resource/styles/genesis_font_axis_ticks_text.qss");
      }

      QString InterFont()
      {
        return GetStyleSheet(":/resource/styles/genesis_font_inter.qss");
      }

    }//namespace Fonts
  }

  //// Legacy
  namespace Legacy
  {
    //// Get Application StyleSheet
    QString GetApplicationStyleSheet()
    {
      return GetStyleSheet(":/resource/styles/app.qss");
    }

    //// Get Custom Window Title StyleSheet
    QString GetCustomWindowTitleStyle()
    {
      return GetStyleSheet(":/resource/styles/custom_window_title.qss");
    }

    //// Get Custom Window Title Mainwindow StyleSheet
    QString GetCustomWindowTitleMainwindowStyle()
    {
      return GetStyleSheet(":/resource/styles/custom_window_title_main.qss");
    }

    //// Get Custom Window Title Menu StyleSheet
    QString GetCustomWindowTitleMenuStyle()
    {
      return GetStyleSheet(":/resource/styles/custom_window_title_menu.qss");
    }

    //// Get Custom Window Menu Container Style
    QString GetCustomWindowMenuContainerStyle()
    {
      return GetStyleSheet(":/resource/styles/custom_window_title_menu_container.qss");
    }

    //// Get Main toolbar stylesheet
    QString GetMainToolbarStyle()
    {
      return GetStyleSheet(":/resource/styles/main_toolbar.qss");
    }

    //// Get Collapsed Widgets View StyleSheet
    QString GetCollapsedWidgetsViewStyle()
    {
      return GetStyleSheet(":/resource/styles/collapsed_widgets_view.qss");
    }

    //// Get Splitter Style
    QString GetSplitterStyle()
    {
      return GetStyleSheet(":/resource/styles/ui_qsplitter.qss");
    }

    //// Get Status Bar Style
    QString GetStatusBarStyle()
    {
      return GetStyleSheet(":/resource/styles/status_bar.qss");
    }

    //// Get Brand Tabbar Style
    QString GetBrandTabbarStyle()
    {
      return GetStyleSheet(":/resource/styles/tabbar_brand.qss");
    }

    //// Get Brand Large Tabbar Sytyle
    QString GetBrandLargeTabbarStyle()
    {
      return GetStyleSheet(":/resource/styles/tabbar_brand_large.qss");
    }

    //// Get Tabbar Style
    QString GetTabbarStyle()
    {
      return GetStyleSheet(":/resource/styles/tabbar.qss");
    }

    //// Get Tabbar Style
    QString GetTabbarLargeStyle()
    {
      return GetStyleSheet(":/resource/styles/tabbar_large.qss");
    }

    //// Get Tabbar Vivid Style
    QString GetTabbarVividStyle()
    {
      return GetStyleSheet(":/resource/styles/tabbar_vivid.qss");
    }

    //// Get Brand Toolbar style
    QString GetBrandToolbarStyle()
    {
      return GetStyleSheet(":/resource/styles/toolbar_brand.qss");
    }

    //// Get Toolbar style
    QString GetToolbarStyle()
    {
      return GetStyleSheet(":/resource/styles/toolbar.qss");
    }

    //// Get Toolbar Transparent style
    QString GetToolbarTransparentStyle()
    {
      return GetStyleSheet(":/resource/styles/toolbar_transparent.qss");
    }

    //// Get Navigation Toolbar Style
    QString GetNavigationToolbarStyle()
    {
      return GetStyleSheet(":/resource/styles/navigation_toolbar.qss");
    }

    /// Get TreeView Style
    QString GetTreeViewStyle()
    {
      return GetStyleSheet(":/resource/styles/ui_qtreeview.qss");
    }

    //// Get Project TreeView Style
    QString GetProjectTreeViewStyle()
    {
      return GetStyleSheet(":/resource/styles/project_tree_view.qss");
    }

    //// Get Page Navigation Brand Style
    QString GetPageNavigationBrandStyle()
    {
      return GetStyleSheet(":/resource/styles/page_navigation_brand.qss");
    }

    //// Get Page Navigation Style
    QString GetPageNavigationStyle()
    {
      return GetStyleSheet(":/resource/styles/page_navigation.qss");
    }

    //// Get Page Navigation Section Label Style
    QString GetPageNavigationSectionLabelStyle()
    {
      return GetStyleSheet(":/resource/styles/page_navigation_section_label_qlabel.qss");
    }

    //// Get Page Navigation Section Label No Padding Style
    QString GetPageNavigationSectionLabelNopaddingStyle()
    {
      return GetStyleSheet(":/resource/styles/page_navigation_section_label_qlabel_nopadding.qss");
    }

    //// Get Page Navigation Radiobutton Delegate Style
    QString GetPageNavigationRadiobuttonDelegateStyle()
    {
      return GetStyleSheet(":/resource/styles/page_navigation_radio_button_menu.qss");
    }

    //// Get Page Navigation Radiobutton Delegate Brand Style
    QString GetPageNavigationRadiobuttonDelegateBrandStyle()
    {
      return GetStyleSheet(":/resource/styles/page_navigation_brand_radio_button_menu.qss");
    }

    /// Get Project Warning Label Style
    QString GetProjectWarningLabelStyle()
    {
      return GetStyleSheet(":/resource/styles/project_warning_label.qss");
    }

    //// Get Properties Style
    QString GetPropertiesStyle()
    {
      return GetStyleSheet(":/resource/styles/properties.qss");
    }

    //// Get Properties Brand Style
    QString GetPropertiesBrandStyle()
    {
      return GetStyleSheet(":/resource/styles/properties_brand.qss");
    }

    //// Get Dock style
    QString GetDockStyle()
    {
      return GetStyleSheet(":/resource/styles/dock_widget.qss");
    }

    //// Get general page style
    QString GetPageStyle()
    {
      return GetStyleSheet(":/resource/styles/page.qss");
    }

    //// Get page caption style
    QString GetPageCaptionStyle()
    {
      return GetStyleSheet(":/resource/styles/page_caption.qss");
    }

    //// Get page section style
    QString GetPageSectionStyle()
    {
      return GetStyleSheet(":/resource/styles/page_section.qss");
    }

    //// Get page caption brand style
    QString GetPageCaptionBrandStyle()
    {
      return GetStyleSheet(":/resource/styles/page_caption_brand.qss");
    }

    //// Get general ui style
    QString GetUiStyle()
    {
      return GetStyleSheet(":/resource/styles/ui.qss");
    }

    //// Get ui specific widgets style
    QString GetUiQPushButtonStyle()           { return GetStyleSheet(":/resource/styles/ui_qpushbutton.qss"); }
    QString GetUiQPushButtonPlainStyle()      { return GetStyleSheet(":/resource/styles/ui_qpushbutton_plain.qss"); }
    QString GetUiQAbstractSpinBoxLabelStyle() { return GetStyleSheet(":/resource/styles/ui_qabstractspinbox_label.qss"); }
    QString GetUiQTabWidgetStyle()            { return GetStyleSheet(":/resource/styles/ui_qtabwidget.qss"); }
    QString GetUiQSliderStyle()               { return GetStyleSheet(":/resource/styles/ui_qslider.qss"); }

    //// Get dialog button style
    QString GetDialogButtonStyle()
    {
      return GetStyleSheet(":/resource/styles/ui_qdialogbuttonbox_qpushbutton.qss");
    }

    //// Get Home Page Style
    QString GetHomePageStyle()
    {
      return GetStyleSheet(":/resource/styles/home_page.qss");
    }

    //// Get Input Error Warning Frame Style
    QString GetInputErrorWarningFrameStyle()
    {
      return GetStyleSheet(":/resource/styles/input_error_warning_frame.qss");
    }

    //// Get progress style
    QString GetProgressStyle()
    {
      return GetStyleSheet(":/resource/styles/progress.qss");
    }

    //// Get Loading Static Style
    QString GetLoadingStaticStyle()
    {
      return GetStyleSheet(":/resource/styles/loading_static.qss");
    }

    //// Get White QWidget Style
    QString GetWhiteQWidgetStyle()
    {
      return GetStyleSheet(":/resource/styles/white_qwidget.qss");
    }

    //////////////////////////////////////////////////////////////////////////////////////
    //// Get Input Error Warning Icon Path
    QString GetInputErrorWarningIconPath()
    {
      return ApplySASS(":/resource/styles/icons/input_error_warning@hdpi.png");
    }
    
  }
}
