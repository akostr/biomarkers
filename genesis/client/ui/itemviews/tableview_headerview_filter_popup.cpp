#include "tableview_headerview_filter_popup.h"
#include "tableview_headerview_filter_popup_editor.h"

#include <genesis_style/style.h>

#include <QVBoxLayout>
#include <QPushButton>

namespace Details
{
  const int IconSize = 24;
  const int Pad = 10;
}

//// Filter popup
FilterPopup::FilterPopup(QMenu* menu, int column, QPointer<TreeModelPresentation> presentation, QPointer<TreeModel> model)
  : QWidget(menu)
  , Column(column)
  , Presentation(presentation)
  , Model(model)
  , Menu(menu)
{
  SetupUi();
}

FilterPopup::~FilterPopup()
{
}

void FilterPopup::SetupUi()
{
  //// Self
  setStyleSheet(Style::Genesis::GetUiStyle());
  
  //// Layout
  Layout = new QVBoxLayout(this);
  Layout->setContentsMargins(Style::Scale(Details::Pad),
                             Style::Scale(Details::Pad),
                             Style::Scale(Details::Pad),
                             Style::Scale(Details::Pad));
  
  //// Column info
  auto columnInfo = Model->GetColumnInfo(Column);
  
  //// Editor
  Layout->addSpacing(Style::Scale(Details::Pad));
  switch (columnInfo.Filter)
  {
    case ColumnInfo::FilterTypeTextValuesList:
      Editor = new FilterPopupEditorValuesList(this, Column, Presentation, Model);
      break;
    case ColumnInfo::FilterTypeTextSubstring:
      Editor = new FilterPopupEditorSubstring(this, Column, Presentation, Model);
      break;
    case ColumnInfo::FilterTypeDateRange:
      Editor = new FilterPopupEditorDateRange(this, Column, Presentation, Model);
      break;
    case ColumnInfo::FilterTypeIntegerRange:
      Editor = new FilterPopupEditorIntegerRange(this, Column, Presentation, Model);
      break;
      
    default:
      break;
  }
  if (Editor)
  {
    Editor->SetData(Presentation->GetColumnFilter(Column));
  }
  Layout->addWidget(Editor);
  
  //// Buttons
  Buttons = new QDialogButtonBox( QDialogButtonBox::Ok|QDialogButtonBox::Cancel, this);
  Buttons->button(QDialogButtonBox::Ok)->setDefault(true);
  Buttons->button(QDialogButtonBox::Ok)->setText(tr("Accept"));
  connect(Buttons, &QDialogButtonBox::rejected,
          [this]()
  {
    //// Done
    Menu->close();
  });
  connect(Buttons, &QDialogButtonBox::accepted,
          [this]()
  {
    //// Done
    if (Editor)
    {
      Presentation->SetColumnFilter(Column, Editor->GetData());
    }
    Menu->close();
  });
  
  Layout->addSpacing(Style::Scale(Details::Pad));
  Layout->addWidget(Buttons);
  Layout->addSpacing(Style::Scale(Details::Pad));
}

