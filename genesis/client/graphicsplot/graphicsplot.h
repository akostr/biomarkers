#ifndef GRAPHICSPLOT_H
#define GRAPHICSPLOT_H

#include <QtCore/qglobal.h>
#include <type_traits>
#include <utility>

// some Qt version/configuration dependent macros to include or exclude certain code paths:
#ifndef GRAPHICSPLOT_USE_OPENGL
#define GRAPHICSPLOT_USE_OPENGL
#endif
#ifdef GRAPHICSPLOT_USE_OPENGL
#include <QOpenGLFunctions>
#  if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
#    define GP_OPENGL_PBUFFER
#  else
#    define GP_OPENGL_FBO
#  endif
#  if QT_VERSION >= QT_VERSION_CHECK(5, 3, 0)
#    define GP_OPENGL_OFFSCREENSURFACE
#  endif
#endif

#if QT_VERSION >= QT_VERSION_CHECK(5, 4, 0)
#  define GP_DEVICEPIXELRATIO_SUPPORTED
#  if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
#    define GP_DEVICEPIXELRATIO_FLOAT
#  endif
#endif

#include <QtCore/QObject>
#include <QtCore/QPointer>
#include <QtCore/QSharedPointer>
#include <QtCore/QTimer>
#include <QtGui/QPainter>
#include <QtGui/QPaintEvent>
#include <QtGui/QMouseEvent>
#include <QtGui/QWheelEvent>
#include <QtGui/QPixmap>
#include <QtCore/QVector>
#include <QtCore/QString>
#include <QtCore/QDateTime>
#include <QtCore/QMultiMap>
#include <QtCore/QFlags>
#include <QtCore/QDebug>
#include <QtCore/QStack>
#include <QtCore/QCache>
#include <qmath.h>
#include <limits>
#include <algorithm>
#ifdef GP_OPENGL_FBO
#  include <QtGui/QOpenGLContext>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#  include <QtGui/QOpenGLFramebufferObject>
#else
#  include <QtOpenGL/QOpenGLFramebufferObject>
#  include <QtOpenGL/QOpenGLPaintDevice>
#endif
#  ifdef GP_OPENGL_OFFSCREENSURFACE
#    include <QtGui/QOffscreenSurface>
#  else
#    include <QtGui/QWindow>
#  endif
#endif
#ifdef GP_OPENGL_PBUFFER
#  include <QtOpenGL/QGLPixelBuffer>
#endif
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
#  include <qnumeric.h>
#  include <QtGui/QWidget>
#  include <QtGui/QPrinter>
#  include <QtGui/QPrintEngine>
#else
#  include <QtNumeric>
#  include <QtWidgets/QWidget>
#  include <QtPrintSupport/QtPrintSupport>
#endif

// data structures from common_core
#include <graphicsplot/graphicsplot_data_structures.h>


class GPPainter;
class GraphicsPlot;
class GPLayerable;
class GPLayoutElement;
class GPLayout;
class GPAxis;
class GPAxisRect;
class GPAxisPainterPrivate;
class GPAbstractPlottable;
class GPGraph;
class GPCurve;
class GPPointAttachedTextElement;
class GPAbstractItem;
class GPPlottableInterface1D;
class GPLegend;
class GPItemPosition;
class GPLayer;
class GPAbstractLegendItem;
class GPSelectionRect;
class GPColorMap;
class GPColorScale;
class GPBars;

class GP_LIB_DECL GPVector2D
{
public:
  GPVector2D();
  GPVector2D(double x, double y);
  GPVector2D(const QPoint &point);
  GPVector2D(const QPointF &point);
  
  // getters:
  double x() const { return mX; }
  double y() const { return mY; }
  double &rx() { return mX; }
  double &ry() { return mY; }
  
  // setters:
  void setX(double x) { mX = x; }
  void setY(double y) { mY = y; }
  
  // non-virtual methods:
  double length() const { return qSqrt(mX*mX+mY*mY); }
  double lengthSquared() const { return mX*mX+mY*mY; }
  QPoint toPoint() const { return QPoint(mX, mY); }
  QPointF toPointF() const { return QPointF(mX, mY); }
  
  bool isNull() const { return qIsNull(mX) && qIsNull(mY); }
  void normalize();
  GPVector2D normalized() const;
  GPVector2D perpendicular() const { return GPVector2D(-mY, mX); }
  double dot(const GPVector2D &vec) const { return mX*vec.mX+mY*vec.mY; }
  double distanceSquaredToLine(const GPVector2D &start, const GPVector2D &end) const;
  double distanceSquaredToLine(const QLineF &line) const;
  double distanceToStraightLine(const GPVector2D &base, const GPVector2D &direction) const;
  
  GPVector2D &operator*=(double factor);
  GPVector2D &operator/=(double divisor);
  GPVector2D &operator+=(const GPVector2D &vector);
  GPVector2D &operator-=(const GPVector2D &vector);
  
private:
  // property members:
  double mX, mY;
  
  friend inline const GPVector2D operator*(double factor, const GPVector2D &vec);
  friend inline const GPVector2D operator*(const GPVector2D &vec, double factor);
  friend inline const GPVector2D operator/(const GPVector2D &vec, double divisor);
  friend inline const GPVector2D operator+(const GPVector2D &vec1, const GPVector2D &vec2);
  friend inline const GPVector2D operator-(const GPVector2D &vec1, const GPVector2D &vec2);
  friend inline const GPVector2D operator-(const GPVector2D &vec);
};
Q_DECLARE_TYPEINFO(GPVector2D, Q_MOVABLE_TYPE);

inline const GPVector2D operator*(double factor, const GPVector2D &vec) { return GPVector2D(vec.mX*factor, vec.mY*factor); }
inline const GPVector2D operator*(const GPVector2D &vec, double factor) { return GPVector2D(vec.mX*factor, vec.mY*factor); }
inline const GPVector2D operator/(const GPVector2D &vec, double divisor) { return GPVector2D(vec.mX/divisor, vec.mY/divisor); }
inline const GPVector2D operator+(const GPVector2D &vec1, const GPVector2D &vec2) { return GPVector2D(vec1.mX+vec2.mX, vec1.mY+vec2.mY); }
inline const GPVector2D operator-(const GPVector2D &vec1, const GPVector2D &vec2) { return GPVector2D(vec1.mX-vec2.mX, vec1.mY-vec2.mY); }
inline const GPVector2D operator-(const GPVector2D &vec) { return GPVector2D(-vec.mX, -vec.mY); }

/*! \relates GPVector2D

  Prints \a vec in a human readable format to the qDebug output.
*/
inline QDebug operator<< (QDebug d, const GPVector2D &vec)
{
    d.nospace() << "GPVector2D(" << vec.x() << ", " << vec.y() << ")";
    return d.space();
}

class GP_LIB_DECL GPPainter : public QPainter
{
  Q_GADGET
public:
  /*!
    Defines special modes the painter can operate in. They disable or enable certain subsets of features/fixes/workarounds,
    depending on whether they are wanted on the respective output device.
  */
  enum PainterMode { pmDefault       = 0x00   ///< <tt>0x00</tt> Default mode for painting on screen devices
                     ,pmVectorized   = 0x01   ///< <tt>0x01</tt> Mode for vectorized painting (e.g. PDF export). For example, this prevents some antialiasing fixes.
                     ,pmNoCaching    = 0x02   ///< <tt>0x02</tt> Mode for all sorts of exports (e.g. PNG, PDF,...). For example, this prevents using cached pixmap labels
                     ,pmNonCosmetic  = 0x04   ///< <tt>0x04</tt> Turns pen widths 0 to 1, i.e. disables cosmetic pens. (A cosmetic pen is always drawn with width 1 pixel in the vector image/pdf viewer, independent of zoom.)
                   };
  Q_ENUMS(PainterMode)
  Q_FLAGS(PainterModes)
  Q_DECLARE_FLAGS(PainterModes, PainterMode)
  
  GPPainter();
  explicit GPPainter(QPaintDevice *device);
  
  // getters:
  bool antialiasing() const { return testRenderHint(QPainter::Antialiasing); }
  PainterModes modes() const { return mModes; }

  // setters:
  void setAntialiasing(bool enabled);
  void setMode(PainterMode mode, bool enabled=true);
  void setModes(PainterModes modes);

  // methods hiding non-virtual base class functions (QPainter bug workarounds):
  bool begin(QPaintDevice *device);
  void setPen(const QPen &pen);
  void setPen(const QColor &color);
  void setPen(Qt::PenStyle penStyle);
  void drawLine(const QLineF &line);
  void drawLine(const QPointF &p1, const QPointF &p2) {drawLine(QLineF(p1, p2));}
  void save();
  void restore();
  
  // non-virtual methods:
  void makeNonCosmetic();
  
protected:
  // property members:
  PainterModes mModes;
  bool mIsAntialiasing;
  
  // non-property members:
  QStack<bool> mAntialiasingStack;
};
Q_DECLARE_OPERATORS_FOR_FLAGS(GPPainter::PainterModes)
Q_DECLARE_METATYPE(GPPainter::PainterMode)

class GP_LIB_DECL GPAbstractPaintBuffer
{
public:
  explicit GPAbstractPaintBuffer(const QSize &size, double devicePixelRatio);
  virtual ~GPAbstractPaintBuffer();
  
  // getters:
  QSize size() const { return mSize; }
  bool invalidated() const { return mInvalidated; }
  double devicePixelRatio() const { return mDevicePixelRatio; }
  
  // setters:
  void setSize(const QSize &size);
  void setInvalidated(bool invalidated=true);
  void setDevicePixelRatio(double ratio);
  
  // introduced virtual methods:
  virtual GPPainter *startPainting() = 0;
  virtual void donePainting() {}
  virtual void draw(GPPainter *painter) const = 0;
  virtual void clear(const QColor &color) = 0;
  
protected:
  // property members:
  QSize mSize;
  double mDevicePixelRatio;
  
  // non-property members:
  bool mInvalidated;
  
  // introduced virtual methods:
  virtual void reallocateBuffer() = 0;
};


class GP_LIB_DECL GPPaintBufferPixmap : public GPAbstractPaintBuffer
{
public:
  explicit GPPaintBufferPixmap(const QSize &size, double devicePixelRatio);
  virtual ~GPPaintBufferPixmap();
  
  // reimplemented virtual methods:
  virtual GPPainter *startPainting() override;
  virtual void draw(GPPainter *painter) const override;
  void clear(const QColor &color) override;
  
protected:
  // non-property members:
  QPixmap mBuffer;
  
  // reimplemented virtual methods:
  virtual void reallocateBuffer() override;
};


#ifdef GP_OPENGL_PBUFFER
class GP_LIB_DECL GPPaintBufferGlPbuffer : public GPAbstractPaintBuffer
{
public:
  explicit GPPaintBufferGlPbuffer(const QSize &size, double devicePixelRatio, int multisamples);
  virtual ~GPPaintBufferGlPbuffer();
  
  // reimplemented virtual methods:
  virtual GPPainter *startPainting() override;
  virtual void draw(GPPainter *painter) const override;
  void clear(const QColor &color) override;
  
protected:
  // non-property members:
  QGLPixelBuffer *mGlPBuffer;
  int mMultisamples;
  
  // reimplemented virtual methods:
  virtual void reallocateBuffer() override;
};
#endif // GP_OPENGL_PBUFFER


#ifdef GP_OPENGL_FBO
class GP_LIB_DECL GPPaintBufferGlFbo : public GPAbstractPaintBuffer
#ifdef GRAPHICSPLOT_USE_OPENGL
                                       , protected QOpenGLFunctions
#endif
{
public:
  explicit GPPaintBufferGlFbo(const QSize &size,
                               double devicePixelRatio, 
                               const QWeakPointer<QOpenGLContext>& glContext, 
                               const QWeakPointer<QOpenGLPaintDevice>& glPaintDevice);
  virtual ~GPPaintBufferGlFbo();
  
  // reimplemented virtual methods:
  virtual GPPainter *startPainting() override;
  virtual void donePainting() override;
  virtual void draw(GPPainter *painter) const override;
  void clear(const QColor &color) override;
  
protected:
  // non-property members:
  QWeakPointer<QOpenGLContext> mGlContext;
  QSurface* mGlSurface;
  QWeakPointer<QOpenGLPaintDevice> mGlPaintDevice;
  QOpenGLFramebufferObject *mGlFrameBuffer;
  
  // reimplemented virtual methods:
  virtual void reallocateBuffer() override;
};
#endif // GP_OPENGL_FBO


class GP_LIB_DECL GPLayer : public QObject
{
  Q_OBJECT
  /// \cond INCLUDE_QPROPERTIES
  Q_PROPERTY(GraphicsPlot* parentPlot READ parentPlot)
  Q_PROPERTY(QString name READ name)
  Q_PROPERTY(int index READ index)
  Q_PROPERTY(QList<GPLayerable*> children READ children)
  Q_PROPERTY(bool visible READ visible WRITE setVisible)
  Q_PROPERTY(LayerMode mode READ mode WRITE setMode)
  /// \endcond
public:
  
  /*!
    Defines the different rendering modes of a layer. Depending on the mode, certain layers can be
    replotted individually, without the need to replot (possibly complex) layerables on other
    layers.

    \see setMode
  */
  enum LayerMode { lmLogical   ///< Layer is used only for rendering order, and shares paint buffer with all other adjacent logical layers.
                   ,lmBuffered ///< Layer has its own paint buffer and may be replotted individually (see \ref replot).
                 };
  Q_ENUMS(LayerMode)
  
  GPLayer(GraphicsPlot* parentPlot, const QString &layerName);
  virtual ~GPLayer();
  
  // getters:
  GraphicsPlot *parentPlot() const { return mParentPlot; }
  QString name() const { return mName; }
  int index() const { return mIndex; }
  QList<GPLayerable*> children() const { return mChildren; }
  bool visible() const { return mVisible; }
  LayerMode mode() const { return mMode; }
  int count() const { return mChildren.size(); }

  // setters:
  void setVisible(bool visible);
  void setMode(LayerMode mode);

  void addChild(GPLayerable *layerable, bool prepend);
  void removeChild(GPLayerable *layerable);
  
  // non-virtual methods:
  void replot();
  
protected:
  // property members:
  GraphicsPlot *mParentPlot;
  QString mName;
  int mIndex;
  QList<GPLayerable*> mChildren;
  bool mVisible;
  LayerMode mMode;
  
  // non-property members:
  QWeakPointer<GPAbstractPaintBuffer> mPaintBuffer;
  
  // non-virtual methods:
  void draw(GPPainter *painter);
  void drawToPaintBuffer();
  
private:
  Q_DISABLE_COPY(GPLayer)
  
  friend class GraphicsPlot;
  friend class GPLayerable;
  friend class ChromatogramPlot;
};
Q_DECLARE_METATYPE(GPLayer::LayerMode)

class GP_LIB_DECL GPLayerable : public QObject
{
  Q_OBJECT
  /// \cond INCLUDE_QPROPERTIES
  Q_PROPERTY(bool visible READ visible WRITE setVisible)
  Q_PROPERTY(GraphicsPlot* parentPlot READ parentPlot)
  Q_PROPERTY(GPLayerable* parentLayerable READ parentLayerable)
  Q_PROPERTY(GPLayer* layer READ layer WRITE setLayer NOTIFY layerChanged)
  Q_PROPERTY(bool antialiased READ antialiased WRITE setAntialiased)
  /// \endcond
public:
  GPLayerable(GraphicsPlot *plot, const QString& targetLayer=QString(), GPLayerable *parentLayerable=0);
  GPLayerable(GraphicsPlot *plot, GPLayer *layer, GPLayerable *parentLayerable=0);
  virtual ~GPLayerable();
  
  // getters:
  bool visible() const { return mVisible; }
  GraphicsPlot *parentPlot() const { return mParentPlot; }
  void setParentPlot(GraphicsPlot* plot) { mParentPlot = plot; }
  GPLayerable *parentLayerable() const { return mParentLayerable.data(); }
  GPLayer *layer() const { return mLayer; }
  bool antialiased() const { return mAntialiased; }
  
  // setters:
  virtual void setVisible(bool on);
  Q_SLOT bool setLayer(GPLayer *layer);
  bool setLayer(const QString &layerName);
  void setAntialiased(bool enabled);
  
  // introduced virtual methods:
  virtual double selectTest(const QPointF &pos, bool onlySelectable, QVariant *details=0) const;
  virtual Qt::CursorShape HitTest(QMouseEvent* event, unsigned* part = nullptr) { Q_UNUSED(event); Q_UNUSED(part); return Qt::ArrowCursor; }
  virtual QString ToolTipTest(const QPointF &pos) const { Q_UNUSED(pos); return QString(); }

  // non-property methods:
  virtual bool realVisibility() const;
  
signals:
  void layerChanged(GPLayer *newLayer);
  
protected:
  // property members:
  bool mVisible;
  GraphicsPlot *mParentPlot;
  QPointer<GPLayerable> mParentLayerable;
  GPLayer *mLayer;
  bool mAntialiased;
  
public:
  // introduced virtual methods:
  void setParentLayerable(GPLayerable* parentLayerable);
  bool moveToLayer(GPLayer *layer, bool prepend);
  virtual void parentPlotInitialized(GraphicsPlot *parentPlot);
  virtual GP::Interaction selectionCategory() const;
  virtual QRect clipRect() const;
  virtual void applyDefaultAntialiasingHint(GPPainter *painter) const = 0;
  virtual void draw(GPPainter *painter) = 0;
  // selection events:
  virtual void selectEvent(QMouseEvent *event, bool additive, const QVariant &details, bool *selectionStateChanged);
  virtual void deselectEvent(bool *selectionStateChanged);
  // low-level mouse events:
  virtual void mousePressEvent(QMouseEvent *event, const QVariant &details);
  virtual void mouseMoveEvent(QMouseEvent *event, const QPointF &startPos);
  virtual void mouseReleaseEvent(QMouseEvent *event, const QPointF &startPos);
  virtual void mouseDoubleClickEvent(QMouseEvent *event, const QVariant &details);
  virtual void wheelEvent(QWheelEvent *event);

protected:
  // non-property methods:
  void initializeParentPlot(GraphicsPlot *parentPlot);
  void applyAntialiasingHint(GPPainter *painter, bool localAntialiased, GP::AntialiasedElement overrideElement) const;
  
private:
  Q_DISABLE_COPY(GPLayerable)
  
  friend class GraphicsPlot;
  friend class GPLayer;
  friend class GPAxisRect;
};

class GP_LIB_DECL GPSelectionRect : public GPLayerable
{
  Q_OBJECT
public:
  explicit GPSelectionRect(GraphicsPlot *parentPlot);
  virtual ~GPSelectionRect();
  
  // getters:
  QRect rect() const { return mRect; }
  GPRange range(const GPAxis *axis) const;
  QPen pen() const { return mPen; }
  QBrush brush() const { return mBrush; }
  bool isActive() const { return mActive; }
  
  // setters:
  virtual void setPen(const QPen &pen);
  virtual void setBrush(const QBrush &brush);
  
  // non-property methods:
  Q_SLOT void cancel();
  
signals:
  void started(QMouseEvent *event);
  void changed(const QRect &rect, QMouseEvent *event);
  void canceled(const QRect &rect, QInputEvent *event);
  void accepted(const QRect &rect, QMouseEvent *event);
  
protected:
  // property members:
  QRect mRect;
  QPen mPen;
  QBrush mBrush;
  // non-property members:
  bool mActive;
  
  // introduced virtual methods:
  virtual bool startSelection(QMouseEvent *event);
  virtual void moveSelection(QMouseEvent *event);
  virtual void endSelection(QMouseEvent *event);
  virtual void keyPressEvent(QKeyEvent *event);
  
  // reimplemented virtual methods
  virtual void applyDefaultAntialiasingHint(GPPainter *painter) const override;
  virtual void draw(GPPainter *painter) override;
  
  friend class GraphicsPlot;
};

class GP_LIB_DECL GPMarginGroup : public QObject
{
  Q_OBJECT
public:
  explicit GPMarginGroup(GraphicsPlot *parentPlot);
  virtual ~GPMarginGroup();
  
  // non-virtual methods:
  QList<GPLayoutElement*> elements(GP::MarginSide side) const { return mChildren.value(side); }
  bool isEmpty() const;
  void clear();

  virtual int commonMargin(GP::MarginSide side) const;
  
protected:
  // non-property members:
  GraphicsPlot *mParentPlot;
  QHash<GP::MarginSide, QList<GPLayoutElement*>> mChildren;
  
  // non-virtual methods:
  void addChild(GP::MarginSide side, GPLayoutElement *element);
  void removeChild(GP::MarginSide side, GPLayoutElement *element);
  
private:
  Q_DISABLE_COPY(GPMarginGroup)
  
  friend class GPLayoutElement;
};


class GP_LIB_DECL GPLayoutElement : public GPLayerable
{
  Q_OBJECT
  /// \cond INCLUDE_QPROPERTIES
  Q_PROPERTY(GPLayout* layout READ layout)
  Q_PROPERTY(QRect rect READ rect)
  Q_PROPERTY(QRect outerRect READ outerRect WRITE setOuterRect)
  Q_PROPERTY(QMargins margins READ margins WRITE setMargins)
  Q_PROPERTY(QMargins minimumMargins READ minimumMargins WRITE setMinimumMargins)
  Q_PROPERTY(QSize minimumSize READ minimumSize WRITE setMinimumSize)
  Q_PROPERTY(QSize maximumSize READ maximumSize WRITE setMaximumSize)
  Q_PROPERTY(SizeConstraintRect sizeConstraintRect READ sizeConstraintRect WRITE setSizeConstraintRect)
  /// \endcond
public:
  /*!
    Defines the phases of the update process, that happens just before a replot. At each phase,
    \ref update is called with the according UpdatePhase value.
  */
  enum UpdatePhase { upPreparation ///< Phase used for any type of preparation that needs to be done before margin calculation and layout
                     ,upMargins    ///< Phase in which the margins are calculated and set
                     ,upLayout     ///< Final phase in which the layout system places the rects of the elements
                   };
  Q_ENUMS(UpdatePhase)
  
  /*!
    Defines to which rect of a layout element the size constraints that can be set via \ref
    setMinimumSize and \ref setMaximumSize apply. The outer rect (\ref outerRect) includes the
    margins (e.g. in the case of a GPAxisRect the axis labels), whereas the inner rect (\ref rect)
    does not.
    
    \see setSizeConstraintRect
  */
  enum SizeConstraintRect { scrInnerRect ///< Minimum/Maximum size constraints apply to inner rect
                            , scrOuterRect ///< Minimum/Maximum size constraints apply to outer rect, thus include layout element margins
                          };
  Q_ENUMS(SizeConstraintRect)

  explicit GPLayoutElement(GraphicsPlot *parentPlot=0);
  virtual ~GPLayoutElement();
  
  // getters:
  GPLayout *layout() const { return mParentLayout; }
  QRect rect() const { return mRect; }
  QRect outerRect() const { return mOuterRect; }
  QMargins margins() const { return mMargins; }
  QMargins minimumMargins() const { return mMinimumMargins; }
  GP::MarginSides autoMargins() const { return mAutoMargins; }
  QSize minimumSize() const 
  { 
    QSize result = mMinimumSize;
    if (mFixedMinimumSize.width() >= 0)
      result = QSize(qMax(result.width(), mFixedMinimumSize.width()), 
                     result.height());
    if (mFixedMinimumSize.height() >= 0)
      result = QSize(result.width(),
                     qMax(result.height(), mFixedMinimumSize.height()));
    return result;
  }
  QSize maximumSize() const 
  { 
    QSize result = mMaximumSize;
    if (mFixedMaximumSize.width() >= 0)
      result = QSize(qMin(result.width(), mFixedMaximumSize.width()),
                     result.height());
    if (mFixedMaximumSize.height() >= 0)
      result = QSize(result.width(),
                     qMin(result.height(), mFixedMaximumSize.height()));
    return result;
  }
  QSize fixedSize() const { return mFixedSize; }
  QSize fixedMinimumSize() const { return mFixedMinimumSize; }
  QSize fixedMaximumSize() const { return mFixedMaximumSize; }
  SizeConstraintRect sizeConstraintRect() const { return mSizeConstraintRect; }
  GPMarginGroup *marginGroup(GP::MarginSide side) const { return mMarginGroups.value(side, (GPMarginGroup*)0); }
  QHash<GP::MarginSide, GPMarginGroup*> marginGroups() const { return mMarginGroups; }
  
  // setters:
  virtual void setOuterRect(const QRect &rect);
  void setMargins(const QMargins &margins);
  void setMinimumMargins(const QMargins &margins);
  void setAutoMargins(GP::MarginSides sides);
  void setMinimumSize(const QSize &size);
  void setMinimumSize(int width, int height);
  void setMaximumSize(const QSize &size);
  void setMaximumSize(int width, int height);
  void setFixedSize(const QSize &size);
  void setFixedSize(int width, int height);
  void setFixedMinimumSize(const QSize &size);
  void setFixedMinimumSize(int width, int height);
  void setFixedMaximumSize(const QSize &size);
  void setFixedMaximumSize(int width, int height);
  void setSizeConstraintRect(SizeConstraintRect constraintRect);
  void setMarginGroup(GP::MarginSides sides, GPMarginGroup *group);
  
  // introduced virtual methods:
  virtual void update(UpdatePhase phase);
  virtual QSize minimumOuterSizeHint() const;
  virtual QSize maximumOuterSizeHint() const;
  virtual QList<GPLayoutElement*> elements(bool recursive) const;
  
  // reimplemented virtual methods:
  virtual double selectTest(const QPointF &pos, bool onlySelectable, QVariant *details=0) const override;

  int layoutOffestX() const { return mLayoutOffestX; }
  int layoutOffestY() const { return mLayoutOffestY; }

  void setLayoutOffsetX(int offx) { mLayoutOffestX = offx; }
  void setLayoutOffsetY(int offy) { mLayoutOffestY = offy; }

  // drag & drop interface
  virtual void dragEnterEvent(QDragEnterEvent *event) { Q_UNUSED(event); }
  virtual void dragMoveEvent(QDragMoveEvent *event) { Q_UNUSED(event); }
  virtual void dropEvent(QDropEvent *event) { Q_UNUSED(event); }
  virtual void dragLeaveEvent(QDragLeaveEvent *event) { Q_UNUSED(event); }
  
protected:
  // property members:
  GPLayout *mParentLayout;
  QSize mMinimumSize, mMaximumSize;
  QSize mFixedSize;
  QSize mFixedMinimumSize;
  QSize mFixedMaximumSize;
  SizeConstraintRect mSizeConstraintRect;
  QRect mRect, mOuterRect;
  QMargins mMargins, mMinimumMargins;
  GP::MarginSides mAutoMargins;
  QHash<GP::MarginSide, GPMarginGroup*> mMarginGroups;
  int mLayoutOffestX;
  int mLayoutOffestY;
  
  // introduced virtual methods:
  virtual int calculateAutoMargin(GP::MarginSide side);
  virtual void layoutChanged();
  
  // reimplemented virtual methods:
  virtual void applyDefaultAntialiasingHint(GPPainter *painter) const override { Q_UNUSED(painter) }
  virtual void draw(GPPainter *painter) override { Q_UNUSED(painter) }
  virtual void parentPlotInitialized(GraphicsPlot *parentPlot) override;

private:
  Q_DISABLE_COPY(GPLayoutElement)
  
  friend class GraphicsPlot;
  friend class GPLayout;
  friend class GPMarginGroup;
};
Q_DECLARE_METATYPE(GPLayoutElement::UpdatePhase)


class GP_LIB_DECL GPLayout : public GPLayoutElement
{
  Q_OBJECT
public:
  explicit GPLayout();
  
  // reimplemented virtual methods:
  virtual void update(UpdatePhase phase) override;
  virtual QList<GPLayoutElement*> elements(bool recursive) const override;
  
  // introduced virtual methods:
  virtual void sizeConstraintsChanged();
  virtual int elementCount() const = 0;
  virtual GPLayoutElement* elementAt(int index) const = 0;
  virtual GPLayoutElement* takeAt(int index) = 0;
  virtual bool take(GPLayoutElement* element) = 0;
  virtual void simplify();
  
  // non-virtual methods:
  bool removeAt(int index);
  bool remove(GPLayoutElement* element);
  void clear();
  
protected:
  // introduced virtual methods:
  virtual void updateLayout();

  // non-virtual methods:
  void adoptElement(GPLayoutElement *el);
  void releaseElement(GPLayoutElement *el);
  QVector<int> getSectionSizes(const QVector<int>& maxSizes, QVector<int> minSizes, QVector<double> stretchFactors, int totalSize) const;
  static QSize getFinalMinimumOuterSize(const GPLayoutElement *el);
  static QSize getFinalMaximumOuterSize(const GPLayoutElement *el);
  
private:
  Q_DISABLE_COPY(GPLayout)
  friend class GPLayoutElement;
};


class GP_LIB_DECL GPLayoutGrid : public GPLayout
{
  Q_OBJECT
  /// \cond INCLUDE_QPROPERTIES
  Q_PROPERTY(int rowCount READ rowCount)
  Q_PROPERTY(int columnCount READ columnCount)
  Q_PROPERTY(QList<double> columnStretchFactors READ columnStretchFactors WRITE setColumnStretchFactors)
  Q_PROPERTY(QList<double> rowStretchFactors READ rowStretchFactors WRITE setRowStretchFactors)
  Q_PROPERTY(int columnSpacing READ columnSpacing WRITE setColumnSpacing)
  Q_PROPERTY(int rowSpacing READ rowSpacing WRITE setRowSpacing)
  Q_PROPERTY(FillOrder fillOrder READ fillOrder WRITE setFillOrder)
  Q_PROPERTY(int wrap READ wrap WRITE setWrap)
  /// \endcond
public:
  
  /*!
    Defines in which direction the grid is filled when using \ref addElement(GPLayoutElement*).
    The column/row at which wrapping into the next row/column occurs can be specified with \ref
    setWrap.

    \see setFillOrder
  */
  enum FillOrder { foRowsFirst    ///< Rows are filled first, and a new element is wrapped to the next column if the row count would exceed \ref setWrap.
                  ,foColumnsFirst ///< Columns are filled first, and a new element is wrapped to the next row if the column count would exceed \ref setWrap.
                };
  Q_ENUMS(FillOrder)
  
  explicit GPLayoutGrid();
  virtual ~GPLayoutGrid();
  
  // getters:
  int rowCount() const { return mElements.size(); }
  int columnCount() const { return mElements.size() > 0 ? mElements.first().size() : 0; }
  QList<double> columnStretchFactors() const { return mColumnStretchFactors; }
  QList<double> rowStretchFactors() const { return mRowStretchFactors; }
  int columnSpacing() const { return mColumnSpacing; }
  int rowSpacing() const { return mRowSpacing; }
  int wrap() const { return mWrap; }
  FillOrder fillOrder() const { return mFillOrder; }
  
  // setters:
  virtual void setColumnStretchFactor(int column, double factor);
  virtual void setColumnStretchFactors(const QList<double> &factors);
  virtual void setRowStretchFactor(int row, double factor);
  virtual void setRowStretchFactors(const QList<double> &factors);
  void setColumnSpacing(int pixels);
  void setRowSpacing(int pixels);
  void setWrap(int count);
  void setFillOrder(FillOrder order, bool rearrange=true);
  
  // reimplemented virtual methods:
  virtual void updateLayout() override;
  virtual int elementCount() const override { return rowCount()*columnCount(); }
  virtual GPLayoutElement* elementAt(int index) const override;
  virtual GPLayoutElement* takeAt(int index) override;
  virtual bool take(GPLayoutElement* element) override;
  virtual QList<GPLayoutElement*> elements(bool recursive) const override;
  virtual void simplify() override;
  virtual QSize minimumOuterSizeHint() const override;
  virtual QSize maximumOuterSizeHint() const override;

  // elements methods
  virtual GPLayoutElement *element(int row, int column) const;
  virtual bool addElement(int row, int column, GPLayoutElement *element);
  virtual bool addElement(GPLayoutElement *element);
  bool hasElement(int row, int column);
  int rowColToIndex(int row, int column) const;
  void indexToRowCol(int index, int &row, int &column) const;
  int elementToIndex(GPLayoutElement* element) const;

  // layout size managment methods
  virtual void expandTo(int newRowCount, int newColumnCount);
  virtual void insertRow(int newIndex);
  virtual void insertColumn(int newIndex);
  virtual void removeRow(int index);
  virtual void removeColumn(int index);
  
protected:
  // property members:
  QList<QList<GPLayoutElement*>> mElements;
  QList<double> mColumnStretchFactors;
  QList<double> mRowStretchFactors;
  int mColumnSpacing, mRowSpacing;
  int mWrap;
  FillOrder mFillOrder;
  
  // non-virtual methods:
  void getMinimumRowColSizes(QVector<int> *minColWidths, QVector<int> *minRowHeights) const;
  void getMaximumRowColSizes(QVector<int> *maxColWidths, QVector<int> *maxRowHeights) const;
  
  int getRowsTotalSpacing() const;
  int getColumnsTotalSpacing() const;

private:
  Q_DISABLE_COPY(GPLayoutGrid)
};
Q_DECLARE_METATYPE(GPLayoutGrid::FillOrder)


class GP_LIB_DECL GPLayoutInset : public GPLayout
{
  Q_OBJECT
public:
  /*!
    Defines how the placement and sizing is handled for a certain element in a GPLayoutInset.
  */
  enum InsetPlacement { ipFree            ///< The element may be positioned/sized arbitrarily, see \ref setInsetRect
                        ,ipBorderAligned  ///< The element is aligned to one of the layout sides, see \ref setInsetAlignment
                      };
  Q_ENUMS(InsetPlacement)
  
  explicit GPLayoutInset();
  virtual ~GPLayoutInset();
  
  // getters:
  InsetPlacement insetPlacement(int index) const;
  Qt::Alignment insetAlignment(int index) const;
  QRectF insetRect(int index) const;
  
  // setters:
  void setInsetPlacement(int index, InsetPlacement placement);
  void setInsetAlignment(int index, Qt::Alignment alignment);
  void setInsetRect(int index, const QRectF &rect);
  
  // reimplemented virtual methods:
  virtual void updateLayout() override;
  virtual int elementCount() const override;
  virtual GPLayoutElement* elementAt(int index) const override;
  virtual GPLayoutElement* takeAt(int index) override;
  virtual bool take(GPLayoutElement* element) override;
  virtual int indexOf(GPLayoutElement* elem);
  virtual void simplify() override {}
  virtual double selectTest(const QPointF &pos, bool onlySelectable, QVariant *details=0) const override;
  
  // non-virtual methods:
  void addElement(GPLayoutElement *element, Qt::Alignment alignment);
  void addElement(GPLayoutElement *element, const QRectF &rect);
  
protected:
  // property members:
  QList<GPLayoutElement*> mElements;
  QList<InsetPlacement> mInsetPlacement;
  QList<Qt::Alignment> mInsetAlignment;
  QList<QRectF> mInsetRect;
  
private:
  Q_DISABLE_COPY(GPLayoutInset)
};
Q_DECLARE_METATYPE(GPLayoutInset::InsetPlacement)

class GP_LIB_DECL GPLineEnding
{
  Q_GADGET
public:
  /*!
    Defines the type of ending decoration for line-like items, e.g. an arrow.
    
    \image html GPLineEnding.png
    
    The width and length of these decorations can be controlled with the functions \ref setWidth
    and \ref setLength. Some decorations like \ref esDisc, \ref esSquare, \ref esDiamond and \ref esBar only
    support a width, the length property is ignored.
    
    \see GPItemLine::setHead, GPItemLine::setTail, GPItemCurve::setHead, GPItemCurve::setTail, GPAxis::setLowerEnding, GPAxis::setUpperEnding
  */
  enum EndingStyle { esNone          ///< No ending decoration
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
  Q_ENUMS(EndingStyle)
  
  GPLineEnding();
  GPLineEnding(EndingStyle style, double width=8, double length=10, bool inverted=false);
  
  // getters:
  EndingStyle style() const { return mStyle; }
  double width() const { return mWidth; }
  double length() const { return mLength; }
  bool inverted() const { return mInverted; }
  
  // setters:
  void setStyle(EndingStyle style);
  void setWidth(double width);
  void setLength(double length);
  void setInverted(bool inverted);
  
  // non-property methods:
  double boundingDistance() const;
  double realLength() const;
  void draw(GPPainter *painter, const GPVector2D &pos, const GPVector2D &dir) const;
  void draw(GPPainter *painter, const GPVector2D &pos, double angle) const;
  void draw(QPainter *painter, const GPVector2D &pos, const GPVector2D &dir) const;
  
protected:
  // property members:
  EndingStyle mStyle;
  double mWidth, mLength;
  bool mInverted;
};
Q_DECLARE_TYPEINFO(GPLineEnding, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(GPLineEnding::EndingStyle)

class GP_LIB_DECL GPAxisTicker : public QObject
{
  Q_OBJECT
public:
  /*!
    Defines the strategies that the axis ticker may follow when choosing the size of the tick step.
    
    \see setTickStepStrategy
  */
  enum TickStepStrategy
  {
    tssReadability    ///< A nicely readable tick step is prioritized over matching the requested number of ticks (see \ref setTickCount)
    ,tssMeetTickCount ///< Less readable tick steps are allowed which in turn facilitates getting closer to the requested tick count
  };
  Q_ENUMS(TickStepStrategy)
  
  GPAxisTicker();
  virtual ~GPAxisTicker();
  
  // getters:
  TickStepStrategy tickStepStrategy() const { return mTickStepStrategy; }
  int tickCount() const { return mTickCount; }
  double tickOrigin() const { return mTickOrigin; }
  
  // setters:
  void setTickStepStrategy(TickStepStrategy strategy);
  void setTickCount(int count);
  void setTickOrigin(double origin);
  
  // introduced virtual methods:
  virtual void generate(const GPRange &range, const QLocale &locale, QChar formatChar, int precision, QVector<double> &ticks, QVector<double> *subTicks, QVector<QString> *tickLabels);
  
  virtual double getTickStep(const GPRange &range);
  virtual QString getTickLabel(double tick, const QLocale &locale, QChar formatChar, int precision);
  virtual QString getTickLabel(double tick);

protected:
  // property members:
  TickStepStrategy mTickStepStrategy;
  int mTickCount;
  double mTickOrigin;
  
  // introduced virtual methods:
  virtual int getSubTickCount(double tickStep);
  virtual QVector<double> createTickVector(double tickStep, const GPRange &range);
  virtual QVector<double> createSubTickVector(int subTickCount, const QVector<double> &ticks);
  virtual QVector<QString> createLabelVector(const QVector<double> &ticks, const QLocale &locale, QChar formatChar, int precision, const GPRange &range);
  
  // non-virtual methods:
  void trimTicks(const GPRange &range, QVector<double> &ticks, bool keepOneOutlier) const;
  double pickClosest(double target, const QVector<double> &candidates) const;
  double getMantissa(double input, double *magnitude=0) const;
  double cleanMantissa(double input) const;
};
Q_DECLARE_METATYPE(GPAxisTicker::TickStepStrategy)
Q_DECLARE_METATYPE(QSharedPointer<GPAxisTicker>)

class GP_LIB_DECL GPAxisTickerDateTime : public GPAxisTicker
{
  Q_OBJECT
public:
  GPAxisTickerDateTime();
  
  // getters:
  QString dateTimeFormat() const { return mDateTimeFormat; }
  Qt::TimeSpec dateTimeSpec() const { return mDateTimeSpec; }
  
  // setters:
  void setDateTimeFormat(const QString &format);
  void setDateTimeSpec(Qt::TimeSpec spec);
  void setTickOrigin(double origin); // hides base class method but calls baseclass implementation ("using" throws off IDEs and doxygen)
  void setTickOrigin(const QDateTime &origin);
  
  // virtual methods:
  virtual QDateTime keyToDateTime(double key);
  virtual double dateTimeToKey(const QDateTime& dateTime);
  virtual double dateTimeToKey(const QDate& date);
  
protected:
  // property members:
  QString mDateTimeFormat;
  Qt::TimeSpec mDateTimeSpec;
  
  // non-property members:
  enum DateStrategy {dsNone, dsUniformTimeInDay, dsUniformDayInMonth} mDateStrategy;
  
  // reimplemented virtual methods:
  virtual double getTickStep(const GPRange &range) override;
  virtual int getSubTickCount(double tickStep) override;
  virtual QString getTickLabel(double tick, const QLocale &locale, QChar formatChar, int precision) override;
  virtual QVector<double> createTickVector(double tickStep, const GPRange &range) override;
};

class GP_LIB_DECL GPAxisTickerTime : public GPAxisTicker
{
  Q_GADGET
public:
  /*!
    Defines the logical units in which fractions of time spans can be expressed.
    
    \see setFieldWidth, setTimeFormat
  */
  enum TimeUnit { tuMilliseconds ///< Milliseconds, one thousandth of a second (%%z in \ref setTimeFormat)
                  ,tuSeconds     ///< Seconds (%%s in \ref setTimeFormat)
                  ,tuMinutes     ///< Minutes (%%m in \ref setTimeFormat)
                  ,tuHours       ///< Hours (%%h in \ref setTimeFormat)
                  ,tuDays        ///< Days (%%d in \ref setTimeFormat)
                };
  Q_ENUMS(TimeUnit)
  
  GPAxisTickerTime();

  // getters:
  QString timeFormat() const { return mTimeFormat; }
  int fieldWidth(TimeUnit unit) const { return mFieldWidth.value(unit); }
  
  // setters:
  void setTimeFormat(const QString &format);
  void setFieldWidth(TimeUnit unit, int width);
  
protected:
  // property members:
  QString mTimeFormat;
  QHash<TimeUnit, int> mFieldWidth;
  
  // non-property members:
  TimeUnit mSmallestUnit, mBiggestUnit;
  QHash<TimeUnit, QString> mFormatPattern;
  
  // reimplemented virtual methods:
  virtual double getTickStep(const GPRange &range) override;
  virtual int getSubTickCount(double tickStep) override;
  virtual QString getTickLabel(double tick, const QLocale &locale, QChar formatChar, int precision) override;
  
  // non-virtual methods:
  void replaceUnit(QString &text, TimeUnit unit, int value) const;
};
Q_DECLARE_METATYPE(GPAxisTickerTime::TimeUnit)

class GP_LIB_DECL GPAxisTickerFixed : public GPAxisTicker
{
  Q_GADGET
public:
  /*!
    Defines how the axis ticker may modify the specified tick step (\ref setTickStep) in order to
    control the number of ticks in the axis range.
    
    \see setScaleStrategy
  */
  enum ScaleStrategy { ssNone      ///< Modifications are not allowed, the specified tick step is absolutely fixed. This might cause a high tick density and overlapping labels if the axis range is zoomed out.
                       ,ssMultiples ///< An integer multiple of the specified tick step is allowed. The used factor follows the base class properties of \ref setTickStepStrategy and \ref setTickCount.
                       ,ssPowers    ///< An integer power of the specified tick step is allowed.
                     };
  Q_ENUMS(ScaleStrategy)
  
  GPAxisTickerFixed();
  
  // getters:
  double tickStep() const { return mTickStep; }
  ScaleStrategy scaleStrategy() const { return mScaleStrategy; }
  
  // setters:
  void setTickStep(double step);
  void setScaleStrategy(ScaleStrategy strategy);
  
protected:
  // property members:
  double mTickStep;
  ScaleStrategy mScaleStrategy;
  
  // reimplemented virtual methods:
  virtual double getTickStep(const GPRange &range) override;
};
Q_DECLARE_METATYPE(GPAxisTickerFixed::ScaleStrategy)

class GP_LIB_DECL GPAxisTickerText : public GPAxisTicker
{
public:
  GPAxisTickerText();
  
  // getters:
  QMap<double, QString> &ticks() { return mTicks; }
  int subTickCount() const { return mSubTickCount; }
  
  // setters:
  void setTicks(const QMap<double, QString>& ticks);
  void setTicks(const QVector<double> &positions, const QVector<QString>& labels);
  void setSubTickCount(int subTicks);
  
  // non-virtual methods:
  void clear();
  void addTick(double position, const QString& label);
  void addTicks(const QMap<double, QString>& ticks);
  void addTicks(const QVector<double> &positions, const QVector<QString> &labels);
  
protected:
  // property members:
  QMap<double, QString> mTicks;
  int mSubTickCount;
  
  // reimplemented virtual methods:
  virtual double getTickStep(const GPRange &range) override;
  virtual int getSubTickCount(double tickStep) override;
  virtual QString getTickLabel(double tick, const QLocale &locale, QChar formatChar, int precision) override;
  virtual QVector<double> createTickVector(double tickStep, const GPRange &range) override;
  
};

class GP_LIB_DECL GPAxisTickerPi : public GPAxisTicker
{
  Q_GADGET
public:
  /*!
    Defines how fractions should be displayed in tick labels.
    
    \see setFractionStyle
  */
  enum FractionStyle { fsFloatingPoint     ///< Fractions are displayed as regular decimal floating point numbers, e.g. "0.25" or "0.125".
                       ,fsAsciiFractions   ///< Fractions are written as rationals using ASCII characters only, e.g. "1/4" or "1/8"
                       ,fsUnicodeFractions ///< Fractions are written using sub- and superscript UTF-8 digits and the fraction symbol.
                     };
  Q_ENUMS(FractionStyle)
  
  GPAxisTickerPi();
  
  // getters:
  QString piSymbol() const { return mPiSymbol; }
  double piValue() const { return mPiValue; }
  bool periodicity() const { return mPeriodicity; }
  FractionStyle fractionStyle() const { return mFractionStyle; }
  
  // setters:
  void setPiSymbol(const QString& symbol);
  void setPiValue(double pi);
  void setPeriodicity(int multiplesOfPi);
  void setFractionStyle(FractionStyle style);
  
protected:
  // property members:
  QString mPiSymbol;
  double mPiValue;
  int mPeriodicity;
  FractionStyle mFractionStyle;
  
  // non-property members:
  double mPiTickStep; // size of one tick step in units of mPiValue
  
  // reimplemented virtual methods:
  virtual double getTickStep(const GPRange &range) override;
  virtual int getSubTickCount(double tickStep) override;
  virtual QString getTickLabel(double tick, const QLocale &locale, QChar formatChar, int precision) override;
  
  // non-virtual methods:
  void simplifyFraction(int &numerator, int &denominator) const;
  QString fractionToString(int numerator, int denominator) const;
  QString unicodeFraction(int numerator, int denominator) const;
  QString unicodeSuperscript(int number) const;
  QString unicodeSubscript(int number) const;
};
Q_DECLARE_METATYPE(GPAxisTickerPi::FractionStyle)

class GP_LIB_DECL GPAxisTickerLog : public GPAxisTicker
{
public:
  GPAxisTickerLog();
  
  // getters:
  double logBase() const { return mLogBase; }
  int subTickCount() const { return mSubTickCount; }
  
  // setters:
  void setLogBase(double base);
  void setSubTickCount(int subTicks);
  
protected:
  // property members:
  double mLogBase;
  int mSubTickCount;
  
  // non-property members:
  double mLogBaseLnInv;
  
  // reimplemented virtual methods:
  virtual double getTickStep(const GPRange &range) override;
  virtual int getSubTickCount(double tickStep) override;
  virtual QVector<double> createTickVector(double tickStep, const GPRange &range) override;
};

class GP_LIB_DECL GPGrid :public GPLayerable
{
  Q_OBJECT
  /// \cond INCLUDE_QPROPERTIES
  Q_PROPERTY(bool subGridVisible READ subGridVisible WRITE setSubGridVisible)
  Q_PROPERTY(bool antialiasedSubGrid READ antialiasedSubGrid WRITE setAntialiasedSubGrid)
  Q_PROPERTY(bool antialiasedZeroLine READ antialiasedZeroLine WRITE setAntialiasedZeroLine)
  Q_PROPERTY(QPen pen READ pen WRITE setPen)
  Q_PROPERTY(QPen subGridPen READ subGridPen WRITE setSubGridPen)
  Q_PROPERTY(QPen zeroLinePen READ zeroLinePen WRITE setZeroLinePen)
  /// \endcond
public:
  explicit GPGrid(GPAxis* parentAxis);
  explicit GPGrid(GPLayerable* parent);

  // getters:
  bool subGridVisible() const { return mSubGridVisible; }
  bool antialiasedSubGrid() const { return mAntialiasedSubGrid; }
  bool antialiasedZeroLine() const { return mAntialiasedZeroLine; }
  QPen pen() const { return mPen; }
  QPen subGridPen() const { return mSubGridPen; }
  QPen zeroLinePen() const { return mZeroLinePen; }
  
  // setters:
  void setSubGridVisible(bool visible);
  void setAntialiasedSubGrid(bool enabled);
  void setAntialiasedZeroLine(bool enabled);
  void setPen(const QPen &pen);
  void setSubGridPen(const QPen &pen);
  void setZeroLinePen(const QPen &pen);
  
protected:
  // property members:
  bool mSubGridVisible;
  bool mAntialiasedSubGrid, mAntialiasedZeroLine;
  QPen mPen, mSubGridPen, mZeroLinePen;
  
  // non-property members:
  GPAxis *mParentAxis;
  
  // reimplemented virtual methods:
  virtual void applyDefaultAntialiasingHint(GPPainter *painter) const override;
  virtual void draw(GPPainter *painter) override;
  
  // virtual methods:
  virtual void drawGridLines(GPPainter *painter) const;
  virtual void drawSubGridLines(GPPainter *painter) const;
  
  friend class GPAxis;
};


class GP_LIB_DECL GPAxis : public GPLayerable
                           , public GPUomDescriptorProvider
{
  Q_OBJECT
  /// \cond INCLUDE_QPROPERTIES
  Q_PROPERTY(AxisType axisType READ axisType)
  Q_PROPERTY(GPAxisRect* axisRect READ axisRect)
  Q_PROPERTY(ScaleType scaleType READ scaleType WRITE setScaleType NOTIFY scaleTypeChanged)
  Q_PROPERTY(GPRange range READ range WRITE setRange NOTIFY rangeChanged)
  Q_PROPERTY(double rangeMinimum READ rangeMinimum WRITE setRangeMinimum)
  Q_PROPERTY(double rangeMaximum READ rangeMaximum WRITE setRangeMaximum)
  Q_PROPERTY(bool rangeReversed READ rangeReversed WRITE setRangeReversed)
  Q_PROPERTY(QSharedPointer<GPAxisTicker> ticker READ ticker WRITE setTicker)
  Q_PROPERTY(bool ticks READ ticks WRITE setTicks)
  Q_PROPERTY(bool tickLabels READ tickLabels WRITE setTickLabels)
  Q_PROPERTY(int tickLabelPadding READ tickLabelPadding WRITE setTickLabelPadding)
  Q_PROPERTY(QFont tickLabelFont READ tickLabelFont WRITE setTickLabelFont)
  Q_PROPERTY(QColor tickLabelColor READ tickLabelColor WRITE setTickLabelColor)
  Q_PROPERTY(double tickLabelRotation READ tickLabelRotation WRITE setTickLabelRotation)
  Q_PROPERTY(LabelSide tickLabelSide READ tickLabelSide WRITE setTickLabelSide)
  Q_PROPERTY(QString numberFormat READ numberFormat WRITE setNumberFormat)
  Q_PROPERTY(int numberPrecision READ numberPrecision WRITE setNumberPrecision)
  Q_PROPERTY(QVector<double> tickVector READ tickVector)
  Q_PROPERTY(QVector<QString> tickVectorLabels READ tickVectorLabels)
  Q_PROPERTY(int tickLengthIn READ tickLengthIn WRITE setTickLengthIn)
  Q_PROPERTY(int tickLengthOut READ tickLengthOut WRITE setTickLengthOut)
  Q_PROPERTY(bool subTicks READ subTicks WRITE setSubTicks)
  Q_PROPERTY(int subTickLengthIn READ subTickLengthIn WRITE setSubTickLengthIn)
  Q_PROPERTY(int subTickLengthOut READ subTickLengthOut WRITE setSubTickLengthOut)
  Q_PROPERTY(QPen basePen READ basePen WRITE setBasePen)
  Q_PROPERTY(QPen tickPen READ tickPen WRITE setTickPen)
  Q_PROPERTY(QPen subTickPen READ subTickPen WRITE setSubTickPen)
  Q_PROPERTY(QFont labelFont READ labelFont WRITE setLabelFont)
  Q_PROPERTY(QColor labelColor READ labelColor WRITE setLabelColor)
  Q_PROPERTY(QString label READ label WRITE setLabel)
  Q_PROPERTY(int labelPadding READ labelPadding WRITE setLabelPadding)
  Q_PROPERTY(int padding READ padding WRITE setPadding)
  Q_PROPERTY(int offset READ offset WRITE setOffset)
  Q_PROPERTY(SelectableParts selectedParts READ selectedParts WRITE setSelectedParts NOTIFY selectionChanged)
  Q_PROPERTY(SelectableParts selectableParts READ selectableParts WRITE setSelectableParts NOTIFY selectableChanged)
  Q_PROPERTY(QFont selectedTickLabelFont READ selectedTickLabelFont WRITE setSelectedTickLabelFont)
  Q_PROPERTY(QFont selectedLabelFont READ selectedLabelFont WRITE setSelectedLabelFont)
  Q_PROPERTY(QColor selectedTickLabelColor READ selectedTickLabelColor WRITE setSelectedTickLabelColor)
  Q_PROPERTY(QColor selectedLabelColor READ selectedLabelColor WRITE setSelectedLabelColor)
  Q_PROPERTY(QPen selectedBasePen READ selectedBasePen WRITE setSelectedBasePen)
  Q_PROPERTY(QPen selectedTickPen READ selectedTickPen WRITE setSelectedTickPen)
  Q_PROPERTY(QPen selectedSubTickPen READ selectedSubTickPen WRITE setSelectedSubTickPen)
  Q_PROPERTY(GPLineEnding lowerEnding READ lowerEnding WRITE setLowerEnding)
  Q_PROPERTY(GPLineEnding upperEnding READ upperEnding WRITE setUpperEnding)
  Q_PROPERTY(GPGrid* grid READ grid)
  /// \endcond
public:
  /*!
    Defines at which side of the axis rect the axis will appear. This also affects how the tick
    marks are drawn, on which side the labels are placed etc.
  */
  enum AxisType { atLeft    = 0x01  ///< <tt>0x01</tt> Axis is vertical and on the left side of the axis rect
                  ,atRight  = 0x02  ///< <tt>0x02</tt> Axis is vertical and on the right side of the axis rect
                  ,atTop    = 0x04  ///< <tt>0x04</tt> Axis is horizontal and on the top side of the axis rect
                  ,atBottom = 0x08  ///< <tt>0x08</tt> Axis is horizontal and on the bottom side of the axis rect
                };
  Q_ENUMS(AxisType)
  Q_FLAGS(AxisTypes)
  Q_DECLARE_FLAGS(AxisTypes, AxisType)
  /*!
    Defines on which side of the axis the tick labels (numbers) shall appear.
    
    \see setTickLabelSide
  */
  enum LabelSide { lsInside    ///< Tick labels will be displayed inside the axis rect and clipped to the inner axis rect
                   ,lsOutside  ///< Tick labels will be displayed outside the axis rect
                 };
  Q_ENUMS(LabelSide)
  /*!
    Defines the scale of an axis.
    \see setScaleType
  */
  enum ScaleType { stLinear       ///< Linear scaling
                   ,stLogarithmic ///< Logarithmic scaling with correspondingly transformed axis coordinates (possibly also \ref setTicker to a \ref GPAxisTickerLog instance).
                 };
  Q_ENUMS(ScaleType)
  /*!
    Defines the selectable parts of an axis.
    \see setSelectableParts, setSelectedParts
  */
  enum SelectablePart { spNone        = 0      ///< None of the selectable parts
                        ,spAxis       = 0x001  ///< The axis backbone and tick marks
                        ,spTickLabels = 0x002  ///< Tick labels (numbers) of this axis (as a whole, not individually)
                        ,spAxisLabel  = 0x004  ///< The axis label
                        ,spAxisFull      = spAxis | spTickLabels | spAxisLabel
                      };
  Q_ENUMS(SelectablePart)
  Q_FLAGS(SelectableParts)
  Q_DECLARE_FLAGS(SelectableParts, SelectablePart)
  
  explicit GPAxis(GPAxisRect *parent, AxisType type);
  virtual ~GPAxis();
  
  // getters:
  AxisType axisType() const { return mAxisType; }
  void setAxisRect(GPAxisRect* rect) { mAxisRect = rect; }
  GPAxisRect* axisRect() const { return mAxisRect.data(); }
  ScaleType scaleType() const { return mScaleType; }
  const GPRange range() const { return mRange; }
  bool rangeReversed() const { return mRangeReversed; }
  bool rangeMinimum() const { return mRangeMinimum; }
  bool rangeMaximum() const { return mRangeMaximum; }
  bool rangeLockedLower() const { return mRangeLockedLower; }
  bool rangeLockedUpper() const { return mRangeLockedUpper; }
  double rangeLockedMiddle() const { return mRangeLockedMiddle; }
  QSharedPointer<GPAxisTicker> ticker() const { return mTicker; }
  bool ticks() const { return mTicks; }
  bool tickLabels() const { return mTickLabels; }
  int tickLabelPadding() const;
  QFont tickLabelFont() const { return mTickLabelFont; }
  QColor tickLabelColor() const { return mTickLabelColor; }
  double tickLabelRotation() const;
  LabelSide tickLabelSide() const;
  QString numberFormat() const;
  int numberPrecision() const { return mNumberPrecision; }
  QVector<double> tickVector() const { return mTickVector; }
  QVector<QString> tickVectorLabels() const { return mTickVectorLabels; }
  QString tickLabel(double coord) const;
  int tickLengthIn() const;
  int tickLengthOut() const;
  bool subTicks() const { return mSubTicks; }
  int subTickLengthIn() const;
  int subTickLengthOut() const;
  QPen basePen() const { return mBasePen; }
  QPen tickPen() const { return mTickPen; }
  QPen subTickPen() const { return mSubTickPen; }
  QFont labelFont() const { return mLabelFont; }
  QColor labelColor() const { return mLabelColor; }
  QString label() const { return mLabel; }
  int labelPadding() const;
  int padding() const { return mPadding; }
  int offset() const;
  SelectableParts selectedParts() const { return mSelectedParts; }
  SelectableParts selectableParts() const { return mSelectableParts; }
  QFont selectedTickLabelFont() const { return mSelectedTickLabelFont; }
  QFont selectedLabelFont() const { return mSelectedLabelFont; }
  QColor selectedTickLabelColor() const { return mSelectedTickLabelColor; }
  QColor selectedLabelColor() const { return mSelectedLabelColor; }
  QPen selectedBasePen() const { return mSelectedBasePen; }
  QPen selectedTickPen() const { return mSelectedTickPen; }
  QPen selectedSubTickPen() const { return mSelectedSubTickPen; }
  GPLineEnding lowerEnding() const;
  GPLineEnding upperEnding() const;
  GPGrid *grid() const { return mGrid; }
  
  // setters:
  Q_SLOT void setScaleType(GPAxis::ScaleType type);
  Q_SLOT void setRange(const GPRange &range);
  void setRange(double lower, double upper);
  void setRange(double position, double size, Qt::AlignmentFlag alignment);
  Q_SLOT void setRangeLower(double lower);
  Q_SLOT void setRangeUpper(double upper);

  void setAxisType(AxisType type);
  void setRangeMinimum(double minimum);
  void setRangeMaximum(double maximum);
  void setRangeReversed(bool reversed);
  void setRangeLockedLower(bool lockedLower);
  void setRangeLockedUpper(bool lockedUpper);
  void setRangeLockedMiddle(double middle);
  void setTicker(const QSharedPointer<GPAxisTicker>& ticker);
  void setTicks(bool show);
  void setTickLabels(bool show);
  void setTickLabelPadding(int padding);
  void setTickLabelFont(const QFont &font);
  void setTickLabelColor(const QColor &color);
  void setTickLabelRotation(double degrees);
  void setTickLabelSide(LabelSide side);
  void setNumberFormat(const QString &formatCode);
  void setNumberPrecision(int precision);
  void setTickLength(int inside, int outside=0);
  void setTickLengthIn(int inside);
  void setTickLengthOut(int outside);
  void setSubTicks(bool show);
  void setSubTickLength(int inside, int outside=0);
  void setSubTickLengthIn(int inside);
  void setSubTickLengthOut(int outside);
  void setBasePen(const QPen &pen);
  void setTickPen(const QPen &pen);
  void setSubTickPen(const QPen &pen);
  void setLabelFont(const QFont &font);
  void setLabelColor(const QColor &color);
  void setLabel(const QString &str);
  void setLabelVisible(bool show);
  void setShowCoords(bool show);
  void setLabelPadding(int padding);
  void setPadding(int padding);
  void setOffset(int offset);
  void setSelectedTickLabelFont(const QFont &font);
  void setSelectedLabelFont(const QFont &font);
  void setSelectedTickLabelColor(const QColor &color);
  void setSelectedLabelColor(const QColor &color);
  void setSelectedBasePen(const QPen &pen);
  void setSelectedTickPen(const QPen &pen);
  void setSelectedSubTickPen(const QPen &pen);
  Q_SLOT void setSelectableParts(const GPAxis::SelectableParts &selectableParts);
  Q_SLOT void setSelectedParts(const GPAxis::SelectableParts &selectedParts);
  void setLowerEnding(const GPLineEnding &ending);
  void setUpperEnding(const GPLineEnding &ending);
  
  // reimplemented virtual methods:
  virtual double selectTest(const QPointF &pos, bool onlySelectable, QVariant *details=0) const override;
  
  // non-property methods:
  Qt::Orientation orientation() const { return mOrientation; }
  int pixelOrientation() const { return rangeReversed() != (orientation()==Qt::Vertical) ? -1 : 1; }
  void moveRange(double diff);
  void scaleRange(double factor);
  void scaleRange(double factor, double center);
  void setScaleRatio(const GPAxis *otherAxis, double ratio=1.0);
  void rescale(bool onlyVisiblePlottables=false);
  double pixelToCoord(const double& value) const;
  double coordToPixel(const double& value, const GPRange &range) const;
  double coordToPixel(const double& value) const;
  double pixelToSICoord(const double& value) const;
  double coordSIToPixel(const double& value) const;
  SelectablePart getPartAt(const QPointF &pos) const;
  QList<GPAbstractPlottable*> plottables() const;
  QList<GPGraph*> graphs() const;
  QList<GPCurve*> curves() const;
  QList<GPPointAttachedTextElement*> pointAttachedTextElements() const;
  QList<GPAbstractItem*> items() const;
  
  static GP::MarginSide axisTypeToMarginSide(AxisType type);
  static AxisType marginSideToAxisType(GP::MarginSide side);
  static Qt::Orientation orientation(AxisType type) { return type==atBottom||type==atTop ? Qt::Horizontal : Qt::Vertical; }
  static AxisType opposite(AxisType type);

  void subscribeTo(GPAxis* source);
  void unsubscribeFrom(GPAxis* source);
  void emitRangeChangedToSybscriptions(const GPRange& newRange);

  //// UOM fetaures
  void setUomSemantics(const QString& semantics);
  QString getUomSemantics() const;
  Q_SLOT void updateUomDescriptor(const QString& semantics);

  bool canChangeUom() const { return mCanChangeUom; }
  void setCanChangeUom(bool can) { mCanChangeUom = can; }

  virtual GPUomDescriptor getUomDescriptor() const override;
  virtual GPUomDataTransformation getUomTransformation(const GPUomDescriptor& source) const override;
  virtual GPUomDataTransformation getUomTransformation(const GPUomDescriptor& source, const GPUomDescriptor& target) const;
  
signals:
  void rangeChanged(const GPRange &newRange);
  void rangeChanged(const GPRange &newRange, const GPRange &oldRange);
  void scaleTypeChanged(GPAxis::ScaleType scaleType);
  void selectionChanged(const GPAxis::SelectableParts &parts);
  void selectableChanged(const GPAxis::SelectableParts &parts);

protected:
  // property members:
  // axis base:
  AxisType mAxisType;
  QPointer<GPAxisRect> mAxisRect;
  //int mOffset; // in GPAxisPainter
  int mPadding;
  Qt::Orientation mOrientation;
  SelectableParts mSelectableParts, mSelectedParts;
  QPen mBasePen, mSelectedBasePen;
  //GPLineEnding mLowerEnding, mUpperEnding; // in GPAxisPainter
  // axis label:
  //int mLabelPadding; // in GPAxisPainter
  QString mLabel;
  bool mLabelVisible;
  bool mShowCoords;
  QFont mLabelFont, mSelectedLabelFont;
  QColor mLabelColor, mSelectedLabelColor;
  // tick labels:
  //int mTickLabelPadding; // in GPAxisPainter
  bool mTickLabels;
  //double mTickLabelRotation; // in GPAxisPainter
  QFont mTickLabelFont, mSelectedTickLabelFont;
  QColor mTickLabelColor, mSelectedTickLabelColor;
  int mNumberPrecision;
  QLatin1Char mNumberFormatChar;
  bool mNumberBeautifulPowers;
  //bool mNumberMultiplyCross; // GPAxisPainter
  // ticks and subticks:
  bool mTicks;
  bool mSubTicks;
  //int mTickLengthIn, mTickLengthOut, mSubTickLengthIn, mSubTickLengthOut; // GPAxisPainter
  QPen mTickPen, mSelectedTickPen;
  QPen mSubTickPen, mSelectedSubTickPen;
  // scale and range:
  GPRange mRange;
  double mRangeMinimum;
  double mRangeMaximum;
  bool mRangeReversed;
  bool mRangeLockedLower;
  bool mRangeLockedUpper;
  double mRangeLockedMiddle;
  ScaleType mScaleType;
  
  // non-property members:
  GPGrid *mGrid;
  GPAxisPainterPrivate *mAxisPainter;
  QSharedPointer<GPAxisTicker> mTicker;
  QVector<double> mTickVector;
  QVector<QString> mTickVectorLabels;
  QVector<double> mSubTickVector;
  bool mCachedMarginValid;
  int mCachedMargin;
  bool mDragging;
  GPRange mDragStartRange;
  GP::AntialiasedElements mAADragBackup, mNotAADragBackup;

  // subscriptions
  QMap<GPAxis*, QPointer<GPAxis>> mSubscriptions;

  QString mUomSemantics;
  GPUomDescriptor mUomDescriptor;
  bool mCanChangeUom;
  
public:
  // reimplemented virtual methods:
  virtual int calculateMargin();
  virtual QString getLabelDisplayText() const;
  virtual void applyDefaultAntialiasingHint(GPPainter *painter) const override;
  virtual void draw(GPPainter *painter) override;
  virtual GP::Interaction selectionCategory() const override;
  // events:
  virtual void selectEvent(QMouseEvent *event, bool additive, const QVariant &details, bool *selectionStateChanged) override;
  virtual void deselectEvent(bool *selectionStateChanged) override;
  // mouse events:
  virtual void mousePressEvent(QMouseEvent *event, const QVariant &details) override;
  virtual void mouseMoveEvent(QMouseEvent *event, const QPointF &startPos) override;
  virtual void mouseReleaseEvent(QMouseEvent *event, const QPointF &startPos) override;
  virtual void wheelEvent(QWheelEvent *event) override;

protected:  
  // non-virtual methods:
  void setupTickVectors();
  QPen getBasePen() const;
  QPen getTickPen() const;
  QPen getSubTickPen() const;
  QFont getTickLabelFont() const;
  QFont getLabelFont() const;
  QColor getTickLabelColor() const;
  QColor getLabelColor() const;
  
private:
  Q_DISABLE_COPY(GPAxis)
  
  friend class GraphicsPlot;
  friend class GPGrid;
  friend class GPAxisRect;
};
Q_DECLARE_OPERATORS_FOR_FLAGS(GPAxis::SelectableParts)
Q_DECLARE_OPERATORS_FOR_FLAGS(GPAxis::AxisTypes)
Q_DECLARE_METATYPE(GPAxis::AxisType)
Q_DECLARE_METATYPE(GPAxis::LabelSide)
Q_DECLARE_METATYPE(GPAxis::ScaleType)
Q_DECLARE_METATYPE(GPAxis::SelectablePart)


class GPAxisPainterPrivate
{
public:
  explicit GPAxisPainterPrivate(GraphicsPlot *parentPlot);
  virtual ~GPAxisPainterPrivate();
  
  virtual void draw(GPPainter *painter);
  virtual int size();
  void clearCache();
  
  QRect axisSelectionBox() const { return mAxisSelectionBox; }
  QRect tickLabelsSelectionBox() const { return mTickLabelsSelectionBox; }
  QRect labelSelectionBox() const { return mLabelSelectionBox; }
  QRect axisFullBox() const { return mAxisFullBox; }
  
  // public property members:
  GPAxis::AxisType type;
  QPen basePen;
  GPLineEnding lowerEnding, upperEnding; // directly accessed by GPAxis setters/getters
  int labelPadding; // directly accessed by GPAxis setters/getters
  QFont labelFont;
  QColor labelColor;
  QString label;
  int tickLabelPadding; // directly accessed by GPAxis setters/getters
  double tickLabelRotation; // directly accessed by GPAxis setters/getters
  GPAxis::LabelSide tickLabelSide; // directly accessed by GPAxis setters/getters
  bool substituteExponent;
  bool numberMultiplyCross; // directly accessed by GPAxis setters/getters
  int tickLengthIn, tickLengthOut, subTickLengthIn, subTickLengthOut; // directly accessed by GPAxis setters/getters
  QPen tickPen, subTickPen;
  QFont tickLabelFont;
  QColor tickLabelColor;
  QRect axisRect, viewportRect;
  double offset; // directly accessed by GPAxis setters/getters
  bool abbreviateDecimalPowers;
  bool reversedEndings;
  
  QVector<double> subTickPositions;
  QVector<double> tickPositions;
  QVector<QString> tickLabels;
  
protected:
  struct CachedLabel
  {
    QPointF offset;
    QPixmap pixmap;
  };
  struct TickLabelData
  {
    QString basePart, expPart, suffixPart;
    QRect baseBounds, expBounds, suffixBounds, totalBounds, rotatedTotalBounds;
    QFont baseFont, expFont;
  };
  GraphicsPlot *mParentPlot;
  QByteArray mTickLabelParameterHash; // to determine whether mLabelCache needs to be cleared due to changed parameters
  QByteArray mLabelParameterHash;
  QCache<QString, CachedLabel> mLabelCache;
  QImage mNameLabelCache;
  QRect mAxisSelectionBox, mTickLabelsSelectionBox, mLabelSelectionBox, mAxisFullBox;
  
  virtual QByteArray generateTickLabelParameterHash() const;
  virtual QByteArray generateLabelParameterHash() const;
  
  virtual void placeTickLabel(GPPainter *painter, double position, int distanceToAxis, const QString &text, QSize *tickLabelsSize);
  virtual void drawTickLabel(GPPainter *painter, double x, double y, const TickLabelData &labelData) const;
  virtual TickLabelData getTickLabelData(const QFont &font, const QString &text) const;
  virtual QPointF getTickLabelDrawOffset(const TickLabelData &labelData) const;
  virtual void getMaxTickLabelSize(const QFont &font, const QString &text, QSize *tickLabelsSize) const;
};

class GP_LIB_DECL GPScatterStyle
{
  Q_GADGET
public:
  /*!
    Represents the various properties of a scatter style instance. For example, this enum is used
    to specify which properties of \ref GPSelectionDecorator::setScatterStyle will be used when
    highlighting selected data points.

    Specific scatter properties can be transferred between \ref GPScatterStyle instances via \ref
    setFromOther.
  */
  enum ScatterProperty { spNone  = 0x00  ///< <tt>0x00</tt> None
                         ,spPen   = 0x01  ///< <tt>0x01</tt> The pen property, see \ref setPen
                         ,spBrush = 0x02  ///< <tt>0x02</tt> The brush property, see \ref setBrush
                         ,spSize  = 0x04  ///< <tt>0x04</tt> The size property, see \ref setSize
                         ,spShape = 0x08  ///< <tt>0x08</tt> The shape property, see \ref setShape
                         ,spAll   = 0xFF  ///< <tt>0xFF</tt> All properties
                       };
  Q_ENUMS(ScatterProperty)
  Q_FLAGS(ScatterProperties)
  Q_DECLARE_FLAGS(ScatterProperties, ScatterProperty)

  /*!
    Defines the shape used for scatter points.

    On plottables/items that draw scatters, the sizes of these visualizations (with exception of
    \ref ssDot and \ref ssPixmap) can be controlled with the \ref setSize function. Scatters are
    drawn with the pen and brush specified with \ref setPen and \ref setBrush.
  */
  enum ScatterShape { ssNone       ///< no scatter symbols are drawn (e.g. in GPGraph, data only represented with lines)
                      ,ssDot       ///< \enumimage{ssDot.png} a single pixel (use \ref ssDisc or \ref ssCircle if you want a round shape with a certain radius)
                      ,ssCross     ///< \enumimage{ssCross.png} a cross
                      ,ssPlus      ///< \enumimage{ssPlus.png} a plus
                      ,ssCircle    ///< \enumimage{ssCircle.png} a circle
                      ,ssDisc      ///< \enumimage{ssDisc.png} a circle which is filled with the pen's color (not the brush as with ssCircle)
                      ,ssSquare    ///< \enumimage{ssSquare.png} a square
                      ,ssDiamond   ///< \enumimage{ssDiamond.png} a diamond
                      ,ssStar      ///< \enumimage{ssStar.png} a star with eight arms, i.e. a combination of cross and plus
                      ,ssTriangle  ///< \enumimage{ssTriangle.png} an equilateral triangle, standing on baseline
                      ,ssTriangleInverted ///< \enumimage{ssTriangleInverted.png} an equilateral triangle, standing on corner
                      ,ssCrossSquare      ///< \enumimage{ssCrossSquare.png} a square with a cross inside
                      ,ssPlusSquare       ///< \enumimage{ssPlusSquare.png} a square with a plus inside
                      ,ssCrossCircle      ///< \enumimage{ssCrossCircle.png} a circle with a cross inside
                      ,ssPlusCircle       ///< \enumimage{ssPlusCircle.png} a circle with a plus inside
                      ,ssPeace     ///< \enumimage{ssPeace.png} a circle, with one vertical and two downward diagonal lines
                      ,ssPixmap    ///< a custom pixmap specified by \ref setPixmap, centered on the data point coordinates
                      ,ssCustom    ///< custom painter operations are performed per scatter (As QPainterPath, see \ref setCustomPath)
                    };
  Q_ENUMS(ScatterShape)

  GPScatterStyle();
  GPScatterStyle(ScatterShape shape, double size=6);
  GPScatterStyle(ScatterShape shape, const QColor &color, double size);
  GPScatterStyle(ScatterShape shape, const QColor &color, const QColor &fill, double size);
  GPScatterStyle(ScatterShape shape, const QPen &pen, const QBrush &brush, double size);
  GPScatterStyle(const QPixmap &pixmap);
  GPScatterStyle(const QPainterPath &customPath, const QPen &pen, const QBrush &brush=Qt::NoBrush, double size=6);
  
  // getters:
  double size() const { return mSize; }
  ScatterShape shape() const { return mShape; }
  QPen pen() const { return mPen; }
  QBrush brush() const { return mBrush; }
  QPixmap pixmap() const { return mPixmap; }
  QPainterPath customPath() const { return mCustomPath; }

  // setters:
  void setFromOther(const GPScatterStyle &other, ScatterProperties properties);
  void setSize(double size);
  void setShape(ScatterShape shape);
  void setPen(const QPen &pen);
  void setBrush(const QBrush &brush);
  void setPixmap(const QPixmap &pixmap);
  void setCustomPath(const QPainterPath &customPath);

  // non-property methods:
  bool isNone() const { return mShape == ssNone; }
  bool isPenDefined() const { return mPenDefined; }
  void undefinePen();
  void applyTo(GPPainter *painter, const QPen &defaultPen) const;
  void drawShape(GPPainter *painter, const QPointF &pos) const;
  void drawShape(GPPainter *painter, double x, double y) const;

protected:
  // property members:
  double mSize;
  ScatterShape mShape;
  QPen mPen;
  QBrush mBrush;
  QPixmap mPixmap;
  QPainterPath mCustomPath;
  
  // non-property members:
  bool mPenDefined;
};
Q_DECLARE_TYPEINFO(GPScatterStyle, Q_MOVABLE_TYPE);
Q_DECLARE_OPERATORS_FOR_FLAGS(GPScatterStyle::ScatterProperties)
Q_DECLARE_METATYPE(GPScatterStyle::ScatterProperty)
Q_DECLARE_METATYPE(GPScatterStyle::ScatterShape)

class GP_LIB_DECL GPSelectionDecorator
{
  Q_GADGET
public:
  GPSelectionDecorator();
  virtual ~GPSelectionDecorator();
  
  // getters:
  QPen pen() const { return mPen; }
  QBrush brush() const { return mBrush; }
  GPScatterStyle scatterStyle() const { return mScatterStyle; }
  GPScatterStyle::ScatterProperties usedScatterProperties() const { return mUsedScatterProperties; }
  
  // setters:
  void setPen(const QPen &pen);
  void setBrush(const QBrush &brush);
  void setScatterStyle(const GPScatterStyle &scatterStyle, GPScatterStyle::ScatterProperties usedProperties=GPScatterStyle::spPen);
  void setUsedScatterProperties(const GPScatterStyle::ScatterProperties &properties);
  
  // non-virtual methods:
  void applyPen(GPPainter *painter) const;
  void applyBrush(GPPainter *painter) const;
  GPScatterStyle getFinalScatterStyle(const GPScatterStyle &unselectedStyle) const;
  
  // introduced virtual methods:
  virtual void copyFrom(const GPSelectionDecorator *other);
  virtual void drawDecoration(GPPainter *painter, GPDataSelection selection);
  
protected:
  // property members:
  QPen mPen;
  QBrush mBrush;
  GPScatterStyle mScatterStyle;
  GPScatterStyle::ScatterProperties mUsedScatterProperties;
  // non-property members:
  GPAbstractPlottable *mPlottable;
  
  // introduced virtual methods:
  virtual bool registerWithPlottable(GPAbstractPlottable *plottable);
  
private:
  Q_DISABLE_COPY(GPSelectionDecorator)
  friend class GPAbstractPlottable;
};
Q_DECLARE_METATYPE(GPSelectionDecorator*)


class GP_LIB_DECL GPAbstractPlottable : public GPLayerable
{
  Q_OBJECT
  /// \cond INCLUDE_QPROPERTIES
  Q_PROPERTY(QString name READ name WRITE setName)
  Q_PROPERTY(bool antialiasedFill READ antialiasedFill WRITE setAntialiasedFill)
  Q_PROPERTY(bool antialiasedScatters READ antialiasedScatters WRITE setAntialiasedScatters)
  Q_PROPERTY(QPen pen READ pen WRITE setPen)
  Q_PROPERTY(QBrush brush READ brush WRITE setBrush)
  Q_PROPERTY(GPAxis* keyAxis READ keyAxis WRITE setKeyAxis)
  Q_PROPERTY(GPAxis* valueAxis READ valueAxis WRITE setValueAxis)
  Q_PROPERTY(GP::SelectionType selectable READ selectable WRITE setSelectable NOTIFY selectableChanged)
  Q_PROPERTY(GPDataSelection selection READ selection WRITE setSelection NOTIFY selectionChanged)
  Q_PROPERTY(GPSelectionDecorator* selectionDecorator READ selectionDecorator WRITE setSelectionDecorator)
  /// \endcond
public:
  GPAbstractPlottable(GPAxis *keyAxis, GPAxis *valueAxis, GPLayerable *parentLayerable = 0);
  virtual ~GPAbstractPlottable();
  
  // getters:
  QString name() const { return mName; }
  bool antialiasedFill() const { return mAntialiasedFill; }
  bool antialiasedScatters() const { return mAntialiasedScatters; }
  QPen pen() const { return mPen; }
  QPen* getPen() { return &mPen; }
  QBrush brush() const { return mBrush; }
  GPAxis *keyAxis() const { return mKeyAxis.data(); }
  GPAxis *valueAxis() const { return mValueAxis.data(); }
  GP::SelectionType selectable() const { return mSelectable; }
  bool selected() const { return !mSelection.isEmpty(); } 
  GPDataSelection selection() const { return mSelection; }
  GPSelectionDecorator *selectionDecorator() const { return mSelectionDecorator; }
  
  // setters:
  void setName(const QString &name);
  void setAntialiasedFill(bool enabled);
  void setAntialiasedScatters(bool enabled);
  void setPen(const QPen &pen);
  void setBrush(const QBrush &brush);
  void setKeyAxis(GPAxis *axis);
  void setValueAxis(GPAxis *axis);
  Q_SLOT void setSelectable(GP::SelectionType selectable);
  Q_SLOT void setSelection(GPDataSelection selection);
  void setSelectionDecorator(GPSelectionDecorator *decorator);

  // introduced virtual methods:
  virtual QRect clipRect() const override;
  virtual double selectTest(const QPointF &pos, bool onlySelectable, QVariant *details=0) const override = 0;
  virtual GPPlottableInterface1D *interface1D() { return 0; }
  virtual GPRange getKeyRange(bool &foundRange, GP::SignDomain inSignDomain=GP::sdBoth) const = 0;
  virtual GPRange getValueRange(bool &foundRange, GP::SignDomain inSignDomain=GP::sdBoth, const GPRange &inKeyRange=GPRange()) const = 0;
  virtual GPDataRange getDataRange() const = 0;
  
  // non-property methods:
  void coordsToPixels(const double& key, const double& value, double &x, double &y) const;
  const QPointF coordsToPixels(const double& key, const double& value) const;
  void pixelsToCoords(const double& x, const double& y, double &key, double &value) const;
  void pixelsToCoords(const QPointF &pixelPos, double &key, double &value) const;
  QPointF pixelsToCoords(const QPointF &pixelPos);
  void rescaleAxes(bool onlyEnlarge=false) const;
  void rescaleKeyAxis(bool onlyEnlarge=false) const;
  void rescaleValueAxis(bool onlyEnlarge=false, bool inKeyRange=false) const;
  bool addToLegend(GPLegend *legend);
  bool addToLegend();
  bool removeFromLegend(GPLegend *legend) const;
  bool removeFromLegend() const;
  
signals:
  void selectionChanged(bool selected);
  void selectionChanged(const GPDataSelection &selection);
  void selectableChanged(GP::SelectionType selectable);

public:
  // events:
  virtual void selectEvent(QMouseEvent *event, bool additive, const QVariant &details, bool *selectionStateChanged) override;
  virtual void deselectEvent(bool *selectionStateChanged) override;
  
protected:
  // property members:
  QString mName;
  bool mAntialiasedFill, mAntialiasedScatters;
  QPen mPen;
  QBrush mBrush;
  QPointer<GPAxis> mKeyAxis, mValueAxis;
  GP::SelectionType mSelectable;
  GPDataSelection mSelection;
  GPSelectionDecorator *mSelectionDecorator;
  
  // reimplemented virtual methods:
  virtual void draw(GPPainter *painter) override = 0;
  virtual GP::Interaction selectionCategory() const override;
  void applyDefaultAntialiasingHint(GPPainter *painter) const override;
  
  // introduced virtual methods:
  virtual void drawLegendIcon(GPPainter *painter, const QRectF &rect) const = 0;
  
  // non-virtual methods:
  void applyFillAntialiasingHint(GPPainter *painter) const;
  void applyScattersAntialiasingHint(GPPainter *painter) const;

private:
  Q_DISABLE_COPY(GPAbstractPlottable)
  
  friend class GraphicsPlot;
  friend class GPAxis;
  friend class GPPlottableLegendItem;
};

class GP_LIB_DECL GPItemAnchor
{
  Q_GADGET
public:
  GPItemAnchor(GraphicsPlot *parentPlot, GPAbstractItem *parentItem, const QString &name, int anchorId=-1);
  virtual ~GPItemAnchor();
  
  // getters:
  QString name() const { return mName; }
  virtual QPointF pixelPosition() const;
  
protected:
  // property members:
  QString mName;
  
  // non-property members:
  GraphicsPlot *mParentPlot;
  GPAbstractItem *mParentItem;
  int mAnchorId;
  QSet<GPItemPosition*> mChildrenX, mChildrenY;
  
  // introduced virtual methods:
  virtual GPItemPosition *toGPItemPosition() { return 0; }
  
  // non-virtual methods:
  void addChildX(GPItemPosition* pos); // called from pos when this anchor is set as parent
  void removeChildX(GPItemPosition *pos); // called from pos when its parent anchor is reset or pos deleted
  void addChildY(GPItemPosition* pos); // called from pos when this anchor is set as parent
  void removeChildY(GPItemPosition *pos); // called from pos when its parent anchor is reset or pos deleted
  
private:
  Q_DISABLE_COPY(GPItemAnchor)
  
  friend class GPItemPosition;
};



class GP_LIB_DECL GPItemPosition : public GPItemAnchor
                                   , public GPUomDescriptorProviderDualListener
{
  Q_GADGET
public:
  /*!
    Defines the ways an item position can be specified. Thus it defines what the numbers passed to
    \ref setCoords actually mean.
    
    \see setType
  */
  enum PositionType { ptAbsolute        ///< Static positioning in pixels, starting from the top left corner of the viewport/widget.
                      ,ptViewportRatio  ///< Static positioning given by a fraction of the viewport size. For example, if you call setCoords(0, 0), the position will be at the top
                                        ///< left corner of the viewport/widget. setCoords(1, 1) will be at the bottom right corner, setCoords(0.5, 0) will be horizontally centered and
                                        ///< vertically at the top of the viewport/widget, etc.
                      ,ptAxisRectRatio  ///< Static positioning given by a fraction of the axis rect size (see \ref setAxisRect). For example, if you call setCoords(0, 0), the position will be at the top
                                        ///< left corner of the axis rect. setCoords(1, 1) will be at the bottom right corner, setCoords(0.5, 0) will be horizontally centered and
                                        ///< vertically at the top of the axis rect, etc. You can also go beyond the axis rect by providing negative coordinates or coordinates larger than 1.
                      ,ptPlotCoords     ///< Dynamic positioning at a plot coordinate defined by two axes (see \ref setAxes).
                    };
  Q_ENUMS(PositionType)
  
  GPItemPosition(GraphicsPlot *parentPlot, GPAbstractItem *parentItem, const QString &name);
  virtual ~GPItemPosition();
  
  // getters:
  PositionType type() const { return typeX(); }
  PositionType typeX() const { return mPositionTypeX; }
  PositionType typeY() const { return mPositionTypeY; }
  GPItemAnchor *parentAnchor() const { return parentAnchorX(); }
  GPItemAnchor *parentAnchorX() const { return mParentAnchorX; }
  GPItemAnchor *parentAnchorY() const { return mParentAnchorY; }
  double key() const { return mKey; }
  double value() const { return mValue; }
  QPointF coords() const { return QPointF(mKey, mValue); }
  GPAxis *keyAxis() const { return mKeyAxis.data(); }
  GPAxis *valueAxis() const { return mValueAxis.data(); }
  GPAxisRect *axisRect() const;
  virtual QPointF pixelPosition() const override;
  
  // setters:
  void setType(PositionType type);
  void setTypeX(PositionType type);
  void setTypeY(PositionType type);
  bool setParentAnchor(GPItemAnchor *parentAnchor, bool keepPixelPosition=false);
  bool setParentAnchorX(GPItemAnchor *parentAnchor, bool keepPixelPosition=false);
  bool setParentAnchorY(GPItemAnchor *parentAnchor, bool keepPixelPosition=false);
  void setCoords(double key, double value);
  void setCoords(const QPointF &coords);
  void setAxes(GPAxis* keyAxis, GPAxis* valueAxis);
  void setAxisRect(GPAxisRect *axisRect);
  void setPixelPosition(const QPointF &pixelPosition);

  // GPUomDescriptorProviderDualListener
  virtual void applyUomForKeys(const GPUomDescriptor& descriptor) override;
  virtual void applyUomForValues(const GPUomDescriptor& descriptor) override;

  virtual void setInputUomForKeys(const GPUomDescriptor& descriptor)
  {
    mInputUomForKeys = descriptor;
  }

  virtual void setInputUomForKeys(int quantity, int units)
  {
    GPUomDescriptor descriptor;
    descriptor.quantity = quantity;
    descriptor.units = units;
    setInputUomForKeys(descriptor);
  }

  virtual void setInputUomForValues(const GPUomDescriptor& descriptor)
  {
    mInputUomForValues = descriptor;
  }

  virtual void setInputUomForValues(int quantity, int units)
  {
    GPUomDescriptor descriptor;
    descriptor.quantity = quantity;
    descriptor.units = units;
    setInputUomForValues(descriptor);
  }
  
public:
  GPUomDescriptor mInputUomForKeys;
  GPUomDescriptor mInputUomForValues;

protected:
  // property members:
  PositionType mPositionTypeX, mPositionTypeY;
  QPointer<GPAxis> mKeyAxis, mValueAxis;
  QPointer<GPAxisRect> mAxisRect;
  double mKey, mValue;
  GPItemAnchor *mParentAnchorX, *mParentAnchorY;
  
  // reimplemented virtual methods:
  virtual GPItemPosition *toGPItemPosition() override { return this; }
  
private:
  Q_DISABLE_COPY(GPItemPosition)
  
};
Q_DECLARE_METATYPE(GPItemPosition::PositionType)


class GP_LIB_DECL GPAbstractItem : public GPLayerable
{
  Q_OBJECT
  /// \cond INCLUDE_QPROPERTIES
  Q_PROPERTY(bool clipToAxisRect READ clipToAxisRect WRITE setClipToAxisRect)
  Q_PROPERTY(GPAxisRect* clipAxisRect READ clipAxisRect WRITE setClipAxisRect)
  Q_PROPERTY(bool selectable READ selectable WRITE setSelectable NOTIFY selectableChanged)
  Q_PROPERTY(bool selected READ selected WRITE setSelected NOTIFY selectionChanged)
  /// \endcond
public:
  explicit GPAbstractItem(GraphicsPlot *parentPlot);
  virtual ~GPAbstractItem();
  
  // getters:
  bool clipToAxisRect() const { return mClipToAxisRect; }
  GPAxisRect *clipAxisRect() const;
  bool selectable() const { return mSelectable; }
  bool selected() const { return mSelected; }
  
  // setters:
  void setClipToAxisRect(bool clip);
  void setClipAxisRect(GPAxisRect *rect);
  Q_SLOT void setSelectable(bool selectable);
  Q_SLOT void setSelected(bool selected);
  
  // reimplemented virtual methods:
  virtual double selectTest(const QPointF &pos, bool onlySelectable, QVariant *details=0) const override = 0;

  // non-virtual methods:
  QList<GPItemPosition*> positions() const { return mPositions; }
  QList<GPItemAnchor*> anchors() const { return mAnchors; }
  GPItemPosition *position(const QString &name) const;
  GPItemAnchor *anchor(const QString &name) const;
  bool hasAnchor(const QString &name) const;

  // uoms accessors dispatched to positions
  virtual void setInputUomForKeys(const GPUomDescriptor& descriptor);
  virtual void setInputUomForKeys(int quantity, int units);
  virtual void setInputUomForValues(const GPUomDescriptor& descriptor);
  virtual void setInputUomForValues(int quantity, int units);
  
signals:
  void selectionChanged(bool selected);
  void selectableChanged(bool selectable);
  
protected:
  // property members:
  bool mClipToAxisRect;
  QPointer<GPAxisRect> mClipAxisRect;
  QList<GPItemPosition*> mPositions;
  QList<GPItemAnchor*> mAnchors;
  bool mSelectable, mSelected;
  
  // reimplemented virtual methods:
  virtual GP::Interaction selectionCategory() const override;
  virtual QRect clipRect() const override;
  virtual void applyDefaultAntialiasingHint(GPPainter *painter) const override;
  virtual void draw(GPPainter *painter) override = 0;
  // events:
  virtual void selectEvent(QMouseEvent *event, bool additive, const QVariant &details, bool *selectionStateChanged) override;
  virtual void deselectEvent(bool *selectionStateChanged) override;
  
  // introduced virtual methods:
  virtual QPointF anchorPixelPosition(int anchorId) const;
  
  // non-virtual methods:
  double rectDistance(const QRectF &rect, const QPointF &pos, bool filledRect) const;
  GPItemPosition *createPosition(const QString &name);
  GPItemAnchor *createAnchor(const QString &name, int anchorId);
  
private:
  Q_DISABLE_COPY(GPAbstractItem)
  
  friend class GraphicsPlot;
  friend class GPItemAnchor;
};

class GP_LIB_DECL GraphicsPlot : public QWidget
{
  Q_OBJECT
  /// \cond INCLUDE_QPROPERTIES
  Q_PROPERTY(QRect viewport READ viewport WRITE setViewport)
  Q_PROPERTY(QPixmap background READ background WRITE setBackground)
  Q_PROPERTY(bool backgroundScaled READ backgroundScaled WRITE setBackgroundScaled)
  Q_PROPERTY(Qt::AspectRatioMode backgroundScaledMode READ backgroundScaledMode WRITE setBackgroundScaledMode)
  Q_PROPERTY(GPLayoutGrid* plotLayout READ plotLayout)
  Q_PROPERTY(bool autoAddPlottableToLegend READ autoAddPlottableToLegend WRITE setAutoAddPlottableToLegend)
  Q_PROPERTY(int selectionTolerance READ selectionTolerance WRITE setSelectionTolerance)
  Q_PROPERTY(bool noAntialiasingOnDrag READ noAntialiasingOnDrag WRITE setNoAntialiasingOnDrag)
  Q_PROPERTY(Qt::KeyboardModifier multiSelectModifier READ multiSelectModifier WRITE setMultiSelectModifier)
  Q_PROPERTY(bool openGl READ openGl WRITE setOpenGl)
  Q_PROPERTY(Qt::KeyboardModifier mZoomModifiers READ zoomModifiers WRITE setZoomModifiers)
  Q_PROPERTY(Qt::KeyboardModifier mDragModifiers READ dragModifiers WRITE setDragModifiers)
  
    /// \endcond
public:
  static QSet<GraphicsPlot*> Instances__;

  /*!
    Defines how a layer should be inserted relative to an other layer.

    \see addLayer, moveLayer
  */
  enum LayerInsertMode { limBelow  ///< Layer is inserted below other layer
                         ,limAbove ///< Layer is inserted above other layer
                       };
  Q_ENUMS(LayerInsertMode)
  
  /*!
    Defines with what timing the GraphicsPlot surface is refreshed after a replot.

    \see replot
  */
  enum RefreshPriority { rpImmediateRefresh ///< Replots immediately and repaints the widget immediately by calling QWidget::repaint() after the replot
                         ,rpQueuedRefresh   ///< Replots immediately, but queues the widget repaint, by calling QWidget::update() after the replot. This way multiple redundant widget repaints can be avoided.
                         ,rpRefreshHint     ///< Whether to use immediate or queued refresh depends on whether the plotting hint \ref GP::phImmediateRefresh is set, see \ref setPlottingHints.
                         ,rpQueuedReplot    ///< Queues the entire replot for the next event loop iteration. This way multiple redundant replots can be avoided. The actual replot is then done with \ref rpRefreshHint priority.
                       };
  Q_ENUMS(RefreshPriority)
  
  explicit GraphicsPlot(QWidget *parent = nullptr);
  virtual ~GraphicsPlot();

  static void blockReplots(bool block);
  static bool getBlockReplots();
  static void replotVisible();
  static void setWatermark(const QString& watermark);
  static void setWatermark(const QPixmap& watermark);

  virtual bool event(QEvent *event) override;
  virtual void showEvent(QShowEvent *event) override;
  virtual void hideEvent(QHideEvent *event) override;
  
  // getters:
  QRect viewport() const { return mViewport; }
  double bufferDevicePixelRatio() const { return mBufferDevicePixelRatio; }
  QPixmap background() const { return mBackgroundPixmap; }
  bool backgroundScaled() const { return mBackgroundScaled; }
  Qt::AspectRatioMode backgroundScaledMode() const { return mBackgroundScaledMode; }
  GPLayoutGrid *plotLayout() const { return mPlotLayout; }
  GP::AntialiasedElements antialiasedElements() const { return mAntialiasedElements; }
  GP::AntialiasedElements notAntialiasedElements() const { return mNotAntialiasedElements; }
  bool autoAddPlottableToLegend() const { return mAutoAddPlottableToLegend; }
  const GP::Interactions interactions() const { return mInteractions; }
  int selectionTolerance() const { return mSelectionTolerance; }
  bool noAntialiasingOnDrag() const { return mNoAntialiasingOnDrag; }
  GP::PlottingHints plottingHints() const { return mPlottingHints; }
  Qt::KeyboardModifier multiSelectModifier() const { return mMultiSelectModifier; }
  GP::SelectionRectMode selectionRectMode() const { return mSelectionRectMode; }
  GPSelectionRect *selectionRect() const { return mSelectionRect; }
  Qt::KeyboardModifier zoomModifiers() const { return mZoomModifiers; }
  Qt::KeyboardModifier dragModifiers() const { return mDragModifiers; }
  bool openGl() const { return mOpenGl; }
  
  // setters:
  void setViewport(const QRect &rect);
  void setBufferDevicePixelRatio(double ratio);
  void setBackground(const QPixmap &pm);
  void setBackground(const QPixmap &pm, bool scaled, Qt::AspectRatioMode mode=Qt::KeepAspectRatioByExpanding);
  void setBackground(const QBrush &brush);
  void setBackgroundScaled(bool scaled);
  void setBackgroundScaledMode(Qt::AspectRatioMode mode);
  void setAntialiasedElements(const GP::AntialiasedElements &antialiasedElements);
  void setAntialiasedElement(GP::AntialiasedElement antialiasedElement, bool enabled=true);
  void setNotAntialiasedElements(const GP::AntialiasedElements &notAntialiasedElements);
  void setNotAntialiasedElement(GP::AntialiasedElement notAntialiasedElement, bool enabled=true);
  void setAutoAddPlottableToLegend(bool on);
  void setInteractions(const GP::Interactions &interactions);
  void setInteraction(const GP::Interaction &interaction, bool enabled=true);
  void setSelectionTolerance(int pixels);
  void setNoAntialiasingOnDrag(bool enabled);
  void setPlottingHints(const GP::PlottingHints &hints);
  void setPlottingHint(GP::PlottingHint hint, bool enabled=true);
  void setMultiSelectModifier(Qt::KeyboardModifier modifier);
  void setSelectionRectMode(GP::SelectionRectMode mode);
  void setSelectionRect(GPSelectionRect *selectionRect);
  void setOpenGl(bool enabled, int multisampling = 16, bool lazy = true);
  void setZoomModifiers(Qt::KeyboardModifier modifiers);
  void setDragModifiers(Qt::KeyboardModifier modifiers);
  void setupPaintBuffers();

  // Legend
  GPLegend* getLegend() const { return legend; };

  // non-property methods:
  // plottable interface:
  GPAbstractPlottable *plottable(int index);
  GPAbstractPlottable *plottable();
  virtual bool removePlottable(GPAbstractPlottable *plottable);
  virtual bool removePlottable(int index);
  int clearPlottables();
  int plottableCount() const;
  QList<GPAbstractPlottable*> selectedPlottables() const;
  GPAbstractPlottable *plottableAt(const QPointF &pos, bool onlySelectable=false) const;
  bool hasPlottable(GPAbstractPlottable *plottable) const;
 
  // specialized interface for GPGraph:
  GPGraph *graph(int index) const;
  GPGraph *graph() const;
  GPGraph *addGraph(GPAxis *keyAxis=0, GPAxis *valueAxis=0);
  bool removeGraph(GPGraph *graph);
  bool removeGraph(int index);
  int clearGraphs();
  int graphCount() const;
  QList<GPGraph*> selectedGraphs() const;

  // specialized interface for GPCurve:
  GPCurve *curve(int index) const;
  GPCurve *curve() const;
  virtual GPCurve *addCurve(GPAxis *keyAxis = 0, GPAxis *valueAxis = 0);
  bool removeCurve(GPCurve *curve);
  bool removeCurve(int index);
  int clearCurves();
  int curveCount() const;
  QList<GPCurve*> selectedCurves() const;

  // item interface:
  GPAbstractItem *item(int index) const;
  GPAbstractItem *item() const;
  bool removeItem(GPAbstractItem *item);
  bool removeItem(int index);
  int clearItems();
  int itemCount() const;
  QList<GPAbstractItem*> selectedItems() const;
  GPAbstractItem *itemAt(const QPointF &pos, bool onlySelectable=false) const;
  bool hasItem(GPAbstractItem *item) const;
  
  // layer interface:
  GPLayer *layer(const QString &name) const;
  GPLayer *layer(int index) const;
  GPLayer *currentLayer() const;
  bool setCurrentLayer(const QString &name);
  bool setCurrentLayer(GPLayer *layer);
  int layerCount() const;
  bool addLayer(const QString &name, GPLayer *otherLayer=0, LayerInsertMode insertMode=limAbove);
  bool removeLayer(GPLayer *layer);
  bool moveLayer(GPLayer *layer, GPLayer *otherLayer, LayerInsertMode insertMode=limAbove);
  
  // axis rect/layout interface:
  int axisRectCount() const;
  GPAxisRect* axisRect(int index=0) const;
  QList<GPAxisRect*> axisRects() const;
  GPLayoutElement* layoutElementAt(const QPointF &pos) const;
  GPAxisRect* axisRectAt(const QPointF &pos) const;
  Q_SLOT void rescaleAxes(bool onlyVisiblePlottables=false);

  GPAxis* getDefaultAxisX();
  GPAxis* getDefaultAxisY();
  GPAxis* getDefaultAxisX2();
  GPAxis* getDefaultAxisY2();

  QList<GPAxis*> selectedAxes() const;
  QList<GPAxis*> axesList() const;

  QList<GPLegend*> selectedLegends() const;
  Q_SLOT void deselectAll();

  virtual void moveAxis(GPAxisRect* rect, GPAxis* axis, GPAxis::AxisType to);
  
  bool savePdf(const QString &fileName, int width=0, int height=0, GP::ExportPen exportPen=GP::epAllowCosmetic, const QString &pdfCreator=QString(), const QString &pdfTitle=QString());
  bool savePng(const QString &fileName, int width=0, int height=0, double scale=1.0, int quality=-1, int resolution=96, GP::ResolutionUnit resolutionUnit=GP::ruDotsPerInch);
  bool savePng(QIODevice* device, int width=0, int height=0, double scale=1.0, int quality=-1, int resolution=96, GP::ResolutionUnit resolutionUnit=GP::ruDotsPerInch);
  bool saveJpg(const QString &fileName, int width=0, int height=0, double scale=1.0, int quality=-1, int resolution=96, GP::ResolutionUnit resolutionUnit=GP::ruDotsPerInch);
  bool saveBmp(const QString &fileName, int width=0, int height=0, double scale=1.0, int resolution=96, GP::ResolutionUnit resolutionUnit=GP::ruDotsPerInch);
  bool saveRastered(const QString &fileName, int width, int height, double scale, const char *format, int quality=-1, int resolution=96, GP::ResolutionUnit resolutionUnit=GP::ruDotsPerInch);
  bool saveRastered(QIODevice* device, int width, int height, double scale, const char *format, int quality=-1, int resolution=96, GP::ResolutionUnit resolutionUnit=GP::ruDotsPerInch);
  QPixmap toPixmap(int width=0, int height=0, double scale=1.0);
  void toPainter(GPPainter *painter, int width=0, int height=0);
  Q_SLOT virtual void replot(GraphicsPlot::RefreshPriority refreshPriority=GraphicsPlot::rpRefreshHint);
  
  GPAxis *xAxis, *yAxis, *xAxis2, *yAxis2;
  GPLegend *legend;

public:
  // helpers:

  template<typename T, typename = void>
  struct point_type : std::false_type {};

  template<typename T>
  struct point_type<T, std::void_t<decltype(std::declval<T>().x()),
                                   decltype(std::declval<T>().y()),
                                   decltype(T(std::declval<T>().x(), std::declval<T>().y()))>> : std::true_type {};

  template<typename T, typename = void>
  struct rect_type : std::false_type {};

  template<typename T>
  struct rect_type<T, std::void_t<decltype(std::declval<T>().topLeft()),
                                  decltype(std::declval<T>().bottomRight()),
                                  decltype(T(std::declval<T>().topLeft(), std::declval<T>().bottomRight()))>> : std::true_type {};

  template <typename T>
  typename std::enable_if<point_type<T>::value, T>::type pixelToCoord(const T& pixelPoint, GPAxis* axisX = nullptr, GPAxis* axisY = nullptr) const
  {
    if(!axisX)
        axisX = xAxis;
    if(!axisY)
        axisY = yAxis;
    decltype(std::declval<T>().x()) x = axisX->pixelToCoord(pixelPoint.x());
    decltype(std::declval<T>().y()) y = axisY->pixelToCoord(pixelPoint.y());
    return T(x, y);
  }
  template <typename T>
  typename std::enable_if<point_type<T>::value, T>::type coordToPixel(const T& coordPoint, GPAxis* axisX = nullptr, GPAxis* axisY = nullptr) const
  {
    if(!axisX)
        axisX = xAxis;
    if(!axisY)
        axisY = yAxis;
    decltype(std::declval<T>().x()) x = axisX->coordToPixel(coordPoint.x());
    decltype(std::declval<T>().y()) y = axisY->coordToPixel(coordPoint.y());
    return T(x, y);
  }

  template<typename T>
  typename std::enable_if<rect_type<T>::value, T>::type pixelToCoord(const T& pixelRect, GPAxis* axisX = nullptr, GPAxis* axisY = nullptr) const
  {
    decltype(std::declval<T>().topLeft()) tl = pixelToCoord(pixelRect.topLeft(), axisX, axisY);
    decltype(std::declval<T>().bottomRight()) br = pixelToCoord(pixelRect.bottomRight(), axisX, axisY);
    return T(tl, br);
  }
  template<typename T>
  typename std::enable_if<rect_type<T>::value, T>::type coordToPixel(const T& coordRect, GPAxis* axisX = nullptr, GPAxis* axisY = nullptr) const
  {
    decltype(std::declval<T>().topLeft()) tl = coordToPixel(coordRect.topLeft(), axisX, axisY);
    decltype(std::declval<T>().bottomRight()) br = coordToPixel(coordRect.bottomRight(), axisX, axisY);
    return T(tl, br);
  }
  
signals:
  void mouseDoubleClick(QMouseEvent *event);
  void mousePress(QMouseEvent *event);
  void mouseMove(QMouseEvent *event);
  void mouseRelease(QMouseEvent *event);
  void mouseWheel(QWheelEvent *event);
  
  void plottableClick(GPAbstractPlottable *plottable, int dataIndex, QMouseEvent *event);
  void plottableDoubleClick(GPAbstractPlottable *plottable, int dataIndex, QMouseEvent *event);
  void itemClick(GPAbstractItem *item, QMouseEvent *event);
  void itemDoubleClick(GPAbstractItem *item, QMouseEvent *event);
  void axisClick(GPAxis *axis, GPAxis::SelectablePart part, QMouseEvent *event);
  void axisDoubleClick(GPAxis *axis, GPAxis::SelectablePart part, QMouseEvent *event);
  void legendClick(GPLegend *legend, GPAbstractLegendItem *item, QMouseEvent *event);
  void legendDoubleClick(GPLegend *legend,  GPAbstractLegendItem *item, QMouseEvent *event);
  
  void selectionChangedByUser();
  void beforeReplot();
  void afterReplot();
  
protected:
  // property members:
  QRect mViewport;
  double mBufferDevicePixelRatio;
  GPLayoutGrid *mPlotLayout;
  bool mAutoAddPlottableToLegend;
  QList<GPAbstractPlottable*> mPlottables;

  QList<GPGraph*> mGraphs;                                        // extra list of plottables also in mPlottables that are of type GPGraph
  QList<GPCurve*> mCurves;                                        // extra list of plottables also in mPlottables that are of type GPCurve
  QList<GPPointAttachedTextElement*> mPointAttachedTextElements;  // extra list of plottables also in mPlottables that are of type GPPointAttachedTextElement

  QList<GPAbstractItem*> mItems;
  QList<GPLayer*> mLayers;
  GP::AntialiasedElements mAntialiasedElements, mNotAntialiasedElements;
  GP::Interactions mInteractions;
  int mSelectionTolerance;
  bool mNoAntialiasingOnDrag;
  QBrush mBackgroundBrush;
  QPixmap mBackgroundPixmap;
  QPixmap mScaledBackgroundPixmap;
  bool mBackgroundScaled;
  Qt::AspectRatioMode mBackgroundScaledMode;
  GPLayer *mCurrentLayer;
  GP::PlottingHints mPlottingHints;
  Qt::KeyboardModifier mMultiSelectModifier;
  GP::SelectionRectMode mSelectionRectMode;
  GPSelectionRect *mSelectionRect;
  bool mOpenGl;
  bool mOpenGlDynamicObjectsAllocation;
  
  // non-property members:
  QList<QSharedPointer<GPAbstractPaintBuffer>> mPaintBuffers;
  QPoint mMousePressPos;
  bool mMouseHasMoved;
  QPointer<GPLayerable> mMouseEventLayerable;
  QPointer<GPLayerable> mMouseSignalLayerable;
  QVariant mMouseEventLayerableDetails;
  QVariant mMouseSignalLayerableDetails;
  bool mReplotting;
  bool mReplotQueued;
  int mOpenGlMultisamples;
  GP::AntialiasedElements mOpenGlAntialiasedElementsBackup;
  bool mOpenGlCacheLabelsBackup;
  Qt::KeyboardModifier mZoomModifiers, mDragModifiers;
#ifdef GP_OPENGL_FBO
  QSharedPointer<QOpenGLContext> mGlContext;
  QSharedPointer<QSurface> mGlSurface;
  QSharedPointer<QOpenGLPaintDevice> mGlPaintDevice;
#endif
  
  // reimplemented virtual methods:
  virtual QSize minimumSizeHint() const override;
  virtual QSize sizeHint() const override;
  virtual void paintEvent(QPaintEvent *event) override;
  virtual void resizeEvent(QResizeEvent *event) override;
  virtual void mouseDoubleClickEvent(QMouseEvent *event) override;
  virtual void mousePressEvent(QMouseEvent *event) override;
  virtual void mouseMoveEvent(QMouseEvent *event) override;
  virtual void mouseReleaseEvent(QMouseEvent *event) override;
  virtual void wheelEvent(QWheelEvent *event) override;

  // drag & drop interface
  virtual void dragEnterEvent(QDragEnterEvent *event) override { Q_UNUSED(event); }
  virtual void dragMoveEvent(QDragMoveEvent *event) override { Q_UNUSED(event); }
  virtual void dropEvent(QDropEvent *event) override { Q_UNUSED(event); }
  virtual void dragLeaveEvent(QDragLeaveEvent *event) override { Q_UNUSED(event); }
  
  // introduced virtual methods:
  virtual void draw(GPPainter *painter);
  virtual void updateLayout();
  virtual void axisRemoved(GPAxis *axis);
  virtual void legendRemoved(GPLegend *legend);
  Q_SLOT virtual void processRectSelection(const QRect& rect, QMouseEvent *event);
  Q_SLOT virtual void processRectZoom(const QRect& rect, QMouseEvent *event);
  Q_SLOT virtual void processPointSelection(QMouseEvent *event);
  

  virtual bool registerPlottable(GPAbstractPlottable *plottable);
  bool registerGraph(GPGraph *graph);
  bool registerCurve(GPCurve *curve);
  bool registerPointAttachedTextElement(GPPointAttachedTextElement* pointAttachedTextElement);
  bool registerItem(GPAbstractItem* item);
  void updateLayerIndices() const;
  GPLayerable *layerableAt(const QPointF &pos, bool onlySelectable, QVariant *selectionDetails=0) const;
  virtual QList<GPLayerable*> layerableListAt(const QPointF &pos, bool onlySelectable, QList<QVariant> *selectionDetails=0) const;
  void drawBackground(GPPainter *painter);
  GPAbstractPaintBuffer *createPaintBuffer();
  bool hasInvalidatedPaintBuffers();
  bool isOpenGlSetup();
  bool setupOpenGlIfNeeded();
  bool setupOpenGl();
  void freeOpenGl();
  
  friend class GPLegend;
  friend class GPAxis;
  friend class GPLayer;
  friend class GPAxisRect;
  friend class GPAbstractPlottable;
  friend class GPGraph;
  friend class GPCurve;
  friend class GPPointAttachedTextElement;
  friend class GPAbstractItem;
  friend class ChromatogramPlot;
};
Q_DECLARE_METATYPE(GraphicsPlot::LayerInsertMode)
Q_DECLARE_METATYPE(GraphicsPlot::RefreshPriority)

class GPReplotsBlocker : public QObject
{
public:
  GPReplotsBlocker()
    : oldblockReplots(GraphicsPlot::getBlockReplots())
  {
    GraphicsPlot::blockReplots(true);
  }

  ~GPReplotsBlocker()
  {
    GraphicsPlot::blockReplots(oldblockReplots);
    if (!oldblockReplots)
      GraphicsPlot::replotVisible();
  }

private:
  bool oldblockReplots;
};

class GPPlottableInterface1D
{
public:
  virtual ~GPPlottableInterface1D() {}
  // introduced pure virtual methods:
  virtual int dataCount() const = 0;
  virtual double dataMainKey(int index) const = 0;
  virtual double dataSortKey(int index) const = 0;
  virtual double dataMainValue(int index) const = 0;
  virtual GPRange dataValueRange(int index) const = 0;
  virtual QPointF dataPixelPosition(int index) const = 0;
  virtual bool sortKeyIsMainKey() const = 0;
  virtual GPDataSelection selectTestRect(const QRectF &rect, bool onlySelectable, bool acceptAllValues = false) const = 0;
  virtual int findBegin(double sortKey, bool expandedRange=true) const = 0;
  virtual int findEnd(double sortKey, bool expandedRange=true) const = 0;
};

template <class DataType>
class GPAbstractPlottable1D : public GPAbstractPlottable, public GPPlottableInterface1D // no GP_LIB_DECL, template class ends up in header (cpp included below)
{
  // No Q_OBJECT macro due to template class
  
public:
  GPAbstractPlottable1D(GPAxis *keyAxis, GPAxis *valueAxis, GPLayerable *parentLayerable=0);
  virtual ~GPAbstractPlottable1D();
  
  // virtual methods of 1d plottable interface:
  QSharedPointer<GPDataContainer<DataType>> data() { return mDataContainer; };
  virtual int dataCount() const override;
  virtual double dataMainKey(int index) const override;
  virtual double dataSortKey(int index) const override;
  virtual double dataMainValue(int index) const override;
  virtual GPRange dataValueRange(int index) const override;
  virtual QPointF dataPixelPosition(int index) const override;
  virtual bool sortKeyIsMainKey() const override;
  virtual GPDataSelection selectTestRect(const QRectF &rect, bool onlySelectable, bool acceptAllValues = false) const override;
  virtual int findBegin(double sortKey, bool expandedRange=true) const override;
  virtual int findEnd(double sortKey, bool expandedRange=true) const override;
  
  // reimplemented virtual methods:
  virtual double selectTest(const QPointF &pos, bool onlySelectable, QVariant *details=0) const override;
  virtual GPPlottableInterface1D *interface1D() override { return this; }
  
protected:
  // property members:
  QSharedPointer<GPDataContainer<DataType>> mDataContainer;
  
  // helpers for subclasses:
  void getDataSegments(QList<GPDataRange> &selectedSegments, QList<GPDataRange> &unselectedSegments) const;
  void drawPolyline(GPPainter *painter, const QVector<QPointF> &lineData) const;

private:
  Q_DISABLE_COPY(GPAbstractPlottable1D)
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// GPPlottableInterface1D
////////////////////////////////////////////////////////////////////////////////////////////////////

/*! \class GPPlottableInterface1D
  \brief Defines an abstract interface for one-dimensional plottables

  This class contains only pure virtual methods which define a common interface to the data
  of one-dimensional plottables.

  For example, it is implemented by the template class \ref GPAbstractPlottable1D (the preferred
  base class for one-dimensional plottables). So if you use that template class as base class of
  your one-dimensional plottable, you won't have to care about implementing the 1d interface
  yourself.

  If your plottable doesn't derive from \ref GPAbstractPlottable1D but still wants to provide a 1d
  interface (e.g. like \ref GPErrorBars does), you should inherit from both \ref
  GPAbstractPlottable and \ref GPPlottableInterface1D and accordingly reimplement the pure
  virtual methods of the 1d interface, matching your data container. Also, reimplement \ref
  GPAbstractPlottable::interface1D to return the \c this pointer.

  If you have a \ref GPAbstractPlottable pointer, you can check whether it implements this
  interface by calling \ref GPAbstractPlottable::interface1D and testing it for a non-zero return
  value. If it indeed implements this interface, you may use it to access the plottable's data
  without needing to know the exact type of the plottable or its data point type.
*/

/* start documentation of pure virtual functions */

/*! \fn virtual int GPPlottableInterface1D::dataCount() const = 0;
  
  Returns the number of data points of the plottable.
*/

/*! \fn virtual GPDataSelection GPPlottableInterface1D::selectTestRect(const QRectF &rect, bool onlySelectable, bool acceptAllValues = false) const = 0;
  
  Returns a data selection containing all the data points of this plottable which are contained (or
  hit by) \a rect. This is used mainly in the selection rect interaction for data selection (\ref
  dataselection "data selection mechanism").
  
  If \a onlySelectable is true, an empty GPDataSelection is returned if this plottable is not
  selectable (i.e. if \ref GPAbstractPlottable::setSelectable is \ref GP::stNone).
  
  \note \a rect must be a normalized rect (positive or zero width and height). This is especially
  important when using the rect of \ref GPSelectionRect::accepted, which is not necessarily
  normalized. Use <tt>QRect::normalized()</tt> when passing a rect which might not be normalized.
*/

/*! \fn virtual double GPPlottableInterface1D::dataMainKey(int index) const = 0
  
  Returns the main key of the data point at the given \a index.
  
  What the main key is, is defined by the plottable's data type. See the \ref
  gpdatacontainer-datatype "GPDataContainer DataType" documentation for details about this naming
  convention.
*/

/*! \fn virtual double GPPlottableInterface1D::dataSortKey(int index) const = 0
  
  Returns the sort key of the data point at the given \a index.
  
  What the sort key is, is defined by the plottable's data type. See the \ref
  gpdatacontainer-datatype "GPDataContainer DataType" documentation for details about this naming
  convention.
*/

/*! \fn virtual double GPPlottableInterface1D::dataMainValue(int index) const = 0
  
  Returns the main value of the data point at the given \a index.
  
  What the main value is, is defined by the plottable's data type. See the \ref
  gpdatacontainer-datatype "GPDataContainer DataType" documentation for details about this naming
  convention.
*/

/*! \fn virtual GPRange GPPlottableInterface1D::dataValueRange(int index) const = 0
  
  Returns the value range of the data point at the given \a index.
  
  What the value range is, is defined by the plottable's data type. See the \ref
  gpdatacontainer-datatype "GPDataContainer DataType" documentation for details about this naming
  convention.
*/

/*! \fn virtual QPointF GPPlottableInterface1D::dataPixelPosition(int index) const = 0

  Returns the pixel position on the widget surface at which the data point at the given \a index
  appears.

  Usually this corresponds to the point of \ref dataMainKey/\ref dataMainValue, in pixel
  coordinates. However, depending on the plottable, this might be a different apparent position
  than just a coord-to-pixel transform of those values. For example, \ref GPBars apparent data
  values can be shifted depending on their stacking, bar grouping or configured base value.
*/

/*! \fn virtual bool GPPlottableInterface1D::sortKeyIsMainKey() const = 0

  Returns whether the sort key (\ref dataSortKey) is identical to the main key (\ref dataMainKey).

  What the sort and main keys are, is defined by the plottable's data type. See the \ref
  gpdatacontainer-datatype "GPDataContainer DataType" documentation for details about this naming
  convention.
*/

/*! \fn virtual int GPPlottableInterface1D::findBegin(double sortKey, bool expandedRange) const = 0

  Returns the index of the data point with a (sort-)key that is equal to, just below, or just above
  \a sortKey. If \a expandedRange is true, the data point just below \a sortKey will be considered,
  otherwise the one just above.

  This can be used in conjunction with \ref findEnd to iterate over data points within a given key
  range, including or excluding the bounding data points that are just beyond the specified range.

  If \a expandedRange is true but there are no data points below \a sortKey, 0 is returned.

  If the container is empty, returns 0 (in that case, \ref findEnd will also return 0, so a loop
  using these methods will not iterate over the index 0).

  \see findEnd, GPDataContainer::findBegin
*/

/*! \fn virtual int GPPlottableInterface1D::findEnd(double sortKey, bool expandedRange) const = 0

  Returns the index one after the data point with a (sort-)key that is equal to, just above, or
  just below \a sortKey. If \a expandedRange is true, the data point just above \a sortKey will be
  considered, otherwise the one just below.

  This can be used in conjunction with \ref findBegin to iterate over data points within a given
  key range, including the bounding data points that are just below and above the specified range.

  If \a expandedRange is true but there are no data points above \a sortKey, the index just above the
  highest data point is returned.

  If the container is empty, returns 0.

  \see findBegin, GPDataContainer::findEnd
*/

/* end documentation of pure virtual functions */


////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// GPAbstractPlottable1D
////////////////////////////////////////////////////////////////////////////////////////////////////

/*! \class GPAbstractPlottable1D
  \brief A template base class for plottables with one-dimensional data

  This template class derives from \ref GPAbstractPlottable and from the abstract interface \ref
  GPPlottableInterface1D. It serves as a base class for all one-dimensional data (i.e. data with
  one key dimension), such as \ref GPGraph and GPCurve.

  The template parameter \a DataType is the type of the data points of this plottable (e.g. \ref
  GPGraphData or \ref GPCurveData). The main purpose of this base class is to provide the member
  \a mDataContainer (a shared pointer to a \ref GPDataContainer "GPDataContainer<DataType>") and
  implement the according virtual methods of the \ref GPPlottableInterface1D, such that most
  subclassed plottables don't need to worry about this anymore.

  Further, it provides a convenience method for retrieving selected/unselected data segments via
  \ref getDataSegments. This is useful when subclasses implement their \ref draw method and need to
  draw selected segments with a different pen/brush than unselected segments (also see \ref
  GPSelectionDecorator).

  This class implements basic functionality of \ref GPAbstractPlottable::selectTest and \ref
  GPPlottableInterface1D::selectTestRect, assuming point-like data points, based on the 1D data
  interface. In spite of that, most plottable subclasses will want to reimplement those methods
  again, to provide a more accurate hit test based on their specific data visualization geometry.
*/

/* start documentation of inline functions */

/*! \fn GPPlottableInterface1D *GPAbstractPlottable1D::interface1D()
  
  Returns a \ref GPPlottableInterface1D pointer to this plottable, providing access to its 1D
  interface.
  
  \seebaseclassmethod
*/

/* end documentation of inline functions */

/*!
  Forwards \a keyAxis and \a valueAxis to the \ref GPAbstractPlottable::GPAbstractPlottable
  "GPAbstractPlottable" constructor and allocates the \a mDataContainer.
*/
template <class DataType>
GPAbstractPlottable1D<DataType>::GPAbstractPlottable1D(GPAxis *keyAxis,
                                                         GPAxis *valueAxis,
                                                         GPLayerable *parentLayerable) :
  GPAbstractPlottable(keyAxis, valueAxis, parentLayerable),
  mDataContainer(new GPDataContainer<DataType>)
{
  mDataContainer->setUomProvider(keyAxis, valueAxis);
}

template <class DataType>
GPAbstractPlottable1D<DataType>::~GPAbstractPlottable1D()
{
}

/*!
  \copydoc GPPlottableInterface1D::dataCount
*/
template <class DataType>
int GPAbstractPlottable1D<DataType>::dataCount() const
{
  return mDataContainer->size();
}

/*!
  \copydoc GPPlottableInterface1D::dataMainKey
*/
template <class DataType>
double GPAbstractPlottable1D<DataType>::dataMainKey(int index) const
{
  if (index >= 0 && index < mDataContainer->size())
  {
    return (mDataContainer->constBegin()+index)->mainKey();
  } else
  {
    qDebug() << Q_FUNC_INFO << "Index out of bounds" << index;
    return 0;
  }
}

/*!
  \copydoc GPPlottableInterface1D::dataSortKey
*/
template <class DataType>
double GPAbstractPlottable1D<DataType>::dataSortKey(int index) const
{
  if (index >= 0 && index < mDataContainer->size())
  {
    return (mDataContainer->constBegin()+index)->sortKey();
  } else
  {
    qDebug() << Q_FUNC_INFO << "Index out of bounds" << index;
    return 0;
  }
}

/*!
  \copydoc GPPlottableInterface1D::dataMainValue
*/
template <class DataType>
double GPAbstractPlottable1D<DataType>::dataMainValue(int index) const
{
  if (index >= 0 && index < mDataContainer->size())
  {
    return (mDataContainer->constBegin()+index)->mainValue();
  } else
  {
    qDebug() << Q_FUNC_INFO << "Index out of bounds" << index;
    return 0;
  }
}

/*!
  \copydoc GPPlottableInterface1D::dataValueRange
*/
template <class DataType>
GPRange GPAbstractPlottable1D<DataType>::dataValueRange(int index) const
{
  if (index >= 0 && index < mDataContainer->size())
  {
    return (mDataContainer->constBegin()+index)->valueRange();
  } else
  {
    qDebug() << Q_FUNC_INFO << "Index out of bounds" << index;
    return GPRange(0, 0);
  }
}

/*!
  \copydoc GPPlottableInterface1D::dataPixelPosition
*/
template <class DataType>
QPointF GPAbstractPlottable1D<DataType>::dataPixelPosition(int index) const
{
  if (index >= 0 && index < mDataContainer->size())
  {
    const typename GPDataContainer<DataType>::const_iterator it = mDataContainer->constBegin()+index;
    return coordsToPixels(it->mainKey(), it->mainValue());
  } else
  {
    qDebug() << Q_FUNC_INFO << "Index out of bounds" << index;
    return QPointF();
  }
}

/*!
  \copydoc GPPlottableInterface1D::sortKeyIsMainKey
*/
template <class DataType>
bool GPAbstractPlottable1D<DataType>::sortKeyIsMainKey() const
{
  return DataType::sortKeyIsMainKey();
}

/*!
  Implements a rect-selection algorithm assuming the data (accessed via the 1D data interface) is
  point-like. Most subclasses will want to reimplement this method again, to provide a more
  accurate hit test based on the true data visualization geometry.

  \seebaseclassmethod
*/
template <class DataType>
GPDataSelection GPAbstractPlottable1D<DataType>::selectTestRect(const QRectF &rect,
                                                                  bool onlySelectable, 
                                                                  bool acceptAllValues) const
{
  GPDataSelection result;
  if ((onlySelectable && mSelectable == GP::stNone) || mDataContainer->isEmpty())
    return result;
  if (!mKeyAxis || !mValueAxis)
    return result;
  
  // convert rect given in pixels to ranges given in plot coordinates:
  double key1, value1, key2, value2;
  pixelsToCoords(rect.topLeft(), key1, value1);
  pixelsToCoords(rect.bottomRight(), key2, value2);
  GPRange keyRange(key1, key2); // GPRange normalizes internally so we don't have to care about whether key1 < key2
  GPRange valueRange(value1, value2);
  typename GPDataContainer<DataType>::const_iterator begin = mDataContainer->constBegin();
  typename GPDataContainer<DataType>::const_iterator end = mDataContainer->constEnd();
  if (DataType::sortKeyIsMainKey()) // we can assume that data is sorted by main key, so can reduce the searched key interval:
  {
    begin = mDataContainer->constFindBegin(keyRange.lower, false);
    end = mDataContainer->constFindEnd(keyRange.upper, false);
  }
  if (begin == end)
    return result;
  
  int currentSegmentBegin = -1; // -1 means we're currently not in a segment that's contained in rect
  for (typename GPDataContainer<DataType>::const_iterator it=begin; it!=end; ++it)
  {
    if (currentSegmentBegin == -1)
    {
      if ((valueRange.contains(it->mainValue()) || acceptAllValues)
        && keyRange.contains(it->mainKey())) // start segment
        currentSegmentBegin = it-mDataContainer->constBegin();
    } else if ((!valueRange.contains(it->mainValue()) && !acceptAllValues)
            || !keyRange.contains(it->mainKey())) // segment just ended
    {
      result.addDataRange(GPDataRange(currentSegmentBegin, it-mDataContainer->constBegin()), false);
      currentSegmentBegin = -1;
    }
  }
  // process potential last segment:
  if (currentSegmentBegin != -1)
    result.addDataRange(GPDataRange(currentSegmentBegin, end-mDataContainer->constBegin()), false);
  
  result.simplify();
  return result;
}

/*!
  \copydoc GPPlottableInterface1D::findBegin
*/
template <class DataType>
int GPAbstractPlottable1D<DataType>::findBegin(double sortKey, bool expandedRange) const
{
  return mDataContainer->constFindBegin(sortKey, expandedRange)-mDataContainer->constBegin();
}

/*!
  \copydoc GPPlottableInterface1D::findEnd
*/
template <class DataType>
int GPAbstractPlottable1D<DataType>::findEnd(double sortKey, bool expandedRange) const
{
  return mDataContainer->constFindEnd(sortKey, expandedRange)-mDataContainer->constBegin();
}

/*!
  Implements a point-selection algorithm assuming the data (accessed via the 1D data interface) is
  point-like. Most subclasses will want to reimplement this method again, to provide a more
  accurate hit test based on the true data visualization geometry.

  \seebaseclassmethod
*/
template <class DataType>
double GPAbstractPlottable1D<DataType>::selectTest(const QPointF &pos, bool onlySelectable, QVariant *details) const
{
  if ((onlySelectable && mSelectable == GP::stNone) || mDataContainer->isEmpty())
    return -1;
  if (!mKeyAxis || !mValueAxis)
    return -1;
  
  GPDataSelection selectionResult;
  double minDistSqr = std::numeric_limits<double>::max();
  int minDistIndex = mDataContainer->size();
  
  typename GPDataContainer<DataType>::const_iterator begin = mDataContainer->constBegin();
  typename GPDataContainer<DataType>::const_iterator end = mDataContainer->constEnd();
  if (DataType::sortKeyIsMainKey()) // we can assume that data is sorted by main key, so can reduce the searched key interval:
  {
    // determine which key range comes into question, taking selection tolerance around pos into account:
    double posKeyMin, posKeyMax, dummy;
    pixelsToCoords(pos-QPointF(mParentPlot->selectionTolerance(), mParentPlot->selectionTolerance()), posKeyMin, dummy);
    pixelsToCoords(pos+QPointF(mParentPlot->selectionTolerance(), mParentPlot->selectionTolerance()), posKeyMax, dummy);
    if (posKeyMin > posKeyMax)
      qSwap(posKeyMin, posKeyMax);
    begin = mDataContainer->constFindBegin(posKeyMin, true);
    end = mDataContainer->constFindEnd(posKeyMax, true);
  }
  if (begin == end)
    return -1;
  GPRange keyRange(mKeyAxis->range());
  GPRange valueRange(mValueAxis->range());
  for (typename GPDataContainer<DataType>::const_iterator it=begin; it!=end; ++it)
  {
    const double mainKey = it->mainKey();
    const double mainValue = it->mainValue();
    if (keyRange.contains(mainKey) && valueRange.contains(mainValue)) // make sure data point is inside visible range, for speedup in cases where sort key isn't main key and we iterate over all points
    {
      const double currentDistSqr = GPVector2D(coordsToPixels(mainKey, mainValue)-pos).lengthSquared();
      if (currentDistSqr < minDistSqr)
      {
        minDistSqr = currentDistSqr;
        minDistIndex = it-mDataContainer->constBegin();
      }
    }
  }
  if (minDistIndex != mDataContainer->size())
    selectionResult.addDataRange(GPDataRange(minDistIndex, minDistIndex+1), false);
  
  selectionResult.simplify();
  if (details)
    details->setValue(selectionResult);
  return qSqrt(minDistSqr);
}

/*!
  Splits all data into selected and unselected segments and outputs them via \a selectedSegments
  and \a unselectedSegments, respectively.

  This is useful when subclasses implement their \ref draw method and need to draw selected
  segments with a different pen/brush than unselected segments (also see \ref
  GPSelectionDecorator).

  \see setSelection
*/
template <class DataType>
void GPAbstractPlottable1D<DataType>::getDataSegments(QList<GPDataRange> &selectedSegments, QList<GPDataRange> &unselectedSegments) const
{
  selectedSegments.clear();
  unselectedSegments.clear();
  if (mSelectable == GP::stWhole) // stWhole selection type draws the entire plottable with selected style if mSelection isn't empty
  {
    if (selected())
      selectedSegments << GPDataRange(0, dataCount());
    else
      unselectedSegments << GPDataRange(0, dataCount());
  }
  else
  {
    GPDataSelection sel(selection());
    sel.simplify();
    selectedSegments = sel.dataRanges();
    unselectedSegments = sel.inverse(GPDataRange(0, dataCount())).dataRanges();
  }
}

/*!
  A helper method which draws a line with the passed \a painter, according to the pixel data in \a
  lineData. NaN points create gaps in the line, as expected from GraphicsPlot's plottables (this is
  the main difference to QPainter's regular drawPolyline, which handles NaNs by lagging or
  crashing).

  Further it uses a faster line drawing technique based on \ref GPPainter::drawLine rather than \c
  QPainter::drawPolyline if the configured \ref GraphicsPlot::setPlottingHints() and \a painter
  style allows.
*/
template <class DataType>
void GPAbstractPlottable1D<DataType>::drawPolyline(GPPainter *painter, const QVector<QPointF> &lineData) const
{
  // if drawing solid line and not in PDF, use much faster line drawing instead of polyline:
  if (mParentPlot->plottingHints().testFlag(GP::phFastPolylines) &&
      painter->pen().style() == Qt::SolidLine &&
      !painter->modes().testFlag(GPPainter::pmVectorized) &&
      !painter->modes().testFlag(GPPainter::pmNoCaching))
  {
    int i = 0;
    bool lastIsNan = false;
    const int lineDataSize = lineData.size();
    while (i < lineDataSize && (qIsNaN(lineData.at(i).y()) || qIsNaN(lineData.at(i).x()))) // make sure first point is not NaN
      ++i;
    ++i; // because drawing works in 1 point retrospect
    while (i < lineDataSize)
    {
      if (!qIsNaN(lineData.at(i).y()) && !qIsNaN(lineData.at(i).x())) // NaNs create a gap in the line
      {
        if (!lastIsNan)
          painter->drawLine(lineData.at(i-1), lineData.at(i));
        else
          lastIsNan = false;
      } else
        lastIsNan = true;
      ++i;
    }
  }
  else
  {
    int segmentStart = 0;
    int i = 0;
    const int lineDataSize = lineData.size();
    while (i < lineDataSize)
    {
      if (qIsNaN(lineData.at(i).y()) || qIsNaN(lineData.at(i).x()) || qIsInf(lineData.at(i).y())) // NaNs create a gap in the line. Also filter Infs which make drawPolyline block
      {
        painter->drawPolyline(lineData.constData()+segmentStart, i-segmentStart); // i, because we don't want to include the current NaN point
        segmentStart = i+1;
      }
      ++i;
    }
    // draw last segment:
    painter->drawPolyline(lineData.constData()+segmentStart, lineDataSize-segmentStart);
  }
}

class GP_LIB_DECL GPColorGradient
{
  Q_GADGET
public:
  /*!
    Defines the color spaces in which color interpolation between gradient stops can be performed.
    
    \see setColorInterpolation
  */
  enum ColorInterpolation { ciRGB  ///< Color channels red, green and blue are linearly interpolated
                            ,ciHSV ///< Color channels hue, saturation and value are linearly interpolated (The hue is interpolated over the shortest angle distance)
                          };
  Q_ENUMS(ColorInterpolation)
  
  /*!
    Defines the available presets that can be loaded with \ref loadPreset. See the documentation
    there for an image of the presets.
  */
  enum GradientPreset { gpGrayscale  ///< Continuous lightness from black to white (suited for non-biased data representation)
                        ,gpHot       ///< Continuous lightness from black over firey colors to white (suited for non-biased data representation)
                        ,gpCold      ///< Continuous lightness from black over icey colors to white (suited for non-biased data representation)
                        ,gpNight     ///< Continuous lightness from black over weak blueish colors to white (suited for non-biased data representation)
                        ,gpCandy     ///< Blue over pink to white
                        ,gpGeography ///< Colors suitable to represent different elevations on geographical maps
                        ,gpIon       ///< Half hue spectrum from black over purple to blue and finally green (creates banding illusion but allows more precise magnitude estimates)
                        ,gpThermal   ///< Colors suitable for thermal imaging, ranging from dark blue over purple to orange, yellow and white
                        ,gpPolar     ///< Colors suitable to emphasize polarity around the center, with blue for negative, black in the middle and red for positive values
                        ,gpSpectrum  ///< An approximation of the visible light spectrum (creates banding illusion but allows more precise magnitude estimates)
                        ,gpJet       ///< Hue variation similar to a spectrum, often used in numerical visualization (creates banding illusion but allows more precise magnitude estimates)
                        ,gpHues      ///< Full hue cycle, with highest and lowest color red (suitable for periodic data, such as angles and phases, see \ref setPeriodic)
                      };
  Q_ENUMS(GradientPreset)
  
  GPColorGradient();
  GPColorGradient(GradientPreset preset);
  bool operator==(const GPColorGradient &other) const;
  bool operator!=(const GPColorGradient &other) const { return !(*this == other); }
  
  // getters:
  int levelCount() const { return mLevelCount; }
  QMap<double, QColor> colorStops() const { return mColorStops; }
  ColorInterpolation colorInterpolation() const { return mColorInterpolation; }
  bool periodic() const { return mPeriodic; }
  
  // setters:
  void setLevelCount(int n);
  void setColorStops(const QMap<double, QColor>& colorStops);
  void setColorStopAt(double position, const QColor &color);
  void setColorInterpolation(ColorInterpolation interpolation);
  void setPeriodic(bool enabled);
  
  // non-property methods:
  void colorize(const double *data, const GPRange &range, QRgb *scanLine, int n, int dataIndexFactor=1, bool logarithmic=false);
  void colorize(const double *data, const unsigned char *alpha, const GPRange &range, QRgb *scanLine, int n, int dataIndexFactor=1, bool logarithmic=false);
  QRgb color(double position, const GPRange &range, bool logarithmic=false);
  void loadPreset(GradientPreset preset);
  void clearColorStops();
  GPColorGradient inverted() const;
  
protected:
  // property members:
  int mLevelCount;
  QMap<double, QColor> mColorStops;
  ColorInterpolation mColorInterpolation;
  bool mPeriodic;
  
  // non-property members:
  QVector<QRgb> mColorBuffer; // have colors premultiplied with alpha (for usage with QImage::Format_ARGB32_Premultiplied)
  bool mColorBufferInvalidated;
  
  // non-virtual methods:
  bool stopsUseAlpha() const;
  void updateColorBuffer();
};
Q_DECLARE_METATYPE(GPColorGradient::ColorInterpolation)
Q_DECLARE_METATYPE(GPColorGradient::GradientPreset)

class GP_LIB_DECL GPSelectionDecoratorBracket : public GPSelectionDecorator
{
  Q_GADGET
public:
  
  /*!
    Defines which shape is drawn at the boundaries of selected data ranges.
    
    Some of the bracket styles further allow specifying a height and/or width, see \ref
    setBracketHeight and \ref setBracketWidth.
  */
  enum BracketStyle { bsSquareBracket ///< A square bracket is drawn.
                      ,bsHalfEllipse   ///< A half ellipse is drawn. The size of the ellipse is given by the bracket width/height properties.
                      ,bsEllipse       ///< An ellipse is drawn. The size of the ellipse is given by the bracket width/height properties.
                      ,bsPlus         ///< A plus is drawn.
                      ,bsUserStyle    ///< Start custom bracket styles at this index when subclassing and reimplementing \ref drawBracket.
  };
  Q_ENUMS(BracketStyle)
  
  GPSelectionDecoratorBracket();
  virtual ~GPSelectionDecoratorBracket();
  
  // getters:
  QPen bracketPen() const { return mBracketPen; }
  QBrush bracketBrush() const { return mBracketBrush; }
  int bracketWidth() const { return mBracketWidth; }
  int bracketHeight() const { return mBracketHeight; }
  BracketStyle bracketStyle() const { return mBracketStyle; }
  bool tangentToData() const { return mTangentToData; }
  int tangentAverage() const { return mTangentAverage; }
  
  // setters:
  void setBracketPen(const QPen &pen);
  void setBracketBrush(const QBrush &brush);
  void setBracketWidth(int width);
  void setBracketHeight(int height);
  void setBracketStyle(BracketStyle style);
  void setTangentToData(bool enabled);
  void setTangentAverage(int pointCount);
  
  // introduced virtual methods:
  virtual void drawBracket(GPPainter *painter, int direction) const;
  
  // virtual methods:
  virtual void drawDecoration(GPPainter *painter, GPDataSelection selection) override;
  
protected:
  // property members:
  QPen mBracketPen;
  QBrush mBracketBrush;
  int mBracketWidth;
  int mBracketHeight;
  BracketStyle mBracketStyle;
  bool mTangentToData;
  int mTangentAverage;
  
  // non-virtual methods:
  double getTangentAngle(const GPPlottableInterface1D *interface1d, int dataIndex, int direction) const;
  QPointF getPixelCoordinates(const GPPlottableInterface1D *interface1d, int dataIndex) const;
};
Q_DECLARE_METATYPE(GPSelectionDecoratorBracket::BracketStyle)

class GP_LIB_DECL GPAxisRect : public GPLayoutElement
{
  Q_OBJECT
  /// \cond INCLUDE_QPROPERTIES
  Q_PROPERTY(QPixmap background READ background WRITE setBackground)
  Q_PROPERTY(bool backgroundScaled READ backgroundScaled WRITE setBackgroundScaled)
  Q_PROPERTY(Qt::AspectRatioMode backgroundScaledMode READ backgroundScaledMode WRITE setBackgroundScaledMode)
  Q_PROPERTY(Qt::Orientations rangeDrag READ rangeDrag WRITE setRangeDrag)
  Q_PROPERTY(Qt::Orientations rangeZoom READ rangeZoom WRITE setRangeZoom)
  /// \endcond
public:
  explicit GPAxisRect(GraphicsPlot *parentPlot, bool setupDefaultAxes=true);
  virtual ~GPAxisRect();
  
  // getters:
  QPixmap background() const { return mBackgroundPixmap; }
  QBrush backgroundBrush() const { return mBackgroundBrush; }
  bool backgroundScaled() const { return mBackgroundScaled; }
  Qt::AspectRatioMode backgroundScaledMode() const { return mBackgroundScaledMode; }
  Qt::Orientations rangeDrag() const { return mRangeDrag; }
  Qt::Orientations rangeZoom() const { return mRangeZoom; }
  GPAxis *rangeDragAxis(Qt::Orientation orientation);
  GPAxis *rangeZoomAxis(Qt::Orientation orientation);
  QList<GPAxis*> rangeDragAxes(Qt::Orientation orientation);
  QList<GPAxis*> rangeZoomAxes(Qt::Orientation orientation);
  double rangeZoomFactor(Qt::Orientation orientation);
  virtual const QRect& axisDrawRect(GPAxis::AxisType) const { return mRect; }

  // setters:
  void setBackground(const QPixmap &pm);
  void setBackground(const QPixmap &pm, bool scaled, Qt::AspectRatioMode mode=Qt::KeepAspectRatioByExpanding);
  void setBackground(const QBrush &brush);
  void setBackgroundScaled(bool scaled);
  void setBackgroundScaledMode(Qt::AspectRatioMode mode);
  void setRangeDrag(Qt::Orientations orientations);
  void setRangeZoom(Qt::Orientations orientations);
  void setRangeDragAxes(GPAxis *horizontal, GPAxis *vertical);
  void setRangeDragAxes(QList<GPAxis*> axes);
  void setRangeDragAxes(QList<GPAxis*> horizontal, QList<GPAxis*> vertical);
  void setRangeZoomAxes(GPAxis *horizontal, GPAxis *vertical);
  void setRangeZoomAxes(QList<GPAxis*> axes);
  void setRangeZoomAxes(QList<GPAxis*> horizontal, QList<GPAxis*> vertical);
  void setRangeZoomFactor(double horizontalFactor, double verticalFactor);
  void setRangeZoomFactor(double factor);
  
  // non-property methods:
  int axisCount(GPAxis::AxisType type) const;
  GPAxis *axis(GPAxis::AxisType type, int index=0) const;
  QList<GPAxis*> axes(GPAxis::AxisTypes types) const;
  QList<GPAxis*> axes() const;
  virtual GPAxis *addAxis(GPAxis::AxisType type, GPAxis *axis=0);
  virtual QList<GPAxis*> addAxes(GPAxis::AxisTypes types);
  void addRangeZoomDragAxis(GPAxis* axis);
  bool removeAxis(GPAxis *axis);
  GPLayoutInset *insetLayout() const { return mInsetLayout; }

  virtual void moveAxis(GPAxis* axis, GPAxis::AxisType to);
  
  void zoom(const QRectF &pixelRect);
  void zoom(const QRectF &pixelRect, const QList<GPAxis*> &affectedAxes);
  void setupFullAxesBox(bool connectRanges=false);
  QList<GPAbstractPlottable*> plottables() const;
  QList<GPGraph*> graphs() const;
  QList<GPCurve*> curves() const;
  QList<GPAbstractItem*> items() const;
  
  // read-only interface imitating a QRect:
  int left() const { return mRect.left(); }
  int right() const { return mRect.right(); }
  int top() const { return mRect.top(); }
  int bottom() const { return mRect.bottom(); }
  int width() const { return mRect.width(); }
  int height() const { return mRect.height(); }
  QSize size() const { return mRect.size(); }
  QPoint topLeft() const { return mRect.topLeft(); }
  QPoint topRight() const { return mRect.topRight(); }
  QPoint bottomLeft() const { return mRect.bottomLeft(); }
  QPoint bottomRight() const { return mRect.bottomRight(); }
  QPoint center() const { return mRect.center(); }
  
  // reimplemented virtual methods:
  virtual void update(UpdatePhase phase) override;
  virtual QList<GPLayoutElement*> elements(bool recursive) const override;

protected:
  // property members:
  QBrush mBackgroundBrush;
  QPixmap mBackgroundPixmap;
  QPixmap mScaledBackgroundPixmap;
  bool mBackgroundScaled;
  Qt::AspectRatioMode mBackgroundScaledMode;
  GPLayoutInset *mInsetLayout;
  Qt::Orientations mRangeDrag, mRangeZoom;
  QList<QPointer<GPAxis>> mRangeDragHorzAxis, mRangeDragVertAxis;
  QList<QPointer<GPAxis>> mRangeZoomHorzAxis, mRangeZoomVertAxis;
  double mRangeZoomFactorHorz, mRangeZoomFactorVert;
  
  // non-property members:
  QList<GPRange> mDragStartHorzRange, mDragStartVertRange;
  GP::AntialiasedElements mAADragBackup, mNotAADragBackup;
  bool mDragging;
  QHash<GPAxis::AxisType, QList<GPAxis*>> mAxes;
  
public:
  // reimplemented virtual methods:
  virtual void applyDefaultAntialiasingHint(GPPainter *painter) const override;
  virtual void draw(GPPainter *painter) override;
  virtual int calculateAutoMargin(GP::MarginSide side) override;
  virtual void layoutChanged() override;
  // events:
  virtual void mousePressEvent(QMouseEvent *event, const QVariant &details) override;
  virtual void mouseMoveEvent(QMouseEvent *event, const QPointF &startPos) override;
  virtual void mouseReleaseEvent(QMouseEvent *event, const QPointF &startPos) override;
  virtual void wheelEvent(QWheelEvent *event) override;
  
  // non-property methods:
  void drawBackground(GPPainter *painter);
  virtual bool takeAxis(GPAxis* axis);
  virtual int indexOf(GPAxis* axis);
  virtual void updateAxesOffset(GPAxis::AxisType type);
  
private:
  Q_DISABLE_COPY(GPAxisRect)
  
  friend class GraphicsPlot;
};

class GP_LIB_DECL GPAbstractLegendItem : public GPLayoutElement
{
  Q_OBJECT
  /// \cond INCLUDE_QPROPERTIES
  Q_PROPERTY(GPLegend* parentLegend READ parentLegend)
  Q_PROPERTY(QFont font READ font WRITE setFont)
  Q_PROPERTY(QColor textColor READ textColor WRITE setTextColor)
  Q_PROPERTY(QFont selectedFont READ selectedFont WRITE setSelectedFont)
  Q_PROPERTY(QColor selectedTextColor READ selectedTextColor WRITE setSelectedTextColor)
  Q_PROPERTY(bool selectable READ selectable WRITE setSelectable NOTIFY selectionChanged)
  Q_PROPERTY(bool selected READ selected WRITE setSelected NOTIFY selectableChanged)
  /// \endcond
public:
  explicit GPAbstractLegendItem(GPLegend *parent);
  
  // getters:
  GPLegend *parentLegend() const { return mParentLegend; }
  QFont font() const { return mFont; }
  QColor textColor() const { return mTextColor; }
  QFont selectedFont() const { return mSelectedFont; }
  QColor selectedTextColor() const { return mSelectedTextColor; }
  bool selectable() const { return mSelectable; }
  bool selected() const { return mSelected; }
  
  // setters:
  void setFont(const QFont &font);
  void setTextColor(const QColor &color);
  void setSelectedFont(const QFont &font);
  void setSelectedTextColor(const QColor &color);
  Q_SLOT void setSelectable(bool selectable);
  Q_SLOT virtual void setSelected(bool selected);
  
  // reimplemented virtual methods:
  virtual double selectTest(const QPointF &pos, bool onlySelectable, QVariant *details=0) const override;
  
signals:
  void selectionChanged(bool selected);
  void selectableChanged(bool selectable);
  
protected:
  // property members:
  GPLegend *mParentLegend;
  QFont mFont;
  QColor mTextColor;
  QFont mSelectedFont;
  QColor mSelectedTextColor;
  bool mSelectable, mSelected;
  
  // reimplemented virtual methods:
  virtual GP::Interaction selectionCategory() const override;
  virtual void applyDefaultAntialiasingHint(GPPainter *painter) const override;
  virtual QRect clipRect() const override;
  virtual void draw(GPPainter *painter) override = 0;
  // events:
  virtual void selectEvent(QMouseEvent *event, bool additive, const QVariant &details, bool *selectionStateChanged) override;
  virtual void deselectEvent(bool *selectionStateChanged) override;
  
private:
  Q_DISABLE_COPY(GPAbstractLegendItem)
  
  friend class GPLegend;
};


class GP_LIB_DECL GPPlottableLegendItem : public GPAbstractLegendItem
{
  Q_OBJECT
public:
  GPPlottableLegendItem(GPLegend *parent, GPAbstractPlottable *plottable);
  
  // getters:
  GPAbstractPlottable *plottable() { return mPlottable; }
  
  Q_SLOT virtual void setSelected(bool selected) override;

protected:
  // property members:
  GPAbstractPlottable *mPlottable;
  
  // reimplemented virtual methods:
  virtual void draw(GPPainter *painter) override;
  virtual QSize minimumOuterSizeHint() const override;
  
  Q_SLOT virtual void plottableSelectionChanged(bool selected);

  // non-virtual methods:
  QPen getIconBorderPen() const;
  QColor getTextColor() const;
  QFont getFont() const;
};


class GP_LIB_DECL GPLegend : public GPLayoutGrid
{
  Q_OBJECT
  /// \cond INCLUDE_QPROPERTIES
  Q_PROPERTY(QPen borderPen READ borderPen WRITE setBorderPen)
  Q_PROPERTY(QBrush brush READ brush WRITE setBrush)
  Q_PROPERTY(QFont font READ font WRITE setFont)
  Q_PROPERTY(QColor textColor READ textColor WRITE setTextColor)
  Q_PROPERTY(QSize iconSize READ iconSize WRITE setIconSize)
  Q_PROPERTY(int iconTextPadding READ iconTextPadding WRITE setIconTextPadding)
  Q_PROPERTY(QPen iconBorderPen READ iconBorderPen WRITE setIconBorderPen)
  Q_PROPERTY(SelectableParts selectableParts READ selectableParts WRITE setSelectableParts NOTIFY selectionChanged)
  Q_PROPERTY(SelectableParts selectedParts READ selectedParts WRITE setSelectedParts NOTIFY selectableChanged)
  Q_PROPERTY(QPen selectedBorderPen READ selectedBorderPen WRITE setSelectedBorderPen)
  Q_PROPERTY(QPen selectedIconBorderPen READ selectedIconBorderPen WRITE setSelectedIconBorderPen)
  Q_PROPERTY(QBrush selectedBrush READ selectedBrush WRITE setSelectedBrush)
  Q_PROPERTY(QFont selectedFont READ selectedFont WRITE setSelectedFont)
  Q_PROPERTY(QColor selectedTextColor READ selectedTextColor WRITE setSelectedTextColor)
  /// \endcond
public:
  /*!
    Defines the selectable parts of a legend
    
    \see setSelectedParts, setSelectableParts
  */
  enum SelectablePart { spNone        = 0x000 ///< <tt>0x000</tt> None
                        ,spLegendBox  = 0x001 ///< <tt>0x001</tt> The legend box (frame)
                        ,spItems      = 0x002 ///< <tt>0x002</tt> Legend items individually (see \ref selectedItems)
                      };
  Q_ENUMS(SelectablePart)
  Q_FLAGS(SelectableParts)
  Q_DECLARE_FLAGS(SelectableParts, SelectablePart)
  
  explicit GPLegend();
  explicit GPLegend(GraphicsPlot* plot);
  virtual ~GPLegend();
  
  // getters:
  QPen borderPen() const { return mBorderPen; }
  QBrush brush() const { return mBrush; }
  QFont font() const { return mFont; }
  QColor textColor() const { return mTextColor; }
  QSize iconSize() const { return mIconSize; }
  int iconTextPadding() const { return mIconTextPadding; }
  QPen iconBorderPen() const { return mIconBorderPen; }
  SelectableParts selectableParts() const { return mSelectableParts; }
  SelectableParts selectedParts() const;
  QPen selectedBorderPen() const { return mSelectedBorderPen; }
  QPen selectedIconBorderPen() const { return mSelectedIconBorderPen; }
  QBrush selectedBrush() const { return mSelectedBrush; }
  QFont selectedFont() const { return mSelectedFont; }
  QColor selectedTextColor() const { return mSelectedTextColor; }
  
  // setters:
  void setBorderPen(const QPen &pen);
  void setBrush(const QBrush &brush);
  void setFont(const QFont &font);
  void setTextColor(const QColor &color);
  void setIconSize(const QSize &size);
  void setIconSize(int width, int height);
  void setIconTextPadding(int padding);
  void setIconBorderPen(const QPen &pen);
  Q_SLOT void setSelectableParts(const SelectableParts &selectableParts);
  Q_SLOT void setSelectedParts(const SelectableParts &selectedParts);
  void setSelectedBorderPen(const QPen &pen);
  void setSelectedIconBorderPen(const QPen &pen);
  void setSelectedBrush(const QBrush &brush);
  void setSelectedFont(const QFont &font);
  void setSelectedTextColor(const QColor &color);
  
  // reimplemented virtual methods:
  virtual double selectTest(const QPointF &pos, bool onlySelectable, QVariant *details=0) const override;
  
  // non-virtual methods:
  GPAbstractLegendItem *item(int index) const;
  GPPlottableLegendItem *itemWithPlottable(const GPAbstractPlottable *plottable) const;
  int itemCount() const;
  bool hasItem(GPAbstractLegendItem *item) const;
  bool hasItemWithPlottable(const GPAbstractPlottable *plottable) const;
  virtual bool addItem(GPAbstractLegendItem *item);
  bool removeItem(int index);
  bool removeItem(GPAbstractLegendItem *item);
  void clearItems();
  QList<GPAbstractLegendItem*> selectedItems() const;
  
signals:
  void selectionChanged(GPLegend::SelectableParts parts);
  void selectableChanged(GPLegend::SelectableParts parts);
  
protected:
  // property members:
  QPen mBorderPen, mIconBorderPen;
  QBrush mBrush;
  QFont mFont;
  QColor mTextColor;
  QSize mIconSize;
  int mIconTextPadding;
  SelectableParts mSelectedParts, mSelectableParts;
  QPen mSelectedBorderPen, mSelectedIconBorderPen;
  QBrush mSelectedBrush;
  QFont mSelectedFont;
  QColor mSelectedTextColor;
  
  // reimplemented virtual methods:
  virtual void parentPlotInitialized(GraphicsPlot *parentPlot) override;
  virtual GP::Interaction selectionCategory() const override;
  virtual void applyDefaultAntialiasingHint(GPPainter *painter) const override;
  virtual void draw(GPPainter *painter) override;
  // events:
  virtual void selectEvent(QMouseEvent *event, bool additive, const QVariant &details, bool *selectionStateChanged) override;
  virtual void deselectEvent(bool *selectionStateChanged) override;
  
  // non-virtual methods:
  QPen getBorderPen() const;
  QBrush getBrush() const;
  
private:
  Q_DISABLE_COPY(GPLegend)
  
  friend class GraphicsPlot;
  friend class GPAbstractLegendItem;
};
Q_DECLARE_OPERATORS_FOR_FLAGS(GPLegend::SelectableParts)
Q_DECLARE_METATYPE(GPLegend::SelectablePart)

class GP_LIB_DECL GPTextElement : public GPLayoutElement
{
  Q_OBJECT
  /// \cond INCLUDE_QPROPERTIES
  Q_PROPERTY(QString text READ text WRITE setText)
  Q_PROPERTY(QFont font READ font WRITE setFont)
  Q_PROPERTY(QColor textColor READ textColor WRITE setTextColor)
  Q_PROPERTY(QFont selectedFont READ selectedFont WRITE setSelectedFont)
  Q_PROPERTY(QColor selectedTextColor READ selectedTextColor WRITE setSelectedTextColor)
  Q_PROPERTY(bool selectable READ selectable WRITE setSelectable NOTIFY selectableChanged)
  Q_PROPERTY(bool selected READ selected WRITE setSelected NOTIFY selectionChanged)
  /// \endcond
public:
  explicit GPTextElement(GraphicsPlot *parentPlot);
  GPTextElement(GraphicsPlot *parentPlot, const QString &text);
  GPTextElement(GraphicsPlot *parentPlot, const QString &text, double pointSize);
  GPTextElement(GraphicsPlot *parentPlot, const QString &text, const QString &fontFamily, double pointSize);
  GPTextElement(GraphicsPlot *parentPlot, const QString &text, const QFont &font);
  
  // getters:
  QString text() const { return mText; }
  int textFlags() const { return mTextFlags; }
  QFont font() const { return mFont; }
  QColor textColor() const { return mTextColor; }
  QFont selectedFont() const { return mSelectedFont; }
  QColor selectedTextColor() const { return mSelectedTextColor; }
  bool selectable() const { return mSelectable; }
  bool selected() const { return mSelected; }
  
  // setters:
  void setText(const QString &text);
  void setTextFlags(int flags);
  void setFont(const QFont &font);
  void setTextColor(const QColor &color);
  void setSelectedFont(const QFont &font);
  void setSelectedTextColor(const QColor &color);
  Q_SLOT void setSelectable(bool selectable);
  Q_SLOT void setSelected(bool selected);
  
  // reimplemented virtual methods:
  virtual double selectTest(const QPointF &pos, bool onlySelectable, QVariant *details=0) const override;
  virtual void mousePressEvent(QMouseEvent *event, const QVariant &details) override;
  virtual void mouseReleaseEvent(QMouseEvent *event, const QPointF &startPos) override;
  virtual void mouseDoubleClickEvent(QMouseEvent *event, const QVariant &details) override;
  
signals:
  void selectionChanged(bool selected);
  void selectableChanged(bool selectable);
  void clicked(QMouseEvent *event);
  void doubleClicked(QMouseEvent *event);
  
protected:
  // property members:
  QString mText;
  int mTextFlags;
  QFont mFont;
  QColor mTextColor;
  QFont mSelectedFont;
  QColor mSelectedTextColor;
  QRect mTextBoundingRect;
  bool mSelectable, mSelected;
  
  // reimplemented virtual methods:
  virtual void applyDefaultAntialiasingHint(GPPainter *painter) const override;
  virtual void draw(GPPainter *painter) override;
  virtual QSize minimumOuterSizeHint() const override;
  virtual QSize maximumOuterSizeHint() const override;
  // events:
  virtual void selectEvent(QMouseEvent *event, bool additive, const QVariant &details, bool *selectionStateChanged) override;
  virtual void deselectEvent(bool *selectionStateChanged) override;
  
  // non-virtual methods:
  QFont mainFont() const;
  QColor mainTextColor() const;
  
private:
  Q_DISABLE_COPY(GPTextElement)
};

class GPColorScaleAxisRectPrivate : public GPAxisRect
{
  Q_OBJECT

public:
  explicit GPColorScaleAxisRectPrivate(GPColorScale *parentColorScale);

protected:
  // re-using some methods of GPAxisRect to make them available to friend class GPColorScale
  using GPAxisRect::calculateAutoMargin;
  using GPAxisRect::mousePressEvent;
  using GPAxisRect::mouseMoveEvent;
  using GPAxisRect::mouseReleaseEvent;
  using GPAxisRect::wheelEvent;
  using GPAxisRect::update;
  virtual void draw(GPPainter *painter) override;
  void updateGradientImage();
  Q_SLOT void axisSelectionChanged(GPAxis::SelectableParts selectedParts);
  Q_SLOT void axisSelectableChanged(GPAxis::SelectableParts selectableParts);

protected:
  GPColorScale *mParentColorScale;
  QImage mGradientImage;
  bool mGradientImageInvalidated;

  friend class GPColorScale;
};


class GP_LIB_DECL GPColorScale : public GPLayoutElement
{
  Q_OBJECT
  /// \cond INCLUDE_QPROPERTIES
  Q_PROPERTY(GPAxis::AxisType type READ type WRITE setType)
  Q_PROPERTY(GPRange dataRange READ dataRange WRITE setDataRange NOTIFY dataRangeChanged)
  Q_PROPERTY(GPAxis::ScaleType dataScaleType READ dataScaleType WRITE setDataScaleType NOTIFY dataScaleTypeChanged)
  Q_PROPERTY(GPColorGradient gradient READ gradient WRITE setGradient NOTIFY gradientChanged)
  Q_PROPERTY(QString label READ label WRITE setLabel)
  Q_PROPERTY(int barWidth READ barWidth WRITE setBarWidth)
  Q_PROPERTY(bool rangeDrag READ rangeDrag WRITE setRangeDrag)
  Q_PROPERTY(bool rangeZoom READ rangeZoom WRITE setRangeZoom)
  /// \endcond
public:
  explicit GPColorScale(GraphicsPlot *parentPlot);
  virtual ~GPColorScale();
  
  // getters:
  GPAxis *axis() const { return mColorAxis.data(); }
  GPAxis::AxisType type() const { return mType; }
  GPRange dataRange() const { return mDataRange; }
  GPAxis::ScaleType dataScaleType() const { return mDataScaleType; }
  GPColorGradient gradient() const { return mGradient; }
  QString label() const;
  int barWidth () const { return mBarWidth; }
  bool rangeDrag() const;
  bool rangeZoom() const;
  
  // setters:
  void setType(GPAxis::AxisType type);
  Q_SLOT void setDataRange(const GPRange &dataRange);
  Q_SLOT void setDataScaleType(GPAxis::ScaleType scaleType);
  Q_SLOT void setGradient(const GPColorGradient &gradient);
  void setLabel(const QString &str);
  void setBarWidth(int width);
  void setRangeDrag(bool enabled);
  void setRangeZoom(bool enabled);
  
  // non-property methods:
  QList<GPColorMap*> colorMaps() const;
  void rescaleDataRange(bool onlyVisibleMaps);
  
  // reimplemented virtual methods:
  virtual void update(UpdatePhase phase) override;
  void draw(GPPainter *painter) override;

  // getters:
  QPen borderPen() const;
  QBrush brush() const;
  QFont font() const;
  QColor textColor() const;
  QMargins borderMargins(const QMargins& margins) const;

  // setters:
  void setBorderPen(const QPen &pen);
  void setBrush(const QBrush &brush);
  void setFont(const QFont &font);
  void setTextColor(const QColor &color);
  void setBorderMargins(const QMargins& margins);
  
signals:
  void dataRangeChanged(const GPRange &newRange);
  void dataScaleTypeChanged(GPAxis::ScaleType scaleType);
  void gradientChanged(const GPColorGradient &newGradient);

protected:
  // ui
  QMargins mBorderMargins;
  QPen mBorderPen;
  QBrush mBrush;
  QFont mFont;
  QColor mTextColor;

  // property members:
  GPAxis::AxisType mType;
  GPRange mDataRange;
  GPAxis::ScaleType mDataScaleType;
  GPColorGradient mGradient;
  int mBarWidth;
  
  // non-property members:
  QPointer<GPColorScaleAxisRectPrivate> mAxisRect;
  QPointer<GPAxis> mColorAxis;
  
  // reimplemented virtual methods:
  virtual void applyDefaultAntialiasingHint(GPPainter *painter) const override;
  // events:
  virtual void mousePressEvent(QMouseEvent *event, const QVariant &details) override;
  virtual void mouseMoveEvent(QMouseEvent *event, const QPointF &startPos) override;
  virtual void mouseReleaseEvent(QMouseEvent *event, const QPointF &startPos) override;
  virtual void wheelEvent(QWheelEvent *event) override;
  
private:
  Q_DISABLE_COPY(GPColorScale)
  
  friend class GPColorScaleAxisRectPrivate;
};

namespace GenesisMarkup{
  class ChromatogramPeak;
}//GenesisMarkup


class GP_LIB_DECL GPGraph : public GPAbstractPlottable1D<GPGraphData>
{
  Q_OBJECT
  /// \cond INCLUDE_QPROPERTIES
  Q_PROPERTY(LineStyle lineStyle READ lineStyle WRITE setLineStyle)
  Q_PROPERTY(GPScatterStyle scatterStyle READ scatterStyle WRITE setScatterStyle)
  Q_PROPERTY(int scatterSkip READ scatterSkip WRITE setScatterSkip)
  Q_PROPERTY(GPGraph* channelFillGraph READ channelFillGraph WRITE setChannelFillGraph)
  Q_PROPERTY(bool adaptiveSampling READ adaptiveSampling WRITE setAdaptiveSampling)
  /// \endcond
public:
  /*!
    Defines how the graph's line is represented visually in the plot. The line is drawn with the
    current pen of the graph (\ref setPen).
    \see setLineStyle
  */
  enum LineStyle { lsNone        ///< data points are not connected with any lines (e.g. data only represented
                                 ///< with symbols according to the scatter style, see \ref setScatterStyle)
                   ,lsLine       ///< data points are connected by a straight line
                   ,lsStepLeft   ///< line is drawn as steps where the step height is the value of the left data point
                   ,lsStepRight  ///< line is drawn as steps where the step height is the value of the right data point
                   ,lsStepCenter ///< line is drawn as steps where the step is in between two data points
                   ,lsImpulse    ///< each data point is represented by a line parallel to the value axis, which reaches from the data point to the zero-value-line
                 };
  Q_ENUMS(LineStyle)
  
  explicit GPGraph(GPAxis *keyAxis, GPAxis *valueAxis);
  virtual ~GPGraph();
  
  // getters:
  QSharedPointer<GPGraphDataContainer> data() const { return mDataContainer; }
  LineStyle lineStyle() const { return mLineStyle; }
  GPScatterStyle scatterStyle() const { return mScatterStyle; }
  int scatterSkip() const { return mScatterSkip; }
  GPGraph *channelFillGraph() const { return mChannelFillGraph.data(); }
  bool adaptiveSampling() const { return mAdaptiveSampling; }
  
  // setters:
  void setData(const QSharedPointer<GPGraphDataContainer>& data);
  void setData(const QVector<double> &keys, const QVector<double> &values, bool alreadySorted=false);
  void setLineStyle(LineStyle ls);
  void setScatterStyle(const GPScatterStyle &style);
  void setScatterSkip(int skip);
  void setChannelFillGraph(GPGraph *targetGraph);
  void setAdaptiveSampling(bool enabled);
  
  // non-property methods:
  void addData(const QVector<double> &keys, const QVector<double> &values, bool alreadySorted=false);
  void addData(double key, double value);
  
  // reimplemented virtual methods:
  virtual double selectTest(const QPointF &pos, bool onlySelectable, QVariant *details=0) const override;
  virtual GPRange getKeyRange(bool &foundRange, GP::SignDomain inSignDomain=GP::sdBoth) const override;
  virtual GPRange getValueRange(bool &foundRange, GP::SignDomain inSignDomain=GP::sdBoth, const GPRange &inKeyRange=GPRange()) const override;
  virtual GPDataRange getDataRange() const override;

protected:
  // property members:
  LineStyle mLineStyle;
  GPScatterStyle mScatterStyle;
  int mScatterSkip;
  QPointer<GPGraph> mChannelFillGraph;
  bool mAdaptiveSampling;
  
  // reimplemented virtual methods:
  virtual void draw(GPPainter *painter) override;
  virtual void drawLegendIcon(GPPainter *painter, const QRectF &rect) const override;
  
  // introduced virtual methods:
  virtual void drawFill(GPPainter *painter, QVector<QPointF> *lines) const;
  virtual void drawScatterPlot(GPPainter *painter, const QVector<QPointF> &scatters, const GPScatterStyle &style) const;
  virtual void drawLinePlot(GPPainter *painter, const QVector<QPointF> &lines) const;
  virtual void drawImpulsePlot(GPPainter *painter, const QVector<QPointF> &lines) const;
  
  virtual void getOptimizedLineData(QVector<GPGraphData> *lineData, const GPGraphDataContainer::const_iterator &begin, const GPGraphDataContainer::const_iterator &end) const;
  virtual void getOptimizedScatterData(QVector<GPGraphData> *scatterData, GPGraphDataContainer::const_iterator begin, GPGraphDataContainer::const_iterator end) const;
  
  // non-virtual methods:
  void getVisibleDataBounds(GPGraphDataContainer::const_iterator &begin, GPGraphDataContainer::const_iterator &end, const GPDataRange &rangeRestriction) const;
  void getLines(QVector<QPointF> *lines, const GPDataRange &dataRange) const;
  void getScatters(QVector<QPointF> *scatters, const GPDataRange &dataRange) const;
  QVector<QPointF> dataToLines(const QVector<GPGraphData> &data) const;
  QVector<QPointF> dataToStepLeftLines(const QVector<GPGraphData> &data) const;
  QVector<QPointF> dataToStepRightLines(const QVector<GPGraphData> &data) const;
  QVector<QPointF> dataToStepCenterLines(const QVector<GPGraphData> &data) const;
  QVector<QPointF> dataToImpulseLines(const QVector<GPGraphData> &data) const;
  QVector<GPDataRange> getNonNanSegments(const QVector<QPointF> *lineData, Qt::Orientation keyOrientation) const;
  QVector<QPair<GPDataRange, GPDataRange>> getOverlappingSegments(const QVector<GPDataRange>& thisSegments, const QVector<QPointF> *thisData, const QVector<GPDataRange>& otherSegments, const QVector<QPointF> *otherData) const;
  bool segmentsIntersect(double aLower, double aUpper, double bLower, double bUpper, int &bPrecedence) const;
  QPointF getFillBasePoint(QPointF matchingDataPoint) const;
  const QPolygonF getFillPolygon(const QVector<QPointF> *lineData, GPDataRange segment) const;
  const QPolygonF getChannelFillPolygon(const QVector<QPointF> *lineData, GPDataRange thisSegment, const QVector<QPointF> *otherData, GPDataRange otherSegment) const;
  int findIndexBelowX(const QVector<QPointF> *data, double x) const;
  int findIndexAboveX(const QVector<QPointF> *data, double x) const;
  int findIndexBelowY(const QVector<QPointF> *data, double y) const;
  int findIndexAboveY(const QVector<QPointF> *data, double y) const;
  double pointDistance(const QPointF &pixelPoint, GPGraphDataContainer::const_iterator &closestData) const;
  
  friend class GraphicsPlot;
  friend class GPLegend;
  friend class GenesisMarkup::ChromatogramPeak;
};
Q_DECLARE_METATYPE(GPGraph::LineStyle)


class GP_LIB_DECL GPCurve : public GPAbstractPlottable1D<GPCurveData>
{
  Q_OBJECT
  /// \cond INCLUDE_QPROPERTIES
  Q_PROPERTY(GPScatterStyle scatterStyle READ scatterStyle WRITE setScatterStyle)
  Q_PROPERTY(int scatterSkip READ scatterSkip WRITE setScatterSkip)
  Q_PROPERTY(LineStyle lineStyle READ lineStyle WRITE setLineStyle)
  /// \endcond
public:
  /*!
    Defines how the curve's line is represented visually in the plot. The line is drawn with the
    current pen of the curve (\ref setPen).
    \see setLineStyle
  */
  enum LineStyle { lsNone  ///< No line is drawn between data points (e.g. only scatters)
                   ,lsLine ///< Data points are connected with a straight line
                 };
  Q_ENUMS(LineStyle)
  
  explicit GPCurve(GPAxis *keyAxis, GPAxis *valueAxis, bool registerInPLot = true);
  virtual ~GPCurve();
  
  // getters:
  QSharedPointer<GPCurveDataContainer> data() const { return mDataContainer; }
  GPScatterStyle scatterStyle() const { return mScatterStyle; }
  int scatterSkip() const { return mScatterSkip; }
  LineStyle lineStyle() const { return mLineStyle; }
  
  // setters:
  void setData(const QSharedPointer<GPCurveDataContainer>& data);
  void setDataContainer(const QSharedPointer<GPCurveDataContainer>& data);
  virtual void setData(const QVector<double> &t, const QVector<double> &keys, const QVector<double> &values, bool alreadySorted=false);
  virtual void setData(const QVector<double> &keys, const QVector<double> &values);
  void setScatterStyle(const GPScatterStyle &style);
  void setScatterSkip(int skip);
  void setLineStyle(LineStyle style);
  
  // non-property methods:
  void addData(const QVector<double> &t, const QVector<double> &keys, const QVector<double> &values, bool alreadySorted=false);
  void addData(const QVector<double> &keys, const QVector<double> &values);
  void addData(double t, double key, double value);
  void addData(double key, double value);
  
  void insertData(int index, double key, double value);
  void insertData(int index, const QVector<double> &keys, const QVector<double> &values);

  // reimplemented virtual methods:
  virtual double selectTest(const QPointF &pos, bool onlySelectable, QVariant *details=0) const override;
  virtual GPRange getKeyRange(bool &foundRange, GP::SignDomain inSignDomain=GP::sdBoth) const override;
  virtual GPRange getValueRange(bool &foundRange, GP::SignDomain inSignDomain=GP::sdBoth, const GPRange &inKeyRange=GPRange()) const override;
  virtual GPDataRange getDataRange() const override;

protected:
  // property members:
  GPScatterStyle mScatterStyle;
  int mScatterSkip;
  LineStyle mLineStyle;
  
  // reimplemented virtual methods:
  virtual void draw(GPPainter *painter) override;
  virtual void drawLegendIcon(GPPainter *painter, const QRectF &rect) const override;
  
  // introduced virtual methods:
  virtual void drawCurveLine(GPPainter *painter, const QVector<QPointF> &lines) const;
  virtual void drawScatterPlot(GPPainter *painter, const QVector<QPointF> &points, const GPScatterStyle &style) const;
  
  // non-virtual methods:
  void getCurveLines(QVector<QPointF> *lines, const GPDataRange &dataRange, double penWidth) const;
  void getScatters(QVector<QPointF> *scatters, const GPDataRange &dataRange, double scatterWidth) const;
  int getRegion(const double& key, const double& value, const double& keyMin, const double& valueMax, const double& keyMax, const double& valueMin) const;
  QPointF getOptimizedPoint(const int& prevRegion, const double& otherKeyPx, const double& otherValuePx, const double& keyPx, const double& valuePx, const double& keyMinPx, const double& valueMaxPx, const double& keyMaxPx, const double& valueMinPx) const;
  QVector<QPointF> getOptimizedCornerPoints(const int& prevRegion, const int& currentRegion, const double& prevKeyPx, const double& prevValuePx, const double& keyPx, const double& valuePx, const double& keyMinPx, const double& valueMaxPx, const double& keyMaxPx, const double& valueMinPx) const;
  bool mayTraverse(const int& prevRegion, const int& currentRegion) const;
  bool getTraverse(const double& prevKeyPx, const double& prevValuePx, const double& keyPx, const double& valuePx, const double& keyMinPx, const double& valueMaxPx, const double& keyMaxPx, const double& valueMinPx, QPointF &crossA, QPointF &crossB) const;
  void getTraverseCornerPoints(const int& prevRegion, const int& currentRegion, const double& keyMinPx, const double& valueMaxPx, const double& keyMaxPx, const double& valueMinPx, QVector<QPointF> &beforeTraverse, QVector<QPointF> &afterTraverse) const;

public:
  double pointDistance(const QPointF &pixelPoint, GPCurveDataContainer::const_iterator &closestData) const;

protected:

  friend class GraphicsPlot;
  friend class GPLegend;
};
Q_DECLARE_METATYPE(GPCurve::LineStyle)

class GP_LIB_DECL GPCurveSegmented : public GPCurve
{
  Q_OBJECT
public:
  explicit GPCurveSegmented(GPAxis *keyAxis, GPAxis *valueAxis, bool registerInPLot = true);
  virtual ~GPCurveSegmented();

  virtual void draw(GPPainter *painter) override;
};

/* end of 'src/plottables/plottable-curve.h' */

class GP_LIB_DECL GPPointAttachedTextElement : public GPAbstractPlottable1D<GPCurveData>
{
  Q_OBJECT
public:
  explicit GPPointAttachedTextElement(GPAxis *keyAxis, GPAxis *valueAxis, const QString& text, bool registerInPLot = true);
  virtual ~GPPointAttachedTextElement();

  void setData(double key, double value);
  QSharedPointer<GPCurveDataContainer> data() const { return mDataContainer; }

  // getters:
  QString text() const;
  QPen pen() const;
  QBrush brush() const;
  Qt::Alignment alignment() const;
  QMargins padding() const;

  // setters;
  void setText(const QString &text);
  void setPen(const QPen &pen);
  void setBrush(const QBrush &brush);
  void setFont(const QFont &font);
  void setAlignment(Qt::Alignment alignment);
  void setOrientation(Qt::Orientation orientation);
  void setPadding(const QMargins &padding);

  virtual void draw(GPPainter *) override;
  virtual GPRange getKeyRange(bool &, GP::SignDomain) const override;
  virtual GPRange getValueRange(bool &, GP::SignDomain, const GPRange &) const override;
  virtual GPDataRange getDataRange() const override;

  virtual void drawLegendIcon(GPPainter *, const QRectF &) const override;

protected:
  QString         mText;
  QPen            mPen;
  QBrush          mBrush;
  QFont           mFont;
  Qt::Alignment   mAlignment;
  Qt::Orientation mOrientation;
  QMargins        mPadding;
};

class GP_LIB_DECL GPBarsGroup : public QObject
{
  Q_OBJECT
  /// \cond INCLUDE_QPROPERTIES
  Q_PROPERTY(SpacingType spacingType READ spacingType WRITE setSpacingType)
  Q_PROPERTY(double spacing READ spacing WRITE setSpacing)
  /// \endcond
public:
  /*!
    Defines the ways the spacing between bars in the group can be specified. Thus it defines what
    the number passed to \ref setSpacing actually means.
    
    \see setSpacingType, setSpacing
  */
  enum SpacingType { stAbsolute       ///< Bar spacing is in absolute pixels
                     ,stAxisRectRatio ///< Bar spacing is given by a fraction of the axis rect size
                     ,stPlotCoords    ///< Bar spacing is in key coordinates and thus scales with the key axis range
                   };
  Q_ENUMS(SpacingType)
  
  GPBarsGroup(GraphicsPlot *parentPlot);
  virtual ~GPBarsGroup();
  
  // getters:
  SpacingType spacingType() const { return mSpacingType; }
  double spacing() const { return mSpacing; }
  
  // setters:
  void setSpacingType(SpacingType spacingType);
  void setSpacing(double spacing);
  
  // non-virtual methods:
  QList<GPBars*> bars() const { return mBars; }
  GPBars* bars(int index) const;
  int size() const { return mBars.size(); }
  bool isEmpty() const { return mBars.isEmpty(); }
  void clear();
  bool contains(GPBars *bars) const { return mBars.contains(bars); }
  void append(GPBars *bars);
  void insert(int i, GPBars *bars);
  void remove(GPBars *bars);
  
protected:
  // non-property members:
  GraphicsPlot *mParentPlot;
  SpacingType mSpacingType;
  double mSpacing;
  QList<GPBars*> mBars;
  
  // non-virtual methods:
  void registerBars(GPBars *bars);
  void unregisterBars(GPBars *bars);
  
  // virtual methods:
  double keyPixelOffset(const GPBars *bars, double keyCoord);
  double getPixelSpacing(const GPBars *bars, double keyCoord);
  
private:
  Q_DISABLE_COPY(GPBarsGroup)
  
  friend class GPBars;
};
Q_DECLARE_METATYPE(GPBarsGroup::SpacingType)


class GP_LIB_DECL GPBars : public GPAbstractPlottable1D<GPBarsData>
{
  Q_OBJECT
  /// \cond INCLUDE_QPROPERTIES
  Q_PROPERTY(double width READ width WRITE setWidth)
  Q_PROPERTY(WidthType widthType READ widthType WRITE setWidthType)
  Q_PROPERTY(GPBarsGroup* barsGroup READ barsGroup WRITE setBarsGroup)
  Q_PROPERTY(double baseValue READ baseValue WRITE setBaseValue)
  Q_PROPERTY(double stackingGap READ stackingGap WRITE setStackingGap)
  Q_PROPERTY(GPBars* barBelow READ barBelow)
  Q_PROPERTY(GPBars* barAbove READ barAbove)
  /// \endcond
public:
  /*!
    Defines the ways the width of the bar can be specified. Thus it defines what the number passed
    to \ref setWidth actually means.
    
    \see setWidthType, setWidth
  */
  enum WidthType { wtAbsolute       ///< Bar width is in absolute pixels
                   ,wtAxisRectRatio ///< Bar width is given by a fraction of the axis rect size
                   ,wtPlotCoords    ///< Bar width is in key coordinates and thus scales with the key axis range
                 };
  Q_ENUMS(WidthType)
  
  explicit GPBars(GPAxis *keyAxis, GPAxis *valueAxis);
  virtual ~GPBars();
  
  // getters:
  double width() const { return mWidth; }
  WidthType widthType() const { return mWidthType; }
  GPBarsGroup *barsGroup() const { return mBarsGroup; }
  double baseValue() const { return mBaseValue; }
  double stackingGap() const { return mStackingGap; }
  GPBars *barBelow() const { return mBarBelow.data(); }
  GPBars *barAbove() const { return mBarAbove.data(); }
  QSharedPointer<GPBarsDataContainer> data() const { return mDataContainer; }
  
  // setters:
  void setData(const QSharedPointer<GPBarsDataContainer>& data);
  void setData(const QVector<double> &keys, const QVector<double> &values, bool alreadySorted=false);
  void setWidth(double width);
  void setWidthType(WidthType widthType);
  void setBarsGroup(GPBarsGroup *barsGroup);
  void setBaseValue(double baseValue);
  void setStackingGap(double pixels);
  
  // non-property methods:
  void addData(const QVector<double> &keys, const QVector<double> &values, bool alreadySorted=false);
  void addData(double key, double value);
  void moveBelow(GPBars *bars);
  void moveAbove(GPBars *bars);
  
  // reimplemented virtual methods:
  virtual GPDataSelection selectTestRect(const QRectF &rect, bool onlySelectable, bool acceptAllValues = false) const override;
  virtual double selectTest(const QPointF &pos, bool onlySelectable, QVariant *details=0) const override;
  virtual GPRange getKeyRange(bool &foundRange, GP::SignDomain inSignDomain=GP::sdBoth) const override;
  virtual GPRange getValueRange(bool &foundRange, GP::SignDomain inSignDomain=GP::sdBoth, const GPRange &inKeyRange=GPRange()) const override;
  virtual GPDataRange getDataRange() const override;
  virtual QPointF dataPixelPosition(int index) const override;
  
protected:
  // property members:
  double mWidth;
  WidthType mWidthType;
  GPBarsGroup *mBarsGroup;
  double mBaseValue;
  double mStackingGap;
  QPointer<GPBars> mBarBelow, mBarAbove;
  
  // reimplemented virtual methods:
  virtual void draw(GPPainter *painter) override;
  virtual void drawLegendIcon(GPPainter *painter, const QRectF &rect) const override;
  
  // non-virtual methods:
  void getVisibleDataBounds(GPBarsDataContainer::const_iterator &begin, GPBarsDataContainer::const_iterator &end) const;
  QRectF getBarRect(double key, double value) const;
  void getPixelWidth(double key, double &lower, double &upper) const;
  double getStackedBaseValue(double key, bool positive) const;
  static void connectBars(GPBars* lower, GPBars* upper);
  
  friend class GraphicsPlot;
  friend class GPLegend;
  friend class GPBarsGroup;
};
Q_DECLARE_METATYPE(GPBars::WidthType)

/* end of 'src/plottables/plottable-bars.h' */

class GP_LIB_DECL GPStatisticalBox : public GPAbstractPlottable1D<GPStatisticalBoxData>
{
  Q_OBJECT
  /// \cond INCLUDE_QPROPERTIES
  Q_PROPERTY(double width READ width WRITE setWidth)
  Q_PROPERTY(double whiskerWidth READ whiskerWidth WRITE setWhiskerWidth)
  Q_PROPERTY(QPen whiskerPen READ whiskerPen WRITE setWhiskerPen)
  Q_PROPERTY(QPen whiskerBarPen READ whiskerBarPen WRITE setWhiskerBarPen)
  Q_PROPERTY(bool whiskerAntialiased READ whiskerAntialiased WRITE setWhiskerAntialiased)
  Q_PROPERTY(QPen medianPen READ medianPen WRITE setMedianPen)
  Q_PROPERTY(GPScatterStyle outlierStyle READ outlierStyle WRITE setOutlierStyle)
  /// \endcond
public:
  explicit GPStatisticalBox(GPAxis *keyAxis, GPAxis *valueAxis);
  
  // getters:
  QSharedPointer<GPStatisticalBoxDataContainer> data() const { return mDataContainer; }
  double width() const { return mWidth; }
  double whiskerWidth() const { return mWhiskerWidth; }
  QPen whiskerPen() const { return mWhiskerPen; }
  QPen whiskerBarPen() const { return mWhiskerBarPen; }
  bool whiskerAntialiased() const { return mWhiskerAntialiased; }
  QPen medianPen() const { return mMedianPen; }
  GPScatterStyle outlierStyle() const { return mOutlierStyle; }

  // setters:
  void setData(const QSharedPointer<GPStatisticalBoxDataContainer>& data);
  void setData(const QVector<double> &keys, const QVector<double> &minimum, const QVector<double> &lowerQuartile, const QVector<double> &median, const QVector<double> &upperQuartile, const QVector<double> &maximum, bool alreadySorted=false);
  void setWidth(double width);
  void setWhiskerWidth(double width);
  void setWhiskerPen(const QPen &pen);
  void setWhiskerBarPen(const QPen &pen);
  void setWhiskerAntialiased(bool enabled);
  void setMedianPen(const QPen &pen);
  void setOutlierStyle(const GPScatterStyle &style);
  
  // non-property methods:
  void addData(const QVector<double> &keys, const QVector<double> &minimum, const QVector<double> &lowerQuartile, const QVector<double> &median, const QVector<double> &upperQuartile, const QVector<double> &maximum, bool alreadySorted=false);
  void addData(double key, double minimum, double lowerQuartile, double median, double upperQuartile, double maximum, const QVector<double> &outliers=QVector<double>());
  
  // reimplemented virtual methods:
  virtual GPDataSelection selectTestRect(const QRectF &rect, bool onlySelectable, bool acceptAllValues = false) const override;
  virtual double selectTest(const QPointF &pos, bool onlySelectable, QVariant *details=0) const override;
  virtual GPRange getKeyRange(bool &foundRange, GP::SignDomain inSignDomain=GP::sdBoth) const override;
  virtual GPRange getValueRange(bool &foundRange, GP::SignDomain inSignDomain=GP::sdBoth, const GPRange &inKeyRange=GPRange()) const override;
  virtual GPDataRange getDataRange() const override;

protected:
  // property members:
  double mWidth;
  double mWhiskerWidth;
  QPen mWhiskerPen, mWhiskerBarPen;
  bool mWhiskerAntialiased;
  QPen mMedianPen;
  GPScatterStyle mOutlierStyle;
  
  // reimplemented virtual methods:
  virtual void draw(GPPainter *painter) override;
  virtual void drawLegendIcon(GPPainter *painter, const QRectF &rect) const override;
  
  // introduced virtual methods:
  virtual void drawStatisticalBox(GPPainter *painter, GPStatisticalBoxDataContainer::const_iterator it, const GPScatterStyle &outlierStyle) const;
  
  // non-virtual methods:
  void getVisibleDataBounds(GPStatisticalBoxDataContainer::const_iterator &begin, GPStatisticalBoxDataContainer::const_iterator &end) const;
  QRectF getQuartileBox(GPStatisticalBoxDataContainer::const_iterator it) const;
  QVector<QLineF> getWhiskerBackboneLines(GPStatisticalBoxDataContainer::const_iterator it) const;
  QVector<QLineF> getWhiskerBarLines(GPStatisticalBoxDataContainer::const_iterator it) const;
  
  friend class GraphicsPlot;
  friend class GPLegend;
};

class GP_LIB_DECL GPColorMapData : public GPUomDescriptorProviderDualListener
{
public:
  GPColorMapData(int keySize, int valueSize, const GPRange &keyRange, const GPRange &valueRange);
  ~GPColorMapData();
  GPColorMapData(const GPColorMapData &other);
  GPColorMapData &operator=(const GPColorMapData &other);
  
  // getters:
  int keySize() const { return mKeySize; }
  int valueSize() const { return mValueSize; }
  GPRange keyRange() const { return mKeyRange; }
  GPRange valueRange() const { return mValueRange; }
  GPRange dataBounds() const { return mDataBounds; }
  double data(double key, double value);
  double cell(int keyIndex, int valueIndex);
  unsigned char alpha(int keyIndex, int valueIndex);
  
  // setters:
  void setSize(int keySize, int valueSize);
  void setKeySize(int keySize);
  void setValueSize(int valueSize);
  void setRange(const GPRange &keyRange, const GPRange &valueRange);
  void setKeyRange(const GPRange &keyRange);
  void setValueRange(const GPRange &valueRange);
  void setData(double key, double value, double z);
  void setCell(int keyIndex, int valueIndex, double z);
  void setAlpha(int keyIndex, int valueIndex, unsigned char alpha);
  
  // non-property methods:
  void recalculateDataBounds();
  void clear();
  void reset();
  void clearAlpha();
  void fill(double z);
  void fillAlpha(unsigned char alpha);
  bool isEmpty() const { return mIsEmpty; }
  void coordToCell(double key, double value, int *keyIndex, int *valueIndex) const;
  void cellToCoord(int keyIndex, int valueIndex, double *key, double *value) const;

  void setInputUomForKeys(const GPUomDescriptor& descriptor)
  {
    mInputUomForKeys = descriptor;
  }

  void setInputUomForKeys(int quantity, int units)
  {
    GPUomDescriptor descriptor;
    descriptor.quantity = quantity;
    descriptor.units = units;
    setInputUomForKeys(descriptor);
  }

  void setInputUomForValues(const GPUomDescriptor& descriptor)
  {
    mInputUomForValues = descriptor;
  }

  void setInputUomForValues(int quantity, int units)
  {
    GPUomDescriptor descriptor;
    descriptor.quantity = quantity;
    descriptor.units = units;
    setInputUomForValues(descriptor);
  }

  void applyUomForKeys(const GPUomDescriptor& descriptor) override;
  void applyUomForValues(const GPUomDescriptor& descriptor) override;

  void applyUomForKeys(GPRange& range, const GPUomDescriptor& source, const GPUomDescriptor& target);
  void applyUomForValues(GPRange& range, const GPUomDescriptor& source, const GPUomDescriptor& target);
  
protected:
  // property members:
  int mKeySize, mValueSize;
  GPRange mKeyRange, mValueRange;
  bool mIsEmpty;
  
  // non-property members:
  double *mData;
  unsigned char *mAlpha;
  GPRange mDataBounds;
  bool mDataModified;
  
  bool createAlpha(bool initializeOpaque=true);

  GPUomDescriptor mInputUomForKeys;
  GPUomDescriptor mInputUomForValues;
  
  friend class GPColorMap;
  friend class GPEXColorMap;
};


class GP_LIB_DECL GPColorMap : public GPAbstractPlottable
{
  Q_OBJECT
  /// \cond INCLUDE_QPROPERTIES
  Q_PROPERTY(GPRange dataRange READ dataRange WRITE setDataRange NOTIFY dataRangeChanged)
  Q_PROPERTY(GPAxis::ScaleType dataScaleType READ dataScaleType WRITE setDataScaleType NOTIFY dataScaleTypeChanged)
  Q_PROPERTY(GPColorGradient gradient READ gradient WRITE setGradient NOTIFY gradientChanged)
  Q_PROPERTY(bool interpolate READ interpolate WRITE setInterpolate)
  Q_PROPERTY(bool tightBoundary READ tightBoundary WRITE setTightBoundary)
  Q_PROPERTY(GPColorScale* colorScale READ colorScale WRITE setColorScale)
  /// \endcond
public:
  explicit GPColorMap(GPAxis *keyAxis, GPAxis *valueAxis);
  virtual ~GPColorMap();
  
  // getters:
  GPColorMapData* data() const { return mMapData; }
  GPRange dataRange() const { return mDataRange; }
  GPAxis::ScaleType dataScaleType() const { return mDataScaleType; }
  bool interpolate() const { return mInterpolate; }
  bool tightBoundary() const { return mTightBoundary; }
  GPColorGradient gradient() const { return mGradient; }
  GPColorScale *colorScale() const { return mColorScale.data(); }
  
  // setters:
  void setData(GPColorMapData *data, bool copy=false);
  Q_SLOT void setDataRange(const GPRange &dataRange);
  Q_SLOT void setDataScaleType(GPAxis::ScaleType scaleType);
  Q_SLOT void setGradient(const GPColorGradient &gradient);
  void setInterpolate(bool enabled);
  void setTightBoundary(bool enabled);
  void setColorScale(GPColorScale *colorScale);
  
  // non-property methods:
  void rescaleDataRange(bool recalculateDataBounds=false);
  Q_SLOT void updateLegendIcon(Qt::TransformationMode transformMode=Qt::SmoothTransformation, const QSize &thumbSize=QSize(32, 18));
  
  // reimplemented virtual methods:
  virtual double selectTest(const QPointF &pos, bool onlySelectable, QVariant *details=0) const override;
  virtual GPRange getKeyRange(bool &foundRange, GP::SignDomain inSignDomain=GP::sdBoth) const override;
  virtual GPRange getValueRange(bool &foundRange, GP::SignDomain inSignDomain=GP::sdBoth, const GPRange &inKeyRange=GPRange()) const override;
  virtual GPDataRange getDataRange() const override;

signals:
  void dataRangeChanged(const GPRange &newRange);
  void dataScaleTypeChanged(GPAxis::ScaleType scaleType);
  void gradientChanged(const GPColorGradient &newGradient);
  
protected:
  // property members:
  GPRange mDataRange;
  GPAxis::ScaleType mDataScaleType;
  GPColorMapData *mMapData;
  GPColorGradient mGradient;
  bool mInterpolate;
  bool mTightBoundary;
  QPointer<GPColorScale> mColorScale;
  
  // non-property members:
  QImage mMapImage, mUndersampledMapImage;
  QPixmap mLegendIcon;
  bool mMapImageInvalidated;
  
  // introduced virtual methods:
  virtual void updateMapImage();
  
  // reimplemented virtual methods:
  virtual void draw(GPPainter *painter) override;
  virtual void drawLegendIcon(GPPainter *painter, const QRectF &rect) const override;
  
  friend class GraphicsPlot;
  friend class GPLegend;
};

class GP_LIB_DECL GPFinancial : public GPAbstractPlottable1D<GPFinancialData>
{
  Q_OBJECT
  /// \cond INCLUDE_QPROPERTIES
  Q_PROPERTY(ChartStyle chartStyle READ chartStyle WRITE setChartStyle)
  Q_PROPERTY(double width READ width WRITE setWidth)
  Q_PROPERTY(WidthType widthType READ widthType WRITE setWidthType)
  Q_PROPERTY(bool twoColored READ twoColored WRITE setTwoColored)
  Q_PROPERTY(QBrush brushPositive READ brushPositive WRITE setBrushPositive)
  Q_PROPERTY(QBrush brushNegative READ brushNegative WRITE setBrushNegative)
  Q_PROPERTY(QPen penPositive READ penPositive WRITE setPenPositive)
  Q_PROPERTY(QPen penNegative READ penNegative WRITE setPenNegative)
  /// \endcond
public:
  /*!
    Defines the ways the width of the financial bar can be specified. Thus it defines what the
    number passed to \ref setWidth actually means.

    \see setWidthType, setWidth
  */
  enum WidthType { wtAbsolute       ///< width is in absolute pixels
                   ,wtAxisRectRatio ///< width is given by a fraction of the axis rect size
                   ,wtPlotCoords    ///< width is in key coordinates and thus scales with the key axis range
                 };
  Q_ENUMS(WidthType)
  
  /*!
    Defines the possible representations of OHLC data in the plot.
    
    \see setChartStyle
  */
  enum ChartStyle { csOhlc         ///< Open-High-Low-Close bar representation
                   ,csCandlestick  ///< Candlestick representation
                  };
  Q_ENUMS(ChartStyle)
  
  explicit GPFinancial(GPAxis *keyAxis, GPAxis *valueAxis);
  virtual ~GPFinancial();
  
  // getters:
  QSharedPointer<GPFinancialDataContainer> data() const { return mDataContainer; }
  ChartStyle chartStyle() const { return mChartStyle; }
  double width() const { return mWidth; }
  WidthType widthType() const { return mWidthType; }
  bool twoColored() const { return mTwoColored; }
  QBrush brushPositive() const { return mBrushPositive; }
  QBrush brushNegative() const { return mBrushNegative; }
  QPen penPositive() const { return mPenPositive; }
  QPen penNegative() const { return mPenNegative; }
  
  // setters:
  void setData(const QSharedPointer<GPFinancialDataContainer>& data);
  void setData(const QVector<double> &keys, const QVector<double> &open, const QVector<double> &high, const QVector<double> &low, const QVector<double> &close, bool alreadySorted=false);
  void setChartStyle(ChartStyle style);
  void setWidth(double width);
  void setWidthType(WidthType widthType);
  void setTwoColored(bool twoColored);
  void setBrushPositive(const QBrush &brush);
  void setBrushNegative(const QBrush &brush);
  void setPenPositive(const QPen &pen);
  void setPenNegative(const QPen &pen);
  
  // non-property methods:
  void addData(const QVector<double> &keys, const QVector<double> &open, const QVector<double> &high, const QVector<double> &low, const QVector<double> &close, bool alreadySorted=false);
  void addData(double key, double open, double high, double low, double close);
  
  // reimplemented virtual methods:
  virtual GPDataSelection selectTestRect(const QRectF &rect, bool onlySelectable, bool acceptAllValues = false) const override;
  virtual double selectTest(const QPointF &pos, bool onlySelectable, QVariant *details=0) const override;
  virtual GPRange getKeyRange(bool &foundRange, GP::SignDomain inSignDomain=GP::sdBoth) const override;
  virtual GPRange getValueRange(bool &foundRange, GP::SignDomain inSignDomain=GP::sdBoth, const GPRange &inKeyRange=GPRange()) const override;
  virtual GPDataRange getDataRange() const override;

  // static methods:
  static GPFinancialDataContainer timeSeriesToOhlc(const QVector<double> &time, const QVector<double> &value, double timeBinSize, double timeBinOffset = 0);
  
protected:
  // property members:
  ChartStyle mChartStyle;
  double mWidth;
  WidthType mWidthType;
  bool mTwoColored;
  QBrush mBrushPositive, mBrushNegative;
  QPen mPenPositive, mPenNegative;
  
  // reimplemented virtual methods:
  virtual void draw(GPPainter *painter) override;
  virtual void drawLegendIcon(GPPainter *painter, const QRectF &rect) const override;
  
  // non-virtual methods:
  void drawOhlcPlot(GPPainter *painter, const GPFinancialDataContainer::const_iterator &begin, const GPFinancialDataContainer::const_iterator &end, bool isSelected);
  void drawCandlestickPlot(GPPainter *painter, const GPFinancialDataContainer::const_iterator &begin, const GPFinancialDataContainer::const_iterator &end, bool isSelected);
  double getPixelWidth(double key, double keyPixel) const;
  double ohlcSelectTest(const QPointF &pos, const GPFinancialDataContainer::const_iterator &begin, const GPFinancialDataContainer::const_iterator &end, GPFinancialDataContainer::const_iterator &closestDataPoint) const;
  double candlestickSelectTest(const QPointF &pos, const GPFinancialDataContainer::const_iterator &begin, const GPFinancialDataContainer::const_iterator &end, GPFinancialDataContainer::const_iterator &closestDataPoint) const;
  void getVisibleDataBounds(GPFinancialDataContainer::const_iterator &begin, GPFinancialDataContainer::const_iterator &end) const;
  QRectF selectionHitBox(GPFinancialDataContainer::const_iterator it) const;
  
  friend class GraphicsPlot;
  friend class GPLegend;
};
Q_DECLARE_METATYPE(GPFinancial::ChartStyle)

class GP_LIB_DECL GPErrorBars : public GPAbstractPlottable, public GPPlottableInterface1D
{
  Q_OBJECT
  /// \cond INCLUDE_QPROPERTIES
  Q_PROPERTY(QSharedPointer<GPErrorBarsDataContainer> data READ data WRITE setData)
  Q_PROPERTY(GPAbstractPlottable* dataPlottable READ dataPlottable WRITE setDataPlottable)
  Q_PROPERTY(ErrorType errorType READ errorType WRITE setErrorType)
  Q_PROPERTY(double whiskerWidth READ whiskerWidth WRITE setWhiskerWidth)
  Q_PROPERTY(double symbolGap READ symbolGap WRITE setSymbolGap)
  /// \endcond
public:
  
  /*!
    Defines in which orientation the error bars shall appear. If your data needs both error
    dimensions, create two \ref GPErrorBars with different \ref ErrorType.

    \see setErrorType
  */
  enum ErrorType { etKeyError    ///< The errors are for the key dimension (bars appear parallel to the key axis)
                   ,etValueError ///< The errors are for the value dimension (bars appear parallel to the value axis)
  };
  Q_ENUMS(ErrorType)
  
  explicit GPErrorBars(GPAxis *keyAxis, GPAxis *valueAxis);
  virtual ~GPErrorBars();
  // getters:
  QSharedPointer<GPErrorBarsDataContainer> data() const { return mDataContainer; }
  GPAbstractPlottable *dataPlottable() const { return mDataPlottable.data(); }
  ErrorType errorType() const { return mErrorType; }
  double whiskerWidth() const { return mWhiskerWidth; }
  double symbolGap() const { return mSymbolGap; }
  
  // setters:
  void setData(const QSharedPointer<GPErrorBarsDataContainer>& data);
  void setData(const QVector<double> &error);
  void setData(const QVector<double> &errorMinus, const QVector<double> &errorPlus);
  void setDataPlottable(GPAbstractPlottable* plottable);
  void setErrorType(ErrorType type);
  void setWhiskerWidth(double pixels);
  void setSymbolGap(double pixels);
  
  // non-property methods:
  void addData(const QVector<double> &error);
  void addData(const QVector<double> &errorMinus, const QVector<double> &errorPlus);
  void addData(double error);
  void addData(double errorMinus, double errorPlus);
  
  // virtual methods of 1d plottable interface:
  virtual int dataCount() const override;
  virtual double dataMainKey(int index) const override;
  virtual double dataSortKey(int index) const override;
  virtual double dataMainValue(int index) const override;
  virtual GPRange dataValueRange(int index) const override;
  virtual QPointF dataPixelPosition(int index) const override;
  virtual bool sortKeyIsMainKey() const override;
  virtual GPDataSelection selectTestRect(const QRectF &rect, bool onlySelectable, bool acceptAllValues = false) const override;
  virtual int findBegin(double sortKey, bool expandedRange=true) const override;
  virtual int findEnd(double sortKey, bool expandedRange=true) const override;
  
  // reimplemented virtual methods:
  virtual double selectTest(const QPointF &pos, bool onlySelectable, QVariant *details=0) const override;
  virtual GPPlottableInterface1D *interface1D() override { return this; }
  
protected:
  // property members:
  QSharedPointer<GPErrorBarsDataContainer> mDataContainer;
  QPointer<GPAbstractPlottable> mDataPlottable;
  ErrorType mErrorType;
  double mWhiskerWidth;
  double mSymbolGap;
  
  // reimplemented virtual methods:
  virtual void draw(GPPainter *painter) override;
  virtual void drawLegendIcon(GPPainter *painter, const QRectF &rect) const override;
  virtual GPRange getKeyRange(bool &foundRange, GP::SignDomain inSignDomain=GP::sdBoth) const override;
  virtual GPRange getValueRange(bool &foundRange, GP::SignDomain inSignDomain=GP::sdBoth, const GPRange &inKeyRange=GPRange()) const override;
  virtual GPDataRange getDataRange() const override;

  // non-virtual methods:
  void getErrorBarLines(GPErrorBarsDataContainer::const_iterator it, QVector<QLineF> &backbones, QVector<QLineF> &whiskers) const;
  void getVisibleDataBounds(GPErrorBarsDataContainer::const_iterator &begin, GPErrorBarsDataContainer::const_iterator &end, const GPDataRange &rangeRestriction) const;
  double pointDistance(const QPointF &pixelPoint, GPErrorBarsDataContainer::const_iterator &closestData) const;
  // helpers:
  void getDataSegments(QList<GPDataRange> &selectedSegments, QList<GPDataRange> &unselectedSegments) const;
  bool errorBarVisible(int index) const;
  bool rectIntersectsLine(const QRectF &pixelRect, const QLineF &line) const;
  
  friend class GraphicsPlot;
  friend class GPLegend;
};

class GP_LIB_DECL GPItemStraightLine : public GPAbstractItem
{
  Q_OBJECT
  /// \cond INCLUDE_QPROPERTIES
  Q_PROPERTY(QPen pen READ pen WRITE setPen)
  Q_PROPERTY(QPen selectedPen READ selectedPen WRITE setSelectedPen)
  /// \endcond
public:
  explicit GPItemStraightLine(GraphicsPlot *parentPlot);
  virtual ~GPItemStraightLine();
  
  // getters:
  QPen pen() const { return mPen; }
  QPen selectedPen() const { return mSelectedPen; }
  
  // setters;
  void setPen(const QPen &pen);
  void setSelectedPen(const QPen &pen);
  
  // reimplemented virtual methods:
  virtual double selectTest(const QPointF &pos, bool onlySelectable, QVariant *details=0) const override;
  
  GPItemPosition * const point1;
  GPItemPosition * const point2;
  
  static QLineF getRectClippedStraightLine(const GPVector2D &point1, const GPVector2D &vec, const QRect &rect);

protected:
  // property members:
  QPen mPen, mSelectedPen;
  
  // reimplemented virtual methods:
  virtual void draw(GPPainter *painter) override;
  
  // non-virtual methods:
  QPen mainPen() const;
};

class GP_LIB_DECL GPItemLine : public GPAbstractItem
{
  Q_OBJECT
  /// \cond INCLUDE_QPROPERTIES
  Q_PROPERTY(QPen pen READ pen WRITE setPen)
  Q_PROPERTY(QPen selectedPen READ selectedPen WRITE setSelectedPen)
  Q_PROPERTY(GPLineEnding head READ head WRITE setHead)
  Q_PROPERTY(GPLineEnding tail READ tail WRITE setTail)
  /// \endcond
public:
  explicit GPItemLine(GraphicsPlot *parentPlot);
  virtual ~GPItemLine();
  
  // getters:
  QPen pen() const { return mPen; }
  QPen selectedPen() const { return mSelectedPen; }
  GPLineEnding head() const { return mHead; }
  GPLineEnding tail() const { return mTail; }
  
  // setters;
  void setPen(const QPen &pen);
  void setSelectedPen(const QPen &pen);
  void setHead(const GPLineEnding &head);
  void setTail(const GPLineEnding &tail);
  
  // reimplemented virtual methods:
  virtual double selectTest(const QPointF &pos, bool onlySelectable, QVariant *details=0) const override;
  
  GPItemPosition * const start;
  GPItemPosition * const end;
  
protected:
  // property members:
  QPen mPen, mSelectedPen;
  GPLineEnding mHead, mTail;
  
  // reimplemented virtual methods:
  virtual void draw(GPPainter *painter) override;
  virtual QLineF getRectClippedLine(const GPVector2D &start, const GPVector2D &end, const QRect &rect) const;
  
  // non-virtual methods:
  QPen mainPen() const;
};

class GP_LIB_DECL GPItemCurve : public GPAbstractItem
{
  Q_OBJECT
  /// \cond INCLUDE_QPROPERTIES
  Q_PROPERTY(QPen pen READ pen WRITE setPen)
  Q_PROPERTY(QPen selectedPen READ selectedPen WRITE setSelectedPen)
  Q_PROPERTY(GPLineEnding head READ head WRITE setHead)
  Q_PROPERTY(GPLineEnding tail READ tail WRITE setTail)
  /// \endcond
public:
  explicit GPItemCurve(GraphicsPlot *parentPlot);
  virtual ~GPItemCurve();
  
  // getters:
  QPen pen() const { return mPen; }
  QPen selectedPen() const { return mSelectedPen; }
  GPLineEnding head() const { return mHead; }
  GPLineEnding tail() const { return mTail; }
  
  // setters;
  void setPen(const QPen &pen);
  void setSelectedPen(const QPen &pen);
  void setHead(const GPLineEnding &head);
  void setTail(const GPLineEnding &tail);
  
  // reimplemented virtual methods:
  virtual double selectTest(const QPointF &pos, bool onlySelectable, QVariant *details=0) const override;
  
  GPItemPosition * const start;
  GPItemPosition * const startDir;
  GPItemPosition * const endDir;
  GPItemPosition * const end;
  
protected:
  // property members:
  QPen mPen, mSelectedPen;
  GPLineEnding mHead, mTail;
  
  // reimplemented virtual methods:
  virtual void draw(GPPainter *painter) override;
  
  // non-virtual methods:
  QPen mainPen() const;
};

class GP_LIB_DECL GPItemRect : public GPAbstractItem
{
  Q_OBJECT
  /// \cond INCLUDE_QPROPERTIES
  Q_PROPERTY(QPen pen READ pen WRITE setPen)
  Q_PROPERTY(QPen selectedPen READ selectedPen WRITE setSelectedPen)
  Q_PROPERTY(QBrush brush READ brush WRITE setBrush)
  Q_PROPERTY(QBrush selectedBrush READ selectedBrush WRITE setSelectedBrush)
  /// \endcond
public:
  explicit GPItemRect(GraphicsPlot *parentPlot);
  virtual ~GPItemRect();
  
  // getters:
  QPen pen() const { return mPen; }
  QPen selectedPen() const { return mSelectedPen; }
  QBrush brush() const { return mBrush; }
  QBrush selectedBrush() const { return mSelectedBrush; }
  
  // setters;
  void setPen(const QPen &pen);
  void setSelectedPen(const QPen &pen);
  void setBrush(const QBrush &brush);
  void setSelectedBrush(const QBrush &brush);
  
  // reimplemented virtual methods:
  virtual double selectTest(const QPointF &pos, bool onlySelectable, QVariant *details=0) const override;
  
  GPItemPosition * const topLeft;
  GPItemPosition * const bottomRight;
  GPItemAnchor * const top;
  GPItemAnchor * const topRight;
  GPItemAnchor * const right;
  GPItemAnchor * const bottom;
  GPItemAnchor * const bottomLeft;
  GPItemAnchor * const left;
  
protected:
  enum AnchorIndex {aiTop, aiTopRight, aiRight, aiBottom, aiBottomLeft, aiLeft};
  
  // property members:
  QPen mPen, mSelectedPen;
  QBrush mBrush, mSelectedBrush;
  
  // reimplemented virtual methods:
  virtual void draw(GPPainter *painter) override;
  virtual QPointF anchorPixelPosition(int anchorId) const override;
  
  // non-virtual methods:
  QPen mainPen() const;
  QBrush mainBrush() const;
};

class GP_LIB_DECL GPItemText : public GPAbstractItem
{
  Q_OBJECT
  /// \cond INCLUDE_QPROPERTIES
  Q_PROPERTY(QColor color READ color WRITE setColor)
  Q_PROPERTY(QColor selectedColor READ selectedColor WRITE setSelectedColor)
  Q_PROPERTY(QPen pen READ pen WRITE setPen)
  Q_PROPERTY(QPen selectedPen READ selectedPen WRITE setSelectedPen)
  Q_PROPERTY(QBrush brush READ brush WRITE setBrush)
  Q_PROPERTY(QBrush selectedBrush READ selectedBrush WRITE setSelectedBrush)
  Q_PROPERTY(QFont font READ font WRITE setFont)
  Q_PROPERTY(QFont selectedFont READ selectedFont WRITE setSelectedFont)
  Q_PROPERTY(QString text READ text WRITE setText)
  Q_PROPERTY(Qt::Alignment positionAlignment READ positionAlignment WRITE setPositionAlignment)
  Q_PROPERTY(Qt::Alignment textAlignment READ textAlignment WRITE setTextAlignment)
  Q_PROPERTY(double rotation READ rotation WRITE setRotation)
  Q_PROPERTY(QMargins padding READ padding WRITE setPadding)
  /// \endcond
public:
  explicit GPItemText(GraphicsPlot *parentPlot);
  virtual ~GPItemText();
  
  // getters:
  QColor color() const { return mColor; }
  QColor selectedColor() const { return mSelectedColor; }
  QPen pen() const { return mPen; }
  QPen selectedPen() const { return mSelectedPen; }
  QBrush brush() const { return mBrush; }
  QBrush selectedBrush() const { return mSelectedBrush; }
  QFont font() const { return mFont; }
  QFont selectedFont() const { return mSelectedFont; }
  QString text() const { return mText; }
  Qt::Alignment positionAlignment() const { return mPositionAlignment; }
  Qt::Alignment textAlignment() const { return mTextAlignment; }
  double rotation() const { return mRotation; }
  QMargins padding() const { return mPadding; }
  
  // setters;
  void setColor(const QColor &color);
  void setSelectedColor(const QColor &color);
  void setPen(const QPen &pen);
  void setSelectedPen(const QPen &pen);
  void setBrush(const QBrush &brush);
  void setSelectedBrush(const QBrush &brush);
  void setFont(const QFont &font);
  void setSelectedFont(const QFont &font);
  void setText(const QString &text);
  void setPositionAlignment(Qt::Alignment alignment);
  void setTextAlignment(Qt::Alignment alignment);
  void setRotation(double degrees);
  void setPadding(const QMargins &padding);
  
  // reimplemented virtual methods:
  virtual double selectTest(const QPointF &pos, bool onlySelectable, QVariant *details=0) const override;
  
  GPItemPosition * const position;
  GPItemAnchor * const topLeft;
  GPItemAnchor * const top;
  GPItemAnchor * const topRight;
  GPItemAnchor * const right;
  GPItemAnchor * const bottomRight;
  GPItemAnchor * const bottom;
  GPItemAnchor * const bottomLeft;
  GPItemAnchor * const left;
  
protected:
  enum AnchorIndex {aiTopLeft, aiTop, aiTopRight, aiRight, aiBottomRight, aiBottom, aiBottomLeft, aiLeft};
  
  // property members:
  QColor mColor, mSelectedColor;
  QPen mPen, mSelectedPen;
  QBrush mBrush, mSelectedBrush;
  QFont mFont, mSelectedFont;
  QString mText;
  Qt::Alignment mPositionAlignment;
  Qt::Alignment mTextAlignment;
  double mRotation;
  QMargins mPadding;
  
  // reimplemented virtual methods:
  virtual void draw(GPPainter *painter) override;
  virtual QPointF anchorPixelPosition(int anchorId) const override;
  
  // non-virtual methods:
  QPointF getTextDrawPoint(const QPointF &pos, const QRectF &rect, Qt::Alignment positionAlignment) const;
  QFont mainFont() const;
  QColor mainColor() const;
  QPen mainPen() const;
  QBrush mainBrush() const;
};

class GP_LIB_DECL GPItemEllipse : public GPAbstractItem
{
  Q_OBJECT
  /// \cond INCLUDE_QPROPERTIES
  Q_PROPERTY(QPen pen READ pen WRITE setPen)
  Q_PROPERTY(QPen selectedPen READ selectedPen WRITE setSelectedPen)
  Q_PROPERTY(QBrush brush READ brush WRITE setBrush)
  Q_PROPERTY(QBrush selectedBrush READ selectedBrush WRITE setSelectedBrush)
  /// \endcond
public:
  explicit GPItemEllipse(GraphicsPlot *parentPlot);
  virtual ~GPItemEllipse();
  
  // getters:
  QPen pen() const { return mPen; }
  QPen selectedPen() const { return mSelectedPen; }
  QBrush brush() const { return mBrush; }
  QBrush selectedBrush() const { return mSelectedBrush; }
  
  // setters;
  void setPen(const QPen &pen);
  void setSelectedPen(const QPen &pen);
  void setBrush(const QBrush &brush);
  void setSelectedBrush(const QBrush &brush);
  
  // reimplemented virtual methods:
  virtual double selectTest(const QPointF &pos, bool onlySelectable, QVariant *details=0) const override;
  
  GPItemPosition * const topLeft;
  GPItemPosition * const bottomRight;
  GPItemAnchor * const topLeftRim;
  GPItemAnchor * const top;
  GPItemAnchor * const topRightRim;
  GPItemAnchor * const right;
  GPItemAnchor * const bottomRightRim;
  GPItemAnchor * const bottom;
  GPItemAnchor * const bottomLeftRim;
  GPItemAnchor * const left;
  GPItemAnchor * const center;
  
protected:
  enum AnchorIndex {aiTopLeftRim, aiTop, aiTopRightRim, aiRight, aiBottomRightRim, aiBottom, aiBottomLeftRim, aiLeft, aiCenter};
  
  // property members:
  QPen mPen, mSelectedPen;
  QBrush mBrush, mSelectedBrush;
  
  // reimplemented virtual methods:
  virtual void draw(GPPainter *painter) override;
  virtual QPointF anchorPixelPosition(int anchorId) const override;
  
  // non-virtual methods:
  QPen mainPen() const;
  QBrush mainBrush() const;
};

class GP_LIB_DECL GPItemPixmap : public GPAbstractItem
{
  Q_OBJECT
  /// \cond INCLUDE_QPROPERTIES
  Q_PROPERTY(QPixmap pixmap READ pixmap WRITE setPixmap)
  Q_PROPERTY(bool scaled READ scaled WRITE setScaled)
  Q_PROPERTY(Qt::AspectRatioMode aspectRatioMode READ aspectRatioMode)
  Q_PROPERTY(Qt::TransformationMode transformationMode READ transformationMode)
  Q_PROPERTY(QPen pen READ pen WRITE setPen)
  Q_PROPERTY(QPen selectedPen READ selectedPen WRITE setSelectedPen)
  /// \endcond
public:
  explicit GPItemPixmap(GraphicsPlot *parentPlot);
  virtual ~GPItemPixmap();
  
  // getters:
  QPixmap pixmap() const { return mPixmap; }
  bool scaled() const { return mScaled; }
  Qt::AspectRatioMode aspectRatioMode() const { return mAspectRatioMode; }
  Qt::TransformationMode transformationMode() const { return mTransformationMode; }
  QPen pen() const { return mPen; }
  QPen selectedPen() const { return mSelectedPen; }
  
  // setters;
  void setPixmap(const QPixmap &pixmap);
  void setScaled(bool scaled, Qt::AspectRatioMode aspectRatioMode=Qt::KeepAspectRatio, Qt::TransformationMode transformationMode=Qt::SmoothTransformation);
  void setPen(const QPen &pen);
  void setSelectedPen(const QPen &pen);
  
  // reimplemented virtual methods:
  virtual double selectTest(const QPointF &pos, bool onlySelectable, QVariant *details=0) const override;
  
  GPItemPosition * const topLeft;
  GPItemPosition * const bottomRight;
  GPItemAnchor * const top;
  GPItemAnchor * const topRight;
  GPItemAnchor * const right;
  GPItemAnchor * const bottom;
  GPItemAnchor * const bottomLeft;
  GPItemAnchor * const left;
  
protected:
  enum AnchorIndex {aiTop, aiTopRight, aiRight, aiBottom, aiBottomLeft, aiLeft};
  
  // property members:
  QPixmap mPixmap;
  QPixmap mScaledPixmap;
  bool mScaled;
  bool mScaledPixmapInvalidated;
  Qt::AspectRatioMode mAspectRatioMode;
  Qt::TransformationMode mTransformationMode;
  QPen mPen, mSelectedPen;
  
  // reimplemented virtual methods:
  virtual void draw(GPPainter *painter) override;
  virtual QPointF anchorPixelPosition(int anchorId) const override;
  
  // non-virtual methods:
  void updateScaledPixmap(QRect finalRect=QRect(), bool flipHorz=false, bool flipVert=false);
  QRect getFinalRect(bool *flippedHorz=0, bool *flippedVert=0) const;
  QPen mainPen() const;
};

class GP_LIB_DECL GPItemTracer : public GPAbstractItem
{
  Q_OBJECT
  /// \cond INCLUDE_QPROPERTIES
  Q_PROPERTY(QPen pen READ pen WRITE setPen)
  Q_PROPERTY(QPen selectedPen READ selectedPen WRITE setSelectedPen)
  Q_PROPERTY(QBrush brush READ brush WRITE setBrush)
  Q_PROPERTY(QBrush selectedBrush READ selectedBrush WRITE setSelectedBrush)
  Q_PROPERTY(double size READ size WRITE setSize)
  Q_PROPERTY(TracerStyle style READ style WRITE setStyle)
  Q_PROPERTY(GPGraph* graph READ graph WRITE setGraph)
  Q_PROPERTY(double graphKey READ graphKey WRITE setGraphKey)
  Q_PROPERTY(bool interpolating READ interpolating WRITE setInterpolating)
  /// \endcond
public:
  /*!
    The different visual appearances a tracer item can have. Some styles size may be controlled with \ref setSize.
    
    \see setStyle
  */
  enum TracerStyle { tsNone        ///< The tracer is not visible
                     ,tsPlus       ///< A plus shaped crosshair with limited size
                     ,tsCrosshair  ///< A plus shaped crosshair which spans the complete axis rect
                     ,tsCircle     ///< A circle
                     ,tsSquare     ///< A square
                   };
  Q_ENUMS(TracerStyle)

  explicit GPItemTracer(GraphicsPlot *parentPlot);
  virtual ~GPItemTracer();

  // getters:
  QPen pen() const { return mPen; }
  QPen selectedPen() const { return mSelectedPen; }
  QBrush brush() const { return mBrush; }
  QBrush selectedBrush() const { return mSelectedBrush; }
  double size() const { return mSize; }
  TracerStyle style() const { return mStyle; }
  GPGraph *graph() const { return mGraph; }
  double graphKey() const { return mGraphKey; }
  bool interpolating() const { return mInterpolating; }

  // setters;
  void setPen(const QPen &pen);
  void setSelectedPen(const QPen &pen);
  void setBrush(const QBrush &brush);
  void setSelectedBrush(const QBrush &brush);
  void setSize(double size);
  void setStyle(TracerStyle style);
  void setGraph(GPGraph *graph);
  void setGraphKey(double key);
  void setInterpolating(bool enabled);

  // reimplemented virtual methods:
  virtual double selectTest(const QPointF &pos, bool onlySelectable, QVariant *details=0) const override;
  
  // non-virtual methods:
  void updatePosition();

  GPItemPosition * const position;

protected:
  // property members:
  QPen mPen, mSelectedPen;
  QBrush mBrush, mSelectedBrush;
  double mSize;
  TracerStyle mStyle;
  GPGraph *mGraph;
  double mGraphKey;
  bool mInterpolating;

  // reimplemented virtual methods:
  virtual void draw(GPPainter *painter) override;

  // non-virtual methods:
  QPen mainPen() const;
  QBrush mainBrush() const;
};
Q_DECLARE_METATYPE(GPItemTracer::TracerStyle)

class GP_LIB_DECL GPItemBracket : public GPAbstractItem
{
  Q_OBJECT
  /// \cond INCLUDE_QPROPERTIES
  Q_PROPERTY(QPen pen READ pen WRITE setPen)
  Q_PROPERTY(QPen selectedPen READ selectedPen WRITE setSelectedPen)
  Q_PROPERTY(double length READ length WRITE setLength)
  Q_PROPERTY(BracketStyle style READ style WRITE setStyle)
  /// \endcond
public:
  /*!
    Defines the various visual shapes of the bracket item. The appearance can be further modified
    by \ref setLength and \ref setPen.
    
    \see setStyle
  */
  enum BracketStyle { bsSquare  ///< A brace with angled edges
                      ,bsRound  ///< A brace with round edges
                      ,bsCurly  ///< A curly brace
                      ,bsCalligraphic ///< A curly brace with varying stroke width giving a calligraphic impression
  };
  Q_ENUMS(BracketStyle)

  explicit GPItemBracket(GraphicsPlot *parentPlot);
  virtual ~GPItemBracket();
  
  // getters:
  QPen pen() const { return mPen; }
  QPen selectedPen() const { return mSelectedPen; }
  double length() const { return mLength; }
  BracketStyle style() const { return mStyle; }
  
  // setters;
  void setPen(const QPen &pen);
  void setSelectedPen(const QPen &pen);
  void setLength(double length);
  void setStyle(BracketStyle style);
  
  // reimplemented virtual methods:
  virtual double selectTest(const QPointF &pos, bool onlySelectable, QVariant *details=0) const override;
  
  GPItemPosition * const left;
  GPItemPosition * const right;
  GPItemAnchor * const center;
  
protected:
  // property members:
  enum AnchorIndex {aiCenter};
  QPen mPen, mSelectedPen;
  double mLength;
  BracketStyle mStyle;
  
  // reimplemented virtual methods:
  virtual void draw(GPPainter *painter) override;
  virtual QPointF anchorPixelPosition(int anchorId) const override;
  
  // non-virtual methods:
  QPen mainPen() const;
};
Q_DECLARE_METATYPE(GPItemBracket::BracketStyle)

/* end of 'src/items/item-bracket.h' */

#endif // GRAPHICSPLOT_H

