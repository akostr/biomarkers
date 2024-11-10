#include "app_proxy_style.h"
#include "constants.h"
#include "style.h"
#include "utils.h"

#include "graphics_effect_focused_input_glow.h"

#include <QStyleOption>
#include <QPainter>
  
#include <QAbstractSpinBox>
#include <QLineEdit>
#include <QTextEdit>
#include <QPlainTextEdit>
#include <QComboBox>
#include <QPushButton>
#include <QCheckBox>
#include <QRadioButton>
#include <QDialogButtonBox>
#include <QCommandLinkButton>
#include <QMessageBox>
#include <QTableView>
#include <QTreeView>
#include <QLabel>
#include <QTabBar>
#include <QToolButton>
#include <QToolBar>
#include <QRegularExpression>
#include <QMouseEvent>
#include <QMenu>
#include <QPainterPath>

#include <QPointer>
#include <QTimer>

#ifndef DISABLE_STYLES

/////////////////////////////////////////////////
//// App Propxy Style
AppPropxyStyle::AppPropxyStyle()
{
}

AppPropxyStyle::~AppPropxyStyle()
{
}

//// Impls
int AppPropxyStyle::styleHint(StyleHint hint, const QStyleOption* option, const QWidget* widget, QStyleHintReturn* returnData) const
{
  switch (hint)
  {
  case QStyle::SH_EtchDisabledText:
    break;
  case QStyle::SH_DitherDisabledText:
    break;
  case QStyle::SH_ScrollBar_MiddleClickAbsolutePosition:
    break;
  case QStyle::SH_ScrollBar_ScrollWhenPointerLeavesControl:
    break;
  case QStyle::SH_TabBar_SelectMouseType:
    break;
  case QStyle::SH_TabBar_Alignment:
    break;
  case QStyle::SH_Header_ArrowAlignment:
    break;
  case QStyle::SH_Slider_SnapToValue:
    break;
  case QStyle::SH_Slider_SloppyKeyEvents:
    break;
  case QStyle::SH_ProgressDialog_CenterCancelButton:
    break;
  case QStyle::SH_ProgressDialog_TextLabelAlignment:
    break;
  case QStyle::SH_PrintDialog_RightAlignButtons:
    break;
  case QStyle::SH_MainWindow_SpaceBelowMenuBar:
    break;
  case QStyle::SH_FontDialog_SelectAssociatedText:
    break;
  case QStyle::SH_Menu_AllowActiveAndDisabled:
    break;
  case QStyle::SH_Menu_SpaceActivatesItem:
    break;
  case QStyle::SH_Menu_SubMenuPopupDelay:
    break;
  case QStyle::SH_ScrollView_FrameOnlyAroundContents:
    break;
  case QStyle::SH_MenuBar_AltKeyNavigation:
    break;
  case QStyle::SH_ComboBox_ListMouseTracking:
    break;
  case QStyle::SH_Menu_MouseTracking:
    break;
  case QStyle::SH_MenuBar_MouseTracking:
    break;
  case QStyle::SH_ItemView_ChangeHighlightOnFocus:
    break;
  case QStyle::SH_Widget_ShareActivation:
    break;
  case QStyle::SH_Workspace_FillSpaceOnMaximize:
    break;
  case QStyle::SH_ComboBox_Popup:
    return 0;
  case QStyle::SH_TitleBar_NoBorder:
    break;
  case QStyle::SH_Slider_StopMouseOverSlider:
    break;
  case QStyle::SH_BlinkCursorWhenTextSelected:
    break;
  case QStyle::SH_RichText_FullWidthSelection:
    break;
  case QStyle::SH_Menu_Scrollable:
    break;
  case QStyle::SH_GroupBox_TextLabelVerticalAlignment:
    break;
  case QStyle::SH_GroupBox_TextLabelColor:
    {
      return Style::GetSASSColor("textColor").rgb();
    }
    break;
  case QStyle::SH_Menu_SloppySubMenus:
    break;
  case QStyle::SH_Table_GridLineColor:
    {
      return Style::GetSASSColor("gridColor").rgb();
    }
    break;
  case QStyle::SH_LineEdit_PasswordCharacter:
    break;
  case QStyle::SH_DialogButtons_DefaultButton:
    break;
  case QStyle::SH_ToolBox_SelectedPageTitleBold:
    break;
  case QStyle::SH_TabBar_PreferNoArrows:
    break;
  case QStyle::SH_ScrollBar_LeftClickAbsolutePosition:
    break;
  case QStyle::SH_ListViewExpand_SelectMouseType:
    break;
  case QStyle::SH_UnderlineShortcut:
    break;
  case QStyle::SH_SpinBox_AnimateButton:
    break;
  case QStyle::SH_SpinBox_KeyPressAutoRepeatRate:
    break;
  case QStyle::SH_SpinBox_ClickAutoRepeatRate:
    break;
  case QStyle::SH_Menu_FillScreenWithScroll:
    break;
  case QStyle::SH_ToolTipLabel_Opacity:
    break;
  case QStyle::SH_DrawMenuBarSeparator:
    break;
  case QStyle::SH_TitleBar_ModifyNotification:
    break;
  case QStyle::SH_Button_FocusPolicy:
    break;
  case QStyle::SH_MessageBox_UseBorderForButtonSpacing:
    break;
  case QStyle::SH_TitleBar_AutoRaise:
    break;
  case QStyle::SH_ToolButton_PopupDelay:
    break;
  case QStyle::SH_FocusFrame_Mask:
    break;
  case QStyle::SH_RubberBand_Mask:
    break;
  case QStyle::SH_WindowFrame_Mask:
    break;
  case QStyle::SH_SpinControls_DisableOnBounds:
    break;
  case QStyle::SH_Dial_BackgroundRole:
    break;
  case QStyle::SH_ComboBox_LayoutDirection:
    break;
  case QStyle::SH_ItemView_EllipsisLocation:
    break;
  case QStyle::SH_ItemView_ShowDecorationSelected:
    break;
  case QStyle::SH_ItemView_ActivateItemOnSingleClick:
    break;
  case QStyle::SH_ScrollBar_ContextMenu:
    break;
  case QStyle::SH_ScrollBar_RollBetweenButtons:
    break;
  case QStyle::SH_Slider_AbsoluteSetButtons:
    break;
  case QStyle::SH_Slider_PageSetButtons:
    break;
  case QStyle::SH_Menu_KeyboardSearch:
    break;
  case QStyle::SH_TabBar_ElideMode:
    break;
  case QStyle::SH_DialogButtonLayout:
    break;
  case QStyle::SH_ComboBox_PopupFrameStyle:
    break;
  case QStyle::SH_MessageBox_TextInteractionFlags:
    break;
  case QStyle::SH_DialogButtonBox_ButtonsHaveIcons:
    break;
  ////case QStyle::SH_SpellCheckUnderlineStyle:
    ////break;
  case QStyle::SH_MessageBox_CenterButtons:
    break;
  case QStyle::SH_Menu_SelectionWrap:
    break;
  case QStyle::SH_ItemView_MovementWithoutUpdatingSelection:
    break;
  case QStyle::SH_ToolTip_Mask:
    break;
  case QStyle::SH_FocusFrame_AboveWidget:
    break;
  case QStyle::SH_TextControl_FocusIndicatorTextCharFormat:
    break;
  case QStyle::SH_WizardStyle:
    break;
  case QStyle::SH_ItemView_ArrowKeysNavigateIntoChildren:
    break;
  case QStyle::SH_Menu_Mask:
    break;
  case QStyle::SH_Menu_FlashTriggeredItem:
    break;
  case QStyle::SH_Menu_FadeOutOnHide:
    break;
  case QStyle::SH_SpinBox_ClickAutoRepeatThreshold:
    break;
  case QStyle::SH_ItemView_PaintAlternatingRowColorsForEmptyArea:
    break;
  case QStyle::SH_FormLayoutWrapPolicy:
    break;
  case QStyle::SH_TabWidget_DefaultTabPosition:
    break;
  case QStyle::SH_ToolBar_Movable:
    break;
  case QStyle::SH_FormLayoutFieldGrowthPolicy:
    break;
  case QStyle::SH_FormLayoutFormAlignment:
    break;
  case QStyle::SH_FormLayoutLabelAlignment:
    break;
  case QStyle::SH_ItemView_DrawDelegateFrame:
    break;
  case QStyle::SH_TabBar_CloseButtonPosition:
    break;
  case QStyle::SH_DockWidget_ButtonsHaveFrame:
    break;
  case QStyle::SH_ToolButtonStyle:
    break;
  case QStyle::SH_RequestSoftwareInputPanel:
    break;
  case QStyle::SH_ScrollBar_Transient:
    break;
  case QStyle::SH_Menu_SupportsSections:
    break;
  case QStyle::SH_ToolTip_WakeUpDelay:
    break;
  case QStyle::SH_ToolTip_FallAsleepDelay:
    break;
  case QStyle::SH_Widget_Animate:
    break;
  case QStyle::SH_Splitter_OpaqueResize:
    break;
  case QStyle::SH_ComboBox_UseNativePopup:
    break;
  case QStyle::SH_LineEdit_PasswordMaskDelay:
    break;
  case QStyle::SH_TabBar_ChangeCurrentDelay:
    break;
  case QStyle::SH_Menu_SubMenuUniDirection:
    break;
  case QStyle::SH_Menu_SubMenuUniDirectionFailCount:
    break;
  case QStyle::SH_Menu_SubMenuSloppySelectOtherActions:
    break;
  case QStyle::SH_Menu_SubMenuSloppyCloseTimeout:
    break;
  case QStyle::SH_Menu_SubMenuResetWhenReenteringParent:
    break;
  case QStyle::SH_Menu_SubMenuDontStartSloppyOnLeave:
    break;
  case QStyle::SH_ItemView_ScrollMode:
    break;
  case QStyle::SH_TitleBar_ShowToolTipsOnButtons:
    break;
  case QStyle::SH_Widget_Animation_Duration:
    break;
  case QStyle::SH_ComboBox_AllowWheelScrolling:
    break;
  case QStyle::SH_SpinBox_ButtonsInsideFrame:
    break;
  case QStyle::SH_SpinBox_StepModifier:
    break;
  case QStyle::SH_CustomBase:
    break;
  default:
    break;
  }
  return Inherited::styleHint(hint, option, widget, returnData);
}

//// Impls
void AppPropxyStyle::drawControl(QStyle::ControlElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
  switch (element)
  {
  case QStyle::CE_HeaderEmptyArea:
  case QStyle::CE_HeaderSection:
  case QStyle::CE_Header:
    {
      painter->save();

      bool empty = element == QStyle::CE_HeaderEmptyArea;
      bool disabled = !(option->state & QStyle::State_Enabled);

      QColor backgroundColor = Style::GetSASSColor("headerViewBackground");
      if (!empty && !disabled)
      {
        if (option->state & QStyle::State_MouseOver)
          backgroundColor = Style::GetSASSColor("headerViewBackgroundHover");
        if (option->state & QStyle::State_Sunken)
          backgroundColor = Style::GetSASSColor("headerViewBackgroundActive");
      }
      painter->fillRect(option->rect, backgroundColor);

      QColor frameColor = Style::GetSASSColor("headerViewBorder");
      if (!empty && !disabled)
      {
        if (option->state & (QStyle::QStyle::State_Sunken))
          frameColor = Style::GetSASSColor("headerViewBorderActive");
        if (option->state & (QStyle::State_MouseOver))
          frameColor = Style::GetSASSColor("headerViewBorderHover");
      }
      painter->setPen(QPen(frameColor));

      QPoint pt0 = option->rect.bottomLeft();
      QPoint pt1 = option->rect.bottomRight();
      QPoint pt2 = option->rect.topRight();
      painter->drawLine(pt0, pt1);
      painter->drawLine(pt2, pt1);

      if (element == QStyle::CE_Header)
      {
        if (const QStyleOptionHeader* header = qstyleoption_cast<const QStyleOptionHeader *>(option))
        {
          QRegion clipRegion = painter->clipRegion();
          painter->setClipRect(option->rect);
          proxy()->drawControl(CE_HeaderSection, header, painter, widget);
          QStyleOptionHeader subopt = *header;
          subopt.rect = subElementRect(SE_HeaderLabel, header, widget);
          if (subopt.rect.isValid())
            proxy()->drawControl(CE_HeaderLabel, &subopt, painter, widget);
          if (header->sortIndicator != QStyleOptionHeader::None)
          {
            subopt.rect = subElementRect(SE_HeaderArrow, option, widget);
            proxy()->drawPrimitive(PE_IndicatorHeaderArrow, &subopt, painter, widget);
          }
          painter->setClipRegion(clipRegion);
        }
      }

      painter->restore();
    }
    break;
  case QStyle::CE_HeaderLabel:
    {
      if (const QStyleOptionHeader* header = qstyleoption_cast<const QStyleOptionHeader *>(option)) 
      {
        QRect rect = header->rect;
        if (!header->icon.isNull()) 
        {
          int iconExtent = proxy()->pixelMetric(PM_SmallIconSize);
          QPixmap pixmap = header->icon.pixmap(widget ? widget->window()->windowHandle() : nullptr, QSize(iconExtent, iconExtent), (header->state & State_Enabled) ? QIcon::Normal : QIcon::Disabled);
          int pixw = pixmap.width() / pixmap.devicePixelRatio();

          QRect aligned = alignedRect(header->direction, QFlag(header->iconAlignment), pixmap.size() / pixmap.devicePixelRatio(), rect);
          QRect inter = aligned.intersected(rect);
          painter->drawPixmap(inter.x(), inter.y(), pixmap,
            inter.x() - aligned.x(), inter.y() - aligned.y(),
            aligned.width() * pixmap.devicePixelRatio(),
            pixmap.height() * pixmap.devicePixelRatio());

          if (header->direction == Qt::LeftToRight)
            rect.setLeft(rect.left() + pixw + 2);
          else
            rect.setRight(rect.right() - pixw - 2);
        }

        QColor textColor = Style::GetSASSColor("textColor");
        static QFont font(Style::GetSASSValue("fontFace"),   9, QFont::Medium);
        static QFont fontOn(Style::GetSASSValue("fontFace"), 9, QFont::DemiBold);

        if (header->state & QStyle::State_On) 
        {
          painter->setFont(fontOn);
        }
        else
        {
          painter->setFont(font);
        }

        QPalette palette = header->palette;
        palette.setColor(QPalette::Text,       textColor);
        palette.setColor(QPalette::ButtonText, textColor);

        drawItemText(painter, rect, header->textAlignment, palette, (header->state & State_Enabled), header->text, QPalette::ButtonText);
      }
    }
    break;
  default:
    {
      Inherited::drawControl(element, option, painter, widget);
    }
    break;
  }
}

void AppPropxyStyle::drawPrimitive(QStyle::PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
  switch (element)
  {
  case QStyle::PE_Frame:
    {
      const QLineEdit* lineEdit = qobject_cast<const QLineEdit*>(widget);
      const QTextEdit* textEdit = qobject_cast<const QTextEdit*>(widget);
      if (lineEdit || textEdit)
      {
        return drawPrimitive(PE_FrameLineEdit, option, painter, widget);
      }

//      QRect rect = option->rect;
      State flags = option->state;

      if (flags & State_Raised)
        return;

      QColor bordercolor = Style::GetSASSColor("frameColor");
      QPen oldPen = painter->pen();

      // Inner white border
      painter->setPen(QPen(option->palette.base().color(), 0));
      painter->drawRect(QRectF(option->rect).adjusted(
        Style::Utils::DPIScaled(0.5), 
        Style::Utils::DPIScaled(0.5),
        Style::Utils::DPIScaled(-1), 
        Style::Utils::DPIScaled(-1)));
      
      // Outer dark border
      painter->setPen(QPen(bordercolor, 0));
      painter->drawRect(QRectF(option->rect).adjusted(0, 0, Style::Utils::DPIScaled(-0.5), Style::Utils::DPIScaled(-0.5)));
      painter->setPen(oldPen);
    }
    return;
  case QStyle::PE_FrameDefaultButton:
    break;
  case QStyle::PE_FrameDockWidget:
    break;
  case QStyle::PE_FrameFocusRect:
    {
      const QTableView* tbv = qobject_cast<const QTableView*>(widget);
      // const QTreeView*  trv = qobject_cast<const QTreeView*>(widget);
      if (tbv /*|| trv*/)
      {
        painter->setPen(Style::GetSASSColor("itemViewFocusRectColor"));
        QRect rect = option->rect;
        rect.setHeight(rect.height() - 1);
        rect.setWidth(rect.width() - 1);
        painter->drawRect(rect);
      }
      return;
    }
    break;
  case QStyle::PE_FrameGroupBox:
    break;
  case QStyle::PE_FrameLineEdit:
    {
      //// break;
      QColor back = Style::GetSASSColor("uiInputElementColorBg");
      QColor fore = Style::GetSASSColor("uiInputElementColorFrameOpaque");
      if (widget->hasFocus())
      {
        fore = Style::GetSASSColor("uiInputElementColorFrameActive");
      }
      if (!widget->isEnabled())
      {
        QColor back = Style::GetSASSColor("uiInputElementColorBgDisabledOpaque");
        QColor fore = Style::GetSASSColor("uiInputElementColorFrameDisabledOpaque");
      }

      painter->save();
      bool wasAntialiased = painter->renderHints() & QPainter::Antialiasing;
      painter->setRenderHint(QPainter::Antialiasing);
      QPainterPath path;
      path.addRoundedRect(QRectF(widget->rect().adjusted(1,1,-1,-1)), 3, 3);
      QPen pen(fore, 1);
      painter->setPen(pen);
      painter->fillPath(path, back);
      painter->drawPath(path);
      painter->setRenderHint(QPainter::Antialiasing, wasAntialiased);
      painter->restore();
      return;
    }
    break;
  case QStyle::PE_FrameMenu:
    break;
  ////case QStyle::PE_FrameStatusBar:
    ////break;
  case QStyle::PE_FrameTabWidget:
    break;
  case QStyle::PE_FrameWindow:
    break;
  case QStyle::PE_FrameButtonBevel:
    break;
  case QStyle::PE_FrameButtonTool:
    break;
  case QStyle::PE_FrameTabBarBase:
    break;
  case QStyle::PE_PanelButtonCommand:
    break;
  case QStyle::PE_PanelButtonBevel:
    break;
  case QStyle::PE_PanelButtonTool:
    break;
  case QStyle::PE_PanelMenuBar:
    break;
  case QStyle::PE_PanelToolBar:
    break;
  case QStyle::PE_PanelLineEdit:
    break;
  case QStyle::PE_IndicatorArrowDown:
    break;
  case QStyle::PE_IndicatorArrowLeft:
    break;
  case QStyle::PE_IndicatorArrowRight:
    break;
  case QStyle::PE_IndicatorArrowUp:
    break;
  case QStyle::PE_IndicatorBranch:
    break;
  case QStyle::PE_IndicatorButtonDropDown:
    break;
  case QStyle::PE_IndicatorItemViewItemCheck:
    {
      QStyleOptionButton button;
      button.QStyleOption::operator=(*option);
      button.state &= ~State_MouseOver;
      return proxy()->drawPrimitive(PE_IndicatorCheckBox, &button, painter, widget);
    }
    break;
  case QStyle::PE_IndicatorRadioButton:
  case QStyle::PE_IndicatorCheckBox:
    {
      // Cache
      static QMap<QString, QString> resolvedIconPaths;
      if (resolvedIconPaths.empty())
      {
        resolvedIconPaths[":/resource/styles/genesis/qcheckbox/checkbox_off_click@hdpi.png"];
        resolvedIconPaths[":/resource/styles/genesis/qcheckbox/checkbox_off_disabled@hdpi.png"];
        resolvedIconPaths[":/resource/styles/genesis/qcheckbox/checkbox_off_hover@hdpi.png"];
        resolvedIconPaths[":/resource/styles/genesis/qcheckbox/checkbox_off_normal@hdpi.png"];
        resolvedIconPaths[":/resource/styles/genesis/qcheckbox/checkbox_on_click@hdpi.png"];
        resolvedIconPaths[":/resource/styles/genesis/qcheckbox/checkbox_on_disabled@hdpi.png"];
        resolvedIconPaths[":/resource/styles/genesis/qcheckbox/checkbox_on_hover@hdpi.png"];
        resolvedIconPaths[":/resource/styles/genesis/qcheckbox/checkbox_on_normal@hdpi.png"];
        resolvedIconPaths[":/resource/styles/genesis/qcheckbox/checkbox_part_click@hdpi.png"];
        resolvedIconPaths[":/resource/styles/genesis/qcheckbox/checkbox_part_disabled@hdpi.png"];
        resolvedIconPaths[":/resource/styles/genesis/qcheckbox/checkbox_part_hover@hdpi.png"];
        resolvedIconPaths[":/resource/styles/genesis/qcheckbox/checkbox_part_normal@hdpi.png"];

        resolvedIconPaths[":/resource/styles/genesis/qcheckbox_small/checkbox_off_click@hdpi.png"];
        resolvedIconPaths[":/resource/styles/genesis/qcheckbox_small/checkbox_off_disabled@hdpi.png"];
        resolvedIconPaths[":/resource/styles/genesis/qcheckbox_small/checkbox_off_hover@hdpi.png"];
        resolvedIconPaths[":/resource/styles/genesis/qcheckbox_small/checkbox_off_normal@hdpi.png"];
        resolvedIconPaths[":/resource/styles/genesis/qcheckbox_small/checkbox_on_click@hdpi.png"];
        resolvedIconPaths[":/resource/styles/genesis/qcheckbox_small/checkbox_on_disabled@hdpi.png"];
        resolvedIconPaths[":/resource/styles/genesis/qcheckbox_small/checkbox_on_hover@hdpi.png"];
        resolvedIconPaths[":/resource/styles/genesis/qcheckbox_small/checkbox_on_normal@hdpi.png"];
        resolvedIconPaths[":/resource/styles/genesis/qcheckbox_small/checkbox_part_click@hdpi.png"];
        resolvedIconPaths[":/resource/styles/genesis/qcheckbox_small/checkbox_part_disabled@hdpi.png"];
        resolvedIconPaths[":/resource/styles/genesis/qcheckbox_small/checkbox_part_hover@hdpi.png"];
        resolvedIconPaths[":/resource/styles/genesis/qcheckbox_small/checkbox_part_normal@hdpi.png"];

        resolvedIconPaths[":/resource/styles/genesis/qradiobutton/radiobutton_off_click.png"];
        resolvedIconPaths[":/resource/styles/genesis/qradiobutton/radiobutton_off_click@2x.png"];
        resolvedIconPaths[":/resource/styles/genesis/qradiobutton/radiobutton_off_disabled.png"];
        resolvedIconPaths[":/resource/styles/genesis/qradiobutton/radiobutton_off_disabled@2x.png"];
        resolvedIconPaths[":/resource/styles/genesis/qradiobutton/radiobutton_off_hover.png"];
        resolvedIconPaths[":/resource/styles/genesis/qradiobutton/radiobutton_off_hover@2x.png"];
        resolvedIconPaths[":/resource/styles/genesis/qradiobutton/radiobutton_off_normal.png"];
        resolvedIconPaths[":/resource/styles/genesis/qradiobutton/radiobutton_off_normal@2x.png"];
        resolvedIconPaths[":/resource/styles/genesis/qradiobutton/radiobutton_on_click.png"];
        resolvedIconPaths[":/resource/styles/genesis/qradiobutton/radiobutton_on_click@2x.png"];
        resolvedIconPaths[":/resource/styles/genesis/qradiobutton/radiobutton_on_disabled.png"];
        resolvedIconPaths[":/resource/styles/genesis/qradiobutton/radiobutton_on_disabled@2x.png"];
        resolvedIconPaths[":/resource/styles/genesis/qradiobutton/radiobutton_on_hover.png"];
        resolvedIconPaths[":/resource/styles/genesis/qradiobutton/radiobutton_on_hover@2x.png"];
        resolvedIconPaths[":/resource/styles/genesis/qradiobutton/radiobutton_on_normal.png"];
        resolvedIconPaths[":/resource/styles/genesis/qradiobutton/radiobutton_on_normal@2x.png"];

        resolvedIconPaths[":/resource/styles/genesis/qradiobutton_small/radiobutton_off_click.png"];
        resolvedIconPaths[":/resource/styles/genesis/qradiobutton_small/radiobutton_off_click@2x.png"];
        resolvedIconPaths[":/resource/styles/genesis/qradiobutton_small/radiobutton_off_disabled.png"];
        resolvedIconPaths[":/resource/styles/genesis/qradiobutton_small/radiobutton_off_disabled@2x.png"];
        resolvedIconPaths[":/resource/styles/genesis/qradiobutton_small/radiobutton_off_hover.png"];
        resolvedIconPaths[":/resource/styles/genesis/qradiobutton_small/radiobutton_off_hover@2x.png"];
        resolvedIconPaths[":/resource/styles/genesis/qradiobutton_small/radiobutton_off_normal.png"];
        resolvedIconPaths[":/resource/styles/genesis/qradiobutton_small/radiobutton_off_normal@2x.png"];
        resolvedIconPaths[":/resource/styles/genesis/qradiobutton_small/radiobutton_on_click.png"];
        resolvedIconPaths[":/resource/styles/genesis/qradiobutton_small/radiobutton_on_click@2x.png"];
        resolvedIconPaths[":/resource/styles/genesis/qradiobutton_small/radiobutton_on_disabled.png"];
        resolvedIconPaths[":/resource/styles/genesis/qradiobutton_small/radiobutton_on_disabled@2x.png"];
        resolvedIconPaths[":/resource/styles/genesis/qradiobutton_small/radiobutton_on_hover.png"];
        resolvedIconPaths[":/resource/styles/genesis/qradiobutton_small/radiobutton_on_hover@2x.png"];
        resolvedIconPaths[":/resource/styles/genesis/qradiobutton_small/radiobutton_on_normal.png"];
        resolvedIconPaths[":/resource/styles/genesis/qradiobutton_small/radiobutton_on_normal@2x.png"];

        for (QMap<QString, QString>::iterator i = resolvedIconPaths.begin(); i != resolvedIconPaths.end(); ++i)
        {
          i.value() = Style::ApplySASS(i.key());
        }
      }
      static QMap<QString, QPixmap> resolvedIcons;
      if (resolvedIcons.empty())
      {
        for (QMap<QString, QString>::iterator i = resolvedIconPaths.begin(); i != resolvedIconPaths.end(); ++i)
        {
          resolvedIcons[i.key()] = QPixmap(i.value());
        }
      }

      // Resolve state
      State flags = option->state;
      QString statePart;
      if (flags & State_On)
        statePart = "on";
      else if (flags & State_NoChange)
        statePart = "part";
      else
        statePart = "off";

      // Resolve flags
      QString flagsPart;
      if (!(flags & State_Enabled))
        flagsPart = "disabled";
      else if (flags & State_Sunken)
        flagsPart = "click";
      else if (flags & State_MouseOver)
        flagsPart = "hover";
      else
        flagsPart = "normal";

      // Rect
      QRect rect = option->rect;

      // Resolve size
      QString sizePart;
      if (rect.width() < 16
       || rect.height() < 16)
      {
        sizePart = "_small";
      }

      // Resolve folder
      QString folderPart;
      QString filePart;
      if (element == QStyle::PE_IndicatorRadioButton)
      {
        folderPart = "qradiobutton";
        filePart   = "radiobutton";
      }
      else if (element == QStyle::PE_IndicatorCheckBox)
      {
        folderPart = "qcheckbox";
        filePart   = "checkbox";
      }

      // Icon key
      QString key = QString(":/resource/styles/genesis/%1%2/%3_%4_%5@hdpi.png").arg(folderPart, sizePart, filePart, statePart, flagsPart);
      QPixmap pixmap = resolvedIcons.value(key);

      int x = rect.x() + rect.width() / 2 - pixmap.width() / 2;
      int y = rect.y() + rect.height() / 2 - pixmap.height() / 2;

      painter->drawPixmap(x, y, pixmap);
      return;
    }
    break;
  case QStyle::PE_IndicatorDockWidgetResizeHandle:
    break;
  case QStyle::PE_IndicatorHeaderArrow:
    {
      if (const QStyleOptionHeader* header = qstyleoption_cast<const QStyleOptionHeader *>(option)) 
      {
        static QPixmap iconUp = QPixmap(Style::ApplySASS(":/resource/styles/qheaderview/sort_up@hdpi.png"));
        static QPixmap iconDn = QPixmap(Style::ApplySASS(":/resource/styles/qheaderview/sort_dn@hdpi.png"));
        QPixmap* pm = nullptr;
        if (header->sortIndicator & QStyleOptionHeader::SortDown)
          pm = &iconUp;
        else
          pm = &iconDn;
        painter->drawPixmap(option->rect.topLeft() + QPoint({3, 1}), *pm);
      }
      return;
    }
    break;
  case QStyle::PE_IndicatorMenuCheckMark:
    break;
  case QStyle::PE_IndicatorProgressChunk:
    break;
  case QStyle::PE_IndicatorSpinDown:
    break;
  case QStyle::PE_IndicatorSpinMinus:
    break;
  case QStyle::PE_IndicatorSpinPlus:
    break;
  case QStyle::PE_IndicatorSpinUp:
    break;
  case QStyle::PE_IndicatorToolBarHandle:
    break;
  case QStyle::PE_IndicatorToolBarSeparator:
    break;
  case QStyle::PE_PanelTipLabel:
    break;
  case QStyle::PE_IndicatorTabTear:
    break;
  case QStyle::PE_PanelScrollAreaCorner:
    break;
  case QStyle::PE_Widget:
    break;
  case QStyle::PE_IndicatorColumnViewArrow:
    break;
  case QStyle::PE_IndicatorItemViewItemDrop:
    break;
  case QStyle::PE_PanelItemViewItem:
    break;
  case QStyle::PE_PanelItemViewRow:
    break;
  case QStyle::PE_PanelStatusBar:
    break;
  case QStyle::PE_IndicatorTabClose:
    break;
  case QStyle::PE_PanelMenu:
    break;
  case QStyle::PE_IndicatorTabTearRight:
    break;
  case QStyle::PE_CustomBase:
    break;
  default:
    break;
  }
  return Inherited::drawPrimitive(element, option, painter, widget);
}

int AppPropxyStyle::pixelMetric(QStyle::PixelMetric metric, const QStyleOption *option, const QWidget *widget) const
{
  switch (metric)
  {
    case PM_ButtonMargin:
    case PM_ButtonDefaultIndicator:
    case PM_MenuButtonIndicator:
    case PM_ButtonShiftHorizontal:
    case PM_ButtonShiftVertical:

    case PM_DefaultFrameWidth:
    case PM_SpinBoxFrameWidth:
    case PM_ComboBoxFrameWidth:

    case PM_MaximumDragDistance:

    case PM_ScrollBarExtent:
    case PM_ScrollBarSliderMin:

    case PM_SliderThickness:             // total slider thickness
    case PM_SliderControlThickness:      // thickness of the business part
    case PM_SliderLength:                // total length of slider
    case PM_SliderTickmarkOffset:        //
    case PM_SliderSpaceAvailable:        // available space for slider to move

    case PM_DockWidgetSeparatorExtent:
    case PM_DockWidgetHandleExtent:
    case PM_DockWidgetFrameWidth:

    case PM_TabBarTabOverlap:
    case PM_TabBarTabHSpace:
    case PM_TabBarTabVSpace:
    case PM_TabBarBaseHeight:
    case PM_TabBarBaseOverlap:

    case PM_ProgressBarChunkWidth:

    case PM_SplitterWidth:
    case PM_TitleBarHeight:

    case PM_MenuScrollerHeight:
    case PM_MenuHMargin:
    case PM_MenuVMargin:
    case PM_MenuPanelWidth:
    case PM_MenuTearoffHeight:
    case PM_MenuDesktopFrameWidth:

    case PM_MenuBarPanelWidth:
    case PM_MenuBarItemSpacing:
    case PM_MenuBarVMargin:
    case PM_MenuBarHMargin:

    case PM_IndicatorWidth:
    case PM_IndicatorHeight:
    case PM_ExclusiveIndicatorWidth:
    case PM_ExclusiveIndicatorHeight:

    case PM_DialogButtonsSeparator:
    case PM_DialogButtonsButtonWidth:
    case PM_DialogButtonsButtonHeight:

    case PM_MdiSubWindowFrameWidth:
    case PM_MdiSubWindowMinimizedWidth:

    case PM_HeaderMargin:
    case PM_HeaderMarkSize:
    case PM_HeaderGripMargin:
      {
        int pm = QCommonStyle::pixelMetric(metric, option, widget);
        return pm * 2;
      }
      break;
    case PM_TabBarTabShiftHorizontal:
    case PM_TabBarTabShiftVertical:
    case PM_TabBarScrollButtonWidth:

    case PM_ToolBarFrameWidth:
    case PM_ToolBarHandleExtent:
    case PM_ToolBarItemSpacing:
    case PM_ToolBarItemMargin:
    case PM_ToolBarSeparatorExtent:
    case PM_ToolBarExtensionExtent:

    case PM_SpinBoxSliderHeight:

    case PM_ToolBarIconSize:
    case PM_ListViewIconSize:
    case PM_IconViewIconSize:
    case PM_SmallIconSize:
    case PM_LargeIconSize:

    case PM_FocusFrameVMargin:
    case PM_FocusFrameHMargin:

    case PM_ToolTipLabelFrameWidth:
    case PM_CheckBoxLabelSpacing:
    case PM_TabBarIconSize:
    case PM_SizeGripSize:
    case PM_DockWidgetTitleMargin:
    case PM_MessageBoxIconSize:
    case PM_ButtonIconSize:

    case PM_DockWidgetTitleBarButtonMargin:

    case PM_RadioButtonLabelSpacing:
    case PM_LayoutLeftMargin:
    case PM_LayoutTopMargin:
    case PM_LayoutRightMargin:
    case PM_LayoutBottomMargin:
    case PM_LayoutHorizontalSpacing:
    case PM_LayoutVerticalSpacing:
    case PM_TabBar_ScrollButtonOverlap:

    case PM_TextCursorWidth:

    case PM_TabCloseIndicatorWidth:
    case PM_TabCloseIndicatorHeight:

    case PM_ScrollView_ScrollBarSpacing:
    case PM_ScrollView_ScrollBarOverlap:
    case PM_SubMenuOverlap:
    case PM_TreeViewIndentation:

    case PM_HeaderDefaultSectionSizeHorizontal:
    case PM_HeaderDefaultSectionSizeVertical:

    case PM_TitleBarButtonIconSize:
    case PM_TitleBarButtonSize:

    case PM_LineEditIconSize:

    default:
      break;
  }
  return QCommonStyle::pixelMetric(metric, option, widget);
}

void AppPropxyStyle::polish(QPalette& palette)
{
  Inherited::polish(palette);
  QPalette definedPalette = Style::Constants::GetPalette();
  for (int colorGroup = 0; colorGroup < QPalette::NColorGroups; colorGroup++)
  {
    for (int colorRole = 0; colorRole < QPalette::NColorRoles; colorRole++)
    {
      QColor color = definedPalette.color((QPalette::ColorGroup)colorGroup, (QPalette::ColorRole)colorRole);
      if (color.isValid())
      {
        palette.setColor((QPalette::ColorGroup)colorGroup, (QPalette::ColorRole)colorRole, color);
      }
    }
  }
}

void AppPropxyStyle::polish(QWidget* widget)
{
  if (QLabel* lbl = qobject_cast<QLabel*>(widget))
  {
    PrePolish(lbl);
  }

  Inherited::polish(widget);

  if (QAbstractSpinBox* asb = qobject_cast<QAbstractSpinBox*>(widget))
  {
    Polish(asb);
  }
  else if (QLineEdit* le = qobject_cast<QLineEdit*>(widget))
  {
    Polish(le);
  }
  else if (QTextEdit* te = qobject_cast<QTextEdit*>(widget))
  {
    Polish(te);
  }
  else if (QPlainTextEdit* pte = qobject_cast<QPlainTextEdit*>(widget))
  {
    Polish(pte);
  }
  else if (QComboBox* cb = qobject_cast<QComboBox*>(widget))
  {
    Polish(cb);
  }
  else if (QCommandLinkButton* clb = qobject_cast<QCommandLinkButton*>(widget))
  {
    Polish(clb);
  }
  else if (QPushButton* pb = qobject_cast<QPushButton*>(widget))
  {
    Polish(pb);
  }
  else if (QCheckBox* cb = qobject_cast<QCheckBox*>(widget))
  {
    Polish(cb);
  }
  else if (QRadioButton* rb = qobject_cast<QRadioButton*>(widget))
  {
    Polish(rb);
  }
  else if (QDialogButtonBox* dbb = qobject_cast<QDialogButtonBox*>(widget))
  {
    Polish(dbb);
  }
  else if (QMessageBox* mb = qobject_cast<QMessageBox*>(widget))
  {
    Polish(mb);
  }
  else if (QTableView* tv = qobject_cast<QTableView*>(widget))
  {
    Polish(tv);
  }
  else if (QLabel* lbl = qobject_cast<QLabel*>(widget))
  {
    Polish(lbl);
  }
  else if (QTabBar* lbl = qobject_cast<QTabBar*>(widget))
  {
    Polish(lbl);
  }
  else if (QToolButton* tbtn = qobject_cast<QToolButton*>(widget))
  {
    Polish(tbtn);
  }
}

void AppPropxyStyle::Polish(QAbstractSpinBox* asb)
{
  PolishInputWidget(asb);
}

void AppPropxyStyle::Polish(QLineEdit* le)
{
  if (qobject_cast<QAbstractSpinBox*>(le->parent()))
    return;
  if (qobject_cast<QComboBox*>(le->parent()))
    return;

  PolishInputWidget(le);
}

void AppPropxyStyle::Polish(QTextEdit* te)
{
  PolishInputWidget(te);
}

void AppPropxyStyle::Polish(QPlainTextEdit* pte)
{
  PolishInputWidget(pte);
}

void AppPropxyStyle::Polish(QComboBox* cb)
{
  PolishInputWidget(cb);
}

void AppPropxyStyle::Polish(QCommandLinkButton* clb)
{
  Polish((QPushButton*)clb);

  clb->setFlat(true);
  clb->setAutoFillBackground(true);
}

void AppPropxyStyle::Polish(QPushButton* pb)
{
  PolishInputWidget(pb);
}

void AppPropxyStyle::Polish(QCheckBox* cb)
{
  PolishInputWidget(cb);
}

void AppPropxyStyle::Polish(QRadioButton* rb)
{
  PolishInputWidget(rb);
}

void AppPropxyStyle::Polish(QDialogButtonBox* dbbArg)
{
  QPointer<QDialogButtonBox> dbb = dbbArg;
  QTimer::singleShot(0, [dbb] {
    if (dbb)
    {
      QSet<QAbstractButton*> primaryButtons;
      primaryButtons << dbb->button(QDialogButtonBox::Ok);
      primaryButtons << dbb->button(QDialogButtonBox::Save);
      primaryButtons << dbb->button(QDialogButtonBox::SaveAll);
      primaryButtons << dbb->button(QDialogButtonBox::Yes);
      primaryButtons << dbb->button(QDialogButtonBox::YesToAll);
      QList<QAbstractButton*> buttons = dbb->buttons();
      for (const auto& b : buttons)
      {
        if (!primaryButtons.contains(b))
        {
          if (QPushButton* pb = qobject_cast<QPushButton*>(b))
          {
            pb->setFlat(true);
          }
        }
      }

    }
  });
}

void AppPropxyStyle::Polish(QMessageBox* mbArg)
{
  QPointer<QMessageBox> mb = mbArg;
  QTimer::singleShot(0, [mb] {
    if (mb)
    {
      QSet<QAbstractButton*> primaryButtons;
      primaryButtons << mb->button(QMessageBox::Ok);
      primaryButtons << mb->button(QMessageBox::Save);
      primaryButtons << mb->button(QMessageBox::SaveAll);
      primaryButtons << mb->button(QMessageBox::Yes);
      primaryButtons << mb->button(QMessageBox::YesToAll);
      QList<QAbstractButton*> buttons = mb->buttons();
      for (const auto& b : buttons)
      {
        if (!primaryButtons.contains(b))
        {
          if (QPushButton* pb = qobject_cast<QPushButton*>(b))
          {
            pb->setFlat(true);
          }
        }
      }
    }
  });
}

void AppPropxyStyle::Polish(QTableView* /*tv*/)
{
}

void AppPropxyStyle::Polish(QToolButton* tbtn)
{
  tbtn->installEventFilter(this);
}

void AppPropxyStyle::PolishInputWidget(QWidget* inputWidget)
{
  QWidget* viewport = inputWidget->parentWidget();
  QTableView* tbv = nullptr;
  if (viewport)
    tbv = qobject_cast<QTableView*>(viewport->parentWidget());
  QTableView* tbvDirect = qobject_cast<QTableView*>(viewport);
  if (tbvDirect)
    tbv = tbvDirect;
  if (tbv)
  {
    bool expand = tbv->showGrid();
    if (expand)
    {
      QPoint pt = inputWidget->pos();
      bool expandLt = pt.x() > 0;
      bool expandTp = pt.y() > 0;
      bool expandRt = true;
      bool expandBt = true;

      pt -= { expandLt ? 1 : 0, 
              expandTp ? 1 : 0 };

      QSize sz = inputWidget->size();
      sz += { (expandLt ? 1 : 0) + (expandRt ? 1 : 0),
              (expandTp ? 1 : 0) + (expandBt ? 1 : 0) };

      inputWidget->move(pt);
      inputWidget->resize(sz);
    }
  }
}

void AppPropxyStyle::Polish(QLabel* lbl)
{
  QString auxiliaryStyle = lbl->whatsThis();
  if (auxiliaryStyle.isEmpty())
  {
    QString fullStyle = lbl->styleSheet();
    if (!fullStyle.isEmpty())
    {
      if (fullStyle.contains("qproperty-whatsThis"))
      {
        QRegularExpression re("qproperty-whatsThis:\\s*\"(.*?)\\\"");
        QRegularExpressionMatch match = re.match(fullStyle);
        if (match.hasMatch())
        {
          auxiliaryStyle = match.captured(match.lastCapturedIndex());
        }
      }
    }
  }
  if (!auxiliaryStyle.isEmpty())
  {
    QMap<QString, QString> properties;
    QStringList pairs = auxiliaryStyle.split(';');
    for (const auto& pair : pairs)
    {
      QStringList pairContent = pair.split(':');
      if (pairContent.size() == 2)
      {
        properties[pairContent.first()] = pairContent.last();
      }
    }

    for (auto property = properties.begin(); property != properties.end(); ++property)
    {
      QFont font = lbl->font();
      if (property.key() == "text-transform")
      {
        if (property.value() == "upper")
        {
          font.setCapitalization(QFont::AllUppercase);
        }
        else if (property.value() == "lower")
        {
          font.setCapitalization(QFont::AllLowercase);
        }
      }
      else if (property.key() == "text-letterspacing")
      {
        QString value = property.value();
        QFont::SpacingType spacingType = QFont::PercentageSpacing;
        value = value.remove('%');
        if (value.endsWith("px"))
        {
          spacingType = QFont::AbsoluteSpacing;
          value = value.remove("px");
        }       
        
        double spacing = value.toDouble();
        font.setLetterSpacing(spacingType, spacing);
      }
      lbl->setFont(font);
    }
  }
}

void AppPropxyStyle::Polish(QTabBar* lbl)
{
  QString auxiliaryStyle = lbl->whatsThis();
  if (auxiliaryStyle.isEmpty())
  {
    QString fullStyle = lbl->styleSheet();
    if (!fullStyle.isEmpty())
    {
      if (fullStyle.contains("qproperty-whatsThis"))
      {
        QRegularExpression re("qproperty-whatsThis:\\s*\"(.*?)\\\"");
        QRegularExpressionMatch match = re.match(fullStyle);
        if (match.hasMatch())
        {
          auxiliaryStyle = match.captured(match.lastCapturedIndex());
        }
      }
    }
  }
  if (!auxiliaryStyle.isEmpty())
  {
    QMap<QString, QString> properties;
    QStringList pairs = auxiliaryStyle.split(';');
    for (const auto& pair : pairs)
    {
      QStringList pairContent = pair.split(':');
      if (pairContent.size() == 2)
      {
        properties[pairContent.first()] = pairContent.last();
      }
    }

    for (auto property = properties.begin(); property != properties.end(); ++property)
    {
      QFont font = lbl->font();
      if (property.key() == "text-transform")
      {
        if (property.value() == "upper")
        {
          font.setCapitalization(QFont::AllUppercase);
        }
        else if (property.value() == "lower")
        {
          font.setCapitalization(QFont::AllLowercase);
        }
      }
      else if (property.key() == "text-letterspacing")
      {
        QString value = property.value();
        QFont::SpacingType spacingType = QFont::PercentageSpacing;
        value = value.remove('%');
        if (value.endsWith("px"))
        {
          spacingType = QFont::AbsoluteSpacing;
          value = value.remove("px");
        }

        double spacing = value.toDouble();
        font.setLetterSpacing(spacingType, spacing);
      }
      lbl->setFont(font);
    }
  }
}

void AppPropxyStyle::PrePolish(QLabel* /*lbl*/)
{
}

QIcon AppPropxyStyle::standardIcon(StandardPixmap standardIcon, const QStyleOption *option, const QWidget *widget) const
{
  switch (standardIcon)
  {
  case QStyle::SP_TitleBarMenuButton:
    break;
  case QStyle::SP_TitleBarMinButton:
    break;
  case QStyle::SP_TitleBarMaxButton:
    break;
  case QStyle::SP_TitleBarCloseButton:
    break;
  case QStyle::SP_TitleBarNormalButton:
    break;
  case QStyle::SP_TitleBarShadeButton:
    break;
  case QStyle::SP_TitleBarUnshadeButton:
    break;
  case QStyle::SP_TitleBarContextHelpButton:
    break;
  case QStyle::SP_DockWidgetCloseButton:
    break;
  case QStyle::SP_MessageBoxInformation:
  case QStyle::SP_MessageBoxWarning:
  case QStyle::SP_MessageBoxCritical:
  case QStyle::SP_MessageBoxQuestion:
    {
      QIcon icon;
      icon.addPixmap(standardPixmap(standardIcon, option, widget));
      return icon;
    }
    break;
  case QStyle::SP_DesktopIcon:
    break;
  case QStyle::SP_TrashIcon:
    break;
  case QStyle::SP_ComputerIcon:
    break;
  case QStyle::SP_DriveFDIcon:
    break;
  case QStyle::SP_DriveHDIcon:
    break;
  case QStyle::SP_DriveCDIcon:
    break;
  case QStyle::SP_DriveDVDIcon:
    break;
  case QStyle::SP_DriveNetIcon:
    break;
  case QStyle::SP_DirOpenIcon:
    break;
  case QStyle::SP_DirClosedIcon:
    break;
  case QStyle::SP_DirLinkIcon:
    break;
  case QStyle::SP_DirLinkOpenIcon:
    break;
  case QStyle::SP_FileIcon:
    break;
  case QStyle::SP_FileLinkIcon:
    break;
  case QStyle::SP_ToolBarHorizontalExtensionButton:
    break;
  case QStyle::SP_ToolBarVerticalExtensionButton:
    break;
  case QStyle::SP_FileDialogStart:
    break;
  case QStyle::SP_FileDialogEnd:
    break;
  case QStyle::SP_FileDialogToParent:
    break;
  case QStyle::SP_FileDialogNewFolder:
    break;
  case QStyle::SP_FileDialogDetailedView:
    break;
  case QStyle::SP_FileDialogInfoView:
    break;
  case QStyle::SP_FileDialogContentsView:
    break;
  case QStyle::SP_FileDialogListView:
    break;
  case QStyle::SP_FileDialogBack:
    break;
  case QStyle::SP_DirIcon:
    break;
  case QStyle::SP_DialogOkButton:
    break;
  case QStyle::SP_DialogCancelButton:
    break;
  case QStyle::SP_DialogHelpButton:
    break;
  case QStyle::SP_DialogOpenButton:
    break;
  case QStyle::SP_DialogSaveButton:
    break;
  case QStyle::SP_DialogCloseButton:
    break;
  case QStyle::SP_DialogApplyButton:
    break;
  case QStyle::SP_DialogResetButton:
    break;
  case QStyle::SP_DialogDiscardButton:
    break;
  case QStyle::SP_DialogYesButton:
    break;
  case QStyle::SP_DialogNoButton:
    break;
  case QStyle::SP_ArrowUp:
    break;
  case QStyle::SP_ArrowDown:
    break;
  case QStyle::SP_ArrowLeft:
    break;
  case QStyle::SP_ArrowRight:
    break;
  case QStyle::SP_ArrowBack:
    break;
  case QStyle::SP_ArrowForward:
    break;
  case QStyle::SP_DirHomeIcon:
    break;
  case QStyle::SP_CommandLink:
    break;
  case QStyle::SP_VistaShield:
    break;
  case QStyle::SP_BrowserReload:
    break;
  case QStyle::SP_BrowserStop:
    break;
  case QStyle::SP_MediaPlay:
    break;
  case QStyle::SP_MediaStop:
    break;
  case QStyle::SP_MediaPause:
    break;
  case QStyle::SP_MediaSkipForward:
    break;
  case QStyle::SP_MediaSkipBackward:
    break;
  case QStyle::SP_MediaSeekForward:
    break;
  case QStyle::SP_MediaSeekBackward:
    break;
  case QStyle::SP_MediaVolume:
    break;
  case QStyle::SP_MediaVolumeMuted:
    break;
  case QStyle::SP_LineEditClearButton:
    break;
  case QStyle::SP_CustomBase:
    break;
  default:
    break;
  }
  return Inherited::standardIcon(standardIcon, option, widget);
}

QPixmap AppPropxyStyle::standardPixmap(StandardPixmap standardPixmap, const QStyleOption *option, const QWidget *widget) const
{
  switch (standardPixmap)
  {
  case QStyle::SP_TitleBarMenuButton:
    break;
  case QStyle::SP_TitleBarMinButton:
    break;
  case QStyle::SP_TitleBarMaxButton:
    break;
  case QStyle::SP_TitleBarCloseButton:
    break;
  case QStyle::SP_TitleBarNormalButton:
    break;
  case QStyle::SP_TitleBarShadeButton:
    break;
  case QStyle::SP_TitleBarUnshadeButton:
    break;
  case QStyle::SP_TitleBarContextHelpButton:
    break;
  case QStyle::SP_DockWidgetCloseButton:
    break;
  case QStyle::SP_MessageBoxInformation:
  case QStyle::SP_MessageBoxWarning:
  case QStyle::SP_MessageBoxCritical:
  case QStyle::SP_MessageBoxQuestion:
    {
      static const QMap<StandardPixmap, QString> paths = {
        { QStyle::SP_MessageBoxInformation, Style::ApplySASS(":/resource/styles/icons/icon_info@hdpi.png") },
        { QStyle::SP_MessageBoxWarning,     Style::ApplySASS(":/resource/styles/icons/icon_warning@hdpi.png") },
        { QStyle::SP_MessageBoxCritical,    Style::ApplySASS(":/resource/styles/icons/icon_error@hdpi.png") },
        { QStyle::SP_MessageBoxQuestion,    Style::ApplySASS(":/resource/styles/icons/icon_question@hdpi.png") },
      };
      return QPixmap(paths[standardPixmap]);
    }
    break;
  case QStyle::SP_DesktopIcon:
    break;
  case QStyle::SP_TrashIcon:
    break;
  case QStyle::SP_ComputerIcon:
    break;
  case QStyle::SP_DriveFDIcon:
    break;
  case QStyle::SP_DriveHDIcon:
    break;
  case QStyle::SP_DriveCDIcon:
    break;
  case QStyle::SP_DriveDVDIcon:
    break;
  case QStyle::SP_DriveNetIcon:
    break;
  case QStyle::SP_DirOpenIcon:
    break;
  case QStyle::SP_DirClosedIcon:
    break;
  case QStyle::SP_DirLinkIcon:
    break;
  case QStyle::SP_DirLinkOpenIcon:
    break;
  case QStyle::SP_FileIcon:
    break;
  case QStyle::SP_FileLinkIcon:
    break;
  case QStyle::SP_ToolBarHorizontalExtensionButton:
    break;
  case QStyle::SP_ToolBarVerticalExtensionButton:
    break;
  case QStyle::SP_FileDialogStart:
    break;
  case QStyle::SP_FileDialogEnd:
    break;
  case QStyle::SP_FileDialogToParent:
    break;
  case QStyle::SP_FileDialogNewFolder:
    break;
  case QStyle::SP_FileDialogDetailedView:
    break;
  case QStyle::SP_FileDialogInfoView:
    break;
  case QStyle::SP_FileDialogContentsView:
    break;
  case QStyle::SP_FileDialogListView:
    break;
  case QStyle::SP_FileDialogBack:
    break;
  case QStyle::SP_DirIcon:
    break;
  case QStyle::SP_DialogOkButton:
    break;
  case QStyle::SP_DialogCancelButton:
    break;
  case QStyle::SP_DialogHelpButton:
    break;
  case QStyle::SP_DialogOpenButton:
    break;
  case QStyle::SP_DialogSaveButton:
    break;
  case QStyle::SP_DialogCloseButton:
    break;
  case QStyle::SP_DialogApplyButton:
    break;
  case QStyle::SP_DialogResetButton:
    break;
  case QStyle::SP_DialogDiscardButton:
    break;
  case QStyle::SP_DialogYesButton:
    break;
  case QStyle::SP_DialogNoButton:
    break;
  case QStyle::SP_ArrowUp:
    break;
  case QStyle::SP_ArrowDown:
    break;
  case QStyle::SP_ArrowLeft:
    break;
  case QStyle::SP_ArrowRight:
    break;
  case QStyle::SP_ArrowBack:
    break;
  case QStyle::SP_ArrowForward:
    break;
  case QStyle::SP_DirHomeIcon:
    break;
  case QStyle::SP_CommandLink:
    break;
  case QStyle::SP_VistaShield:
    break;
  case QStyle::SP_BrowserReload:
    break;
  case QStyle::SP_BrowserStop:
    break;
  case QStyle::SP_MediaPlay:
    break;
  case QStyle::SP_MediaStop:
    break;
  case QStyle::SP_MediaPause:
    break;
  case QStyle::SP_MediaSkipForward:
    break;
  case QStyle::SP_MediaSkipBackward:
    break;
  case QStyle::SP_MediaSeekForward:
    break;
  case QStyle::SP_MediaSeekBackward:
    break;
  case QStyle::SP_MediaVolume:
    break;
  case QStyle::SP_MediaVolumeMuted:
    break;
  case QStyle::SP_LineEditClearButton:
    break;
  case QStyle::SP_CustomBase:
    break;
  default:
    break;
  }
  return Inherited::standardPixmap(standardPixmap, option, widget);
}

bool AppPropxyStyle::eventFilter(QObject *watched, QEvent *event)
{
  if (QToolButton* button = qobject_cast<QToolButton*>(watched))
  {
    if (event->type() == QEvent::MouseButtonPress)
    {
      //// Show menu via popup() not exec()
      QMenu* menu = button->menu();      
      QMouseEvent* e = (QMouseEvent*)event;
      if (menu
          && e->button() == Qt::LeftButton)
      {
        QPoint global = button->mapToGlobal(QPoint(0, button->height()));
        menu->popup(global);
        return true;
      }
    }
  }
  return QCommonStyle::eventFilter(watched, event);
}

#endif // !DISABLE_STYLES
