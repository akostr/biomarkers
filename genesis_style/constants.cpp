#include "constants.h"
#include "style.h"

#include <QMap>

namespace Style
{
  namespace Constants
  {
    namespace Details
    {
      QMap<QPalette::ColorGroup, QString> ColorGroups({
        { QPalette::Active,    "Active" },
        { QPalette::Disabled,  "Disabled" },
        { QPalette::Inactive,  "Inactive" },
      });

      QMap<QPalette::ColorRole, QString> ColorRole({
        { QPalette::WindowText,       "WindowText" },
        { QPalette::Button,           "Button" },
        { QPalette::Light,            "Light" },
        { QPalette::Midlight,         "Midlight" },
        { QPalette::Dark,             "Dark" },
        { QPalette::Mid,              "Mid" },
        { QPalette::Text,             "Text" },
        { QPalette::BrightText,       "BrightText" },
        { QPalette::ButtonText,       "ButtonText" },
        { QPalette::Base,             "Base" },
        { QPalette::Window,           "Window" },
        { QPalette::Shadow,           "Shadow" },
        { QPalette::Highlight,        "Highlight" },
        { QPalette::HighlightedText,  "HighlightedText" },
        { QPalette::Link,             "Link" },
        { QPalette::LinkVisited,      "LinkVisited" },
        { QPalette::AlternateBase,    "AlternateBase" },
        { QPalette::NoRole,           "NoRole" },
        { QPalette::ToolTipBase,      "ToolTipBase" },
        { QPalette::ToolTipText,      "ToolTipText" },
        { QPalette::PlaceholderText,  "PlaceholderText" },
      });

      // Active, Disabled, 
    }

    //// Get color code
    QString GetColorName(QPalette::ColorGroup group, QPalette::ColorRole role)
    {
      QString sassGroup = Details::ColorGroups.value(group);
      QString sassRole  = Details::ColorRole.value(role);
      QString sassKey   = QString("palette::%1::%2").arg(sassRole, sassGroup);
      return GetSASSValue(sassKey);
    }

    //// Get color object
    QColor GetColor(QPalette::ColorGroup group, QPalette::ColorRole role)
    {
      QString code = GetColorName(group, role);
      if (!code.isEmpty())
      {
        return QColor(code);
      }
      return QColor();
    }

    //// Get default application palette
    QPalette GetPalette()
    {
      QPalette palette(Qt::transparent);
      {
        for (int colorGroup = 0; colorGroup < QPalette::NColorGroups; colorGroup++)
        {
          for (int colorRole = 0; colorRole < QPalette::NColorRoles; colorRole++)
          {
            QColor c = GetColor((QPalette::ColorGroup)colorGroup, (QPalette::ColorRole)colorRole);
            palette.setColor((QPalette::ColorGroup)colorGroup, (QPalette::ColorRole)colorRole, c);
          }
        }
      }
      return palette;
    }
  }
}
