#pragma once

#include "graphicsplot.h"

#include <QMap>
#include <map>
#include <set>
#include <functional>

class QDateTimeEdit;

class GraphicsPlotExtended;
class GPAxisAwareTicker;
class GPAxisContainer;
class GPPolygon;
class GPPolygonGroup;
class GPPolygonLayer;
class GPMagnifier;
class GPInteractiveCurve;
class GPAxisInfiniteLine;
class GPEXObliqueInfiniteLine;
class GPRulerLine;
class GPRulerDistance;
class GPCaptionItem;
class GPZonesRect;
class GPEXLegend;
class GPTextLegendItem;
class GPCompassItem;

//// namespace GP {
//// }

class GPEXDraggingInterface
{
public:
  GPEXDraggingInterface() : mDraggingPart(0), mInteractive(true) {}

  virtual void DragStart(QMouseEvent* e) { DragStart(e, 0); }
  virtual void DragStart(QMouseEvent*, unsigned) {}
  virtual void DragMove(QMouseEvent*) {}
  virtual void DragAccept(QMouseEvent*) {}
  virtual void DragUpdateData(QMouseEvent*) {}
  virtual bool IsDragging() const { return mDraggingPart != 0; }

  virtual void setInteractive(bool interactive) { mInteractive = interactive; }
  virtual bool isInteractive() { return mInteractive; }

protected:
  unsigned mDraggingPart;
  bool mInteractive;
};

namespace GPShortcutKeys
{
  static const int SKViewAll              = (int)Qt::CTRL + (int)Qt::Key_A;
  static const int SKCopyImage            = (int)Qt::CTRL + (int)Qt::Key_C;
  static const int SKCopyImageWithCoords  = (int)Qt::CTRL + (int)Qt::ALT + (int)Qt::Key_C;
  static const int SKExportExcel          = (int)Qt::CTRL + (int)Qt::Key_E;
  static const int SKOpenSettings         = (int)Qt::CTRL + (int)Qt::Key_P;
}

namespace GPObjectsKeys
{
  static const char* OKProjectId = "ProjectId";
  static const char* OKPlotId = "PlotId";
  static const char* OKDataId = "DataId";

  static const char* OKCurveId = "CurveId";
  static const char* OKAxisId = "AxisId";
  static const char* OKLegendId = "LegendId";
}

namespace GPPropertiesKeys
{
  static const char* PKName = "Name";

  static const char* PKDefaultColor = "DefaultColor";
  static const char* PKDefaultFont = "DefaultFont";
  static const char* PKDefaultPenStyle = "DefaultStyle";
  static const char* PKDefaultPenWidth = "DefaultWidth";
  static const char* PKDefaultBrushColor = "DefaultBrushColor";
  static const char* PKDefaultBrushStyle = "DefaultBrushStyle";

  static const char* PKDefaultScatterShape = "DefaultScatterShape";
  static const char* PKDefaultScatterSize = "DefaultScatterSize";
  static const char* PKDefaultScatterColor = "DefaultScatterColor";
  static const char* PKDefaultScatterPenWidth = "DefaultScatterPwenWidth";
  static const char* PKDefaultScatterBrush = "DefaultScatterBrush";

  static const char* PKDefaultVisibility = "DefaultVisibility";
  static const char* PKDefaultText = "DefaultText";

  static const char* PKAxisDisableLogView = "AxisDisableLogView";
}

namespace GPJsonKeys
{
  static const char* JKUom         = "uom";
  static const char* JKUomKeys     = "uom_keys";
  static const char* JKUomValues   = "uom_values";
  static const char* JKUomQuantity = "uom_quantity";
  static const char* JKUomUnits    = "uom_units";

  static const char* JKAxisType = "axis_type";
  static const char* JKAxisScaleType = "axis_scale_type";
  static const char* JKAxisRangeLower = "range_lower";
  static const char* JKAxisRangeUpper = "range_upper";
  static const char* JKAxisTickerStep = "ticker_step";

  static const char* JKPenColor = "pen_color";
  static const char* JKPenWidth = "pen_width";
  static const char* JKPenStyle = "pen_style";
  static const char* JKBrushColor = "brush_color";
  static const char* JKBrushStyle = "brush_style";

  static const char* JKScatterPenColor = "scatter_pen_color";
  static const char* JKScatterPointSize = "scatter_point_size";
  static const char* JKScatterPointStyle = "scatter_point_style";
  static const char* JKScatterBrushColor = "scatter_brush_color";
  static const char* JKScatterBrushStyle = "scatter_brush_style";

  static const char* JKCaptionHasLine = "has_line";
  static const char* JKCaptionPositionX = "pos_x";
  static const char* JKCaptionPositionY = "pos_y";
  static const char* JKCaptionPointPositionX = "point_pos_x";
  static const char* JKCaptionPointPositionY = "point_pos_y";
  static const char* JKCaptionParentType = "parent_type";
  static const char* JKCaptionParentId = "parent_id";

  static const char* JKZonesColors = "colors";
  static const char* JKZonesLines = "lines";
  static const char* JKZonesVisibility = "zones_visibility";
  static const char* JKZonesNames = "zones_names";
  static const char* JKZonesStyle = "style";
  static const char* JKZonesRangeMin = "range_min";
  static const char* JKZonesRangeMax = "zones_max";
  static const char* JKZonesInteractive = "interactive";
  static const char* JKZonesAmountSettings = "amount_settings";

  static const char* JKFont = "font";
  static const char* JKFontColor = "font_color";

  static const char* JKId = "id";
  static const char* JKVisibility = "visible";
  static const char* JKPositionIndex = "position_index";
  static const char* JKText = "text";

  static const char* JKVAlign = "v_align";
  static const char* JKHAlign = "h_align";
}

namespace GPDefaultColors
{
  static const QColor DCInsertingRect = QColor(97, 221, 97);
  static const QColor DCInsertingPosition = QColor(97, 221, 97);
  static const QColor DCSelectionRect = QColor(0, 191, 255);
}

namespace GraphicsPlotExtendedUtils
{
  ////////////////////////////////////////////////////////
  //// Interpolation functor
  class Interpolation
  {
  public:
    Interpolation() = default;
    Interpolation(const std::vector<double>& x, const std::vector<double>& y);
    Interpolation(const QVector<double>& x, QVector<double>& y);

    Interpolation(const Interpolation& other) = default;
    Interpolation& operator=(const Interpolation& other) = default;

    virtual ~Interpolation();

    bool IsEmpty() const;

    double interp1d(double) const;
    double interp1dWithNaN(double) const;

    double derivative(double, double) const;

  private:
    int searchId(double) const;

    std::vector<double> x;
    std::vector<double> y;
    std::vector<double> koeff0;
    std::vector<double> koeff1;
  };

  std::vector<double> operator* (const std::vector<double>& v1, const std::vector<double>& v2);

  std::vector<double> accumulateVector(const std::vector<double>& vect);
  std::vector<double> diffVector(const std::vector<double>& vect);

  Interpolation getAccumulatedDataInterpolation(const std::vector<double>& data, const std::vector<double>& time);
  double getRangeAmount(const Interpolation &dataInterp, double lhs, double rhs);
  double getAverageProperty(const Interpolation& dataInterp, double lhs, double rhs);

  QVector<double> getCurveKeyData(GPCurve* curve);
  std::vector<double> getCurveKeyDataStd(GPCurve* curve);

  void setContainerData(const QSharedPointer<GPDataContainer<GPCurveData>>& c
                        , const std::vector<double>& t
                        , const std::vector<double>& v
                        , int start = -1
                        , int end = -1);

  double interpolate(const QSharedPointer<GPDataContainer<GPCurveData>>& c, const double& t, bool withNaN = false);
  double interpolate(const QSharedPointer<GPDataContainer<GPGraphData>>& c, const double& t, bool withNaN = false);
  double interpolate(const std::vector<double> &time, const std::vector<double> &value, const double &t, bool withNaN = false);

  template<typename T> double interpolate(const QSharedPointer<T>& c, const double& t, bool withNaN = false)
  {
    if (!c->isEmpty())
    {
      auto right = std::lower_bound(c->begin(), c->end(), t, [](const typename T::data_type& el, const double& v) {
        return el.key < v || qIsNaN(el.key);
      });

      if (right == c->begin())
      {
        if (qIsNaN(right->key) || fabs(t - right->key) < std::numeric_limits<double>::epsilon())
          return right->value;
        return withNaN ? qQNaN() : right->value;
      }

      if (right != c->end())
      {
        if (fabs(right->key - t) < std::numeric_limits<double>::epsilon())
          return right->value;

        auto left = right - 1;
        if (qIsNaN(right->value) || qIsNaN(left->value))
          return qQNaN();

        return left->value + (t - left->key) * (right->value - left->value) / (right->key - left->key);
      }

      if (qIsNaN(c->last()->key) || fabs(t - c->last()->key) < std::numeric_limits<double>::epsilon())
        return c->last()->value;
      return withNaN ? qQNaN() : c->last()->value;
    }
    return withNaN ? qQNaN() : 0.0;
  }
}

namespace GPSettings {
  QVariantMap getSettings(QObject* obj);
  void setSettings(QObject*obj, QVariantMap &settings);

  QVariantMap getSettings(GPAxis* axis);
  void setSettings(GPAxis* axis, QVariantMap &settings);

  QVariantMap getSettings(GPCurve* curve);
  void setSettings(GPCurve* curve, QVariantMap &settings);

  QVariantMap getSettings(GPLegend* legend);
  void setSettings(GPLegend* legend, QVariantMap &settings);

  QVariantMap getSettings(GPTextElement* textElem);
  void setSettings(GPTextElement* textElem, QVariantMap &settings);

  QVariantMap getSettings(GPCaptionItem* caption);
  void setSettings(GPCaptionItem* caption, QVariantMap &settings);

  QVariantMap getSettings(GPZonesRect* zones);
  void setSettings(GPZonesRect* zones, QVariantMap &settings);
}

struct GraphicsPlotExtendedStyle
{
  GraphicsPlotExtendedStyle();

  static QMap<int, QColor> GetDefaultColorMap();
  static QColor GetDefaultColor(int index);
  static QString GetDefaultColorName(int index);

  static void SetDefaultLegendStyle(GPLegend* legend);
  static void SetDefaultAxisStyle(GPAxis* axis);

  // Curve styles
  double DefaultPenWidth;

  // Captions style
  QFont CaptionsFont;
  QColor CaptionsColor;

private:
  static QMap<int, QColor> DefaultColorMap;

  friend class GraphicsPlotExtended;
};

///////////////////////////////////////////////////////
// GraphicsPlotExtended
class GraphicsPlotExtended : public GraphicsPlot
{
  Q_OBJECT

  friend QVariantMap getSettings(GraphicsPlotExtended*);
  friend void setSettings(GraphicsPlotExtended*, QVariantMap &);

public:
  enum PlotSettingsTabFlags
  {
    EnableAxesTab = 1,
    EnableCurvesTab = 2,
    EnableLegendTab = 4,
    EnableResetButton = 8,
    EnableAllTabs = EnableAxesTab | EnableCurvesTab | EnableLegendTab | EnableResetButton
  };

  explicit GraphicsPlotExtended(QWidget* parent = nullptr, unsigned flags = GP::AADefault);
  virtual ~GraphicsPlotExtended();

  void setViewAllDefaultFlags(unsigned flags);
  unsigned getViewAllDefaultFlags() const;

public:
// Flags setters/getters
  void setFlags(const unsigned &flags);
  unsigned getFlags() const;

  void setFlag(const unsigned &flag, bool active = true);
  bool testFlag(const unsigned &flag) const;

  void setPlotSettingsFlags(const unsigned &flags);
  unsigned plotSettingsFlag();

// Style
  GraphicsPlotExtendedStyle& getStyle() const;

// View helpers
  virtual void SetAxisGridVisible(GPAxis* axis, bool visible);
  virtual void SetAxisListGridVisible(const QList<GPAxis*>& axes, bool visible);
  virtual void SetAxisScaleType(GPAxis* axis, GPAxis::ScaleType scaleType);

// Associating methods
  void AddAssociatedPlot(GraphicsPlotExtended* plot);

  static void bindAxes(GPAxis* axis1, GPAxis* axis2);
  static QSet<GPAxis*> getBoundAxes(GPAxis* axis);

  static void bindPlots(GraphicsPlotExtended* plot1, GraphicsPlotExtended* plot2, GP::BoundProperties bp = GP::BoundPropertiesAll);
  static QMap<GraphicsPlotExtended*, GP::BoundProperties> getBoundPlotsMap(GraphicsPlotExtended* plot);
  static QSet<GraphicsPlotExtended*> getBoundPlots(GraphicsPlotExtended* plot);

// Axes interface
  Qt::Orientation getKeyAxisOrientation() const;
  void setKeyAxisOrientation(const Qt::Orientation &value);

  GPAxis* addAxis(Qt::Orientation orientation, const QString& label = QString(), const QString& id = QString());
  GPAxis* addAxis(GPAxis::AxisType axisType, const QString& label = QString(), const QString& id = QString());
  GPAxis* addAxis(GPAxis::AxisType axisType, GPAxis* axis, const QString& label = QString(), const QString& id = QString());
  GPAxis* addAxis(GPAxisRect* rect, GPAxis::AxisType axisType, GPAxis* axis = nullptr, const QString& label = QString(), const QString& id = QString());

  virtual void moveAxis(GPAxisRect* rect, GPAxis* axis, GPAxis::AxisType to) override;

  QList<GPAxis *> GetAllAxes();
  QList<QPointer<GPAxis>> GetAllZoomableAxes(GPAxisRect* parent);
  QList<QPointer<GPAxis>> GetAllDraggableAxes();
  QList<GPAxis*> GetAllHorizontalAxes();
  QList<GPAxis*> GetAllVerticalAxes();
  QList<GPAxis*> GetHorizontalAxesWithGrid();
  QList<GPAxis*> GetVerticalAxesWithGrid();
  bool IsZoomable(GPAxis* axis);
  bool IsDraggable(GPAxis* axis);

  void SetAxisInteraction(GPAxis* axis, GPAxisRect* rect, QFlags<GP::Interaction> iteraction);
  QFlags<GP::Interaction> GetAxisInteraction(GPAxis* axis);

// Curves interface
  virtual GPCurve* addCurve(int index, GPAxis* keyAxis, GPAxis* valueAxis);
  virtual GPCurve* addCurve(GPAxis* keyAxis = nullptr, GPAxis* valueAxis = nullptr) override;
  virtual GPCurve* addCurve(int index);

// Subitems interface
  GPPolygonGroup*   addPolygonGroup(GPAxis* keyAxis = 0, GPAxis* valueAxis = 0);
  void               removePolygonGroup(GPPolygonGroup* group);
  GPZonesRect*      addZonesRect(GPAxis* keyAxis = nullptr);
  GPCaptionItem*    addCaptionItem(GPLayerable* target, const QString& text, bool hasLine, QPoint pos);

  void SetRulerShowAxesLabels(bool show);
  bool GetRulerShowAxesLabels() const;

// A faster method than inherited
  void fastClearPlottables();

public slots:
#if defined(USE_XLNT)
  void ExportToExcel();
#endif

  void SaveImage();
  void CopyImage();

  void ShowSettingsDialog();

  // External axis binding
  void setLockAxes(bool lock);
  bool getLockAxes() const;

  void setLockAxesInteraction(bool lock);
  bool getLockAxesInteraction() const;

  void setTitleText(const QString& text);
  QString getTitleText() const;
  GPTextElement* getTitle();

  GPEXLegend* legendEx() const;
  void setLegendVisible(GPLegend* legend, bool visible);
  bool getLegendVisible(GPLegend* legend) const;
  void setLegendPosition(GPLegend* legend, Qt::Alignment align);
  Qt::Alignment getLegendPosition(GPLegend* legend) const;

  void AxisReseted();
  void CurvesUpdated();
  void queuedReplot();

  virtual void viewAll(const QSet<GPAxis*>& affectedAxes, unsigned flags);
  Q_INVOKABLE virtual void viewAll(unsigned flags);
  virtual void viewAll(bool forceViewAll = false);
  virtual void viewAll(const QSet<GPAxis*>& affectedAxes, bool forceViewAll = true);

  void processPointSelection(QMouseEvent* event) override;

signals:
  void axesLockChanged(bool);
  void axesInteractionChanged(bool);
  void viewportResized();

  ///////////////////////////////////////////////////////
  // save/load
public slots:
  virtual void LoadSettings();
  virtual void SaveSettings();
  virtual void ResetSettings();

  virtual bool event(QEvent *event) override;

public slots:
  virtual QString GetObjectKey(QObject* obj);

  static void setDefaultSettings(QObject* obj, const QColor &color, const QFont &font);
  static void setDefaultSettings(QObject* obj, const QColor &color);
  static void setDefaultSettings(QObject* obj, const QFont &font);

  static void setDefaultSettings(GPCurve* curve, const QPen &pen, const QBrush &brush, const GPScatterStyle &scatter);
  static void setDefaultSettings(GPCurve* curve, const QPen &pen);
  static void setDefaultSettings(GPCurve* curve, const QBrush &brush);
  static void setDefaultSettings(GPCurve* curve, const GPScatterStyle &scatter);

  static void setDefaultSettings(GPTextElement* textElem, const QString& text);

  virtual void LoadSettings(GPCurve* curve);
  virtual void LoadSettings(GPAxis *axis);
  virtual void LoadSettings(GPLegend *legend);

  virtual void SaveAxes();
  virtual void LoadAxes();
  virtual void LoadAxis(GPAxis* axis);
  virtual void LoadAxisDefault(GPAxis* axis);

  virtual void SaveCurves();
  virtual void LoadCurves();
  virtual void LoadCurve(GPCurve* curve);
  virtual void LoadCurveDefault(GPCurve* curve);

  virtual void SaveLegends();
  virtual void LoadLegends();
  virtual void LoadLegend(GPLegend* legend);
  virtual void LoadLegendDefault(GPLegend* legend);

  virtual void SaveTitle();
  virtual void LoadTitle();
  virtual void LoadTitleDefault();

  virtual void SaveRulers();
  virtual void LoadRulers();

  virtual void SaveCaptions();
  virtual void LoadCaptions();

  virtual void SaveZones();
  virtual void LoadZones();

  virtual void LoadObjects();
  virtual void SaveObjects();

protected:
  virtual void SetSettings(const QUuid& projectId, const QString& key, const QVariant &settings);
  virtual QVariant GetSettings(const QUuid& projectId, const QString& key, const QVariant &defaultValue = QVariantMap());

  virtual bool LoadObjectSettings(const QUuid& projectId, QObject* obj);
  virtual void SaveObjectSettings(const QUuid& projectId, QObject* obj);

protected slots:
  void OnAssociatedPlotChanged();

  void AddRuler(QPoint pos);
  void RemoveRuler();
  void RemoveRulers();

  void SetMeasuringPolygonDraw(bool active);
  void AddMeasuringPolygonPoint(QPoint mousePos);

  void SetMagnifierDraw(bool active);

protected:
  virtual void ViewAllAxis(GPAxis* ax, GPRange &range, unsigned flags = GP::ViewAllDefault);

  virtual void axisRemoved(GPAxis *axis) override;

  static void OnBoundAxisRemoved(QObject* obj);
  static void OnBoundPlotsRemoved(QObject* obj);

  virtual void CreateShortcuts();
  virtual void FillContextMenu(QContextMenuEvent* event, QMenu& menu, std::map<QAction*, std::function<void(void)>>& actions);
  virtual void CreateGridActions(QContextMenuEvent* event, QMenu& menu, std::map<QAction*, std::function<void(void)>>& actions);
  virtual QWidget* CreateAxisRangeActions(QWidget* parent, GPAxis* axis);
  virtual QWidget* CreateAxisTickerActions(QWidget *parent, GPAxis* axis);

  ///////////////////////////////////////////////////////
  // Widget events
  virtual void mousePressEvent(QMouseEvent* event) override;
  virtual void mouseDoubleClickEvent(QMouseEvent *event) override;
  virtual void mouseMoveEvent(QMouseEvent* event) override;
  virtual void mouseReleaseEvent(QMouseEvent* event) override;

  virtual void resizeEvent(QResizeEvent *event) override;
  virtual void wheelEvent(QWheelEvent* event) override;
  virtual void showEvent(QShowEvent* event) override;
  virtual void contextMenuEvent(QContextMenuEvent* event) override;

  virtual bool eventFilter(QObject* object, QEvent* event) override;
  virtual Qt::CursorShape GetCursorAt(const QPointF& pos);

protected:
  Qt::Orientation KeyAxisOrientation;
  GraphicsPlotExtendedStyle Style;
  unsigned Flags;
  unsigned PlotSettingsFlags;
  unsigned ViewAllDefaultFlags;

  GPTextElement* Title;

  // Menu
  QPointer<QMenu>           ContextMenu;

  // Zooming
  QList<QPointer<GPAxis>>  ZoomingAxis;
  QMap<GPAxis*, GPRange>  ZoomingAxisStartRangeX;
  QMap<GPAxis*, GPRange>  ZoomingAxisStartRangeY;
  QMap<GPAxis*, double>    ZoomingAxisStartCenterX;
  QMap<GPAxis*, double>    ZoomingAxisStartCenterY;
  bool                      ZoomingAxisMoved;
  QPoint                    ZoomingAxisStartPoint;
  QPoint                    GestureTouchMousePressPos;

  // Axis interaction
  QMap<GPAxis*, QFlags<GP::Interaction>>  AxisInteraction;
  QList<GPAxis*>                   Axes;

  // Bound axis
  static QMap<GPAxis*, QSet<GPAxis*>>                 BoundAxis;
  static QMap<GPAxis*, QList<QMetaObject::Connection>> BoundAxisConnections;
  static QMap<GraphicsPlotExtended*, QMap<GraphicsPlotExtended*, GP::BoundProperties>> BoundPlots;
  static QMap<GraphicsPlotExtended*, QList<QMetaObject::Connection>>                   BoundPlotsConnections;

  // Interactive objects
  GPEXDraggingInterface* DraggingObject;

  QPointer<GPZonesRect> ZonesRect;

  QPointer<GPCurve> MeasuringPolygon;
  QPointer<GPItemText> MeasuringPolygonLabel;

  QPointer<GPMagnifier> Magnifier;

  QList<QPointer<GPRulerLine>> LineRulers;
  QPointer<GPRulerDistance> DistanceRuler;

  QMap<QUuid, QPointer<GPCaptionItem>> Captions;

  GPPolygonLayer* PolygonLayer;
  QList<QPointer<GraphicsPlotExtended>> AssociatedPlots;

  bool LockAxes;
  bool LockAxesInteraction;
  bool PendingViewAll;
  bool DisplayCursorCoords;
  bool RulerShowAxesLabels;
  unsigned PendingViewAllFlags;
  QSet<GPAxis*> PendingViewAllAxes;

  bool InDragAndDrop;
  bool InDrawingMeasuringPolygon;
  bool InDrawingMagnifier;

  friend class PlotSettingsDialog;
  friend class ZonesEditDialog;
  friend class GPMagnifier;
  friend class GPAxisContainer;
};

typedef GraphicsPlotExtended GPPlotBinder;

///////////////////////////////////////////////////////
// Plot magnifier
class GPMagnifier : public GPLayerable
{
public:
  GPMagnifier(GraphicsPlot* parent);
  ~GPMagnifier();

  void ApplyZoom();

  virtual void applyDefaultAntialiasingHint(GPPainter* /*painter*/) const override {}
  virtual void draw(GPPainter* painter) override;
};

///////////////////////////////////////////////////////
// Simple text legend item
class GPTextLegendItem : public GPAbstractLegendItem
{
  Q_OBJECT

public:
  explicit GPTextLegendItem(GPLegend *parent, const QString& text = QString());
  ~GPTextLegendItem();

  void setText(const QString& text);
  QString getText() const;

protected:
  virtual void draw(GPPainter *painter) Q_DECL_OVERRIDE;
  virtual QSize minimumOuterSizeHint() const Q_DECL_OVERRIDE;

  QString mText;
};


///////////////////////////////////////////////////////
// legend with scrollable plottables section
class GPEXLegend : public GPLegend
{
  Q_OBJECT

public:
  explicit GPEXLegend(GraphicsPlot *parent);
  virtual ~GPEXLegend();

  bool addItem(GPAbstractLegendItem *item);
  virtual QSize minimumOuterSizeHint() const;

  void setOuterRect(const QRect &rect);
  void sizeConstraintsChanged();
  void updateLayout();
  void wheelEvent(QWheelEvent *event);

  void updateScrollAreaItems();

protected:
  void selectItem(GPPlottableLegendItem* item, Qt::KeyboardModifiers modifier);
  void setScrollBarVisible(bool visible);
  void updateScrollBarGeometry();

protected slots:
  void onScrollBarValueChanged();

protected:
  int mNeedUpdate;
  int mVisibleScrolledCount;

  QScrollBar* mScrollBar;
  const int mScrollBarWidth = 10;
  int mScrolledItemsSectionStartIndex;
  int mScrolledItemsSectionTop;

  QList<GPAbstractLegendItem*> mScrolledItems;
  QPointer<GPAbstractLegendItem> mLastSelectedItem;

  friend class GraphicsPlotExtended;
};


///////////////////////////////////////////////////////
// grid layout with vertical/horizontal splitters
class GPLayoutSplitterGrid : public GPLayoutGrid
{
  Q_OBJECT

public:
  explicit GPLayoutSplitterGrid(GraphicsPlot *parentPlot, Qt::Orientation orientation);
  virtual ~GPLayoutSplitterGrid();

  void setBlockLayoutUpdate(bool block);

  void setColumnStretchFactor(int column, double factor);
  void setColumnStretchFactors(const QList<double> &factors);
  void setRowStretchFactor(int row, double factor);
  void setRowStretchFactors(const QList<double> &factors);

  void swapColumns(int left, int right);
  void swapRows(int top, int bottom);

  void insertSplitter(int index);
  void removeSplitter(int index);
  void removeAllSplitters();
  virtual void resetSplitters();

  void setSplitterWidth(int width);
  void setSplitterLineWidth(int width);
  void setSplitterLineColor(const QColor& color);
  void setSplitterLineStyle(Qt::PenStyle style);

  void insertRow(int newIndex);
  void insertColumn(int newIndex);
  void removeRow(int index);
  void removeColumn(int index);

  void sizeConstraintsChanged();
  QSize minimumOuterSizeHint() const;

  void mousePressEvent(QMouseEvent *event, const QVariant &details);
  void mouseMoveEvent(QMouseEvent *event, const QPointF &startPos);
  void mouseReleaseEvent(QMouseEvent *event, const QPointF &startPos);
  void mouseDoubleClickEvent(QMouseEvent *event, const QVariant &details);

  virtual Qt::CursorShape HitTest(QMouseEvent* event, unsigned*);

  void draw(GPPainter *painter);

protected:
  virtual void moveSplitter(double delta);

  void moveSplitterIndexes(int fromIndex, int increment);
  int splitterAt(QPoint pos);
  void checkFactors();

  int getColumnMinimumWidth(int column);
  int getRowMinimumHeight(int row);

  virtual void calculateSectionSizes(QVector<int>& rowHeights, QVector<int>& colWidths);
  void updateLayout();

protected:
  Qt::Orientation mOrientation;

  QList<double> mDefaultColumnStretchFactors;
  QList<double> mDefaultRowStretchFactors;

  int mSplitterWidth;
  int mSplitterLineWidth;
  QColor mSplitterLineColor;
  Qt::PenStyle mSplitterLineStyle;

  QSet<int> mSplitters;
  QHash<int, int> mSplittersCoords;

  int mDraggingSplitterIndex;
  QList<double> mDraggingStretchFactors;
  double mDraggingSizeLeft;
  double mDraggingSizeRight;

  bool mBlockLayoutUpdate;
};

class GPEXLayoutInfiniteGrid : public GPLayoutSplitterGrid
{
  Q_OBJECT

public:
  explicit GPEXLayoutInfiniteGrid(GraphicsPlot *parentPlot, Qt::Orientation orientation);
  virtual ~GPEXLayoutInfiniteGrid();

  void setDefaultSectionSize(int size);
  int getDefaultSectionSize() const;

  virtual void resetSplitters();

  bool addElement(int row, int column, GPLayoutElement *element);

protected:
  virtual void moveSplitter(double delta);
  virtual void calculateSectionSizes(QVector<int>& rowHeights, QVector<int>& colWidths);

protected:
  bool mHasInfiniteRect;
  int mDefaultSectionSize;
};

///////////////////////////////////////////////////////
// Axis container item
class GPAxisContainer : public GPLayoutElement
{
  Q_OBJECT

public:
  explicit GPAxisContainer(GPAxis::AxisType type, GPAxisRect* parentRect = 0, bool acceptDrops = false);
  ~GPAxisContainer();

  GPAxisRect* getAxisRect() const;
  void setAxisRect(GPAxisRect* rect);
  void setAxesType(GPAxis::AxisType type);

  virtual void addAxis(GPAxis* axis);
  virtual void addAxis(int index, GPAxis* axis);

  virtual QList<QPointer<GPAxis>> axesList();
  virtual void moveAxis(GPAxis* axis);
  virtual void takeAxis(GPAxis* axis);

  void setMaximumAxes(int count);
  int getMaximumAxes() const;

  bool acceptDrops();
  int count() const;
  GPAxis* axisAt(int index);
  virtual void clear();

  virtual void update(UpdatePhase phase);

  virtual void setOuterRect(const QRect &rect);
  QSize minimumOuterSizeHint() const;
  const QRect &axisDrawRect();

  QList<GPAxis*> dropAxes(const QMimeData *mimeData, int fromIndex = 0);

  void dragEnterEvent(QDragEnterEvent *event);
  void dragMoveEvent(QDragMoveEvent *event);
  void dropEvent(QDropEvent *event);
  void dragLeaveEvent(QDragLeaveEvent *event);

  void draw(GPPainter *painter);

signals:
  void axisMovedFromRect(GPAxisRect* axisRect);

protected:
  virtual int calculateAutoMargin(GP::MarginSide side);

  void moveToTop();
  void updateSize() const;

protected:
  GPAxis::AxisType mType;
  GPAxisRect* mAxisRect;
  QRect mAxisDrawRect;
  int mMaximumAxes;
  QList<QPointer<GPAxis>> mAxes;

  int mInsertIndex;
  GPAxis* mAxisUnderDrop;
  mutable QSize mCachedSize;
};

///////////////////////////////////////////////////////
// Track axis
class GPTrackAxisPainterPrivate : public GPAxisPainterPrivate
{
public:
  explicit GPTrackAxisPainterPrivate(GraphicsPlot *parentPlot);
  virtual ~GPTrackAxisPainterPrivate();

  virtual QImage drawToBuffer(int w, int h, QFont font, const QColor& color, const QString& text) const;
  virtual QImage drawHTMLToBuffer(QFont font, const QColor& color, const QString& text) const;

  void draw(GPPainter *painter);
  int size();

  GPAxis::SelectableParts selectedParts;
  int minimumSize;
  int rangeLabelMargin;
  GPRange axisRange;
  bool labelOnTop;
  bool htmlLabel;

  bool hasCuttedLabel;
  bool hasCuttedRange;

protected:
  GPRange mOldRange;
  QImage mBottomLabelCache;
  QImage mTopLabelCache;
};

class GPTrackAxis : public GPAxis
{
  Q_OBJECT

  enum DragState
  {
    dsBlocked = 0,
    dsNone,
    dsStart,
    dsMove
  };

public:
  explicit GPTrackAxis(GPAxisRect *parent, AxisType type);
  ~GPTrackAxis();

  void setCurve(GPCurve* getCurve);
  GPCurve* getCurve() const;

  void setHasHTMLLabel(bool html);

  QString ToolTipTest(const QPointF &pos) const;

  virtual int calculateMargin();
  void draw(GPPainter *painter);

  void mousePressEvent(QMouseEvent *event, const QVariant &details);
  void mouseMoveEvent(QMouseEvent *event, const QPointF &startPos);

protected:
  QPointer<GPCurve> mCurve;
  DragState mDraggingState;
};

class GPEXTrackGrid : public GPGrid
{
  Q_OBJECT

public:
  explicit GPEXTrackGrid(GPAxisRect *parentRect, GPAxis *parentAxis);
  explicit GPEXTrackGrid(GPAxisRect *parentRect, GPAxis::AxisType keyAxisType);

  virtual bool realVisibility() const override;

  int getLinesCount();
  void setLinesCount(int count);

  virtual void draw(GPPainter *painter) Q_DECL_OVERRIDE;

  // virtual methods:
  virtual void drawGridLines(GPPainter *painter) const override;
  virtual void drawSubGridLines(GPPainter *painter) const override;

protected:
  int mValueTicksCount;
  Qt::Orientation mOrientation;
  GPAxisRect* mParentRect;
};

class GPTrackRect : public GPAxisRect
{
  Q_OBJECT

public:
  enum ActionsFlags
  {
    TRCanMove = 0x00000001,
    TRCanRemove = 0x00000002,

    TRCanDropAxis = 0x00000004,
    TROnlyOneAxis = 0x00000008,

    TRDefault = TRCanMove | TRCanRemove | TRCanDropAxis
  };

  explicit GPTrackRect(GraphicsPlot *parentPlot,
                          GPAxis::AxisType keyAxisType,
                          GPAxis::AxisType valueAxisType,
                          GPAxis* keyAxis = nullptr);
  virtual ~GPTrackRect();

  void setFlags(unsigned flags);
  unsigned getFlags() const;

  GPAxis *addAxis(GPAxis::AxisType type, GPAxis *axis = 0);
  void addAxis(int index, GPAxis *axis);
  bool takeAxis(GPAxis* axis);
  int axisCount(GPAxis::AxisType type) const;

  int margin() const;

  int getValueGridLinesCount();
  void setValueGridLinesCount(int count);

  int getKeyGridLinesCount();
  void setKeyGridLinesCount(int count);

  bool getKeyGridVisible() const;
  void setKeyGridVisible(bool visible);
  bool getValueGridVisible() const;
  void setValueGridVisible(bool visible);

  GPAxisContainer* valueAxisContainer();
  GPAxis* keyAxis() const;

  virtual GPAxisContainer* addContainer(GPAxis::AxisType type, bool acceptDrops = false);
  virtual void addContainer(GPAxis::AxisType type, GPAxisContainer* container);
  virtual GPAxisContainer* takeContainer(GPAxis::AxisType type);

  int calculateAutoMargin(GP::MarginSide side);
  const QRect& axisDrawRect(GPAxis::AxisType type) const;

  void dragEnterEvent(QDragEnterEvent *event);
  void dragMoveEvent(QDragMoveEvent *event);
  void dropEvent(QDropEvent *event);
  void dragLeaveEvent(QDragLeaveEvent *event);

protected:
  void updateContainerMaximumAxes();

protected:
  unsigned mFlags;

  QMap<GPAxis::AxisType, QPointer<GPAxisContainer>> mContainers;
  int mMargin;

  GPEXTrackGrid* mKeyGrid;
  GPEXTrackGrid* mValueGrid;

  GPAxis::AxisType mKeyAxisType;
  GPAxis::AxisType mValueAxisType;
  GPAxis* mKeyAxis;
};

///////////////////////////////////////////////////////
// Track plot
class QTracksPlot : public GraphicsPlotExtended
{
  Q_OBJECT

public:
  enum TrackManagementFlags
  {
    ExAddGrids = 0x80000000,
    ExRemoveGrids = 0x40000000,
    ExMoveGrids = 0x20000000,

    ExAddRemoveGrids = ExAddGrids | ExRemoveGrids | ExMoveGrids,

    ExAddTracks = 0x10000000,
    ExRemoveTracks = 0x08000000,
    ExMoveTracks = 0x04000000,
    ExBoundedTrackKeyAxes = 0x02000000,

    ExAddRemoveTracks = ExAddTracks | ExRemoveTracks | ExMoveTracks,

    ExAddAxes = 0x01000000,
    ExMoveAxes = 0x00800000,
    ExRemoveAxes = 0x00400000,

    ExAddRemoveAxes = ExAddAxes | ExMoveAxes | ExRemoveAxes,

    ExAutoRemoveTrackPlottables = 0x00200000,
    ExAutoRemoveTracks = 0x00100000,
    ExAutoRemoveGrids = 0x00080000,

    ExAutoRemoveAll = ExAutoRemoveTrackPlottables | ExAutoRemoveTracks | ExAutoRemoveGrids,

    ExTracksDefault = GP::AADefault | ExAddRemoveGrids | ExAddRemoveTracks | ExAddRemoveAxes | ExAutoRemoveAll | ExBoundedTrackKeyAxes
  };

  using GraphicsPlotExtended::addCurve;

  explicit QTracksPlot(QWidget* parent = nullptr, unsigned flags = ExTracksDefault);
  virtual ~QTracksPlot();

  GPLayoutSplitterGrid* gridLayout() const;

  void setTrackAxisOrientation(GPAxis::AxisType type);
  GPAxis::AxisType getTrackAxisOrientation() const;

  void setDefaultTrackRectFlags(unsigned flags);
  unsigned getDefaultTrackRectFlags() const;

  void setHasInfiniteScene();
  bool getHasInfiniteScene() const;

  void setKeyAxesDefaultName(const QString& name);

  GPLayoutSplitterGrid* addTracksGroup(int gridIndex = 0, QString title = QString());
  void removeTracksGroup(int gridIndex);
  int getTracksGroupCount() const;

  GPTrackRect* addTrackRect(int gridIndex, int rectIndex, bool fixedSize = false);
  void removeTrackRect(int gridIndex, int rectIndex);
  int getTrackRectsCount(int gridIndex);
  void setTrackFlags(int gridIndex, int rectIndex, unsigned flags);

  void removeTrackPlottables(int gridIndex, int rectIndex);
  void removeTrackGroupPlottables(int gridIndex);
  void clearPlot();

  GPTrackAxis* addTrackAxis(int gridIndex, int rectIndex, GPTrackAxis* axis = nullptr);
  GPTrackAxis* addTrackAxis(int gridIndex, int rectIndex, const QString& label, const QString& id = QString());

  using GraphicsPlotExtended::addAxis;
  GPAxis* addAxis(int gridIndex, int rectIndex, GPAxis::AxisType type, GPAxis* axis = nullptr);
  void removeAxis(GPAxis *axis);
  GPAxis* getKeyAxisForTrack(GPTrackAxis* trackAxis);
  GPAxis* getKeyAxisForGrid(int gridIndex);

  virtual GPCurve* addCurve(int index, GPTrackAxis* trackAxis);
  virtual GPCurve* addCurve(GPTrackAxis *trackAxis);

  // Grids interface
  void setValueGridLinesCount(int gridIndex, int count);
  int getValueGridLinesCount(int gridIndex);

  bool getKeyGridVisible(int gridIndex);
  void setKeyGridVisible(int gridIndex, bool visible);

  bool getValueGridVisible(int gridIndex);
  void setValueGridVisible(int gridIndex, bool visible);

  // coord to object
  int gridIndexAt(QPoint pos);
  int rectIndexAt(QPoint pos);
  int rectIndexAt(int gridIndex, QPoint pos);

  GPLayoutSplitterGrid* gridAt(QPoint pos);
  GPTrackRect* rectAt(QPoint pos);
  GPTrackRect* rectAt(int gridIndex, QPoint pos);

  // Object to index
  int getGridIndex(GPLayoutSplitterGrid* grid);
  int getRectIndex(GPLayoutSplitterGrid *grid, GPTrackRect* rect);

  // GraphicsPlotExtended methods
  virtual void FillContextMenu(QContextMenuEvent *event, QMenu &menu, std::map<QAction *, std::function<void ()>> &actions) override;
  virtual void CreateGridActions(QContextMenuEvent* event, QMenu& menu, std::map<QAction*, std::function<void(void)>>& actions) override;

  void keyPressEvent(QKeyEvent *event) override;

  // Drag & Drop
  virtual void dragEnterEvent(QDragEnterEvent *event) override;
  virtual void dragMoveEvent(QDragMoveEvent *event) override;
  virtual void dropEvent(QDropEvent *event) override;
  virtual void dragLeaveEvent(QDragLeaveEvent *event) override;

  void paintEvent(QPaintEvent *event) override;

protected:
  void axisRemoved(GPAxis *axis) override;
  void resetLastDropTargetItemState(GPLayoutElement* newItem = nullptr);
  void getRectPositionInGrid(int rectIndex, int &row, int &column);
  virtual void getInsertPosition(QPoint pos);

protected slots:
  void onAxisMovedFromRect(GPAxisRect* oldAxisRect);

protected:
  GPAxis::AxisType mTrackAxisOrientation;
  unsigned mDefaultTrackRectFlags;
  bool mKeyAxesReversed;
  bool mHasInfiniteScene;
  int mDefaultTrackSize;
  QString mKeyAxesDefaultName;
  GPMarginGroup mTracksMarginGroup;
  GPLayoutSplitterGrid* mGridLayout;

  QList<GPLayoutSplitterGrid*> mTrackGrids;
  QMap<GPLayoutSplitterGrid*, QList<GPTrackRect*>> mTrackRects;
  QMap<GPLayoutSplitterGrid*, GPAxis*> mTrackGridToMainKeyAxis;
  QMap<GPLayoutSplitterGrid*, GPAxis*> mTrackGridToSecondKeyAxis;

  QPointer<GPLayoutElement> mLastDropTarget;
  int mInsertionGridIndex;
  int mInsertionRectIndex;
  bool mInsertionGrid;
  bool mInsertionRect;

  friend class GPAxisContainer;
};

///////////////////////////////////////////////////////
// Compass legend item
class GPCompassItem : public GPAbstractLegendItem
{
  Q_OBJECT

public:
  explicit GPCompassItem(GPLegend *parent);
  ~GPCompassItem();

protected:
  virtual void draw(GPPainter *painter) Q_DECL_OVERRIDE;
  virtual QSize minimumOuterSizeHint() const Q_DECL_OVERRIDE;
};

///////////////////////////////////////////////////////
// Zones
struct GPZonesElement
{
  GPZonesElement(double v = 0, bool s = true, bool l = false, const QString& n = QString())
    : mValue(v)
    , mVisible(s)
    , mLocked(l)
    , mName(n)
  {}

  double mValue;
  bool mVisible;
  bool mLocked;
  QString mName;
};

class GPZonesData : public QObject
                   , public GPUomDescriptorProviderListener
{
  Q_OBJECT

public:
  GPZonesData();

  const GPZonesElement& at(int i);
  GPZonesElement& first();
  GPZonesElement& last();
  void append(const GPZonesElement& e);
  int size() const;
  void resize(int size);
  void clear();

  QVector<GPZonesElement>::iterator begin();
  QVector<GPZonesElement>::iterator end();

  void setLine(int index, double value);
  void setName(int index, const QString& value);
  void setLocked(int index, const bool& value);
  void setVisible(int index, const bool& value);

  void setRange(double min, double max);
  QPair<double, double> getRange() const;

  void setLines(const QVector<double>& zones);
  QVector<double> getLines() const;

  void setNames(const QVector<QString>& names);
  QVector<QString> getNames() const;

  void setLocked(const QVector<bool>& locked);
  QVector<bool> getLocked() const;

  void setVisibility(const QVector<bool>& visibility);
  QVector<bool> getVisibility() const;

  virtual void setInputUom(const GPUomDescriptor& descriptor);
  virtual void setInputUom(int quantity, int units);

  virtual void applyUom(const GPUomDescriptor& descriptor) override;
  virtual void applyUom(const GPUomDescriptor& source, const GPUomDescriptor& target,
                        QVector<GPZonesElement>::iterator from,
                        QVector<GPZonesElement>::iterator to);

signals:
  void dataChanged();

protected:
  bool rangeIsValid();
  void fixInvalidatedRange();

protected:
  QVector<GPZonesElement> mData;
  QPair<double, double> mRange;

public:
  GPUomDescriptor mInputUom;
};

class GPZonesRect : public GPSelectionRect
                   , public GPEXDraggingInterface
{
  Q_OBJECT

  typedef std::function<double(double, double)> amountFunction;
  typedef std::function<bool(void)> amountTestFunction;

public:
  struct GPZonesRectStyle
  {
    GPZonesRectStyle()
      : linePen(QColor(44,160,44, 120), 2)
      , zonesBrush({QColor(23,190,207, 20),
                          QColor(188,189,34, 20)})
      , textAlignment(Qt::AlignVCenter)
      , textDrawAngle(0.0)
    {
    }

    GPZonesRectStyle(const GPZonesRectStyle& other) = default;
    GPZonesRectStyle& operator=(const GPZonesRectStyle& other) = default;

    QVariantMap toVariantMap();
    void fromVariantMap(QVariantMap map);

    QPen linePen;
    QList<QBrush>  zonesBrush;
    QFont textFont;
    QColor textColor;
    Qt::AlignmentFlag textAlignment;

    double textDrawAngle;
  };

public:
  GPZonesRect(GraphicsPlot* parentPlot);
  ~GPZonesRect();

  void DragStart(QMouseEvent* event, unsigned) override;
  void DragMove(QMouseEvent* event) override;
  void DragAccept(QMouseEvent* event) override;
  bool IsDragging() const override;

  QVariantMap toVariantMap();
  void fromVariantMap(QVariantMap map);
  void setupDefaultLines();

  bool isLegendVisible();

  void setData(const QSharedPointer<GPZonesData>& data);
  QSharedPointer<GPZonesData> data();

  GPLegend* getLegend();
  bool isLegendVisible() const;

  void setStyle(const GPZonesRectStyle& style);
  GPZonesRectStyle getStyle() const;

  void setKeyAxis(GPAxis* keyAxis);
  GPAxis* getKeyAxis() const;

  void setClipRect(bool clip);
  bool getClipRect() const;

  void setAmountSettings(const QVariantMap& settings);
  QVariantMap getAmountSettings() const;

  void onTargetUpdated();

  virtual bool startSelection(QMouseEvent* event) override;
  virtual void moveSelection(QMouseEvent* event) override;
  virtual void endSelection(QMouseEvent* event) override;
  virtual void draw(GPPainter* painter) override;

  virtual double selectTest(const QPointF& pos, bool onlySelectable, QVariant* details = nullptr) const override;
  virtual Qt::CursorShape HitTest(QMouseEvent* event, unsigned*) override;
  QRect clipRect() const override;
  void setVisible(bool on) override;

public slots:
  void updateZonesAmount();

signals:
  void zonesChanged();

protected:
  QSharedPointer<GPZonesData> mData;
  QPointer<GPAxis> mKeyAxis;
  bool mClipRect;

  int mDraggingIndexLeft;
  int mDraggingIndexRight;
  QPoint mDragStart;

  QPointer<GPLegend> mLegend;
  Qt::Orientation mOrientation;
  GPZonesRectStyle mStyle;

  QVariantMap amountSettings;
  QString amountDesc;
  QString amountDim;
  amountFunction amountFunc;
  amountTestFunction amountTestFunc;
};


///////////////////////////////////////////////////////
// Color sections rect
class GPSectionsRect : public GPSelectionRect
                      , public GPUomDescriptorProviderListener
{
  Q_OBJECT

public:
  GPSectionsRect(GraphicsPlot* parentPlot, GPAxis* keyAxis, GPAxisRect *rect = nullptr);
  ~GPSectionsRect();

  void setIcons(const QMap<QString, QString>& icons);
  void setSections(const QVector<double>& sections, const QVector<QString>& icons);

  void setKeyAxis(GPAxis* axis);
  GPAxis* keyAxis();

  virtual void draw(GPPainter *painter) override;

  virtual void setInputUom(const GPUomDescriptor& descriptor);
  virtual void setInputUom(int quantity, int units);

  virtual void applyUom(const GPUomDescriptor& descriptor) override;
  virtual void applyUom(const GPUomDescriptor& source, const GPUomDescriptor& target,
                        QVector<double>::iterator from,
                        QVector<double>::iterator to);

protected:
  QPointer<GPAxis> mKeyAxis;
  QPointer<GPAxisRect> mAxisRect;
  QMap<QString, QPixmap> mIcons;
  QVector<double> mSections;
  QVector<QString> mSectionIcons;
  GPUomDescriptor mInputUom;
};


///////////////////////////////////////////////////////
// Polygon
class GPPolygon : public GPAbstractPlottable1D<GPCurveData>
{
public:
  GPPolygon(GPAxis* keyAxis, GPAxis* valueAxis, GPLayerable* parent=0);
  virtual ~GPPolygon();

  void setData(const QSharedPointer<GPCurveDataContainer>& data);
  void setData(const QVector<double>& t, const QVector<double>& keys, const QVector<double>& values, bool alreadySorted = false);
  void setData(const QVector<double>& keys, const QVector<double>& values);

  virtual GPRange getKeyRange(bool &foundRange, GP::SignDomain inSignDomain = GP::sdBoth) const override;
  virtual GPRange getValueRange(bool &foundRange, GP::SignDomain inSignDomain = GP::sdBoth, const GPRange &inKeyRange = GPRange()) const override;
  virtual GPDataRange getDataRange() const override;

  virtual void draw(GPPainter* painter) override;
  virtual void drawLegendIcon(GPPainter* painter, const QRectF &rect) const override;

private:
  void updateRegistration();
  void drawCurveLine(GPPainter* painter, const QVector<QPointF>& lines) const;

  // Painting
  int getRegion(double key, double value, double keyMin, double valueMax, double keyMax, double valueMin) const;
  void getCurveLines(QVector<QPointF>* lines, const GPDataRange& dataRange, double penWidth) const;
  QPointF getOptimizedPoint(int prevRegion, double prevKey, double prevValue, double key, double value, double keyMin, double valueMax, double keyMax, double valueMin) const;
  QVector<QPointF> getOptimizedCornerPoints(int prevRegion, int currentRegion, double prevKey, double prevValue, double key, double value, double keyMin, double valueMax, double keyMax, double valueMin) const;
  bool mayTraverse(int prevRegion, int currentRegion) const;
  bool getTraverse(double prevKey, double prevValue, double key, double value, double keyMin, double valueMax, double keyMax, double valueMin, QPointF &crossA, QPointF &crossB) const;
  void getTraverseCornerPoints(int prevRegion, int currentRegion, double keyMin, double valueMax, double keyMax, double valueMin, QVector<QPointF>& beforeTraverse, QVector<QPointF>& afterTraverse) const;
  double pointDistance(const QPointF &pixelPoint, GPCurveDataContainer::const_iterator &closestData) const;

private:
  GPPolygonGroup* Group;
};

///////////////////////////////////////////////////////
// Polygon group
class GPPolygonGroup : public GPLayerable
{
  Q_OBJECT
public:
  GPPolygonGroup(GraphicsPlot* parentPlot, GPAxis* keyAxis, GPAxis* valueAxis);
  ~GPPolygonGroup();

  GPPolygon* addPolygon(GPAxis* keyAxis = 0, GPAxis* valueAxis = 0);
  GPPolygon* getPolygon(double leftKey, double topValue);
  GPPolygon* getDefaultPolygon();
  void removePolygon(GPPolygon* polygon);
  void clear();
  size_t size();

  void setPen(const QPen& pen);
  void setBrush(const QBrush& brush);

  // Implement GPLayerable
  virtual void applyDefaultAntialiasingHint(GPPainter* painter) const override;
  virtual void draw(GPPainter* painter) override;

  GPAxis* keyAxis();
  GPAxis* valueAxis();

  void registerPolygon(double leftKey, double topValue, GPPolygon* polygon);

private:
  QPointer<GPAxis> KeyAxs;
  QPointer<GPAxis> ValueAxs;

  QSharedPointer<QPen>   Pen;
  QSharedPointer<QBrush> Brush;

  std::set<QPointer<GPPolygon>> Polygons;
  QMap<std::pair<double, double>, QPointer<GPPolygon>> Index;
};

///////////////////////////////////////////////////////
// Polygon capable layer
class GPPolygonLayer : public GPLayer
{
  Q_OBJECT
public:
  GPPolygonLayer(GraphicsPlot* parentPlot, const QString& layerName);
  virtual ~GPPolygonLayer();

  GPPolygonGroup* addPolygonGroup(GPAxis* keyAxis = 0, GPAxis* valueAxis = 0);
  void removePolygonGroup(GPPolygonGroup* group);

private:
  QList<QSharedPointer<GPPolygonGroup>> PolygonGroups;
};

///////////////////////////////////////////////////////
// Extended color map
class GPEXColorMap : public GPColorMap
{
  Q_OBJECT

public:
  explicit GPEXColorMap(GPAxis *keyAxis, GPAxis *valueAxis);
  virtual ~GPEXColorMap();

  void clear();

  GPPolygon* contour() { return mContour; }

  void setContourVisible(bool visible);
  bool contourVisible();

protected:
  void updateMapImage() override;
  void updateContour();

  bool eventFilter(QObject *watched, QEvent *event) override;
  virtual void ShowValueToolTip(const QPoint& pos);

  virtual void draw(GPPainter *painter) Q_DECL_OVERRIDE;

protected:
  GPPolygon* mContour;
  QVector<QPointF> mClipPolygon;
};

///////////////////////////////////////////////////////
// Srv curve
class GPSRVCurve : public GPCurve
{
  Q_OBJECT

public:
  enum SRVDataType
  {
    DataTypeAzimuth = 0,
    DataTypeSMin,
    DataTypeSMax
  };

  using GPCurve::setData;

  explicit GPSRVCurve(GPAxis *keyAxis, GPAxis *valueAxis);
  virtual ~GPSRVCurve();

  void setData(const QVector<double> &t,
               const QVector<double> &x,
               const QVector<double> &y,
               const QVector<double> &a,
               const QVector<double> &s,
               const double &fractureLength,
               unsigned dataType = DataTypeAzimuth);

  void clearData();

  virtual GPRange getKeyRange(bool &foundRange, GP::SignDomain inSignDomain = GP::sdBoth) const Q_DECL_OVERRIDE;
  virtual GPRange getValueRange(bool &foundRange, GP::SignDomain inSignDomain = GP::sdBoth, const GPRange &inKeyRange=GPRange()) const Q_DECL_OVERRIDE;

protected:
  virtual void draw(GPPainter *painter) Q_DECL_OVERRIDE;
  virtual void drawLine(GPPainter *painter, QPointF p1, QPointF p2, const QRectF &clipRect);
  virtual void drawRect(GPPainter *painter, QPointF p1, QPointF p2, const QRectF &clipRect);
  QPointF rotatePoint(double cx, double cy, double az, double px, double py);

protected:
  QVector<QPointF> mPoints;
  bool mFilledByStress;

  double minX, maxX;
  double minY, maxY;
};

///////////////////////////////////////////////////////
// Default axis ticker
class GPAxisAwareTicker : public GPAxisTicker
{
  Q_OBJECT

public:
  static const char* HoursFormat;
  static const char* MinutesFormat;
  static const char* SecondsFormat;

public:
  explicit GPAxisAwareTicker(GPAxis* axis);
  virtual ~GPAxisAwareTicker();

  void SetAxis(GPAxis* axis);
  GPAxis* GetAxis() const;

  static QString getQDateTimeFormat(GPAxis* axis);
  static QString getQDateTimeFormat(GPAxisTicker* ticker);

  static QVariant toFormatedKey(GPAxisTicker* ticker, double keyValue);
  static double fromFormatedKey(GPAxisTicker* ticker, const QVariant& value);

  static QVariant toFormatedKey(GPAxis* axis, double keyValue);
  static double fromFormatedKey(GPAxis* axis, const QVariant& value);

  QVariant toFormatedKey(double keyValue) const;
  double fromFormatedKey(const QVariant& value) const;

  QString dateTimeFormat() const { return mDateTimeFormat; }
  void setDateTimeFormat(const QString& format) { mDateTimeFormat = format; }
  bool isFormatedTime() const;

  using GPAxisTicker::getTickLabel;
  virtual QString getTickLabel(double tick, const QLocale &locale, QChar formatChar, int precision) override;
  virtual QVector<QString> createLabelVector(const QVector<double>& ticks, const QLocale& locale, QChar formatChar, int precision, const GPRange& range) override;

public slots:
  double getPreferredStep() const;
  void setPreferredStep(double step);

protected:
  virtual QVector<double> createTickVector(double tickStep, const GPRange& range) override;
  virtual QVector<double> createSubTickVector(int subTickCount, const QVector<double>& ticks) override;

private:
  QPointer<GPAxis> Axis;
  double PreferredStep;
  QString mDateTimeFormat;
};

///////////////////////////////////////////////////////
// Interactive curve
class GPInteractiveCurve : public GPCurve , public GPEXDraggingInterface
{
  Q_OBJECT

public:
  enum HitPart
  {
    HitPartNone,
    HitPartDataPoint,
    HitPartDataLine,
  };

  using GPEXDraggingInterface::DragStart;

  explicit GPInteractiveCurve(GPAxis* keyAxis, GPAxis* valueAxis, double slope = qQNaN(), bool registerInPlot = true);
  virtual ~GPInteractiveCurve();

  void SetColor(const QColor& color);

  virtual QSharedPointer<GPDataContainer<GPCurveData>> GetDataContainer();
  void setInteractive(bool interactive) override;
  bool isInteractive() override;

  virtual void DragStart(QMouseEvent* event, unsigned part) override;
  virtual void DragStart(QMouseEvent* event, GPCurveData *dataPoint = 0);
  virtual void DragMove(QMouseEvent* event) override;
  virtual void DragAccept(QMouseEvent* event) override;

  virtual GPCurveData *GetDraggingDataPoint();
  virtual Qt::CursorShape HitTest(QMouseEvent* event, unsigned* part) override;
  virtual void SetSlope(double slope);
  virtual double GetSlope();

signals:
  void moved();
  void movedComplete();

protected:
  virtual void DragUpdateData(QMouseEvent* event) override;
  double Slope;
  QVector<GPCurveData> DraggingData;
  double DraggingDataPoint;
  double DraggingSlope;
  QPoint DraggingStartPos;
};

///////////////////////////////////////////////////////
// Curve label
class GPCurveLabel : public GPLayerable
{
  Q_OBJECT

public:
  explicit GPCurveLabel(GPCurve* curve);
  ~GPCurveLabel();

  void setText(const QString& text);
  void setFont(const QFont& font);
  void setColor(const QColor& color);

  virtual void applyDefaultAntialiasingHint(GPPainter *) const override {}
  virtual void draw(GPPainter *painter) Q_DECL_OVERRIDE;

protected:
  QString mText;
  QFont mFont;
  QColor mColor;

  QPointer<GPCurve> mCurve;
};


///////////////////////////////////////////////////////
// Movable data container
class GPMovableDataContainer : public GPRepresentationDataContainer<GPCurveData>
{
public:
  GPMovableDataContainer();

  //// Data
  double GetKeyShift() const;
  double GetValueShift() const;
  void SetKeyShift(double k);
  void SetValueShift(double v);

  //// Uom suuport for data
  virtual void applyUomForKeys(const GPUomDescriptor& target) override;
  virtual void applyUomForValues(const GPUomDescriptor& target) override;

  void updateData() override;

protected:
  double KeyShift;
  double ValueShift;
};

///////////////////////////////////////////////////////
// Movable curve
class GPMovableCurve : public GPCurve
{
  Q_OBJECT

public:
  explicit GPMovableCurve(GPAxis *keyAxis, GPAxis *valueAxis, bool registerInPlot = true);

  virtual void setData(const QVector<double> &t, const QVector<double> &keys, const QVector<double> &values, bool alreadySorted);
  virtual void setData(const QVector<double> &keys, const QVector<double> &values);

  double GetKeyShift() const;
  double GetValueShift() const;
  void SetKeyShift(double k);
  void SetValueShift(double v);
  void UpdateData();

  virtual Qt::CursorShape HitTest(QMouseEvent *event, unsigned *part);
  virtual void mousePressEvent(QMouseEvent* event, const QVariant& /*details*/);
  virtual void mouseMoveEvent(QMouseEvent* event, const QPointF& startPos);
  virtual void mouseReleaseEvent(QMouseEvent* event, const QPointF& startPos);

signals:
  void moved();

protected:
  bool Dragging;
};


///////////////////////////////////////////////////////
// Curve mark point
class GPCurveMarkPoint : public GPLayerable
                        , public GPUomDescriptorProviderDualListener
{
  Q_OBJECT

public:
  explicit GPCurveMarkPoint(GPCurve* curve, QPoint pixelPos);
  explicit GPCurveMarkPoint(GPCurve* curve, double keyPos);

  void applyDefaultAntialiasingHint(GPPainter*) const override {}
  double selectTest(const QPointF &pos, bool onlySelectable, QVariant *details = nullptr) const override;
  void draw(GPPainter* painter) override;

  virtual Qt::CursorShape HitTest(QMouseEvent *event, unsigned *part) override;
  virtual void mousePressEvent(QMouseEvent* event, const QVariant& details) override;
  virtual void mouseMoveEvent(QMouseEvent* event, const QPointF& startPos) override;
  virtual void mouseReleaseEvent(QMouseEvent* event, const QPointF& startPos) override;

  // GPUomDescriptorProviderDualListener
  virtual void applyUomForKeys(const GPUomDescriptor& descriptor) override;
  virtual void applyUomForValues(const GPUomDescriptor& descriptor) override;

protected:
  GPCurve* Curve;
  double KeyPos;
  bool Dragging;
};

///////////////////////////////////////////////////////
// Interactive point
class GPInteractivePoint : public GPInteractiveCurve
{
  Q_OBJECT

public:
  explicit GPInteractivePoint(GPAxis* keyAxis, GPAxis* valueAxis, bool interactive = true);
  ~GPInteractivePoint();

public slots:
  void setLabelText(const QString& name);
  void setLabelFont(const QFont& font);
  void setLabelColor(const QColor& color);

  void setCoord(const QPointF& point);
  void setCoord(double Key, double val);
  QPointF getCoord() const;
  QPointF getPixelCoord() const;
  double key();
  double value();

protected:
  GPCurveLabel* mLabel;
};

///////////////////////////////////////////////////////
// Infinite rect
class GPInfiniteRect : public GPItemRect
{
  Q_OBJECT

public:
  GPInfiniteRect(GraphicsPlot* parentPlot, Qt::Orientation mOrientation = Qt::Vertical);
  GPInfiniteRect(GraphicsPlot* parentPlot, Qt::Orientation mOrientation, GPAxis* keyAxis, GPAxis* valueAxis);
  virtual ~GPInfiniteRect();

  void setColors(const QBrush& rectBrush, const QPen& linesPen);

  virtual void draw(GPPainter* painter) Q_DECL_OVERRIDE;

protected:
  Qt::Orientation mOrientation;
};

///////////////////////////////////////////////////////
// Infinite line
class GPAxisInfiniteLine : public GPItemLine
                          , public GPEXDraggingInterface
{
  Q_OBJECT
public:
  enum HitPart
  {
    HitPartNone,
    HitPartStart,
    HitPartEnd,
    HitPartLine
  };

public:
  explicit GPAxisInfiniteLine(GraphicsPlot* parentPlot);
  explicit GPAxisInfiniteLine(GraphicsPlot* parentPlot, GPAxis* keyAxis, GPAxis* valueAxis);
  explicit GPAxisInfiniteLine(GPAxis* valueAxis);

  virtual ~GPAxisInfiniteLine();

  void   setValue(const double& value);
  double getValue() const;

  virtual Qt::CursorShape HitTest(QMouseEvent* event, unsigned* part);

  virtual void DragStart(QMouseEvent* event, unsigned part);
  virtual void DragMove(QMouseEvent* event);
  virtual void DragAccept(QMouseEvent* event);
  virtual void DragUpdateData(QMouseEvent* event);
  virtual bool IsDragging() const;

  virtual QLineF getRectClippedLine(const GPVector2D &start, const GPVector2D &end, const QRect &rect) const;

protected:
  QPoint  DraggingStartPos;
  QPointF DraggingStartPosStart;
  QPointF DraggingStartPosEnd;
};

///////////////////////////////////////////////////////
// fixed orientated infinite line
class GPOrientatedInfiniteLine : public GPAxisInfiniteLine
{
  Q_OBJECT

public:
  GPOrientatedInfiniteLine(GraphicsPlot* parentPlot, Qt::Orientation mOrientation = Qt::Vertical);
  virtual ~GPOrientatedInfiniteLine();

  void setKeyAxis(GPAxis* axis);
  void setValueAxis(GPAxis* axis);
  void setClipRect(const QRect& rect);

  virtual Qt::CursorShape HitTest(QMouseEvent* event, unsigned* part) override;
  double selectTest(const QPointF &pos, bool onlySelectable, QVariant *details) const override;

  virtual void DragUpdateData(QMouseEvent* event) override;
  virtual void DragAccept(QMouseEvent* event) override;

  virtual void setValue(const double& value);
  virtual double getValue() const;

  GP::Interaction selectionCategory() const override;

signals:
  void moved();
  void movedComplete();

protected:
  virtual QRect clipRect() const override;

protected:
  Qt::Orientation mOrientation;
  QRect mClipRect;
};


///////////////////////////////////////////////////////
//  Selection rect
class GPSelectionIntervals : public GPLayerable
                            , public GPUomDescriptorProviderListener
{
  Q_OBJECT

public:
  enum SelectionMode
  {
    NonInteractiveSelection,
    InteractiveRange
  };

  explicit GPSelectionIntervals(SelectionMode workmode, GPAxis* keyAxis, GraphicsPlot* parentPlot = nullptr);
  explicit GPSelectionIntervals(GPAxis* keyAxis);

  virtual ~GPSelectionIntervals();

  void setHorizontalLinesPen(const QPen& pen);
  void setVerticalLinePen(const QPen& pen);
  void setSelectionBrush(const QBrush& brush);
  void setMargin(int margin);
  void setLeftSideAxisRect(GPAxisRect* rect);
  void setInteractive(bool interactive);

  void setSelection(const QVector<QPair<double, double>>& ranges);
  void setNames(const QVector<QString>& names);

  void setStartStop(double start, double stop);
  double getStart() const;
  double getStop() const;

  virtual void applyDefaultAntialiasingHint(GPPainter *painter) const override;
  virtual void draw(GPPainter *painter) override;

  virtual void setInputUom(const GPUomDescriptor& descriptor);
  virtual void setInputUom(int quantity, int units);

  virtual void applyUom(const GPUomDescriptor& descriptor) override;
  virtual void applyUom(const GPUomDescriptor& source, const GPUomDescriptor& target,
                        QVector<QPair<double, double>>::iterator from,
                        QVector<QPair<double, double>>::iterator to);

signals:
  void rangeChanged();

protected slots:
  void OnStartLineMoved();
  void OnStopLineMoved();

protected:
  SelectionMode mMode;
  GPAxis* mKeyAxis;
  GPAxisRect* mLeftSideAxisRect;

  QPen mHorizontalLinesPen;
  QPen mVerticalLinePen;
  QBrush mSelectionBrush;
  int mMargin;

  QVector<QPair<double, double>> mSelections;
  QVector<QString> mNames;
  GPOrientatedInfiniteLine* mStartLine;
  GPOrientatedInfiniteLine* mStopLine;

  GPUomDescriptor mInputUom;
};


///////////////////////////////////////////////////////
// Range spin box
class RangeSpinBox : public QDoubleSpinBox
{
  Q_OBJECT
public:
  explicit RangeSpinBox(QWidget* parent = nullptr);
};

///////////////////////////////////////////////////////
//  ruler line
class GPRulerLine : public GPOrientatedInfiniteLine
{
  Q_OBJECT

public:
  GPRulerLine(GraphicsPlot* parentPlot, Qt::Orientation orientation = Qt::Vertical);
  GPRulerLine(GraphicsPlot* parentPlot, Qt::Orientation orientation, double keyCoord);
  GPRulerLine(GraphicsPlot* parentPlot, Qt::Orientation orientation, QPoint pos);
  virtual ~GPRulerLine();

public slots:
  virtual void setValue(const double &value) override;
  virtual double getValue() const override;

  virtual void OnAxisReseted();
  virtual void OnCurveUpdated();
  virtual void UpdatePoints();

protected slots:
  virtual void SetCurrentAxis();
  virtual void UpdateValue();
  virtual void UpdateAxisList();
  virtual void UpdateWidgetPosition();
  virtual void ClearPoints();

protected:
  QPointer<QWidget> ValueWidget;

  QComboBox* AxisBox;
  QDoubleSpinBox* ValueBox;
  QDateTimeEdit* TimeBox;

  GPAxis* CurrentKeyAxis;
  QList<GPInteractivePoint*> Points;
};


///////////////////////////////////////////////////////
//  distance ruler
class GPRulerDistance : public GPInteractiveCurve
{
  Q_OBJECT

public:
  explicit GPRulerDistance(GPAxis* axisX, GPAxis* axisY, QPoint pos);
  ~GPRulerDistance();

protected:
  virtual void DragUpdateData(QMouseEvent* event);
  virtual void UpdateDistanceLabel();

protected:
  GPCurveLabel* mLabel;
};


///////////////////////////////////////////////////////
// Caption item
class GPCaptionItem : public GPLayerable
                     , public GPEXDraggingInterface
                     , public GPUomDescriptorProviderDualListener
{
  Q_OBJECT

  enum DraggingPart
  {
    HitPartNone,
    HitPartText,
    HitPartPoint
  };

public:
  GPCaptionItem(GraphicsPlot* parent);
  GPCaptionItem(GPLayerable* layerable, const QString& text, bool hasLine, const QPointF& pos);
  ~GPCaptionItem();

  void setParent(GPLayerable* layerable);
  void setParent(const QString& type, const QString& id);
  QString getParentType() const;
  QString getParentId() const;

  void setId(const QUuid& id);
  QUuid getId() const;

  void setText(const QString& text);
  QString getText() const;

  void setFont(const QFont& font);
  QFont getFont() const;

  void setTextColor(const QColor& color);
  QColor getTextColor() const;

  void setTarget(GPCurve* target);
  GPCurve* getTarget() const;

  void setHasLine(bool has);
  bool getHasLine() const;

  void setTextPos(const QPointF& pos);
  QPointF getTextPos() const;

  void setPointPos(const QPointF& pos);
  QPointF getPointPos();

  virtual void DragStart(QMouseEvent* event, unsigned part) override;
  virtual void DragMove(QMouseEvent* event) override;
  virtual void DragAccept(QMouseEvent* event) override;

  virtual Qt::CursorShape HitTest(QMouseEvent* event, unsigned* part) override;
  virtual double selectTest(const QPointF &pos, bool onlySelectable, QVariant *details) const override;
  void applyDefaultAntialiasingHint(GPPainter*) const override {}

  virtual bool eventFilter(QObject* obj, QEvent* e) override;
  virtual void mouseDoubleClickEvent(QMouseEvent *event, const QVariant &details) override;

  void setAxes(GPAxis* keyAxes, GPAxis* valueAxes);

  void applyUomForKeys(const GPUomDescriptor& descriptor) override;
  void applyUomForValues(const GPUomDescriptor& descriptor) override;

protected:
  void draw(GPPainter *painter) override;

  QPointF pixelsToCoords(const QPointF& point) const;
  QPointF coordsToPixels(const QPointF& point) const;

protected:
  QUuid mId;
  QString mText;
  QFont mFont;
  bool mHasLine;
  int mPadding;
  QColor mTextColor;

  QPointer<GPCurve> mTarget;
  QPointer<GPAxis> mKeyAxis;
  QPointer<GPAxis> mValueAxis;

  QPointF mTextPos;
  QPointF mPointPos;

  QPointer<QMenu> mRunningMenu;
};
