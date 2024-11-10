#pragma once

#ifndef GRAPHICSPLOT_GLOBAL_INCLUDED
#define GRAPHICSPLOT_GLOBAL_INCLUDED
#endif // ~!GRAPHICSPLOT_GLOBAL_INCLUDED

#include <QtCore/qglobal.h>
#include <QtCore/QtCore>
#include <QtCore/QMargins>

// decl definitions for shared library compilation/usage:
#if defined(GRAPHICSPLOT_COMPILE_LIBRARY)
#  define GP_LIB_DECL Q_DECL_EXPORT
#elif defined(GRAPHICSPLOT_USE_LIBRARY)
#  define GP_LIB_DECL Q_DECL_IMPORT
#else
#  define GP_LIB_DECL
#endif

// define empty macro for Q_DECL_OVERRIDE if it doesn't exist (Qt < 5)
#ifndef Q_DECL_OVERRIDE
#  define Q_DECL_OVERRIDE
#endif

/*!
  The GP Namespace contains general enums, QFlags and functions used throughout the GraphicsPlot
  library.

  It provides QMetaObject-based reflection of its enums and flags via \a GP::staticMetaObject.
*/
class GP : public QObject { // when in moc-run, make it look like a class, so we get Q_GADGET, Q_ENUMS/Q_FLAGS features in namespace
  Q_OBJECT
public:
  Q_ENUMS(ExportPen)
  Q_ENUMS(ResolutionUnit)
  Q_ENUMS(SignDomain)
  Q_ENUMS(MarginSide)
  Q_FLAGS(MarginSides)
  Q_ENUMS(AntialiasedElement)
  Q_FLAGS(AntialiasedElements)
  Q_ENUMS(PlottingHint)
  Q_FLAGS(PlottingHints)
  Q_ENUMS(Interaction)
  Q_FLAGS(Interactions)
  Q_ENUMS(SelectionRectMode)
  Q_ENUMS(SelectionType)

  enum AvailableActionsFlags
  {
    AAMenu = 0x00000001,
    AAViewActions = 0x00000002,
    AAGrid = 0x00000004,
    AALegend = 0x00000008,
    AACaptions = 0x00000010,
    AARulers = 0x00000020,
    AADistanceRuler = 0x00000040,
    AASettingsEdit = 0x00000080,
    AAZones = 0x00000100,
    AAZonesManagement = 0x00000200,
    AAZonesAmount = 0x00000400,
    AAMeasuringPolygon = 0x00000800,
    AAMagnifier = 0x00001000,
    AAExcelExport = 0x00002000,
    AADefault = AAMenu | AAViewActions | AAGrid | AALegend | AACaptions | AARulers | AASettingsEdit | AAMagnifier | AAExcelExport
  };

  enum ViewAllFlags
  {
    ViewAllNone                     = 0x00000000,

    // Horizontal
    ViewAllHorizontalExpandLower    = 0x00000001,
    ViewAllHorizontalExpandUpper    = 0x00000002,
    ViewAllHorizontalExpand         = ViewAllHorizontalExpandLower | ViewAllHorizontalExpandUpper,

    ViewAllHorizontalContractLower  = 0x00000004,
    ViewAllHorizontalContractUpper  = 0x00000008,
    ViewAllHorizontalContract       = ViewAllHorizontalContractLower | ViewAllHorizontalContractUpper,

    ViewAllHorizontalAdjustLower    = ViewAllHorizontalExpandLower | ViewAllHorizontalContractLower,
    ViewAllHorizontalAdjustUpper    = ViewAllHorizontalExpandUpper | ViewAllHorizontalContractUpper,
    ViewAllHorizontalAdjust         = ViewAllHorizontalExpand | ViewAllHorizontalContract,

    ViewAllHorizontalZeroLower      = 0x00000010,
    ViewAllHorizontalZeroUpper      = 0x00000020,

    ViewAllHorizontalLowerMask      = ViewAllHorizontalExpandLower | ViewAllHorizontalContractLower | ViewAllHorizontalZeroLower,
    ViewAllHorizontalUpperMask      = ViewAllHorizontalExpandUpper | ViewAllHorizontalContractUpper | ViewAllHorizontalZeroUpper,
    ViewAllHorizontalMask           = ViewAllHorizontalLowerMask | ViewAllHorizontalUpperMask,

    ViewAllHorizontalMarginLower    = 0x00000040,
    ViewAllHorizontalMarginUpper    = 0x00000080,
    ViewAllHorizontalMargin         = ViewAllHorizontalMarginLower | ViewAllHorizontalMarginUpper,

    // Vertical
    ViewAllVerticalExpandLower      = 0x00000100,
    ViewAllVerticalExpandUpper      = 0x00000200,
    ViewAllVerticalExpand           = ViewAllVerticalExpandLower | ViewAllVerticalExpandUpper,

    ViewAllVerticalContractLower    = 0x00000400,
    ViewAllVerticalContractUpper    = 0x00000800,
    ViewAllVerticalContract         = ViewAllVerticalContractLower | ViewAllVerticalContractUpper,
    
    ViewAllVerticalAdjustLower      = ViewAllVerticalExpandLower | ViewAllVerticalContractLower,
    ViewAllVerticalAdjustUpper      = ViewAllVerticalExpandUpper | ViewAllVerticalContractUpper,
    ViewAllVerticalAdjust           = ViewAllVerticalExpand | ViewAllVerticalContract,

    ViewAllVerticalZeroLower        = 0x00001000,
    ViewAllVerticalZeroUpper        = 0x00002000,

    ViewAllVerticalLowerMask        = ViewAllVerticalExpandLower | ViewAllVerticalContractLower | ViewAllVerticalZeroLower,
    ViewAllVerticalUpperMask        = ViewAllVerticalExpandUpper | ViewAllVerticalContractUpper | ViewAllVerticalZeroUpper,
    ViewAllVerticalMask             = ViewAllVerticalLowerMask | ViewAllVerticalUpperMask,

    ViewAllVerticalMarginLower      = 0x00004000,
    ViewAllVerticalMarginUpper      = 0x00008000,
    ViewAllVerticalMargin           = ViewAllVerticalMarginLower | ViewAllVerticalMarginUpper,

    // Combined
    ViewAllExpandAll                = ViewAllHorizontalExpand | ViewAllVerticalExpand,
    ViewAllContractAll              = ViewAllHorizontalContract | ViewAllVerticalContract,
    ViewAllMarginAll                = ViewAllHorizontalMargin | ViewAllVerticalMargin,

    ViewAllAdjustAll                = ViewAllExpandAll | ViewAllContractAll,

    ViewAllKeepOriginalRatio        = 0x00010000,

    ViewAllForce                    = 0x00020000,
    ViewAllUser                     = 0x00400000,
    ViewAllImmediately              = 0x00800000,
    ViewAllNewAxes                  = 0x01000000,
    
    ViewAllDefault                  = ViewAllAdjustAll | ViewAllMarginAll,
  };

  enum BoundProperties
  {
    BoundPropertiesNone                 = 0,

    BoundPropertiesLockAxes             = 0x00000001,
    BoundPropertiesBlockAxesInteraction = 0x00000002,

    BoundPropertiesAll                  = BoundPropertiesLockAxes
                                        | BoundPropertiesBlockAxesInteraction
  };

/*!
  Defines the different units in which the image resolution can be specified in the export
  functions.

  \see GraphicsPlot::savePng, GraphicsPlot::saveJpg, GraphicsPlot::saveBmp, GraphicsPlot::saveRastered
*/
enum ResolutionUnit { ruDotsPerMeter       ///< Resolution is given in dots per meter (dpm)
                      ,ruDotsPerCentimeter ///< Resolution is given in dots per centimeter (dpcm)
                      ,ruDotsPerInch       ///< Resolution is given in dots per inch (DPI/PPI)
                    };

/*!
  Defines how cosmetic pens (pens with numerical width 0) are handled during export.

  \see GraphicsPlot::savePdf
*/
enum ExportPen { epNoCosmetic     ///< Cosmetic pens are converted to pens with pixel width 1 when exporting
                 ,epAllowCosmetic ///< Cosmetic pens are exported normally (e.g. in PDF exports, cosmetic pens always appear as 1 pixel on screen, independent of viewer zoom level)
               };

/*!
  Represents negative and positive sign domain, e.g. for passing to \ref
  GPAbstractPlottable::getKeyRange and \ref GPAbstractPlottable::getValueRange.

  This is primarily needed when working with logarithmic axis scales, since only one of the sign
  domains can be visible at a time.
*/
enum SignDomain { sdNegative  ///< The negative sign domain, i.e. numbers smaller than zero
                  ,sdBoth     ///< Both sign domains, including zero, i.e. all numbers
                  ,sdPositive ///< The positive sign domain, i.e. numbers greater than zero
                };

/*!
  Defines the sides of a rectangular entity to which margins can be applied.

  \see GPLayoutElement::setAutoMargins, GPAxisRect::setAutoMargins
*/
enum MarginSide { msLeft     = 0x01 ///< <tt>0x01</tt> left margin
                  ,msRight   = 0x02 ///< <tt>0x02</tt> right margin
                  ,msTop     = 0x04 ///< <tt>0x04</tt> top margin
                  ,msBottom  = 0x08 ///< <tt>0x08</tt> bottom margin
                  ,msAll     = 0xFF ///< <tt>0xFF</tt> all margins
                  ,msNone    = 0x00 ///< <tt>0x00</tt> no margin
                };
Q_DECLARE_FLAGS(MarginSides, MarginSide)

/*!
  Defines what objects of a plot can be forcibly drawn antialiased/not antialiased. If an object is
  neither forcibly drawn antialiased nor forcibly drawn not antialiased, it is up to the respective
  element how it is drawn. Typically it provides a \a setAntialiased function for this.

  \c AntialiasedElements is a flag of or-combined elements of this enum type.

  \see GraphicsPlot::setAntialiasedElements, GraphicsPlot::setNotAntialiasedElements
*/
enum AntialiasedElement { aeAxes           = 0x0001 ///< <tt>0x0001</tt> Axis base line and tick marks
                          ,aeGrid          = 0x0002 ///< <tt>0x0002</tt> Grid lines
                          ,aeSubGrid       = 0x0004 ///< <tt>0x0004</tt> Sub grid lines
                          ,aeLegend        = 0x0008 ///< <tt>0x0008</tt> Legend box
                          ,aeLegendItems   = 0x0010 ///< <tt>0x0010</tt> Legend items
                          ,aePlottables    = 0x0020 ///< <tt>0x0020</tt> Main lines of plottables
                          ,aeItems         = 0x0040 ///< <tt>0x0040</tt> Main lines of items
                          ,aeScatters      = 0x0080 ///< <tt>0x0080</tt> Scatter symbols of plottables (excluding scatter symbols of type ssPixmap)
                          ,aeFills         = 0x0100 ///< <tt>0x0100</tt> Borders of fills (e.g. under or between graphs)
                          ,aeZeroLine      = 0x0200 ///< <tt>0x0200</tt> Zero-lines, see \ref GPGrid::setZeroLinePen
                          ,aeOther         = 0x8000 ///< <tt>0x8000</tt> Other elements that don't fit into any of the existing categories
                          ,aeAll           = 0xFFFF ///< <tt>0xFFFF</tt> All elements
                          ,aeNone          = 0x0000 ///< <tt>0x0000</tt> No elements
                        };
Q_DECLARE_FLAGS(AntialiasedElements, AntialiasedElement)

/*!
  Defines plotting hints that control various aspects of the quality and speed of plotting.

  \see GraphicsPlot::setPlottingHints
*/
enum PlottingHint { phNone              = 0x000 ///< <tt>0x000</tt> No hints are set
                    ,phFastPolylines    = 0x001 ///< <tt>0x001</tt> Graph/Curve lines are drawn with a faster method. This reduces the quality especially of the line segment
                                                ///<                joins, thus is most effective for pen sizes larger than 1. It is only used for solid line pens.
                    ,phImmediateRefresh = 0x002 ///< <tt>0x002</tt> causes an immediate repaint() instead of a soft update() when GraphicsPlot::replot() is called with parameter \ref GraphicsPlot::rpRefreshHint.
                                                ///<                This is set by default to prevent the plot from freezing on fast consecutive replots (e.g. user drags ranges with mouse).
                    ,phCacheLabels      = 0x004 ///< <tt>0x004</tt> axis (tick) labels will be cached as pixmaps, increasing replot performance.
                  };
Q_DECLARE_FLAGS(PlottingHints, PlottingHint)

/*!
  Defines the mouse interactions possible with GraphicsPlot.

  \c Interactions is a flag of or-combined elements of this enum type.

  \see GraphicsPlot::setInteractions
*/
enum Interaction { iRangeDrag         = 0x001 ///< <tt>0x001</tt> Axis ranges are draggable (see \ref GPAxisRect::setRangeDrag, \ref GPAxisRect::setRangeDragAxes)
                   ,iRangeZoom        = 0x002 ///< <tt>0x002</tt> Axis ranges are zoomable with the mouse wheel (see \ref GPAxisRect::setRangeZoom, \ref GPAxisRect::setRangeZoomAxes)
                   ,iMultiSelect      = 0x004 ///< <tt>0x004</tt> The user can select multiple objects by holding the modifier set by \ref GraphicsPlot::setMultiSelectModifier while clicking
                   ,iSelectPlottables = 0x008 ///< <tt>0x008</tt> Plottables are selectable (e.g. graphs, curves, bars,... see GPAbstractPlottable)
                   ,iSelectAxes       = 0x010 ///< <tt>0x010</tt> Axes are selectable (or parts of them, see GPAxis::setSelectableParts)
                   ,iSelectLegend     = 0x020 ///< <tt>0x020</tt> Legends are selectable (or their child items, see GPLegend::setSelectableParts)
                   ,iSelectItems      = 0x040 ///< <tt>0x040</tt> Items are selectable (Rectangles, Arrows, Textitems, etc. see \ref GPAbstractItem)
                   ,iSelectOther      = 0x080 ///< <tt>0x080</tt> All other objects are selectable (e.g. your own derived layerables, other layout elements,...)
                 };
Q_DECLARE_FLAGS(Interactions, Interaction)

/*!
  Defines the behaviour of the selection rect.

  \see GraphicsPlot::setSelectionRectMode, GraphicsPlot::selectionRect, GPSelectionRect
*/
enum SelectionRectMode { srmNone    ///< The selection rect is disabled, and all mouse events are forwarded to the underlying objects, e.g. for axis range dragging
                         ,srmZoom   ///< When dragging the mouse, a selection rect becomes active. Upon releasing, the axes that are currently set as range zoom axes (\ref GPAxisRect::setRangeZoomAxes) will have their ranges zoomed accordingly.
                         ,srmSelect ///< When dragging the mouse, a selection rect becomes active. Upon releasing, plottable data points that were within the selection rect are selected, if the plottable's selectability setting permits. (See  \ref dataselection "data selection mechanism" for details.)
                         ,srmCustom ///< When dragging the mouse, a selection rect becomes active. It is the programmer's responsibility to connect according slots to the selection rect's signals (e.g. \ref GPSelectionRect::accepted) in order to process the user interaction.
                       };

/*!
  Defines the different ways a plottable can be selected. These images show the effect of the
  different selection types, when the indicated selection rect was dragged:

  <center>
  <table>
  <tr>
    <td>\image html selectiontype-none.png stNone</td>
    <td>\image html selectiontype-whole.png stWhole</td>
    <td>\image html selectiontype-singledata.png stSingleData</td>
    <td>\image html selectiontype-datarange.png stDataRange</td>
    <td>\image html selectiontype-multipledataranges.png stMultipleDataRanges</td>
  </tr>
  </table>
  </center>

  \see GPAbstractPlottable::setSelectable, GPDataSelection::enforceType
*/
enum SelectionType { stNone                ///< The plottable is not selectable
                     ,stWhole              ///< Selection behaves like \ref stMultipleDataRanges, but if there are any data points selected, the entire plottable is drawn as selected.
                     ,stSingleData         ///< One individual data point can be selected at a time
                     ,stDataRange          ///< Multiple contiguous data points (a data range) can be selected
                     ,stMultipleDataRanges ///< Any combination of data points/ranges can be selected
                    };

/*! \internal

  Returns whether the specified \a value is considered an invalid data value for plottables (i.e.
  is \e nan or \e +/-inf). This function is used to check data validity upon replots, when the
  compiler flag \c GRAPHICSPLOT_CHECK_DATA is set.
*/
static bool isInvalidData(double value)
{
  return qIsNaN(value) || qIsInf(value);
}

/*! \internal
  \overload

  Checks two arguments instead of one.
*/
static bool isInvalidData(double value1, double value2)
{
  return isInvalidData(value1) || isInvalidData(value2);
}

/*! \internal

  Sets the specified \a side of \a margins to \a value

  \see getMarginValue
*/
static void setMarginValue(QMargins &margins, GP::MarginSide side, int value)
{
  switch (side)
  {
    case GP::msLeft: margins.setLeft(value); break;
    case GP::msRight: margins.setRight(value); break;
    case GP::msTop: margins.setTop(value); break;
    case GP::msBottom: margins.setBottom(value); break;
    case GP::msAll: margins = QMargins(value, value, value, value); break;
    default: break;
  }
}

/*! \internal

  Returns the value of the specified \a side of \a margins. If \a side is \ref GP::msNone or
  \ref GP::msAll, returns 0.

  \see setMarginValue
*/
static int getMarginValue(const QMargins &margins, GP::MarginSide side)
{
  switch (side)
  {
    case GP::msLeft: return margins.left();
    case GP::msRight: return margins.right();
    case GP::msTop: return margins.top();
    case GP::msBottom: return margins.bottom();
    default: break;
  }
  return 0;
}


//// extern const QMetaObject staticMetaObject; // in moc-run we create a static meta object for GP "fake" object. This line is the link to it via GP::staticMetaObject in normal operation as namespace

}; // end of namespace GP

Q_DECLARE_OPERATORS_FOR_FLAGS(GP::AntialiasedElements)
Q_DECLARE_OPERATORS_FOR_FLAGS(GP::PlottingHints)
Q_DECLARE_OPERATORS_FOR_FLAGS(GP::MarginSides)
Q_DECLARE_OPERATORS_FOR_FLAGS(GP::Interactions)
Q_DECLARE_METATYPE(GP::ExportPen)
Q_DECLARE_METATYPE(GP::ResolutionUnit)
Q_DECLARE_METATYPE(GP::SignDomain)
Q_DECLARE_METATYPE(GP::MarginSide)
Q_DECLARE_METATYPE(GP::AntialiasedElement)
Q_DECLARE_METATYPE(GP::PlottingHint)
Q_DECLARE_METATYPE(GP::Interaction)
Q_DECLARE_METATYPE(GP::SelectionRectMode)
Q_DECLARE_METATYPE(GP::SelectionType)
