#pragma once

#include "graphicsplot_global.h"

#include "standard_models/uom_settings.h"

#include <QtCore/QDebug>

#include <set>

#ifndef GRAPHICSPLOT_DATA_STRUCTURES_INCLUDED
#define GRAPHICSPLOT_DATA_STRUCTURES_INCLUDED
#endif // ~!GRAPHICSPLOT_DATA_STRUCTURES_INCLUDED

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// GPRange
class GP_LIB_DECL GPRange
{
public:
  double lower, upper;

  GPRange();
  GPRange(double lower, double upper);

  bool operator==(const GPRange& other) const { return fabs(lower - other.lower) < std::numeric_limits<double>::epsilon()
        && fabs(upper - other.upper) < std::numeric_limits<double>::epsilon(); }
  bool operator!=(const GPRange& other) const { return !(*this == other); }
  bool operator <(const GPRange& other) const { return (lower < other.lower) || ((lower == other.lower) && (upper < other.upper)); }

  GPRange &operator+=(const double& value) { lower+=value; upper+=value; return *this; }
  GPRange &operator-=(const double& value) { lower-=value; upper-=value; return *this; }
  GPRange &operator*=(const double& value) { lower*=value; upper*=value; return *this; }
  GPRange &operator/=(const double& value) { lower/=value; upper/=value; return *this; }
  friend inline const GPRange operator+(const GPRange&, double);
  friend inline const GPRange operator+(double, const GPRange&);
  friend inline const GPRange operator-(const GPRange& range, double value);
  friend inline const GPRange operator*(const GPRange& range, double value);
  friend inline const GPRange operator*(double value, const GPRange& range);
  friend inline const GPRange operator/(const GPRange& range, double value);

  double size() const { return upper-lower; }
  double center() const { return (upper+lower)*0.5; }
  void normalize() { if (lower > upper) qSwap(lower, upper); }
  void expand(const GPRange &otherRange);
  void expand(double includeCoord);
  GPRange expanded(const GPRange &otherRange) const;
  GPRange expanded(double includeCoord) const;
  GPRange bounded(double lowerBound, double upperBound) const;
  GPRange boundedSize(double minimum, double maximum) const;
  GPRange sanitizedForLogScale() const;
  GPRange sanitizedForLinScale() const;
  bool contains(double value) const { return value >= lower && value <= upper; }

  static bool validRange(double lower, double upper);
  static bool validRange(const GPRange &range);
  static const double minRange;
  static const double maxRange;

};
Q_DECLARE_TYPEINFO(GPRange, Q_MOVABLE_TYPE);

/*! \relates GPRange

  Prints \a range in a human readable format to the qDebug output.
*/
inline QDebug operator<< (QDebug d, const GPRange &range)
{
    d.nospace() << "GPRange(" << range.lower << ", " << range.upper << ")";
    return d.space();
}

/*!
  Adds \a value to both boundaries of the range.
*/
inline const GPRange operator+(const GPRange& range, double value)
{
  GPRange result(range);
  result += value;
  return result;
}

/*!
  Adds \a value to both boundaries of the range.
*/
inline const GPRange operator+(double value, const GPRange& range)
{
  GPRange result(range);
  result += value;
  return result;
}

/*!
  Subtracts \a value from both boundaries of the range.
*/
inline const GPRange operator-(const GPRange& range, double value)
{
  GPRange result(range);
  result -= value;
  return result;
}

/*!
  Multiplies both boundaries of the range by \a value.
*/
inline const GPRange operator*(const GPRange& range, double value)
{
  GPRange result(range);
  result *= value;
  return result;
}

/*!
  Multiplies both boundaries of the range by \a value.
*/
inline const GPRange operator*(double value, const GPRange& range)
{
  GPRange result(range);
  result *= value;
  return result;
}

/*!
  Divides both boundaries of the range by \a value.
*/
inline const GPRange operator/(const GPRange& range, double value)
{
  GPRange result(range);
  result /= value;
  return result;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// GPDataRange

class GP_LIB_DECL GPDataRange
{
public:
  GPDataRange();
  GPDataRange(int begin, int end);

  bool operator==(const GPDataRange& other) const { return mBegin == other.mBegin && mEnd == other.mEnd; }
  bool operator!=(const GPDataRange& other) const { return !(*this == other); }

  // getters:
  int begin() const { return mBegin; }
  int end() const { return mEnd; }
  int size() const { return mEnd-mBegin; }
  int length() const { return size(); }

  // setters:
  void setBegin(int begin) { mBegin = begin; }
  void setEnd(int end)  { mEnd = end; }

  // non-property methods:
  bool isValid() const { return (mEnd >= mBegin) && (mBegin >= 0); }
  bool isEmpty() const { return length() == 0; }
  GPDataRange bounded(const GPDataRange &other) const;
  GPDataRange expanded(const GPDataRange &other) const;
  GPDataRange intersection(const GPDataRange &other) const;
  GPDataRange adjusted(int changeBegin, int changeEnd) const { return GPDataRange(mBegin+changeBegin, mEnd+changeEnd); }
  bool intersects(const GPDataRange &other) const;
  bool contains(const GPDataRange &other) const;

private:
  // property members:
  int mBegin, mEnd;
};
Q_DECLARE_TYPEINFO(GPDataRange, Q_MOVABLE_TYPE);


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// GPDataSelection

class GP_LIB_DECL GPDataSelection
{
public:
  explicit GPDataSelection();
  explicit GPDataSelection(const GPDataRange &range);

  bool operator==(const GPDataSelection& other) const;
  bool operator!=(const GPDataSelection& other) const { return !(*this == other); }
  GPDataSelection &operator+=(const GPDataSelection& other);
  GPDataSelection &operator+=(const GPDataRange& other);
  GPDataSelection &operator-=(const GPDataSelection& other);
  GPDataSelection &operator-=(const GPDataRange& other);
  friend inline const GPDataSelection operator+(const GPDataSelection& a, const GPDataSelection& b);
  friend inline const GPDataSelection operator+(const GPDataRange& a, const GPDataSelection& b);
  friend inline const GPDataSelection operator+(const GPDataSelection& a, const GPDataRange& b);
  friend inline const GPDataSelection operator+(const GPDataRange& a, const GPDataRange& b);
  friend inline const GPDataSelection operator-(const GPDataSelection& a, const GPDataSelection& b);
  friend inline const GPDataSelection operator-(const GPDataRange& a, const GPDataSelection& b);
  friend inline const GPDataSelection operator-(const GPDataSelection& a, const GPDataRange& b);
  friend inline const GPDataSelection operator-(const GPDataRange& a, const GPDataRange& b);

  // getters:
  int dataRangeCount() const { return mDataRanges.size(); }
  int dataPointCount() const;
  GPDataRange dataRange(int index=0) const;
  QList<GPDataRange> dataRanges() const { return mDataRanges; }
  GPDataRange span() const;

  // non-property methods:
  void addDataRange(const GPDataRange &dataRange, bool simplify=true);
  void clear();
  bool isEmpty() const { return mDataRanges.isEmpty(); }
  void simplify();
  void enforceType(GP::SelectionType type);
  bool contains(const GPDataSelection &other) const;
  GPDataSelection intersection(const GPDataRange &other) const;
  GPDataSelection intersection(const GPDataSelection &other) const;
  GPDataSelection inverse(const GPDataRange &outerRange) const;

private:
  // property members:
  QList<GPDataRange> mDataRanges;

  inline static bool lessThanDataRangeBegin(const GPDataRange &a, const GPDataRange &b) { return a.begin() < b.begin(); }
};
Q_DECLARE_METATYPE(GPDataSelection)


/*!
  Return a \ref GPDataSelection with the data points in \a a joined with the data points in \a b.
  The resulting data selection is already simplified (see \ref GPDataSelection::simplify).
*/
inline const GPDataSelection operator+(const GPDataSelection& a, const GPDataSelection& b)
{
  GPDataSelection result(a);
  result += b;
  return result;
}

/*!
  Return a \ref GPDataSelection with the data points in \a a joined with the data points in \a b.
  The resulting data selection is already simplified (see \ref GPDataSelection::simplify).
*/
inline const GPDataSelection operator+(const GPDataRange& a, const GPDataSelection& b)
{
  GPDataSelection result(a);
  result += b;
  return result;
}

/*!
  Return a \ref GPDataSelection with the data points in \a a joined with the data points in \a b.
  The resulting data selection is already simplified (see \ref GPDataSelection::simplify).
*/
inline const GPDataSelection operator+(const GPDataSelection& a, const GPDataRange& b)
{
  GPDataSelection result(a);
  result += b;
  return result;
}

/*!
  Return a \ref GPDataSelection with the data points in \a a joined with the data points in \a b.
  The resulting data selection is already simplified (see \ref GPDataSelection::simplify).
*/
inline const GPDataSelection operator+(const GPDataRange& a, const GPDataRange& b)
{
  GPDataSelection result(a);
  result += b;
  return result;
}

/*!
  Return a \ref GPDataSelection with the data points which are in \a a but not in \a b.
*/
inline const GPDataSelection operator-(const GPDataSelection& a, const GPDataSelection& b)
{
  GPDataSelection result(a);
  result -= b;
  return result;
}

/*!
  Return a \ref GPDataSelection with the data points which are in \a a but not in \a b.
*/
inline const GPDataSelection operator-(const GPDataRange& a, const GPDataSelection& b)
{
  GPDataSelection result(a);
  result -= b;
  return result;
}

/*!
  Return a \ref GPDataSelection with the data points which are in \a a but not in \a b.
*/
inline const GPDataSelection operator-(const GPDataSelection& a, const GPDataRange& b)
{
  GPDataSelection result(a);
  result -= b;
  return result;
}

/*!
  Return a \ref GPDataSelection with the data points which are in \a a but not in \a b.
*/
inline const GPDataSelection operator-(const GPDataRange& a, const GPDataRange& b)
{
  GPDataSelection result(a);
  result -= b;
  return result;
}

/*! \relates GPDataRange

  Prints \a dataRange in a human readable format to the qDebug output.
*/
inline QDebug operator<< (QDebug d, const GPDataRange &dataRange)
{
    d.nospace() << "[" << dataRange.begin() << ".." << dataRange.end()-1 << "]";
    return d.space();
}

/*! \relates GPDataSelection

  Prints \a selection in a human readable format to the qDebug output.
*/
inline QDebug operator<< (QDebug d, const GPDataSelection &selection)
{
    d.nospace() << "GPDataSelection(";
    for (int i=0; i<selection.dataRangeCount(); ++i)
    {
      if (i != 0)
        d << ", ";
      d << selection.dataRange(i);
    }
    d << ")";
    return d.space();
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// GPDataContainer

/*! \relates GPDataContainer
  Returns whether the sort key of \a a is less than the sort key of \a b.

  \see GPDataContainer::sort
*/
template <class DataType>
inline bool gpLessThanSortKey(const DataType &a, const DataType &b) { return a.sortKey() < b.sortKey(); }

//// UOM descriptor
struct GPUomDescriptor
{
public:
  GPUomDescriptor()
    : quantity(0)
    , units(0)
  {
  }

  bool operator==(const GPUomDescriptor& other) const
  {
    return quantity == other.quantity 
        && units == other.units;
  }

  bool operator!=(const GPUomDescriptor& other) const
  {
    return !(*this == other);
  }

  bool isNull() const
  {
    return quantity == 0 || units == 0;
  }

  bool canConvert(const GPUomDescriptor& other) const
  {
    return !isNull() 
        && !other.isNull() 
        && quantity == other.quantity;
  }

  void load(const QVariantMap m)
  {
    quantity = UOMSettings::GetQuantityFromStringKey(m["uom_quantity"].toString());
    units    = UOMSettings::GetUnitsFromStringKey(m["uom_units"].toString());
  }

  void save(QVariantMap& m)
  {
    m["uom_quantity"] = UOMSettings::GetStringKey((UOM::PhysicalQuantity)quantity);
    m["uom_units"]    = UOMSettings::GetStringKey((UOM::PhysicalQuantity)quantity, units);
  }

  unsigned quantity;
  unsigned units;
};

//// Fwds
class GPUomDescriptorProviderListener;
class GPUomDescriptorProviderDualListener;

typedef std::function<double/*result*/(double /*value*/, const GPUomDescriptor& /*source*/, const GPUomDescriptor& /*tagret*/)> GPUomDataTransformation;

//// UOM provider interface (shall be implemented in axis)
class GPUomDescriptorProvider
{
public:
  GPUomDescriptorProvider();
  virtual ~GPUomDescriptorProvider();

  //// interface
  virtual GPUomDescriptor getUomDescriptor() const = 0;
  virtual GPUomDataTransformation getUomTransformation(const GPUomDescriptor& source) const = 0;

  //// methods
  void registerListener(GPUomDescriptorProviderListener* listener);
  void unregisterListener(GPUomDescriptorProviderListener* listener);
  
  void notifyUomChanged();

protected:
  QSet<GPUomDescriptorProviderListener*> listeners;
};

//// UOM provider listener interface (shall be implemented in data container)
class GPUomDescriptorProviderListener
{
public:
  GPUomDescriptorProviderListener();
  virtual ~GPUomDescriptorProviderListener();

  virtual void                      setUomProvider(GPUomDescriptorProvider* provider);
  virtual GPUomDescriptorProvider* getUomProvider() const;

  virtual const GPUomDescriptor&   getUomDescriptor() const;

  virtual void uomChanged();
  virtual void uomProviderDestroyed();

  virtual void applyUom(const GPUomDescriptor& descriptor) = 0;

public:
  GPUomDescriptor          uomDescriptor;
  GPUomDescriptorProvider* uomProvider;
};

//// Helper for dual listener
class GPUomDescriptorProviderListenerHelper : public GPUomDescriptorProviderListener
{
public:
  explicit GPUomDescriptorProviderListenerHelper(GPUomDescriptorProviderDualListener* dual);
  ~GPUomDescriptorProviderListenerHelper();

  virtual void applyUom(const GPUomDescriptor& descriptor) override;

protected:
  GPUomDescriptorProviderDualListener* dualDelegate;
};

//// Dual listener
class GPUomDescriptorProviderDualListener
{
public:
  enum ListenerType
  {
    ListenerTypeKeys,
    ListenerTypeValues
  };
public:
  GPUomDescriptorProviderDualListener();
  virtual ~GPUomDescriptorProviderDualListener();

  virtual void setUomProvider(GPUomDescriptorProvider* providerKeys, GPUomDescriptorProvider* providerValues);

  virtual void applyUom(GPUomDescriptorProviderListener* listener, const GPUomDescriptor& descriptor);
  virtual void applyUomForKeys(const GPUomDescriptor& descriptor) = 0;
  virtual void applyUomForValues(const GPUomDescriptor& descriptor) = 0;

public:
  GPUomDescriptorProviderListenerHelper listenerKeys;
  GPUomDescriptorProviderListenerHelper listenerValues;
};

template <class DataType>
class GPDataContainer : public GPUomDescriptorProviderDualListener// no GP_LIB_DECL, template class ends up in header (cpp included below)
{
public:
  typedef DataType data_type;
  typedef typename QVector<DataType>::const_iterator const_iterator;
  typedef typename QVector<DataType>::iterator iterator;

  GPDataContainer();

  // getters:
  bool isEmpty() const { return size() == 0; }
  bool autoSqueeze() const { return mAutoSqueeze; }

  // setters:
  void setAutoSqueeze(bool enabled);

  // get
  const QVector<DataType>& get() const { return mData; };

  // methods than shall involve uom
  virtual void set(const GPDataContainer<DataType> &data);
  virtual void set(const QVector<DataType>& data, bool alreadySorted = false);
  virtual void add(const GPDataContainer<DataType>& data);
  virtual void add(const QVector<DataType>& data, bool alreadySorted = false);
  virtual void add(const DataType &data);

  // methods:
  virtual int size() const { return mData.size()-mPreallocSize; }
  virtual void removeBefore(double sortKey);
  virtual void removeAfter(double sortKey);
  virtual void remove(double sortKeyFrom, double sortKeyTo);
  virtual void remove(double sortKey);
  virtual void clear();

  virtual void sort();
  virtual void squeeze(bool preAllocation=true, bool postAllocation=true);

  virtual const_iterator constBegin() const { return mData.constBegin()+mPreallocSize; }
  virtual const_iterator constEnd() const { return mData.constEnd(); }
  virtual iterator begin() { return mData.begin()+mPreallocSize; }
  virtual iterator end() { return mData.end(); }
  virtual const_iterator constFindBegin(double sortKey, bool expandedRange=true) const;
  virtual const_iterator constFindEnd(double sortKey, bool expandedRange=true) const;
  virtual iterator findBegin(double sortKey, bool expandedRange=true);
  virtual iterator findEnd(double sortKey, bool expandedRange=true);
  virtual const_iterator at(int index) const { return constBegin() + qBound(0, index, size()); }
  virtual const_iterator first() const { return constBegin(); }
  virtual const_iterator last() const { return constBegin() + qBound(0, size() - 1, size()); }
  virtual iterator atNonConst(int index) { return begin() + qBound(0, index, size()); }
  virtual GPRange keyRange(bool &foundRange, GP::SignDomain signDomain=GP::sdBoth);
  virtual GPRange valueRange(bool &foundRange, GP::SignDomain signDomain=GP::sdBoth, const GPRange &inKeyRange=GPRange());
  virtual GPDataRange dataRange() const { return GPDataRange(0, size()); }
  void limitIteratorsToDataRange(const_iterator &begin, const_iterator &end, const GPDataRange &dataRange) const;

    // uom transformations:
  void applyUomForFields(ListenerType listenerType, 
                         std::vector<double DataType::*> fields, 
                         const GPUomDescriptor& source,
                         const GPUomDescriptor& target,
                         iterator from,
                         iterator to)
  {
    if (size() != 0)
    {
      GPUomDataTransformation transformation = (listenerType == ListenerTypeKeys)
                                              ? listenerKeys.getUomProvider()->getUomTransformation(source)
                                              : listenerValues.getUomProvider()->getUomTransformation(source);
      for (iterator i = from; i != to; ++i)
      { 
        DataType* d = &*i;
        for (auto f : fields)
        {
          *d.*f = transformation(*d.*f, source, target);
        }
      }
    }
  }

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

  virtual void applyUomForKeys(const GPUomDescriptor& target) override
  {
    applyUomForKeys(listenerKeys.getUomDescriptor(), target, begin(), end());
  }

  virtual void applyUomForValues(const GPUomDescriptor& target) override
  {
    applyUomForValues(listenerValues.getUomDescriptor(), target, begin(), end());
  }

  virtual void applyUomForKeys(const GPUomDescriptor& source, const GPUomDescriptor& target, iterator from, iterator to);
  virtual void applyUomForValues(const GPUomDescriptor& source, const GPUomDescriptor& target, iterator from, iterator to);

  virtual void applyUomForInput(int afterIndex)
  {
    // start 
    iterator start = begin() + afterIndex;

    // keys
    auto uomDescriptorKeys = listenerKeys.getUomDescriptor();
    if (mInputUomForKeys != uomDescriptorKeys)
    {
      if (mInputUomForKeys.canConvert(uomDescriptorKeys))
      {
        applyUomForKeys(mInputUomForKeys, listenerKeys.getUomDescriptor(), start, end());
      }
      else
      {
        Q_ASSERT(!"Source uom descriptor differs from target and conversion cannot be performed, consider using setInputUomForKeys(), setInputUomForValues() prior to setData()");
      }
    }

    // values
    auto uomDescriptorValues = listenerValues.getUomDescriptor();
    if (mInputUomForValues != uomDescriptorValues)
    {
      if (mInputUomForValues.canConvert(uomDescriptorValues))
      {
        applyUomForValues(mInputUomForValues, listenerValues.getUomDescriptor(), start, end());
      }
      else
      {
        Q_ASSERT(!"Source uom descriptor differs from target and conversion cannot be performed, consider using setInputUomForKeys(), setInputUomForValues() prior to setData()");
      }
    }
  }

public:
  GPUomDescriptor mInputUomForKeys;
  GPUomDescriptor mInputUomForValues;

protected:
  // property members:
  bool mAutoSqueeze;

  // non-property memebers:
  QVector<DataType> mData;
  int mPreallocSize;
  int mPreallocIteration;

  // non-virtual methods:
  virtual void preallocateGrow(int minimumPreallocSize);
  virtual void performAutoSqueeze();
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// GPDataContainer
////////////////////////////////////////////////////////////////////////////////////////////////////

/*! \class GPDataContainer
  \brief The generic data container for one-dimensional plottables

  This class template provides a fast container for data storage of one-dimensional data. The data
  type is specified as template parameter (called \a DataType in the following) and must provide
  some methods as described in the \ref gpdatacontainer-datatype "next section".

  The data is stored in a sorted fashion, which allows very quick lookups by the sorted key as well
  as retrieval of ranges (see \ref findBegin, \ref findEnd, \ref keyRange) using binary search. The
  container uses a preallocation and a postallocation scheme, such that appending and prepending
  data (with respect to the sort key) is very fast and minimizes reallocations. If data is added
  which needs to be inserted between existing keys, the merge usually can be done quickly too,
  using the fact that existing data is always sorted. The user can further improve performance by
  specifying that added data is already itself sorted by key, if he can guarantee that this is the
  case (see for example \ref add(const QVector<DataType>& data, bool alreadySorted)).

  The data can be accessed with the provided const iterators (\ref constBegin, \ref constEnd). If
  it is necessary to alter existing data in-place, the non-const iterators can be used (\ref begin,
  \ref end). Changing data members that are not the sort key (for most data types called \a key) is
  safe from the container's perspective.

  Great care must be taken however if the sort key is modified through the non-const iterators. For
  performance reasons, the iterators don't automatically cause a re-sorting upon their
  manipulation. It is thus the responsibility of the user to leave the container in a sorted state
  when finished with the data manipulation, before calling any other methods on the container. A
  complete re-sort (e.g. after finishing all sort key manipulation) can be done by calling \ref
  sort. Failing to do so can not be detected by the container efficiently and will cause both
  rendering artifacts and potential data loss.

  Implementing one-dimensional plottables that make use of a \ref GPDataContainer<T> is usually
  done by subclassing from \ref GPAbstractPlottable1D "GPAbstractPlottable1D<T>", which
  introduces an according \a mDataContainer member and some convenience methods.

  \section gpdatacontainer-datatype Requirements for the DataType template parameter

  The template parameter <tt>DataType</tt> is the type of the stored data points. It must be
  trivially copyable and have the following public methods, preferably inline:

  \li <tt>double sortKey() const</tt>\n Returns the member variable of this data point that is the
  sort key, defining the ordering in the container. Often this variable is simply called \a key.

  \li <tt>static DataType fromSortKey(double sortKey)</tt>\n Returns a new instance of the data
  type initialized with its sort key set to \a sortKey.

  \li <tt>static bool sortKeyIsMainKey()</tt>\n Returns true if the sort key is equal to the main
  key (see method \c mainKey below). For most plottables this is the case. It is not the case for
  example for \ref GPCurve, which uses \a t as sort key and \a key as main key. This is the reason
  why GPCurve unlike GPGraph can display parametric curves with loops.

  \li <tt>double mainKey() const</tt>\n Returns the variable of this data point considered the main
  key. This is commonly the variable that is used as the coordinate of this data point on the key
  axis of the plottable. This method is used for example when determining the automatic axis
  rescaling of key axes (\ref GPAxis::rescale).

  \li <tt>double mainValue() const</tt>\n Returns the variable of this data point considered the
  main value. This is commonly the variable that is used as the coordinate of this data point on
  the value axis of the plottable.

  \li <tt>GPRange valueRange() const</tt>\n Returns the range this data point spans in the value
  axis coordinate. If the data is single-valued (e.g. GPGraphData), this is simply a range with
  both lower and upper set to the main data point value. However if the data points can represent
  multiple values at once (e.g GPFinancialData with its \a high, \a low, \a open and \a close
  values at each \a key) this method should return the range those values span. This method is used
  for example when determining the automatic axis rescaling of value axes (\ref
  GPAxis::rescale).
*/

/* start documentation of inline functions */

/*! \fn int GPDataContainer<DataType>::size() const

  Returns the number of data points in the container.
*/

/*! \fn bool GPDataContainer<DataType>::isEmpty() const

  Returns whether this container holds no data points.
*/

/*! \fn GPDataContainer::const_iterator GPDataContainer<DataType>::constBegin() const

  Returns a const iterator to the first data point in this container.
*/

/*! \fn GPDataContainer::const_iterator GPDataContainer<DataType>::constEnd() const

  Returns a const iterator to the element past the last data point in this container.
*/

/*! \fn GPDataContainer::iterator GPDataContainer<DataType>::begin() const

  Returns a non-const iterator to the first data point in this container.

  You can manipulate the data points in-place through the non-const iterators, but great care must
  be taken when manipulating the sort key of a data point, see \ref sort, or the detailed
  description of this class.
*/

/*! \fn GPDataContainer::iterator GPDataContainer<DataType>::end() const

  Returns a non-const iterator to the element past the last data point in this container.

  You can manipulate the data points in-place through the non-const iterators, but great care must
  be taken when manipulating the sort key of a data point, see \ref sort, or the detailed
  description of this class.
*/

/*! \fn GPDataContainer::const_iterator GPDataContainer<DataType>::at(int index) const

  Returns a const iterator to the element with the specified \a index. If \a index points beyond
  the available elements in this container, returns \ref constEnd, i.e. an iterator past the last
  valid element.

  You can use this method to easily obtain iterators from a \ref GPDataRange, see the \ref
  dataselection-accessing "data selection page" for an example.
*/

/*! \fn GPDataRange GPDataContainer::dataRange() const

  Returns a \ref GPDataRange encompassing the entire data set of this container. This means the
  begin index of the returned range is 0, and the end index is \ref size.
*/

/* end documentation of inline functions */

/*!
  Constructs a GPDataContainer used for plottable classes that represent a series of key-sorted
  data
*/
template <class DataType>
GPDataContainer<DataType>::GPDataContainer() :
  mAutoSqueeze(true),
  mPreallocSize(0),
  mPreallocIteration(0)
{
}

/*!
  Sets whether the container automatically decides when to release memory from its post- and
  preallocation pools when data points are removed. By default this is enabled and for typical
  applications shouldn't be changed.

  If auto squeeze is disabled, you can manually decide when to release pre-/postallocation with
  \ref squeeze.
*/
template <class DataType>
void GPDataContainer<DataType>::setAutoSqueeze(bool enabled)
{
  if (mAutoSqueeze != enabled)
  {
    mAutoSqueeze = enabled;
    if (mAutoSqueeze)
      performAutoSqueeze();
  }
}

/*! \overload

  Replaces the current data in this container with the provided \a data.

  \see add, remove
*/
template <class DataType>
void GPDataContainer<DataType>::set(const GPDataContainer<DataType> &data)
{
  clear();
  add(data);
}

/*! \overload

  Replaces the current data in this container with the provided \a data

  If you can guarantee that the data points in \a data have ascending order with respect to the
  DataType's sort key, set \a alreadySorted to true to avoid an unnecessary sorting run.

  \see add, remove
*/
template <class DataType>
void GPDataContainer<DataType>::set(const QVector<DataType>& data, bool alreadySorted)
{
  mData = data;
  mPreallocSize = 0;
  mPreallocIteration = 0;
  if (!alreadySorted)
    sort();

  //// Changed, apply uom
  applyUomForInput(0);
}

/*! \overload

  Adds the provided \a data to the current data in this container.

  \see set, remove
*/
template <class DataType>
void GPDataContainer<DataType>::add(const GPDataContainer<DataType> &data)
{
  if (data.isEmpty())
    return;

  const int n = data.size();
  const int oldSize = size();

  if (oldSize > 0 && !gpLessThanSortKey<DataType>(*constBegin(), *(data.constEnd()-1))) // prepend if new data keys are all smaller than or equal to existing ones
  {
    if (mPreallocSize < n)
      preallocateGrow(n);
    mPreallocSize -= n;
    std::copy(data.constBegin(), data.constEnd(), begin());
  } 
  else // don't need to prepend, so append and merge if necessary
  {
    mData.resize(mData.size()+n);
    std::copy(data.constBegin(), data.constEnd(), end()-n);
    if (oldSize > 0 && !gpLessThanSortKey<DataType>(*(constEnd()-n-1), *(constEnd()-n))) // if appended range keys aren't all greater than existing ones, merge the two partitions
      std::inplace_merge(begin(), end()-n, end(), gpLessThanSortKey<DataType>);
  }

  //// Changed, apply uom
  //// @todo - this is completely wrong with unsorted data
  applyUomForInput(oldSize); 
}

/*!
  Adds the provided data points in \a data to the current data.

  If you can guarantee that the data points in \a data have ascending order with respect to the
  DataType's sort key, set \a alreadySorted to true to avoid an unnecessary sorting run.

  \see set, remove
*/
template <class DataType>
void GPDataContainer<DataType>::add(const QVector<DataType>& data, bool alreadySorted)
{
  if (data.isEmpty())
    return;

  if (isEmpty())
  {
    set(data, alreadySorted);
    return;
  }

  const int n = data.size();
  const int oldSize = size();

  if (alreadySorted && oldSize > 0 && !gpLessThanSortKey<DataType>(*constBegin(), *(data.constEnd()-1))) // prepend if new data is sorted and keys are all smaller than or equal to existing ones
  {
    if (mPreallocSize < n)
      preallocateGrow(n);
    mPreallocSize -= n;
    std::copy(data.constBegin(), data.constEnd(), begin());
  } 
  else // don't need to prepend, so append and then sort and merge if necessary
  {
    mData.resize(mData.size()+n);
    std::copy(data.constBegin(), data.constEnd(), end()-n);
    if (!alreadySorted) // sort appended subrange if it wasn't already sorted
      std::sort(end()-n, end(), gpLessThanSortKey<DataType>);
    if (oldSize > 0 && !gpLessThanSortKey<DataType>(*(constEnd()-n-1), *(constEnd()-n))) // if appended range keys aren't all greater than existing ones, merge the two partitions
      std::inplace_merge(begin(), end()-n, end(), gpLessThanSortKey<DataType>);
  }

  //// Changed, apply uom
  //// @todo - this is completely wrong with unsorted data
  applyUomForInput(oldSize);
}

/*! \overload

  Adds the provided single data point to the current data.

  \see remove
*/
template <class DataType>
void GPDataContainer<DataType>::add(const DataType &data)
{
  const int oldSize = size();

  if (isEmpty() || !gpLessThanSortKey<DataType>(data, *(constEnd()-1))) // quickly handle appends if new data key is greater or equal to existing ones
  {
    mData.append(data);
  } 
  else if (gpLessThanSortKey<DataType>(data, *constBegin()))  // quickly handle prepends using preallocated space
  {
    if (mPreallocSize < 1)
      preallocateGrow(1);
    --mPreallocSize;
    *begin() = data;
  } 
  else // handle inserts, maintaining sorted keys
  {
    GPDataContainer<DataType>::iterator insertionPoint = std::lower_bound(begin(), end(), data, gpLessThanSortKey<DataType>);
    mData.insert(insertionPoint, data);
  }

  //// Changed, apply uom
  //// @todo - this is completely wrong with unsorted data
  applyUomForInput(oldSize);
}

/*!
  Removes all data points with (sort-)keys smaller than or equal to \a sortKey.

  \see removeAfter, remove, clear
*/
template <class DataType>
void GPDataContainer<DataType>::removeBefore(double sortKey)
{
  GPDataContainer<DataType>::iterator it = begin();
  GPDataContainer<DataType>::iterator itEnd = std::lower_bound(begin(), end(), DataType::fromSortKey(sortKey), gpLessThanSortKey<DataType>);
  mPreallocSize += itEnd-it; // don't actually delete, just add it to the preallocated block (if it gets too large, squeeze will take care of it)
  if (mAutoSqueeze)
    performAutoSqueeze();
}

/*!
  Removes all data points with (sort-)keys greater than or equal to \a sortKey.

  \see removeBefore, remove, clear
*/
template <class DataType>
void GPDataContainer<DataType>::removeAfter(double sortKey)
{
  GPDataContainer<DataType>::iterator it = std::upper_bound(begin(), end(), DataType::fromSortKey(sortKey), gpLessThanSortKey<DataType>);
  GPDataContainer<DataType>::iterator itEnd = end();
  mData.erase(it, itEnd); // typically adds it to the postallocated block
  if (mAutoSqueeze)
    performAutoSqueeze();
}

/*!
  Removes all data points with (sort-)keys between \a sortKeyFrom and \a sortKeyTo. if \a
  sortKeyFrom is greater or equal to \a sortKeyTo, the function does nothing. To remove a single
  data point with known (sort-)key, use \ref remove(double sortKey).

  \see removeBefore, removeAfter, clear
*/
template <class DataType>
void GPDataContainer<DataType>::remove(double sortKeyFrom, double sortKeyTo)
{
  if (sortKeyFrom >= sortKeyTo || isEmpty())
    return;

  GPDataContainer<DataType>::iterator it = std::lower_bound(begin(), end(), DataType::fromSortKey(sortKeyFrom), gpLessThanSortKey<DataType>);
  GPDataContainer<DataType>::iterator itEnd = std::upper_bound(it, end(), DataType::fromSortKey(sortKeyTo), gpLessThanSortKey<DataType>);
  mData.erase(it, itEnd);
  if (mAutoSqueeze)
    performAutoSqueeze();
}

/*! \overload

  Removes a single data point at \a sortKey. If the position is not known with absolute (binary)
  precision, consider using \ref remove(double sortKeyFrom, double sortKeyTo) with a small
  fuzziness interval around the suspected position, depeding on the precision with which the
  (sort-)key is known.

  \see removeBefore, removeAfter, clear
*/
template <class DataType>
void GPDataContainer<DataType>::remove(double sortKey)
{
  GPDataContainer::iterator it = std::lower_bound(begin(), end(), DataType::fromSortKey(sortKey), gpLessThanSortKey<DataType>);
  if (it != end() && it->sortKey() == sortKey)
  {
    if (it == begin())
      ++mPreallocSize; // don't actually delete, just add it to the preallocated block (if it gets too large, squeeze will take care of it)
    else
      mData.erase(it);
  }
  if (mAutoSqueeze)
    performAutoSqueeze();
}

/*!
  Removes all data points.

  \see remove, removeAfter, removeBefore
*/
template <class DataType>
void GPDataContainer<DataType>::clear()
{
  mData.clear();
  mPreallocIteration = 0;
  mPreallocSize = 0;
}

/*!
  Re-sorts all data points in the container by their sort key.

  When setting, adding or removing points using the GPDataContainer interface (\ref set, \ref add,
  \ref remove, etc.), the container makes sure to always stay in a sorted state such that a full
  resort is never necessary. However, if you choose to directly manipulate the sort key on data
  points by accessing and modifying it through the non-const iterators (\ref begin, \ref end), it
  is your responsibility to bring the container back into a sorted state before any other methods
  are called on it. This can be achieved by calling this method immediately after finishing the
  sort key manipulation.
*/
template <class DataType>
void GPDataContainer<DataType>::sort()
{
  std::sort(begin(), end(), gpLessThanSortKey<DataType>);
}

/*!
  Frees all unused memory that is currently in the preallocation and postallocation pools.

  Note that GPDataContainer automatically decides whether squeezing is necessary, if \ref
  setAutoSqueeze is left enabled. It should thus not be necessary to use this method for typical
  applications.

  The parameters \a preAllocation and \a postAllocation control whether pre- and/or post allocation
  should be freed, respectively.
*/
template <class DataType>
void GPDataContainer<DataType>::squeeze(bool preAllocation, bool postAllocation)
{
  if (preAllocation)
  {
    if (mPreallocSize > 0)
    {
      std::copy(begin(), end(), mData.begin());
      mData.resize(size());
      mPreallocSize = 0;
    }
    mPreallocIteration = 0;
  }
  if (postAllocation)
    mData.squeeze();
}

/*!
  Returns an iterator to the data point with a (sort-)key that is equal to, just below, or just
  above \a sortKey. If \a expandedRange is true, the data point just below \a sortKey will be
  considered, otherwise the one just above.

  This can be used in conjunction with \ref findEnd to iterate over data points within a given key
  range, including or excluding the bounding data points that are just beyond the specified range.

  If \a expandedRange is true but there are no data points below \a sortKey, \ref constBegin is
  returned.

  If the container is empty, returns \ref constEnd.

  \see findEnd, GPPlottableInterface1D::findBegin
*/
template <class DataType>
typename GPDataContainer<DataType>::const_iterator GPDataContainer<DataType>::constFindBegin(double sortKey, bool expandedRange) const
{
  if (isEmpty())
    return constEnd();

  GPDataContainer<DataType>::const_iterator it = std::lower_bound(constBegin(), constEnd(), DataType::fromSortKey(sortKey), gpLessThanSortKey<DataType>);
  if (expandedRange && it != constBegin()) // also covers it == constEnd case, and we know --constEnd is valid because mData isn't empty
    --it;
  return it;
}

template<class DataType>
typename GPDataContainer<DataType>::iterator GPDataContainer<DataType>::findBegin(double sortKey, bool expandedRange)
{
  if (isEmpty())
    return end();

  GPDataContainer<DataType>::iterator it = std::lower_bound(begin(), end(), DataType::fromSortKey(sortKey), gpLessThanSortKey<DataType>);
  if (expandedRange && it != begin()) // also covers it == constEnd case, and we know --constEnd is valid because mData isn't empty
    --it;
  return it;
}

/*!
  Returns an iterator to the element after the data point with a (sort-)key that is equal to, just
  above or just below \a sortKey. If \a expandedRange is true, the data point just above \a sortKey
  will be considered, otherwise the one just below.

  This can be used in conjunction with \ref findBegin to iterate over data points within a given
  key range, including the bounding data points that are just below and above the specified range.

  If \a expandedRange is true but there are no data points above \a sortKey, \ref constEnd is
  returned.

  If the container is empty, \ref constEnd is returned.

  \see findBegin, GPPlottableInterface1D::findEnd
*/
template <class DataType>
typename GPDataContainer<DataType>::const_iterator GPDataContainer<DataType>::constFindEnd(double sortKey, bool expandedRange) const
{
  if (isEmpty())
    return constEnd();

  GPDataContainer<DataType>::const_iterator it = std::upper_bound(constBegin(), constEnd(), DataType::fromSortKey(sortKey), gpLessThanSortKey<DataType>);
  if (expandedRange && it != constEnd())
    ++it;
  return it;
}

template<class DataType>
typename GPDataContainer<DataType>::iterator GPDataContainer<DataType>::findEnd(double sortKey, bool expandedRange)
{
  if (isEmpty())
    return end();

  GPDataContainer<DataType>::iterator it = std::upper_bound(begin(), end(), DataType::fromSortKey(sortKey), gpLessThanSortKey<DataType>);
  if (expandedRange && it != end())
    ++it;
  return it;
}

/*!
  Returns the range encompassed by the (main-)key coordinate of all data points. The output
  parameter \a foundRange indicates whether a sensible range was found. If this is false, you
  should not use the returned GPRange (e.g. the data container is empty or all points have the
  same key).

  Use \a signDomain to control which sign of the key coordinates should be considered. This is
  relevant e.g. for logarithmic plots which can mathematically only display one sign domain at a
  time.

  If the DataType reports that its main key is equal to the sort key (\a sortKeyIsMainKey), as is
  the case for most plottables, this method uses this fact and finds the range very quickly.

  \see valueRange
*/
template <class DataType>
GPRange GPDataContainer<DataType>::keyRange(bool &foundRange, GP::SignDomain signDomain)
{
  if (isEmpty())
  {
    foundRange = false;
    return GPRange();
  }
  GPRange range;
  bool haveLower = false;
  bool haveUpper = false;
  double current;

  GPDataContainer<DataType>::const_iterator it = constBegin();
  GPDataContainer<DataType>::const_iterator itEnd = constEnd();
  if (signDomain == GP::sdBoth) // range may be anywhere
  {
    if (DataType::sortKeyIsMainKey()) // if DataType is sorted by main key (e.g. GPGraph, but not GPCurve), use faster algorithm by finding just first and last key with non-NaN value
    {
      while (it != itEnd) // find first non-nan going up from left
      {
        if (!qIsNaN(it->mainValue()))
        {
          range.lower = it->mainKey();
          haveLower = true;
          break;
        }
        ++it;
      }
      it = itEnd;
      while (it != constBegin()) // find first non-nan going down from right
      {
        --it;
        if (!qIsNaN(it->mainValue()))
        {
          range.upper = it->mainKey();
          haveUpper = true;
          break;
        }
      }
    } else // DataType is not sorted by main key, go through all data points and accordingly expand range
    {
      while (it != itEnd)
      {
        if (!qIsNaN(it->mainValue()))
        {
          current = it->mainKey();
          if (current < range.lower || !haveLower)
          {
            range.lower = current;
            haveLower = true;
          }
          if (current > range.upper || !haveUpper)
          {
            range.upper = current;
            haveUpper = true;
          }
        }
        ++it;
      }
    }
  } else if (signDomain == GP::sdNegative) // range may only be in the negative sign domain
  {
    while (it != itEnd)
    {
      if (!qIsNaN(it->mainValue()))
      {
        current = it->mainKey();
        if ((current < range.lower || !haveLower) && current < 0)
        {
          range.lower = current;
          haveLower = true;
        }
        if ((current > range.upper || !haveUpper) && current < 0)
        {
          range.upper = current;
          haveUpper = true;
        }
      }
      ++it;
    }
  } else if (signDomain == GP::sdPositive) // range may only be in the positive sign domain
  {
    while (it != itEnd)
    {
      if (!qIsNaN(it->mainValue()))
      {
        current = it->mainKey();
        if ((current < range.lower || !haveLower) && current > 0)
        {
          range.lower = current;
          haveLower = true;
        }
        if ((current > range.upper || !haveUpper) && current > 0)
        {
          range.upper = current;
          haveUpper = true;
        }
      }
      ++it;
    }
  }

  foundRange = haveLower && haveUpper;
  return range;
}

/*!
  Returns the range encompassed by the value coordinates of the data points in the specified key
  range (\a inKeyRange), using the full \a DataType::valueRange reported by the data points. The
  output parameter \a foundRange indicates whether a sensible range was found. If this is false,
  you should not use the returned GPRange (e.g. the data container is empty or all points have the
  same value).

  If \a inKeyRange has both lower and upper bound set to zero (is equal to <tt>GPRange()</tt>),
  all data points are considered, without any restriction on the keys.

  Use \a signDomain to control which sign of the value coordinates should be considered. This is
  relevant e.g. for logarithmic plots which can mathematically only display one sign domain at a
  time.

  \see keyRange
*/
template <class DataType>
GPRange GPDataContainer<DataType>::valueRange(bool &foundRange, GP::SignDomain signDomain, const GPRange &inKeyRange)
{
  if (isEmpty())
  {
    foundRange = false;
    return GPRange();
  }
  GPRange range;
  const bool restrictKeyRange = inKeyRange != GPRange();
  bool haveLower = false;
  bool haveUpper = false;
  GPRange current;
  GPDataContainer<DataType>::const_iterator itBegin = constBegin();
  GPDataContainer<DataType>::const_iterator itEnd = constEnd();
  if (DataType::sortKeyIsMainKey() && restrictKeyRange)
  {
    itBegin = constFindBegin(inKeyRange.lower);
    itEnd = constFindEnd(inKeyRange.upper);
  }
  if (signDomain == GP::sdBoth) // range may be anywhere
  {
    for (GPDataContainer<DataType>::const_iterator it = itBegin; it != itEnd; ++it)
    {
      if (restrictKeyRange && (it->mainKey() < inKeyRange.lower || it->mainKey() > inKeyRange.upper))
        continue;
      current = it->valueRange();
      if ((current.lower < range.lower || !haveLower) && !qIsNaN(current.lower))
      {
        range.lower = current.lower;
        haveLower = true;
      }
      if ((current.upper > range.upper || !haveUpper) && !qIsNaN(current.upper))
      {
        range.upper = current.upper;
        haveUpper = true;
      }
    }
  } else if (signDomain == GP::sdNegative) // range may only be in the negative sign domain
  {
    for (GPDataContainer<DataType>::const_iterator it = itBegin; it != itEnd; ++it)
    {
      if (restrictKeyRange && (it->mainKey() < inKeyRange.lower || it->mainKey() > inKeyRange.upper))
        continue;
      current = it->valueRange();
      if ((current.lower < range.lower || !haveLower) && current.lower < 0 && !qIsNaN(current.lower))
      {
        range.lower = current.lower;
        haveLower = true;
      }
      if ((current.upper > range.upper || !haveUpper) && current.upper < 0 && !qIsNaN(current.upper))
      {
        range.upper = current.upper;
        haveUpper = true;
      }
    }
  } else if (signDomain == GP::sdPositive) // range may only be in the positive sign domain
  {
    for (GPDataContainer<DataType>::const_iterator it = itBegin; it != itEnd; ++it)
    {
      if (restrictKeyRange && (it->mainKey() < inKeyRange.lower || it->mainKey() > inKeyRange.upper))
        continue;
      current = it->valueRange();
      if ((current.lower < range.lower || !haveLower) && current.lower > 0 && !qIsNaN(current.lower))
      {
        range.lower = current.lower;
        haveLower = true;
      }
      if ((current.upper > range.upper || !haveUpper) && current.upper > 0 && !qIsNaN(current.upper))
      {
        range.upper = current.upper;
        haveUpper = true;
      }
    }
  }

  foundRange = haveLower && haveUpper;
  return range;
}

/*!
  Makes sure \a begin and \a end mark a data range that is both within the bounds of this data
  container's data, as well as within the specified \a dataRange. The initial range described by
  the passed iterators \a begin and \a end is never expanded, only contracted if necessary.

  This function doesn't require for \a dataRange to be within the bounds of this data container's
  valid range.
*/
template <class DataType>
void GPDataContainer<DataType>::limitIteratorsToDataRange(const_iterator &begin, const_iterator &end, const GPDataRange &dataRange) const
{
  GPDataRange iteratorRange(begin-constBegin(), end-constBegin());
  iteratorRange = iteratorRange.bounded(dataRange.bounded(this->dataRange()));
  begin = constBegin()+iteratorRange.begin();
  end = constBegin()+iteratorRange.end();
}

/*! \internal

  Increases the preallocation pool to have a size of at least \a minimumPreallocSize. Depending on
  the preallocation history, the container will grow by more than requested, to speed up future
  consecutive size increases.

  if \a minimumPreallocSize is smaller than or equal to the current preallocation pool size, this
  method does nothing.
*/
template <class DataType>
void GPDataContainer<DataType>::preallocateGrow(int minimumPreallocSize)
{
  if (minimumPreallocSize <= mPreallocSize)
    return;

  int newPreallocSize = minimumPreallocSize;
  newPreallocSize += (1u<<qBound(4, mPreallocIteration+4, 15)) - 12; // do 4 up to 32768-12 preallocation, doubling in each intermediate iteration
  ++mPreallocIteration;

  int sizeDifference = newPreallocSize-mPreallocSize;
  mData.resize(mData.size()+sizeDifference);
  std::copy_backward(mData.begin()+mPreallocSize, mData.end()-sizeDifference, mData.end());
  mPreallocSize = newPreallocSize;
}

/*! \internal

  This method decides, depending on the total allocation size and the size of the unused pre- and
  postallocation pools, whether it is sensible to reduce the pools in order to free up unused
  memory. It then possibly calls \ref squeeze to do the deallocation.

  If \ref setAutoSqueeze is enabled, this method is called automatically each time data points are
  removed from the container (e.g. \ref remove).

  \note when changing the decision parameters, care must be taken not to cause a back-and-forth
  between squeezing and reallocation due to the growth strategy of the internal QVector and \ref
  preallocateGrow. The hysteresis between allocation and deallocation should be made high enough
  (at the expense of possibly larger unused memory from time to time).
*/
template <class DataType>
void GPDataContainer<DataType>::performAutoSqueeze()
{
  const int totalAlloc = mData.capacity();
  const int postAllocSize = totalAlloc-mData.size();
  const int usedSize = size();
  bool shrinkPostAllocation = false;
  bool shrinkPreAllocation = false;
  if (totalAlloc > 650000) // if allocation is larger, shrink earlier with respect to total used size
  {
    shrinkPostAllocation = postAllocSize > usedSize*1.5; // QVector grow strategy is 2^n for static data. Watch out not to oscillate!
    shrinkPreAllocation = mPreallocSize*10 > usedSize;
  } else if (totalAlloc > 1000) // below 10 MiB raw data be generous with preallocated memory, below 1k points don't even bother
  {
    shrinkPostAllocation = postAllocSize > usedSize*5;
    shrinkPreAllocation = mPreallocSize > usedSize*1.5; // preallocation can grow into postallocation, so can be smaller
  }

  if (shrinkPreAllocation || shrinkPostAllocation)
    squeeze(shrinkPreAllocation, shrinkPostAllocation);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// GPGraphData

template <class DataType>
class GPRepresentationDataContainer : public GPDataContainer<DataType>
{
public:
  typedef typename GPDataContainer<DataType>::const_iterator const_iterator;
  typedef typename GPDataContainer<DataType>::iterator iterator;

  GPRepresentationDataContainer()
    : GPDataContainer<DataType>()
  {

  }

  GPDataContainer<DataType> originalData();

  // methods:
  virtual int size() const override { return mOriginalData.size(); }
  virtual void set(const QVector<DataType>& data, bool alreadySorted=false) override;
  virtual void add(const GPDataContainer<DataType>& data) override;
  virtual void add(const QVector<DataType>& data, bool alreadySorted=false) override;
  virtual void add(const DataType &data) override;
  virtual void removeBefore(double sortKey) override;
  virtual void removeAfter(double sortKey) override;
  virtual void remove(double sortKeyFrom, double sortKeyTo) override;
  virtual void remove(double sortKey) override;
  virtual void clear() override;

  virtual void sort() override;
  virtual void squeeze(bool preAllocation=true, bool postAllocation=true)  override;

  virtual const_iterator constBegin() const override;
  virtual const_iterator constEnd() const override;
  virtual iterator begin() override;
  virtual iterator end() override;
  virtual const_iterator constFindBegin(double sortKey, bool expandedRange=true) const override;
  virtual const_iterator constFindEnd(double sortKey, bool expandedRange=true) const override;
  virtual iterator findBegin(double sortKey, bool expandedRange=true) override;
  virtual iterator findEnd(double sortKey, bool expandedRange=true) override;
  virtual GPRange keyRange(bool &foundRange, GP::SignDomain signDomain=GP::sdBoth) override;
  virtual GPRange valueRange(bool &foundRange, GP::SignDomain signDomain=GP::sdBoth, const GPRange &inKeyRange=GPRange()) override;

  void invalidateData(int index) { mInvalidatedData.insert(index); }
  void invalidateData(const std::set<int>& indexes) { mInvalidatedData.insert(indexes.begin(), indexes.end()); }
  void invalidateData(int begin, int end) { for (int i = 0; i <= end; ++i) mInvalidatedData.insert(i);}
  void invalidateData(const_iterator begin, const_iterator end) { invalidateData(std::distance(GPDataContainer<DataType>::mData.begin(), begin), std::distance(GPDataContainer<DataType>::mData.begin(), end) - 1); }

  // uoms must be dispatched to original data
  virtual void setInputUomForKeys(const GPUomDescriptor& descriptor) override
  {
    GPDataContainer<DataType>::setInputUomForKeys(descriptor);
    mOriginalData.setInputUomForKeys(descriptor);
  }

  virtual void setInputUomForValues(const GPUomDescriptor& descriptor) override
  {
    GPDataContainer<DataType>::setInputUomForValues(descriptor);
    mOriginalData.setInputUomForValues(descriptor);
  }

  virtual void setUomProvider(GPUomDescriptorProvider* providerKeys, GPUomDescriptorProvider* providerValues) override
  {
    GPDataContainer<DataType>::setUomProvider(providerKeys, providerValues);
    mOriginalData.setUomProvider(providerKeys, providerValues);
  }

protected:
  virtual void updateData() = 0;

protected:
  mutable QReadWriteLock mDataLock;
  GPDataContainer<DataType> mOriginalData;
  std::set<int> mInvalidatedData;
};

template <class DataType>
void GPRepresentationDataContainer<DataType>::set(const QVector<DataType>& data, bool alreadySorted)
{
  QWriteLocker lw(&mDataLock);
  mOriginalData.set(data, alreadySorted);
  invalidateData({ 0, mOriginalData.size() - 1});
}

template <class DataType>
void GPRepresentationDataContainer<DataType>::add(const GPDataContainer<DataType>& data)
{
  QWriteLocker lw(&mDataLock);
  mOriginalData.add(data);
  invalidateData({ mOriginalData.size() - data.size(), mOriginalData.size() - 1});
}

template <class DataType>
void GPRepresentationDataContainer<DataType>::add(const QVector<DataType>& data, bool alreadySorted)
{
  QWriteLocker lw(&mDataLock);
  mOriginalData.add(data, alreadySorted);
  invalidateData({ (int)mOriginalData.size() - (int)data.size(), (int)mOriginalData.size() - 1});
}

template <class DataType>
void GPRepresentationDataContainer<DataType>::add(const DataType &data)
{
  QWriteLocker lw(&mDataLock);
  mOriginalData.add(data);
  invalidateData(mOriginalData.size() - 1);
}

template <class DataType>
void GPRepresentationDataContainer<DataType>::removeBefore(double sortKey)
{
  QWriteLocker lw(&mDataLock);
  mOriginalData.removeBefore(sortKey);
  GPDataContainer<DataType>::removeAfter(sortKey);
}

template <class DataType>
void GPRepresentationDataContainer<DataType>::removeAfter(double sortKey)
{
  QWriteLocker lw(&mDataLock);
  mOriginalData.removeAfter(sortKey);
  GPDataContainer<DataType>::removeAfter(sortKey);
}

template <class DataType>
void GPRepresentationDataContainer<DataType>::remove(double sortKeyFrom, double sortKeyTo)
{
  QWriteLocker lw(&mDataLock);
  mOriginalData.remove(sortKeyFrom, sortKeyTo);
  GPDataContainer<DataType>::remove(sortKeyFrom, sortKeyTo);
}

template <class DataType>
void GPRepresentationDataContainer<DataType>::remove(double sortKey)
{
  QWriteLocker lw(&mDataLock);
  mOriginalData.remove(sortKey);
  GPDataContainer<DataType>::remove(sortKey);
}

template <class DataType>
void GPRepresentationDataContainer<DataType>::clear()
{
  QWriteLocker lw(&mDataLock);
  mOriginalData.clear();
  GPDataContainer<DataType>::clear();
}

template <class DataType>
void GPRepresentationDataContainer<DataType>::sort()
{
  QWriteLocker lw(&mDataLock);
  GPDataContainer<DataType>::sort();
}

template <class DataType>
void GPRepresentationDataContainer<DataType>::squeeze(bool preAllocation, bool postAllocation)
{
  QWriteLocker lw(&mDataLock);
  GPDataContainer<DataType>::squeeze(preAllocation, postAllocation);
}

template <class DataType>
typename GPRepresentationDataContainer<DataType>::const_iterator GPRepresentationDataContainer<DataType>::constBegin() const
{
  QReadLocker l(&mDataLock);
  return GPDataContainer<DataType>::constBegin();
}

template <class DataType>
typename GPRepresentationDataContainer<DataType>::const_iterator GPRepresentationDataContainer<DataType>::constEnd() const
{
  QReadLocker l(&mDataLock);
  return GPDataContainer<DataType>::constEnd();
}

template <class DataType>
typename GPRepresentationDataContainer<DataType>::iterator GPRepresentationDataContainer<DataType>::begin()
{
  QReadLocker l(&mDataLock);
  return GPDataContainer<DataType>::begin();
}

template <class DataType>
typename GPRepresentationDataContainer<DataType>::iterator GPRepresentationDataContainer<DataType>::end()
{
  QReadLocker l(&mDataLock);
  return GPDataContainer<DataType>::end();
}

template <class DataType>
typename GPRepresentationDataContainer<DataType>::const_iterator GPRepresentationDataContainer<DataType>::constFindBegin(double sortKey, bool expandedRange) const
{
  QReadLocker l(&mDataLock);
  return GPDataContainer<DataType>::constFindBegin(sortKey, expandedRange);
}

template <class DataType>
typename GPRepresentationDataContainer<DataType>::const_iterator GPRepresentationDataContainer<DataType>::constFindEnd(double sortKey, bool expandedRange) const
{
  QReadLocker l(&mDataLock);
  return GPDataContainer<DataType>::constFindEnd(sortKey, expandedRange);
}

template <class DataType>
typename GPRepresentationDataContainer<DataType>::iterator GPRepresentationDataContainer<DataType>::findBegin(double sortKey, bool expandedRange)
{
  QReadLocker l(&mDataLock);
  return GPDataContainer<DataType>::findBegin(sortKey, expandedRange);
}

template <class DataType>
typename GPRepresentationDataContainer<DataType>::iterator GPRepresentationDataContainer<DataType>::findEnd(double sortKey, bool expandedRange)
{
  QReadLocker l(&mDataLock);
  return GPDataContainer<DataType>::findEnd(sortKey, expandedRange);
}

template <class DataType>
GPRange GPRepresentationDataContainer<DataType>::keyRange(bool& foundRange, GP::SignDomain signDomain)
{
  QReadLocker l(&mDataLock);
  return GPDataContainer<DataType>::keyRange(foundRange, signDomain);
}

template <class DataType>
GPRange GPRepresentationDataContainer<DataType>::valueRange(bool& foundRange, GP::SignDomain signDomain, const GPRange& inKeyRange)
{
  QReadLocker l(&mDataLock);
  return GPDataContainer<DataType>::valueRange(foundRange, signDomain, inKeyRange);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// GPGraphData

class GP_LIB_DECL GPGraphData
{
public:
  GPGraphData();
  GPGraphData(double key, double value);

  inline double sortKey() const { return key; }
  inline static GPGraphData fromSortKey(double sortKey) { return GPGraphData(sortKey, 0); }
  inline static bool sortKeyIsMainKey() { return true; }

  inline double mainKey() const { return key; }
  inline double mainValue() const { return value; }

  inline GPRange valueRange() const { return GPRange(value, value); }

  double key, value;
};
Q_DECLARE_TYPEINFO(GPGraphData, Q_PRIMITIVE_TYPE);


/*! \typedef GPGraphDataContainer

  Container for storing \ref GPGraphData points. The data is stored sorted by \a key.

  This template instantiation is the container in which GPGraph holds its data. For details about
  the generic container, see the documentation of the class template \ref GPDataContainer.

  \see GPGraphData, GPGraph::setData
*/
typedef GPDataContainer<GPGraphData> GPGraphDataContainer;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// GPCurveData

class GP_LIB_DECL GPCurveData
{
public:
  GPCurveData();
  GPCurveData(const GPCurveData& other) = default;
  GPCurveData& operator=(const GPCurveData& other) = default;
  GPCurveData(double t, double key, double value);

  inline double sortKey() const { return t; }
  inline static GPCurveData fromSortKey(double sortKey) { return GPCurveData(sortKey, 0, 0); }
  inline static bool sortKeyIsMainKey() { return false; }

  inline double mainKey() const { return key; }
  inline double mainValue() const { return value; }

  inline GPRange valueRange() const { return GPRange(value, value); }

  double t, key, value;
};
Q_DECLARE_TYPEINFO(GPCurveData, Q_PRIMITIVE_TYPE);

/*! \typedef GPCurveDataContainer

  Container for storing \ref GPCurveData points. The data is stored sorted by \a t, so the \a
  sortKey() (returning \a t) is different from \a mainKey() (returning \a key).

  This template instantiation is the container in which GPCurve holds its data. For details about
  the generic container, see the documentation of the class template \ref GPDataContainer.

  \see GPCurveData, GPCurve::setData
*/
typedef GPDataContainer<GPCurveData> GPCurveDataContainer;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// GPBarsData

class GP_LIB_DECL GPBarsData
{
public:
  GPBarsData();
  GPBarsData(double key, double value);

  inline double sortKey() const { return key; }
  inline static GPBarsData fromSortKey(double sortKey) { return GPBarsData(sortKey, 0); }
  inline static bool sortKeyIsMainKey() { return true; }

  inline double mainKey() const { return key; }
  inline double mainValue() const { return value; }

  inline GPRange valueRange() const { return GPRange(value, value); } // note that bar base value isn't held in each GPBarsData and thus can't/shouldn't be returned here

  double key, value;
};
Q_DECLARE_TYPEINFO(GPBarsData, Q_PRIMITIVE_TYPE);


/*! \typedef GPBarsDataContainer

  Container for storing \ref GPBarsData points. The data is stored sorted by \a key.

  This template instantiation is the container in which GPBars holds its data. For details about
  the generic container, see the documentation of the class template \ref GPDataContainer.

  \see GPBarsData, GPBars::setData
*/
typedef GPDataContainer<GPBarsData> GPBarsDataContainer;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// GPFinancialData

class GP_LIB_DECL GPFinancialData
{
public:
  GPFinancialData();
  GPFinancialData(double key, double open, double high, double low, double close);

  inline double sortKey() const { return key; }
  inline static GPFinancialData fromSortKey(double sortKey) { return GPFinancialData(sortKey, 0, 0, 0, 0); }
  inline static bool sortKeyIsMainKey() { return true; }

  inline double mainKey() const { return key; }
  inline double mainValue() const { return open; }

  inline GPRange valueRange() const { return GPRange(low, high); } // open and close must lie between low and high, so we don't need to check them

  double key, open, high, low, close;
};
Q_DECLARE_TYPEINFO(GPFinancialData, Q_PRIMITIVE_TYPE);


/*! \typedef GPFinancialDataContainer

  Container for storing \ref GPFinancialData points. The data is stored sorted by \a key.

  This template instantiation is the container in which GPFinancial holds its data. For details
  about the generic container, see the documentation of the class template \ref GPDataContainer.

  \see GPFinancialData, GPFinancial::setData
*/
typedef GPDataContainer<GPFinancialData> GPFinancialDataContainer;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// GPErrorBarsData

class GP_LIB_DECL GPErrorBarsData
{
public:
  GPErrorBarsData();
  explicit GPErrorBarsData(double error);
  GPErrorBarsData(double errorMinus, double errorPlus);

  double errorMinus, errorPlus;
};
Q_DECLARE_TYPEINFO(GPErrorBarsData, Q_PRIMITIVE_TYPE);


/*! \typedef GPErrorBarsDataContainer

  Container for storing \ref GPErrorBarsData points. It is a typedef for <tt>QVector<\ref
  GPErrorBarsData></tt>.

  This is the container in which \ref GPErrorBars holds its data. Unlike most other data
  containers for plottables, it is not based on \ref GPDataContainer. This is because the error
  bars plottable is special in that it doesn't store its own key and value coordinate per error
  bar. It adopts the key and value from the plottable to which the error bars shall be applied
  (\ref GPErrorBars::setDataPlottable). So the stored \ref GPErrorBarsData doesn't need a
  sortable key, but merely an index (as \c QVector provides), which maps one-to-one to the indices
  of the other plottable's data.

  \see GPErrorBarsData, GPErrorBars::setData
*/
typedef QVector<GPErrorBarsData> GPErrorBarsDataContainer;

class GP_LIB_DECL GPStatisticalBoxData
{
public:
  GPStatisticalBoxData();
  GPStatisticalBoxData(double key, double minimum, double lowerQuartile, double median, double upperQuartile, double maximum, const QVector<double>& outliers = QVector<double>());

  inline double sortKey() const { return key; }
  inline static GPStatisticalBoxData fromSortKey(double sortKey) { return GPStatisticalBoxData(sortKey, 0, 0, 0, 0, 0); }
  inline static bool sortKeyIsMainKey() { return true; }

  inline double mainKey() const { return key; }
  inline double mainValue() const { return median; }

  inline GPRange valueRange() const
  {
    GPRange result(minimum, maximum);
    for (QVector<double>::const_iterator it = outliers.constBegin(); it != outliers.constEnd(); ++it)
      result.expand(*it);
    return result;
  }

  double key, minimum, lowerQuartile, median, upperQuartile, maximum;
  QVector<double> outliers;
};
Q_DECLARE_TYPEINFO(GPStatisticalBoxData, Q_MOVABLE_TYPE);


/*! \typedef GPStatisticalBoxDataContainer

Container for storing \ref GPStatisticalBoxData points. The data is stored sorted by \a key.

This template instantiation is the container in which GPStatisticalBox holds its data. For
details about the generic container, see the documentation of the class template \ref
GPDataContainer.

\see GPStatisticalBoxData, GPStatisticalBox::setData
*/
typedef GPDataContainer<GPStatisticalBoxData> GPStatisticalBoxDataContainer;
