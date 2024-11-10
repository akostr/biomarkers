#pragma once

#include <QAbstractItemModel>
#include <QPointF>
#include <QPixmap>
#include <QContextMenuEvent>
#include <QMenu>
#include <QJsonObject>
#include <graphicsplot/graphicsplot.h>


#include "ui/plots/gp_items/gpshape.h"
#include "logic/enums.h"

class PlotTemplateModel;
class PlotTemplateItem;
class TextItem;
class LineItem;
class GPVector2D;

enum EndingStyle {
  esNone          ///< No ending decoration
  ,esFlatArrow    ///< A filled arrow head with a straight/flat back (a triangle)
  ,esSpikeArrow   ///< A filled arrow head with an indented back
  ,esLineArrow    ///< A non-filled arrow head with open back
  ,esDisc         ///< A filled circle
  ,esSquare       ///< A filled square
  ,esDiamond      ///< A filled diamond (45 degrees rotated square)
  ,esBar          ///< A bar perpendicular to the line
  ,esHalfBar      ///< A bar perpendicular to the line, pointing out to only one side (to which side can be changed with \ref setInverted)
  ,esSkewedBar    ///< A bar that is skewed (skew controllable via \ref setLength)
};

namespace PROP
{
Q_NAMESPACE
constexpr char opacity[] = "opacity";
constexpr char showOnlyAxisRect[] = "showOnlyAxisRect";
constexpr char color[] = "color";
constexpr char transparency[] = "transparency";
constexpr char fillColor[] = "fillColor";
constexpr char penStyle[] = "penStyle";
constexpr char penColor[] = "penColor";
constexpr char penWidth[] = "penWidth";
constexpr char lineEndingType[] = "lineEndingType";
constexpr char font[] = "font";
constexpr char text[] = "text";
constexpr char lineClose[] = "lineClose";
constexpr char fontSize[] = "fontSize";
constexpr char bold[] = "bold";
constexpr char italic[] = "italic";
constexpr char unknown[] = "unknown";

//usage example:
//QCoreApplication::translate("PROP", mPropTitles[color]);
const static QMap<QString, const char*> mPropTitles {
    {opacity, QT_TRANSLATE_NOOP("PROP", "Opacity")},
    {showOnlyAxisRect, QT_TRANSLATE_NOOP("PROP", "Show only axis rect")},
    {color, QT_TRANSLATE_NOOP("PROP", "Color")},
    {transparency, QT_TRANSLATE_NOOP("PROP", "Transparency")},
    {fillColor, QT_TRANSLATE_NOOP("PROP", "Fill color")},
    {penStyle, QT_TRANSLATE_NOOP("PROP", "Pen style")},
    {penColor, QT_TRANSLATE_NOOP("PROP", "Pen color")},
    {lineEndingType, QT_TRANSLATE_NOOP("PROP", "Line ending type")},
    {penWidth, QT_TRANSLATE_NOOP("PROP", "Pen width")},
    {font, QT_TRANSLATE_NOOP("PROP", "Font")},
    {text, QT_TRANSLATE_NOOP("PROP", "Text")},
    {lineClose, QT_TRANSLATE_NOOP("PROP", "Line close")},
    {fontSize, QT_TRANSLATE_NOOP("PROP", "Font size")},
    {bold, QT_TRANSLATE_NOOP("PROP", "Bold")},
    {italic, QT_TRANSLATE_NOOP("PROP", "Italic")},
    {unknown, QT_TRANSLATE_NOOP("PROP", "Unknown")}
};
}

struct Axis
{
  QString title;
  double from;
  double to;
  bool isLinear;

  Axis() : title(), from(0), to(10), isLinear(true) {}


  Axis &operator=(const Axis &other)
  {
    this->title = other.title;
    this->from = other.from;
    this->to = other.to;
    this->isLinear = other.isLinear;
    return *this;
  }

  bool operator==(Axis axis)
  {
    return title == axis.title
        && abs(to - axis.to) <= std::numeric_limits<double>::epsilon()
        && abs(from - axis.from) <= std::numeric_limits<double>::epsilon()
        && isLinear == axis.isLinear;
  }
  bool operator!=(Axis axis) { return !this->operator==(axis);}
};

class RootItem : public QObject
{
  Q_OBJECT
public:
  RootItem();
  QList <PlotTemplateItem*>& items();

  QJsonObject saveToJson();
  void loadFromJson(const QJsonObject&);
  PlotTemplateItem *addItem(int type);

  void setAxisX(Axis);
  void setAxisY(Axis);
  void setSize(QSize);
  void setPlotType(TemplatePlot::PlotType);

  const Axis& axisX() { return m_xAxis; }
  const Axis& axisY() { return m_yAxis; }
  const QSize& size() { return m_size; }
  TemplatePlot::PlotType plotType() { return m_plotType; }

signals:
  void axisXChanged(Axis);
  void axisYChanged(Axis);
  void sizeChanged(QSize size);
  void loadedFromJson(bool isCorrect);
  void typeChanged(TemplatePlot::PlotType plotType);

protected:
  QList <PlotTemplateItem*> m_items;
  Axis m_xAxis;
  Axis m_yAxis;
  QSize m_size;
  TemplatePlot::PlotType m_plotType;
};

class ParameterItem;

class PlotTemplateItem : public QObject
{
  Q_OBJECT
public:
  enum ItemType
  {
    PixmapType,
    ShapeType,
    PolylineType,
    TextType,
    LineType,
    InvalidType
  };
  PlotTemplateItem(RootItem *parent);

  virtual QJsonObject saveToJson();
  virtual void loadFromJson(const QJsonObject&);
  virtual ItemType type() const { return InvalidType; }

  RootItem *root() const { return m_root; }
  QString name() const { return m_name; }
  bool selected() const { return m_selected; }
  bool isEnabled() const { return m_isEnabled; }
  bool isVisible() const { return m_isVisible; }
  // double opacity() const { return m_opacity; }
  double rotate() const { return m_rotate; }
  bool showOnlyAxisRect() const { return m_showOnlyAxisRect; }
  int index() const { return m_root->items().indexOf(this); }
  QUuid uuid() { return m_uuid; }

public slots:
  void setName(QString name);
  void setSelected(bool);
  void setEnabled(bool);
  void setVisible(bool);
  // void setOpacity(double opacity);
  void setRotate(double);
  void setShowOnlyAxisRect(bool showOnlyAxisRect);
  void setUuid(QUuid);

signals:
  void nameChanged(QString name);
  void selectedChanged(bool);
  void enabledChanged(bool);
  void visibleChanged(bool);
  void rotateChanged(double);
  void showOnlyAxisRectChanged(bool);

protected:
  RootItem *m_root;
  QString m_name;
  bool m_selected;
  bool m_isEnabled;
  bool m_isVisible;
  // double m_opacity;
  double m_rotate;
  bool m_showOnlyAxisRect;
  QUuid m_uuid;

private:
  Q_PROPERTY(int type READ type FINAL)
  Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged FINAL)
  Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged FINAL)
  Q_PROPERTY(bool visible READ isVisible WRITE setVisible NOTIFY visibleChanged FINAL)
  Q_PROPERTY(double rotate READ rotate WRITE setRotate NOTIFY rotateChanged FINAL)
  Q_PROPERTY(bool showOnlyAxisRect READ showOnlyAxisRect WRITE setShowOnlyAxisRect NOTIFY showOnlyAxisRectChanged FINAL)
};

class PixmapItem : public PlotTemplateItem
{
  Q_OBJECT
public:
  enum ParemetersRows
  {
    OpacityRow,
    InvalidRow,
    LastRow = InvalidRow - 1

  };

  PixmapItem(RootItem *parent);
  virtual QJsonObject saveToJson() override;
  virtual void loadFromJson(const QJsonObject&) override;
  virtual ItemType type() const override { return PixmapType; }

  QPointF topLeftCoordinate() const { return m_topLeftCoordinate; }
  QPointF bottomRightCoordinate() const { return m_bottomRightCoordinate; }
  QPointF topLeftEditableCoordinate() const { return m_topLeftEditableCoordinate; }
  QPointF bottomRightEditableCoordinate() const { return m_bottomRightEditableCoordinate; }
  const QPixmap& pixmap() const { return m_pixmap; }

  double opacity() const;
  void setOpacity(double new_opacity);

public slots:
  void setTopLeftCoordinate(QPointF);
  void setBottomRightCoordinate(QPointF);
  void setTopLeftEditableCoordinate(QPointF);
  void setBottomRightEditableCoordinate(QPointF);
  void setPixmap(const QPixmap&);

signals:
  void topLeftCoordinateChanged(QPointF);
  void bottomRightCoordinateChanged(QPointF);
  void topLeftEditableCoordinateChanged(QPointF);
  void bottomRightEditableCoordinateChanged(QPointF);
  void pixmapChanged(const QPixmap&);
  void opacityChanged(double opacity);

private:
  QPointF m_topLeftCoordinate;
  QPointF m_bottomRightCoordinate;
  QPointF m_topLeftEditableCoordinate;
  QPointF m_bottomRightEditableCoordinate;
  QPixmap m_pixmap;
  double m_opacity;
  Q_PROPERTY(QPointF topLeftCoordinate READ topLeftCoordinate WRITE setTopLeftCoordinate NOTIFY topLeftCoordinateChanged FINAL)
  Q_PROPERTY(QPointF bottomRightCoordinate READ bottomRightCoordinate WRITE setBottomRightCoordinate NOTIFY bottomRightCoordinateChanged FINAL)
  Q_PROPERTY(QPixmap pixmap READ pixmap WRITE setPixmap NOTIFY pixmapChanged FINAL)
  Q_PROPERTY(double opacity READ opacity WRITE setOpacity NOTIFY opacityChanged FINAL)
};

class PolylineItem : public PlotTemplateItem
{
  Q_OBJECT

  using PointsList = QList<GPVector2D>;
  //fixed
  Q_PROPERTY(Qt::PenStyle penStyle READ penStyle WRITE setPenStyle NOTIFY penStyleChanged RESET resetPenStyle FINAL)
  Q_PROPERTY(QColor penColor READ penColor WRITE setPenColor NOTIFY penColorChanged RESET resetPenColor FINAL)
  Q_PROPERTY(qreal penWidth READ penWidth WRITE setPenWidth NOTIFY penWidthChanged RESET resetPenWidth FINAL)
  Q_PROPERTY(PointsList points READ points WRITE setPoints NOTIFY pointsChanged RESET resetPoints FINAL)
  Q_PROPERTY(bool lineClosed READ isLineClosed WRITE setLineClosed NOTIFY lineClosingStateChanged FINAL)
  //optional
  Q_PROPERTY(QVariant fillColor READ fillColor WRITE setFillColor NOTIFY fillColorChanged RESET resetFillColor FINAL)
  Q_PROPERTY(QVariant transparency READ transparency WRITE setTransparency NOTIFY transparencyChanged RESET resetTransparency FINAL)
  Q_PROPERTY(QVariant lineEndingType READ lineEndingType WRITE setLineEndingType NOTIFY lineEndingChanged RESET resetLineEndingType FINAL)
public:
  //@TODO: fix enum copying later (copied from GPLineEnding)
  using LineEndingType = GPLineEnding::EndingStyle;
  PolylineItem(RootItem *rootItem);
  virtual QJsonObject saveToJson() override;
  virtual void loadFromJson(const QJsonObject&json) override;
  virtual ItemType type() const override { return PolylineType; };

public slots:
  //fixed
  void setPenStyle(Qt::PenStyle newPenStyle);
  void setPenColor(const QColor &newColor);
  void setPenWidth(qreal width);
  void setPoints(const PointsList& points);
  //optional
  void setFillColor(QVariant colorv);
  void setTransparency(QVariant transparencyv);//0.0 - 1.0
  void setLineEndingType(QVariant lineEndingTypev);
  void setLineClosed(bool closed);

  //fixed
  Qt::PenStyle penStyle();
  QColor penColor();
  qreal penWidth();
  PointsList points();
  //optional
  QVariant fillColor();
  QVariant transparency();
  QVariant lineEndingType();
  bool isLineClosed();

  //fixed
  void resetPenStyle();
  void resetPenColor();
  void resetPenWidth();
  void resetPoints();
  //optional
  void resetFillColor();
  void resetTransparency();
  void resetLineEndingType();

private:
  //fixed
  Qt::PenStyle mPenStyle;
  QColor mPenColor;
  qreal mPenWidth;
  PointsList mPoints;
  //optional
  QVariant mFillColor;//QColor
  QVariant mTransparency;//qreal
  QVariant mLineEndingType;//LineEndingType
  bool m_isLineClosed = false;

signals:
  //fixed
  void penStyleChanged();
  void penColorChanged();
  void penWidthChanged();
  void pointsChanged();
  void lineClosingStateChanged();
  //optional
  void fillColorChanged();
  void transparencyChanged();
  void lineEndingChanged();
};

class ShapeItem : public PlotTemplateItem
{
  Q_OBJECT

public:
  ShapeItem(RootItem *parent);
  virtual QJsonObject saveToJson() override;
  virtual void loadFromJson(const QJsonObject&) override;

  QPointF coordinate() { return m_coordinate; }
  QSizeF size() { return m_size; }
  QColor color() { return m_color; }
  GPShape::ShapeType shapeType() { return m_shapeType; }
  virtual ItemType type() const override { return ShapeType; }

public slots:
  void setCoordinate(QPointF pos);
  void setSize(int size);
  void setSize(QSizeF size);
  void setColor(QColor color);
  void setShape(QPainterPath path);
  void setShape(GPShape::ShapeType type);

signals:
  void coordinateChanged(QPointF coordinate);
  void sizeChanged(QSizeF size);
  void colorChanged(QColor color);
  void shapeTypeChanged(GPShape::ShapeType type);

private:
  QPointF m_coordinate;
  QSizeF m_size;
  QColor m_color;
  GPShape::ShapeType m_shapeType;
};

class PlotTemplateModel : public QAbstractItemModel
{
  Q_OBJECT
public:
  PlotTemplateModel(QObject *parent = nullptr);
  ~PlotTemplateModel();


   virtual QModelIndex index(int row, int column,
                            const QModelIndex &parent = QModelIndex()) const override;
  virtual QModelIndex parent(const QModelIndex &child) const override;

  virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;

  virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
  virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

  Qt::ItemFlags flags(const QModelIndex& index) const override;
  RootItem* getRootItem() { return m_rootItem; }

  void FillContextMenu(PlotTemplateItem*item, QMenu& menu, std::map<QAction*, std::function<void(void)>>& actions);

  QJsonObject saveToJson();
  void loadFromJson(const QJsonObject&);
  void cancelChanges();
  void setCustomValues(QString key, QVariant value);

public slots:
  void removeAllItems();
  void removeItem(PlotTemplateItem *item);
  PixmapItem* addPixmap();
  PolylineItem* addPolyline();
  ShapeItem* addShape();
  TextItem* addText();
  LineItem* addLine();
  void moveItemLayer(int fromRow, int toRow);

signals:
  void itemAdded(PlotTemplateItem*);
  void itemRemoved(PlotTemplateItem*);

private:
  RootItem *m_rootItem;
  QJsonObject m_jsonObject;
  bool m_hasChanges;
  QHash<QString, QVariant> m_customValues;
};



