#include "graphicsplot_data_structures.h"

//// UOM provider interface (shall be implemented in axis)
GPUomDescriptorProvider::GPUomDescriptorProvider()
{
}

GPUomDescriptorProvider::~GPUomDescriptorProvider()
{
  for (auto l : listeners)
  {
    l->uomProviderDestroyed();
  }
}

void GPUomDescriptorProvider::registerListener(GPUomDescriptorProviderListener* listener)
{
  listeners.insert(listener);
}

void GPUomDescriptorProvider::unregisterListener(GPUomDescriptorProviderListener* listener)
{
  listeners.remove(listener);
}

void GPUomDescriptorProvider::notifyUomChanged()
{
  for (auto l : listeners)
  {
    l->uomChanged();
  }
}

//// UOM provider listener interface (shall be implemented in data container)
GPUomDescriptorProviderListener::GPUomDescriptorProviderListener()
  : uomProvider(nullptr)
{
}

GPUomDescriptorProviderListener::~GPUomDescriptorProviderListener()
{
  if (uomProvider)
  {
    uomProvider->unregisterListener(this);
  }
}

void GPUomDescriptorProviderListener::setUomProvider(GPUomDescriptorProvider* provider)
{
  if (uomProvider != provider)
  {
    //// Unregister
    if (uomProvider)
    {
      uomProvider->unregisterListener(this);
    }

    //// Set
    uomProvider = provider;

    //// Register
    if (uomProvider)
    {
      uomProvider->registerListener(this);
      uomChanged();
    }
  }
}

GPUomDescriptorProvider* GPUomDescriptorProviderListener::getUomProvider() const
{
  return uomProvider;
}

const GPUomDescriptor& GPUomDescriptorProviderListener::getUomDescriptor() const
{
  return uomDescriptor;
}

void GPUomDescriptorProviderListener::uomChanged()
{
  GPUomDescriptor newUomDescriptor = uomProvider->getUomDescriptor();
  if (newUomDescriptor != uomDescriptor)
  {
    if (uomDescriptor.canConvert(newUomDescriptor))
    {
      applyUom(newUomDescriptor);
    }
    uomDescriptor = newUomDescriptor;
  }
}

void GPUomDescriptorProviderListener::uomProviderDestroyed()
{
  uomProvider = nullptr;
}

//// Helper for dual listener
GPUomDescriptorProviderListenerHelper::GPUomDescriptorProviderListenerHelper(GPUomDescriptorProviderDualListener* dual)
  : GPUomDescriptorProviderListener()
  , dualDelegate(dual)
{
}

GPUomDescriptorProviderListenerHelper::~GPUomDescriptorProviderListenerHelper()
{
}

void GPUomDescriptorProviderListenerHelper::applyUom(const GPUomDescriptor& descriptor)
{
  return dualDelegate->applyUom(this, descriptor);
}

//// Dual listener
GPUomDescriptorProviderDualListener::GPUomDescriptorProviderDualListener()
  : listenerKeys(this)
  , listenerValues(this)
{
}

GPUomDescriptorProviderDualListener::~GPUomDescriptorProviderDualListener()
{
}

void GPUomDescriptorProviderDualListener::setUomProvider(GPUomDescriptorProvider* providerKeys, GPUomDescriptorProvider* providerValues)
{
  listenerKeys.setUomProvider(providerKeys);
  listenerValues.setUomProvider(providerValues);
}

void GPUomDescriptorProviderDualListener::applyUom(GPUomDescriptorProviderListener* listener, const GPUomDescriptor& descriptor)
{
  if (listener == &listenerKeys)
  {
    applyUomForKeys(descriptor);
  }
  else if (listener == &listenerValues)
  {
    applyUomForValues(descriptor);
  }
}

//// uom transformations for GPCurveData
template<>
void GPDataContainer<GPCurveData>::applyUomForKeys(const GPUomDescriptor& source, const GPUomDescriptor& target, iterator from, iterator to)
{
  applyUomForFields(ListenerTypeKeys,
    {
      &GPCurveData::key
    },
    source,
    target,
    from,
    to);
}

template<>
void GPDataContainer<GPCurveData>::applyUomForValues(const GPUomDescriptor& source, const GPUomDescriptor& target, iterator from, iterator to)
{
  applyUomForFields(ListenerTypeValues,
    {
      &GPCurveData::value
    },
    source,
    target,
    from,
    to);
}

//// uom transformations for GPGraphData
template<>
void GPDataContainer<GPGraphData>::applyUomForKeys(const GPUomDescriptor& source, const GPUomDescriptor& target, iterator from, iterator to)
{
  applyUomForFields(ListenerTypeKeys,
    {
      &GPGraphData::key
    },
    source,
    target,
    from,
    to);
}

template<>
void GPDataContainer<GPGraphData>::applyUomForValues(const GPUomDescriptor& source, const GPUomDescriptor& target, iterator from, iterator to)
{
  applyUomForFields(ListenerTypeValues,
    {
      &GPGraphData::value
    },
    source,
    target,
    from, 
    to);
}

//// uom transformations for GPBarsData
template<>
void GPDataContainer<GPBarsData>::applyUomForKeys(const GPUomDescriptor& source, const GPUomDescriptor& target, iterator from, iterator to)
{
  applyUomForFields(ListenerTypeKeys,
    {
      &GPBarsData::key
    },
    source,
    target,
    from,
    to);
} 

template<>
void GPDataContainer<GPBarsData>::applyUomForValues(const GPUomDescriptor& source, const GPUomDescriptor& target, iterator from, iterator to)
{
  applyUomForFields(ListenerTypeValues,
    {
      &GPBarsData::value
    },
    source,
    target,
    from,
    to);
}

//// uom transformations for GPFinancialData
template<>
void GPDataContainer<GPFinancialData>::applyUomForKeys(const GPUomDescriptor& source, const GPUomDescriptor& target, iterator from, iterator to)
{
  applyUomForFields(ListenerTypeKeys,
    {
      &GPFinancialData::key
    },
    source,
    target,
    from,
    to);
}

template<>
void GPDataContainer<GPFinancialData>::applyUomForValues(const GPUomDescriptor& source, const GPUomDescriptor& target, iterator from, iterator to)
{
  applyUomForFields(ListenerTypeValues,
    {
      &GPFinancialData::open,
      &GPFinancialData::high,
      &GPFinancialData::low,
      &GPFinancialData::close
    },
    source,
    target,
    from,
    to);
}

//// uom transformations for GPStatisticalBoxData
template<>
void GPDataContainer<GPStatisticalBoxData>::applyUomForKeys(const GPUomDescriptor& source, const GPUomDescriptor& target, iterator from, iterator to)
{
  applyUomForFields(ListenerTypeKeys,
    {
      &GPStatisticalBoxData::key
    },
    source,
    target,
    from,
    to);
}

template<>
void GPDataContainer<GPStatisticalBoxData>::applyUomForValues(const GPUomDescriptor& source, const GPUomDescriptor& target, iterator from, iterator to)
{
  applyUomForFields(ListenerTypeValues,
    {
      &GPStatisticalBoxData::minimum,
      &GPStatisticalBoxData::lowerQuartile,
      &GPStatisticalBoxData::median, 
      &GPStatisticalBoxData::upperQuartile,
      &GPStatisticalBoxData::maximum
    },
    source,
    target,
    from,
    to);

  // @todo - deal with .outliers same way
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// GPRange
////////////////////////////////////////////////////////////////////////////////////////////////////
/*! \class GPRange
  \brief Represents the range an axis is encompassing.

  contains a \a lower and \a upper double value and provides convenience input, output and
  modification functions.

  \see GPAxis::setRange
*/

/* start of documentation of inline functions */

/*! \fn double GPRange::size() const

  Returns the size of the range, i.e. \a upper-\a lower
*/

/*! \fn double GPRange::center() const

  Returns the center of the range, i.e. (\a upper+\a lower)*0.5
*/

/*! \fn void GPRange::normalize()

  Makes sure \a lower is numerically smaller than \a upper. If this is not the case, the values are
  swapped.
*/

/*! \fn bool GPRange::contains(double value) const

  Returns true when \a value lies within or exactly on the borders of the range.
*/

/*! \fn GPRange &GPRange::operator+=(const double& value)

  Adds \a value to both boundaries of the range.
*/

/*! \fn GPRange &GPRange::operator-=(const double& value)

  Subtracts \a value from both boundaries of the range.
*/

/*! \fn GPRange &GPRange::operator*=(const double& value)

  Multiplies both boundaries of the range by \a value.
*/

/*! \fn GPRange &GPRange::operator/=(const double& value)

  Divides both boundaries of the range by \a value.
*/

/* end of documentation of inline functions */

/*!
  Minimum range size (\a upper - \a lower) the range changing functions will accept. Smaller
  intervals would cause errors due to the 11-bit exponent of double precision numbers,
  corresponding to a minimum magnitude of roughly 1e-308.

  \warning Do not use this constant to indicate "arbitrarily small" values in plotting logic (as
  values that will appear in the plot)! It is intended only as a bound to compare against, e.g. to
  prevent axis ranges from obtaining underflowing ranges.

  \see validRange, maxRange
*/
const double GPRange::minRange = 1e-280;

/*!
  Maximum values (negative and positive) the range will accept in range-changing functions.
  Larger absolute values would cause errors due to the 11-bit exponent of double precision numbers,
  corresponding to a maximum magnitude of roughly 1e308.

  \warning Do not use this constant to indicate "arbitrarily large" values in plotting logic (as
  values that will appear in the plot)! It is intended only as a bound to compare against, e.g. to
  prevent axis ranges from obtaining overflowing ranges.

  \see validRange, minRange
*/
const double GPRange::maxRange = 1e250;

/*!
  Constructs a range with \a lower and \a upper set to zero.
*/
GPRange::GPRange() :
  lower(0),
  upper(0)
{
}

/*! \overload

  Constructs a range with the specified \a lower and \a upper values.

  The resulting range will be normalized (see \ref normalize), so if \a lower is not numerically
  smaller than \a upper, they will be swapped.
*/
GPRange::GPRange(double lower, double upper) :
  lower(lower),
  upper(upper)
{
  normalize();
}

/*! \overload

  Expands this range such that \a otherRange is contained in the new range. It is assumed that both
  this range and \a otherRange are normalized (see \ref normalize).

  If this range contains NaN as lower or upper bound, it will be replaced by the respective bound
  of \a otherRange.

  If \a otherRange is already inside the current range, this function does nothing.

  \see expanded
*/
void GPRange::expand(const GPRange &otherRange)
{
  if (lower > otherRange.lower || qIsNaN(lower))
    lower = otherRange.lower;
  if (upper < otherRange.upper || qIsNaN(upper))
    upper = otherRange.upper;
}

/*! \overload

  Expands this range such that \a includeCoord is contained in the new range. It is assumed that
  this range is normalized (see \ref normalize).

  If this range contains NaN as lower or upper bound, the respective bound will be set to \a
  includeCoord.

  If \a includeCoord is already inside the current range, this function does nothing.

  \see expand
*/
void GPRange::expand(double includeCoord)
{
  if (lower > includeCoord || qIsNaN(lower))
    lower = includeCoord;
  if (upper < includeCoord || qIsNaN(upper))
    upper = includeCoord;
}


/*! \overload

  Returns an expanded range that contains this and \a otherRange. It is assumed that both this
  range and \a otherRange are normalized (see \ref normalize).

  If this range contains NaN as lower or upper bound, the returned range's bound will be taken from
  \a otherRange.

  \see expand
*/
GPRange GPRange::expanded(const GPRange &otherRange) const
{
  GPRange result = *this;
  result.expand(otherRange);
  return result;
}

/*! \overload

  Returns an expanded range that includes the specified \a includeCoord. It is assumed that this
  range is normalized (see \ref normalize).

  If this range contains NaN as lower or upper bound, the returned range's bound will be set to \a
  includeCoord.

  \see expand
*/
GPRange GPRange::expanded(double includeCoord) const
{
  GPRange result = *this;
  result.expand(includeCoord);
  return result;
}

/*!
  Returns this range, possibly modified to not exceed the bounds provided as \a lowerBound and \a
  upperBound. If possible, the size of the current range is preserved in the process.

  If the range shall only be bounded at the lower side, you can set \a upperBound to \ref
  GPRange::maxRange. If it shall only be bounded at the upper side, set \a lowerBound to -\ref
  GPRange::maxRange.
*/
GPRange GPRange::bounded(double lowerBound, double upperBound) const
{
  if (lowerBound > upperBound)
    qSwap(lowerBound, upperBound);

  GPRange result(lower, upper);
  if (result.lower < lowerBound)
  {
    result.lower = lowerBound;
    result.upper = lowerBound + size();
    if (result.upper > upperBound || qFuzzyCompare(size(), upperBound-lowerBound))
      result.upper = upperBound;
  } else if (result.upper > upperBound)
  {
    result.upper = upperBound;
    result.lower = upperBound - size();
    if (result.lower < lowerBound || qFuzzyCompare(size(), upperBound-lowerBound))
      result.lower = lowerBound;
  }

  return result;
}

GPRange GPRange::boundedSize(double minimum, double maximum) const
{
  GPRange result(lower, upper);
  if (result.size() < minimum)
  {
    result.lower = center() - minimum / 2.0;
    result.upper = center() + minimum / 2.0;
  }
  if (result.size() > maximum)
  {
    result.lower = center() + maximum / 2.0;
    result.upper = center() + minimum / 2.0;
  }
  return result;
}

/*!
  Returns a sanitized version of the range. Sanitized means for logarithmic scales, that
  the range won't span the positive and negative sign domain, i.e. contain zero. Further
  \a lower will always be numerically smaller (or equal) to \a upper.

  If the original range does span positive and negative sign domains or contains zero,
  the returned range will try to approximate the original range as good as possible.
  If the positive interval of the original range is wider than the negative interval, the
  returned range will only contain the positive interval, with lower bound set to \a rangeFac or
  \a rangeFac *\a upper, whichever is closer to zero. Same procedure is used if the negative interval
  is wider than the positive interval, this time by changing the \a upper bound.
*/
GPRange GPRange::sanitizedForLogScale() const
{
  double rangeFac = 1e-3;
  GPRange sanitizedRange(lower, upper);
  sanitizedRange.normalize();
  // can't have range spanning negative and positive values in log plot, so change range to fix it
  //if (qFuzzyCompare(sanitizedRange.lower+1, 1) && !qFuzzyCompare(sanitizedRange.upper+1, 1))
  if (fabs(sanitizedRange.lower) < std::numeric_limits<double>::epsilon() && fabs(sanitizedRange.upper) > std::numeric_limits<double>::epsilon())
  {
    // case lower is 0
    if (rangeFac < sanitizedRange.upper*rangeFac)
      sanitizedRange.lower = rangeFac;
    else
      sanitizedRange.lower = sanitizedRange.upper*rangeFac;
  } //else if (!qFuzzyCompare(lower+1, 1) && qFuzzyCompare(upper+1, 1))
  else if (fabs(sanitizedRange.lower) > std::numeric_limits<double>::epsilon() && fabs(sanitizedRange.upper) < std::numeric_limits<double>::epsilon())
  {
    // case upper is 0
    if (-rangeFac > sanitizedRange.lower*rangeFac)
      sanitizedRange.upper = -rangeFac;
    else
      sanitizedRange.upper = sanitizedRange.lower*rangeFac;
  } else if (sanitizedRange.lower < 0 && sanitizedRange.upper > 0)
  {
    // find out whether negative or positive interval is wider to decide which sign domain will be chosen
    if (-sanitizedRange.lower > sanitizedRange.upper)
    {
      // negative is wider, do same as in case upper is 0
      if (-rangeFac > sanitizedRange.lower*rangeFac)
        sanitizedRange.upper = -rangeFac;
      else
        sanitizedRange.upper = sanitizedRange.lower*rangeFac;
    } else
    {
      // positive is wider, do same as in case lower is 0
      if (rangeFac < sanitizedRange.upper*rangeFac)
        sanitizedRange.lower = rangeFac;
      else
        sanitizedRange.lower = sanitizedRange.upper*rangeFac;
    }
  }
  // due to normalization, case lower>0 && upper<0 should never occur, because that implies upper<lower
  return sanitizedRange;
}

/*!
  Returns a sanitized version of the range. Sanitized means for linear scales, that
  \a lower will always be numerically smaller (or equal) to \a upper.
*/
GPRange GPRange::sanitizedForLinScale() const
{
  GPRange sanitizedRange(lower, upper);
  sanitizedRange.normalize();
  return sanitizedRange;
}

/*!
  Checks, whether the specified range is within valid bounds, which are defined
  as GPRange::maxRange and GPRange::minRange.
  A valid range means:
  \li range bounds within -maxRange and maxRange
  \li range size above minRange
  \li range size below maxRange
*/
bool GPRange::validRange(double lower, double upper)
{
  return (lower > -maxRange &&
          upper < maxRange &&
          qAbs(lower-upper) > minRange &&
          qAbs(lower-upper) < maxRange &&
          !(lower > 0 && qIsInf(upper/lower)) &&
          !(upper < 0 && qIsInf(lower/upper)));
}

/*!
  \overload
  Checks, whether the specified range is within valid bounds, which are defined
  as GPRange::maxRange and GPRange::minRange.
  A valid range means:
  \li range bounds within -maxRange and maxRange
  \li range size above minRange
  \li range size below maxRange
*/
bool GPRange::validRange(const GPRange &range)
{
  return (range.lower > -maxRange &&
          range.upper < maxRange &&
          qAbs(range.lower-range.upper) > minRange &&
          qAbs(range.lower-range.upper) < maxRange &&
          !(range.lower > 0 && qIsInf(range.upper/range.lower)) &&
          !(range.upper < 0 && qIsInf(range.lower/range.upper)));
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// GPDataRange
////////////////////////////////////////////////////////////////////////////////////////////////////

/*! \class GPDataRange
  \brief Describes a data range given by begin and end index

  GPDataRange holds two integers describing the begin (\ref setBegin) and end (\ref setEnd) index
  of a contiguous set of data points. The end index points to the data point above the last data point that's part of
  the data range, similarly to the nomenclature used in standard iterators.

  Data Ranges are not bound to a certain plottable, thus they can be freely exchanged, created and
  modified. If a non-contiguous data set shall be described, the class \ref GPDataSelection is
  used, which holds and manages multiple instances of \ref GPDataRange. In most situations, \ref
  GPDataSelection is thus used.

  Both \ref GPDataRange and \ref GPDataSelection offer convenience methods to work with them,
  e.g. \ref bounded, \ref expanded, \ref intersects, \ref intersection, \ref adjusted, \ref
  contains. Further, addition and subtraction operators (defined in \ref GPDataSelection) can be
  used to join/subtract data ranges and data selections (or mixtures), to retrieve a corresponding
  \ref GPDataSelection.

  %GraphicsPlot's \ref dataselection "data selection mechanism" is based on \ref GPDataSelection and
  GPDataRange.

  \note Do not confuse \ref GPDataRange with \ref GPRange. A \ref GPRange describes an interval
  in floating point plot coordinates, e.g. the current axis range.
*/

/* start documentation of inline functions */

/*! \fn int GPDataRange::size() const

  Returns the number of data points described by this data range. This is equal to the end index
  minus the begin index.

  \see length
*/

/*! \fn int GPDataRange::length() const

  Returns the number of data points described by this data range. Equivalent to \ref size.
*/

/*! \fn void GPDataRange::setBegin(int begin)

  Sets the begin of this data range. The \a begin index points to the first data point that is part
  of the data range.

  No checks or corrections are made to ensure the resulting range is valid (\ref isValid).

  \see setEnd
*/

/*! \fn void GPDataRange::setEnd(int end)

  Sets the end of this data range. The \a end index points to the data point just above the last
  data point that is part of the data range.

  No checks or corrections are made to ensure the resulting range is valid (\ref isValid).

  \see setBegin
*/

/*! \fn bool GPDataRange::isValid() const

  Returns whether this range is valid. A valid range has a begin index greater or equal to 0, and
  an end index greater or equal to the begin index.

  \note Invalid ranges should be avoided and are never the result of any of GraphicsPlot's methods
  (unless they are themselves fed with invalid ranges). Do not pass invalid ranges to GraphicsPlot's
  methods. The invalid range is not inherently prevented in GPDataRange, to allow temporary
  invalid begin/end values while manipulating the range. An invalid range is not necessarily empty
  (\ref isEmpty), since its \ref length can be negative and thus non-zero.
*/

/*! \fn bool GPDataRange::isEmpty() const

  Returns whether this range is empty, i.e. whether its begin index equals its end index.

  \see size, length
*/

/*! \fn GPDataRange GPDataRange::adjusted(int changeBegin, int changeEnd) const

  Returns a data range where \a changeBegin and \a changeEnd were added to the begin and end
  indices, respectively.
*/

/* end documentation of inline functions */

/*!
  Creates an empty GPDataRange, with begin and end set to 0.
*/
GPDataRange::GPDataRange() :
  mBegin(0),
  mEnd(0)
{
}

/*!
  Creates a GPDataRange, initialized with the specified \a begin and \a end.

  No checks or corrections are made to ensure the resulting range is valid (\ref isValid).
*/
GPDataRange::GPDataRange(int begin, int end) :
  mBegin(begin),
  mEnd(end)
{
}

/*!
  Returns a data range that matches this data range, except that parts exceeding \a other are
  excluded.

  This method is very similar to \ref intersection, with one distinction: If this range and the \a
  other range share no intersection, the returned data range will be empty with begin and end set
  to the respective boundary side of \a other, at which this range is residing. (\ref intersection
  would just return a range with begin and end set to 0.)
*/
GPDataRange GPDataRange::bounded(const GPDataRange &other) const
{
  GPDataRange result(intersection(other));
  if (result.isEmpty()) // no intersection, preserve respective bounding side of otherRange as both begin and end of return value
  {
    if (mEnd <= other.mBegin)
      result = GPDataRange(other.mBegin, other.mBegin);
    else
      result = GPDataRange(other.mEnd, other.mEnd);
  }
  return result;
}

/*!
  Returns a data range that contains both this data range as well as \a other.
*/
GPDataRange GPDataRange::expanded(const GPDataRange &other) const
{
  return GPDataRange(qMin(mBegin, other.mBegin), qMax(mEnd, other.mEnd));
}

/*!
  Returns the data range which is contained in both this data range and \a other.

  This method is very similar to \ref bounded, with one distinction: If this range and the \a other
  range share no intersection, the returned data range will be empty with begin and end set to 0.
  (\ref bounded would return a range with begin and end set to one of the boundaries of \a other,
  depending on which side this range is on.)

  \see GPDataSelection::intersection
*/
GPDataRange GPDataRange::intersection(const GPDataRange &other) const
{
  GPDataRange result(qMax(mBegin, other.mBegin), qMin(mEnd, other.mEnd));
  if (result.isValid())
    return result;
  return GPDataRange();
}

/*!
  Returns whether this data range and \a other share common data points.

  \see intersection, contains
*/
bool GPDataRange::intersects(const GPDataRange &other) const
{
   return !( (mBegin > other.mBegin && mBegin >= other.mEnd) ||
             (mEnd <= other.mBegin && mEnd < other.mEnd) );
}

/*!
  Returns whether all data points described by this data range are also in \a other.

  \see intersects
*/
bool GPDataRange::contains(const GPDataRange &other) const
{
  return mBegin <= other.mBegin && mEnd >= other.mEnd;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// GPDataSelection
////////////////////////////////////////////////////////////////////////////////////////////////////

/*! \class GPDataSelection
  \brief Describes a data set by holding multiple GPDataRange instances

  GPDataSelection manages multiple instances of GPDataRange in order to represent any (possibly
  disjoint) set of data selection.

  The data selection can be modified with addition and subtraction operators which take
  GPDataSelection and GPDataRange instances, as well as methods such as \ref addDataRange and
  \ref clear. Read access is provided by \ref dataRange, \ref dataRanges, \ref dataRangeCount, etc.

  The method \ref simplify is used to join directly adjacent or even overlapping GPDataRange
  instances. GPDataSelection automatically simplifies when using the addition/subtraction
  operators. The only case when \ref simplify is left to the user, is when calling \ref
  addDataRange, with the parameter \a simplify explicitly set to false. This is useful if many data
  ranges will be added to the selection successively and the overhead for simplifying after each
  iteration shall be avoided. In this case, you should make sure to call \ref simplify after
  completing the operation.

  Use \ref enforceType to bring the data selection into a state complying with the constraints for
  selections defined in \ref GP::SelectionType.

  %GraphicsPlot's \ref dataselection "data selection mechanism" is based on GPDataSelection and
  GPDataRange.

  \section gpdataselection-iterating Iterating over a data selection

  As an example, the following code snippet calculates the average value of a graph's data
  \ref GPAbstractPlottable::selection "selection":

  \snippet documentation/doc-code-snippets/mainwindow.cpp gpdataselection-iterating-1

*/

/* start documentation of inline functions */

/*! \fn int GPDataSelection::dataRangeCount() const

  Returns the number of ranges that make up the data selection. The ranges can be accessed by \ref
  dataRange via their index.

  \see dataRange, dataPointCount
*/

/*! \fn QList<GPDataRange> GPDataSelection::dataRanges() const

  Returns all data ranges that make up the data selection. If the data selection is simplified (the
  usual state of the selection, see \ref simplify), the ranges are sorted by ascending data point
  index.

  \see dataRange
*/

/*! \fn bool GPDataSelection::isEmpty() const

  Returns true if there are no data ranges, and thus no data points, in this GPDataSelection
  instance.

  \see dataRangeCount
*/

/* end documentation of inline functions */

/*!
  Creates an empty GPDataSelection.
*/
GPDataSelection::GPDataSelection()
{
}

/*!
  Creates a GPDataSelection containing the provided \a range.
*/
GPDataSelection::GPDataSelection(const GPDataRange &range)
{
  mDataRanges.append(range);
}

/*!
  Returns true if this selection is identical (contains the same data ranges with the same begin
  and end indices) to \a other.

  Note that both data selections must be in simplified state (the usual state of the selection, see
  \ref simplify) for this operator to return correct results.
*/
bool GPDataSelection::operator==(const GPDataSelection &other) const
{
  if (mDataRanges.size() != other.mDataRanges.size())
    return false;
  for (int i=0; i<mDataRanges.size(); ++i)
  {
    if (mDataRanges.at(i) != other.mDataRanges.at(i))
      return false;
  }
  return true;
}

/*!
  Adds the data selection of \a other to this data selection, and then simplifies this data
  selection (see \ref simplify).
*/
GPDataSelection &GPDataSelection::operator+=(const GPDataSelection &other)
{
  mDataRanges << other.mDataRanges;
  simplify();
  return *this;
}

/*!
  Adds the data range \a other to this data selection, and then simplifies this data selection (see
  \ref simplify).
*/
GPDataSelection &GPDataSelection::operator+=(const GPDataRange &other)
{
  addDataRange(other);
  return *this;
}

/*!
  Removes all data point indices that are described by \a other from this data selection.
*/
GPDataSelection &GPDataSelection::operator-=(const GPDataSelection &other)
{
  for (int i=0; i<other.dataRangeCount(); ++i)
    *this -= other.dataRange(i);

  return *this;
}

/*!
  Removes all data point indices that are described by \a other from this data selection.
*/
GPDataSelection &GPDataSelection::operator-=(const GPDataRange &other)
{
  if (other.isEmpty() || isEmpty())
    return *this;

  simplify();
  int i=0;
  while (i < mDataRanges.size())
  {
    const int thisBegin = mDataRanges.at(i).begin();
    const int thisEnd = mDataRanges.at(i).end();
    if (thisBegin >= other.end())
      break; // since data ranges are sorted after the simplify() call, no ranges which contain other will come after this

    if (thisEnd > other.begin()) // ranges which don't fulfill this are entirely before other and can be ignored
    {
      if (thisBegin >= other.begin()) // range leading segment is encompassed
      {
        if (thisEnd <= other.end()) // range fully encompassed, remove completely
        {
          mDataRanges.removeAt(i);
          continue;
        } // only leading segment is encompassed, trim accordingly
        mDataRanges[i].setBegin(other.end());
      } else // leading segment is not encompassed
      {
        if (thisEnd <= other.end()) // only trailing segment is encompassed, trim accordingly
        {
          mDataRanges[i].setEnd(other.begin());
        } else // other lies inside this range, so split range
        {
          mDataRanges[i].setEnd(other.begin());
          mDataRanges.insert(i+1, GPDataRange(other.end(), thisEnd));
          break; // since data ranges are sorted (and don't overlap) after simplify() call, we're done here
        }
      }
    }
    ++i;
  }

  return *this;
}

/*!
  Returns the total number of data points contained in all data ranges that make up this data
  selection.
*/
int GPDataSelection::dataPointCount() const
{
  int result = 0;
  for (int i=0; i<mDataRanges.size(); ++i)
    result += mDataRanges.at(i).length();
  return result;
}

/*!
  Returns the data range with the specified \a index.

  If the data selection is simplified (the usual state of the selection, see \ref simplify), the
  ranges are sorted by ascending data point index.

  \see dataRangeCount
*/
GPDataRange GPDataSelection::dataRange(int index) const
{
  if (index >= 0 && index < mDataRanges.size())
  {
    return mDataRanges.at(index);
  } 
  qDebug() << Q_FUNC_INFO << "index out of range:" << index;
  return GPDataRange();
}

/*!
  Returns a \ref GPDataRange which spans the entire data selection, including possible
  intermediate segments which are not part of the original data selection.
*/
GPDataRange GPDataSelection::span() const
{
  if (isEmpty())
    return GPDataRange();
  return GPDataRange(mDataRanges.first().begin(), mDataRanges.last().end());
}

/*!
  Adds the given \a dataRange to this data selection. This is equivalent to the += operator but
  allows disabling immediate simplification by setting \a simplify to false. This can improve
  performance if adding a very large amount of data ranges successively. In this case, make sure to
  call \ref simplify manually, after the operation.
*/
void GPDataSelection::addDataRange(const GPDataRange &dataRange, bool simplify)
{
  mDataRanges.append(dataRange);
  if (simplify)
    this->simplify();
}

/*!
  Removes all data ranges. The data selection then contains no data points.

  \ref isEmpty
*/
void GPDataSelection::clear()
{
  mDataRanges.clear();
}

/*!
  Sorts all data ranges by range begin index in ascending order, and then joins directly adjacent
  or overlapping ranges. This can reduce the number of individual data ranges in the selection, and
  prevents possible double-counting when iterating over the data points held by the data ranges.

  This method is automatically called when using the addition/subtraction operators. The only case
  when \ref simplify is left to the user, is when calling \ref addDataRange, with the parameter \a
  simplify explicitly set to false.
*/
void GPDataSelection::simplify()
{
  // remove any empty ranges:
  for (int i=mDataRanges.size()-1; i>=0; --i)
  {
    if (mDataRanges.at(i).isEmpty())
      mDataRanges.removeAt(i);
  }
  if (mDataRanges.isEmpty())
    return;

  // sort ranges by starting value, ascending:
  std::sort(mDataRanges.begin(), mDataRanges.end(), lessThanDataRangeBegin);

  // join overlapping/contiguous ranges:
  for (int i = mDataRanges.size() - 1; i > 0; --i)
  {
    const auto& rl = mDataRanges[i - 1];
    const auto& rr = mDataRanges[i];
    if (rl.end() >= rr.begin()) // range i overlaps/joins with i-1, so expand range i-1 appropriately and remove range i from list
    {
      mDataRanges[i-1].setEnd(qMax(rl.end(), rr.end()));
      mDataRanges.removeAt(i);
    }
  }
}

/*!
  Makes sure this data selection conforms to the specified \a type selection type. Before the type
  is enforced, \ref simplify is called.

  Depending on \a type, enforcing means adding new data points that were previously not part of the
  selection, or removing data points from the selection. If the current selection already conforms
  to \a type, the data selection is not changed.

  \see GP::SelectionType
*/
void GPDataSelection::enforceType(GP::SelectionType type)
{
  simplify();
  switch (type)
  {
    case GP::stNone:
    {
      mDataRanges.clear();
      break;
    }
    case GP::stWhole:
    {
      // whole selection isn't defined by data range, so don't change anything (is handled in plottable methods)
      break;
    }
    case GP::stSingleData:
    {
      // reduce all data ranges to the single first data point:
      if (!mDataRanges.isEmpty())
      {
        if (mDataRanges.size() > 1)
          mDataRanges = QList<GPDataRange>() << mDataRanges.first();
        if (mDataRanges.first().length() > 1)
          mDataRanges.first().setEnd(mDataRanges.first().begin()+1);
      }
      break;
    }
    case GP::stDataRange:
    {
      mDataRanges = QList<GPDataRange>() << span();
      break;
    }
    case GP::stMultipleDataRanges:
    {
      // this is the selection type that allows all concievable combinations of ranges, so do nothing
      break;
    }
  }
}

/*!
  Returns true if the data selection \a other is contained entirely in this data selection, i.e.
  all data point indices that are in \a other are also in this data selection.

  \see GPDataRange::contains
*/
bool GPDataSelection::contains(const GPDataSelection &other) const
{
  if (other.isEmpty()) return false;

  int otherIndex = 0;
  int thisIndex = 0;
  while (thisIndex < mDataRanges.size() && otherIndex < other.mDataRanges.size())
  {
    if (mDataRanges.at(thisIndex).contains(other.mDataRanges.at(otherIndex)))
      ++otherIndex;
    else
      ++thisIndex;
  }
  return thisIndex < mDataRanges.size(); // if thisIndex ran all the way to the end to find a containing range for the current otherIndex, other is not contained in this
}

/*!
  Returns a data selection containing the points which are both in this data selection and in the
  data range \a other.

  A common use case is to limit an unknown data selection to the valid range of a data container,
  using \ref GPDataContainer::dataRange as \a other. One can then safely iterate over the returned
  data selection without exceeding the data container's bounds.
*/
GPDataSelection GPDataSelection::intersection(const GPDataRange &other) const
{
  GPDataSelection result;
  for (int i=0; i<mDataRanges.size(); ++i)
    result.addDataRange(mDataRanges.at(i).intersection(other), false);
  result.simplify();
  return result;
}

/*!
  Returns a data selection containing the points which are both in this data selection and in the
  data selection \a other.
*/
GPDataSelection GPDataSelection::intersection(const GPDataSelection &other) const
{
  GPDataSelection result;
  for (int i=0; i<other.dataRangeCount(); ++i)
    result += intersection(other.dataRange(i));
  result.simplify();
  return result;
}

/*!
  Returns a data selection which is the exact inverse of this data selection, with \a outerRange
  defining the base range on which to invert. If \a outerRange is smaller than the \ref span of
  this data selection, it is expanded accordingly.

  For example, this method can be used to retrieve all unselected segments by setting \a outerRange
  to the full data range of the plottable, and calling this method on a data selection holding the
  selected segments.
*/
GPDataSelection GPDataSelection::inverse(const GPDataRange &outerRange) const
{
  if (isEmpty())
    return GPDataSelection(outerRange);
  GPDataRange fullRange = outerRange.expanded(span());

  GPDataSelection result;
  // first unselected segment:
  if (mDataRanges.first().begin() != fullRange.begin())
    result.addDataRange(GPDataRange(fullRange.begin(), mDataRanges.first().begin()), false);
  // intermediate unselected segments:
  for (int i=1; i<mDataRanges.size(); ++i)
    result.addDataRange(GPDataRange(mDataRanges.at(i-1).end(), mDataRanges.at(i).begin()), false);
  // last unselected segment:
  if (mDataRanges.last().end() != fullRange.end())
    result.addDataRange(GPDataRange(mDataRanges.last().end(), fullRange.end()), false);
  result.simplify();
  return result;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// GPGraphData
////////////////////////////////////////////////////////////////////////////////////////////////////

/*! \class GPGraphData
  \brief Holds the data of one single data point for GPGraph.

  The stored data is:
  \li \a key: coordinate on the key axis of this data point (this is the \a mainKey and the \a sortKey)
  \li \a value: coordinate on the value axis of this data point (this is the \a mainValue)

  The container for storing multiple data points is \ref GPGraphDataContainer. It is a typedef for
  \ref GPDataContainer with \ref GPGraphData as the DataType template parameter. See the
  documentation there for an explanation regarding the data type's generic methods.

  \see GPGraphDataContainer
*/

/* start documentation of inline functions */

/*! \fn double GPGraphData::sortKey() const

  Returns the \a key member of this data point.

  For a general explanation of what this method is good for in the context of the data container,
  see the documentation of \ref GPDataContainer.
*/

/*! \fn static GPGraphData GPGraphData::fromSortKey(double sortKey)

  Returns a data point with the specified \a sortKey. All other members are set to zero.

  For a general explanation of what this method is good for in the context of the data container,
  see the documentation of \ref GPDataContainer.
*/

/*! \fn static static bool GPGraphData::sortKeyIsMainKey()

  Since the member \a key is both the data point key coordinate and the data ordering parameter,
  this method returns true.

  For a general explanation of what this method is good for in the context of the data container,
  see the documentation of \ref GPDataContainer.
*/

/*! \fn double GPGraphData::mainKey() const

  Returns the \a key member of this data point.

  For a general explanation of what this method is good for in the context of the data container,
  see the documentation of \ref GPDataContainer.
*/

/*! \fn double GPGraphData::mainValue() const

  Returns the \a value member of this data point.

  For a general explanation of what this method is good for in the context of the data container,
  see the documentation of \ref GPDataContainer.
*/

/*! \fn GPRange GPGraphData::valueRange() const

  Returns a GPRange with both lower and upper boundary set to \a value of this data point.

  For a general explanation of what this method is good for in the context of the data container,
  see the documentation of \ref GPDataContainer.
*/

/* end documentation of inline functions */

/*!
  Constructs a data point with key and value set to zero.
*/
GPGraphData::GPGraphData() :
  key(0),
  value(0)
{
}

/*!
  Constructs a data point with the specified \a key and \a value.
*/
GPGraphData::GPGraphData(double key, double value) :
  key(key),
  value(value)
{
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// GPCurveData
////////////////////////////////////////////////////////////////////////////////////////////////////

/*! \class GPCurveData
  \brief Holds the data of one single data point for GPCurve.

  The stored data is:
  \li \a t: the free ordering parameter of this curve point, like in the mathematical vector <em>(x(t), y(t))</em>. (This is the \a sortKey)
  \li \a key: coordinate on the key axis of this curve point (this is the \a mainKey)
  \li \a value: coordinate on the value axis of this curve point (this is the \a mainValue)

  The container for storing multiple data points is \ref GPCurveDataContainer. It is a typedef for
  \ref GPDataContainer with \ref GPCurveData as the DataType template parameter. See the
  documentation there for an explanation regarding the data type's generic methods.

  \see GPCurveDataContainer
*/

/* start documentation of inline functions */

/*! \fn double GPCurveData::sortKey() const

  Returns the \a t member of this data point.

  For a general explanation of what this method is good for in the context of the data container,
  see the documentation of \ref GPDataContainer.
*/

/*! \fn static GPCurveData GPCurveData::fromSortKey(double sortKey)

  Returns a data point with the specified \a sortKey (assigned to the data point's \a t member).
  All other members are set to zero.

  For a general explanation of what this method is good for in the context of the data container,
  see the documentation of \ref GPDataContainer.
*/

/*! \fn static static bool GPCurveData::sortKeyIsMainKey()

  Since the member \a key is the data point key coordinate and the member \a t is the data ordering
  parameter, this method returns false.

  For a general explanation of what this method is good for in the context of the data container,
  see the documentation of \ref GPDataContainer.
*/

/*! \fn double GPCurveData::mainKey() const

  Returns the \a key member of this data point.

  For a general explanation of what this method is good for in the context of the data container,
  see the documentation of \ref GPDataContainer.
*/

/*! \fn double GPCurveData::mainValue() const

  Returns the \a value member of this data point.

  For a general explanation of what this method is good for in the context of the data container,
  see the documentation of \ref GPDataContainer.
*/

/*! \fn GPRange GPCurveData::valueRange() const

  Returns a GPRange with both lower and upper boundary set to \a value of this data point.

  For a general explanation of what this method is good for in the context of the data container,
  see the documentation of \ref GPDataContainer.
*/

/* end documentation of inline functions */

/*!
  Constructs a curve data point with t, key and value set to zero.
*/
GPCurveData::GPCurveData() :
  t(0),
  key(0),
  value(0)
{
}

/*!
  Constructs a curve data point with the specified \a t, \a key and \a value.
*/
GPCurveData::GPCurveData(double t, double key, double value) :
  t(t),
  key(key),
  value(value)
{
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// GPBarsData
////////////////////////////////////////////////////////////////////////////////////////////////////

/*! \class GPBarsData
  \brief Holds the data of one single data point (one bar) for GPBars.

  The stored data is:
  \li \a key: coordinate on the key axis of this bar (this is the \a mainKey and the \a sortKey)
  \li \a value: height coordinate on the value axis of this bar (this is the \a mainValue)

  The container for storing multiple data points is \ref GPBarsDataContainer. It is a typedef for
  \ref GPDataContainer with \ref GPBarsData as the DataType template parameter. See the
  documentation there for an explanation regarding the data type's generic methods.

  \see GPBarsDataContainer
*/

/* start documentation of inline functions */

/*! \fn double GPBarsData::sortKey() const

  Returns the \a key member of this data point.

  For a general explanation of what this method is good for in the context of the data container,
  see the documentation of \ref GPDataContainer.
*/

/*! \fn static GPBarsData GPBarsData::fromSortKey(double sortKey)

  Returns a data point with the specified \a sortKey. All other members are set to zero.

  For a general explanation of what this method is good for in the context of the data container,
  see the documentation of \ref GPDataContainer.
*/

/*! \fn static static bool GPBarsData::sortKeyIsMainKey()

  Since the member \a key is both the data point key coordinate and the data ordering parameter,
  this method returns true.

  For a general explanation of what this method is good for in the context of the data container,
  see the documentation of \ref GPDataContainer.
*/

/*! \fn double GPBarsData::mainKey() const

  Returns the \a key member of this data point.

  For a general explanation of what this method is good for in the context of the data container,
  see the documentation of \ref GPDataContainer.
*/

/*! \fn double GPBarsData::mainValue() const

  Returns the \a value member of this data point.

  For a general explanation of what this method is good for in the context of the data container,
  see the documentation of \ref GPDataContainer.
*/

/*! \fn GPRange GPBarsData::valueRange() const

  Returns a GPRange with both lower and upper boundary set to \a value of this data point.

  For a general explanation of what this method is good for in the context of the data container,
  see the documentation of \ref GPDataContainer.
*/

/* end documentation of inline functions */

/*!
  Constructs a bar data point with key and value set to zero.
*/
GPBarsData::GPBarsData() :
  key(0),
  value(0)
{
}

/*!
  Constructs a bar data point with the specified \a key and \a value.
*/
GPBarsData::GPBarsData(double key, double value) :
  key(key),
  value(value)
{
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// GPFinancialData
////////////////////////////////////////////////////////////////////////////////////////////////////

/*! \class GPFinancialData
  \brief Holds the data of one single data point for GPFinancial.

  The stored data is:
  \li \a key: coordinate on the key axis of this data point (this is the \a mainKey and the \a sortKey)
  \li \a open: The opening value at the data point (this is the \a mainValue)
  \li \a high: The high/maximum value at the data point
  \li \a low: The low/minimum value at the data point
  \li \a close: The closing value at the data point

  The container for storing multiple data points is \ref GPFinancialDataContainer. It is a typedef
  for \ref GPDataContainer with \ref GPFinancialData as the DataType template parameter. See the
  documentation there for an explanation regarding the data type's generic methods.

  \see GPFinancialDataContainer
*/

/* start documentation of inline functions */

/*! \fn double GPFinancialData::sortKey() const

  Returns the \a key member of this data point.

  For a general explanation of what this method is good for in the context of the data container,
  see the documentation of \ref GPDataContainer.
*/

/*! \fn static GPFinancialData GPFinancialData::fromSortKey(double sortKey)

  Returns a data point with the specified \a sortKey. All other members are set to zero.

  For a general explanation of what this method is good for in the context of the data container,
  see the documentation of \ref GPDataContainer.
*/

/*! \fn static static bool GPFinancialData::sortKeyIsMainKey()

  Since the member \a key is both the data point key coordinate and the data ordering parameter,
  this method returns true.

  For a general explanation of what this method is good for in the context of the data container,
  see the documentation of \ref GPDataContainer.
*/

/*! \fn double GPFinancialData::mainKey() const

  Returns the \a key member of this data point.

  For a general explanation of what this method is good for in the context of the data container,
  see the documentation of \ref GPDataContainer.
*/

/*! \fn double GPFinancialData::mainValue() const

  Returns the \a open member of this data point.

  For a general explanation of what this method is good for in the context of the data container,
  see the documentation of \ref GPDataContainer.
*/

/*! \fn GPRange GPFinancialData::valueRange() const

  Returns a GPRange spanning from the \a low to the \a high value of this data point.

  For a general explanation of what this method is good for in the context of the data container,
  see the documentation of \ref GPDataContainer.
*/

/* end documentation of inline functions */

/*!
  Constructs a data point with key and all values set to zero.
*/
GPFinancialData::GPFinancialData() :
  key(0),
  open(0),
  high(0),
  low(0),
  close(0)
{
}

/*!
  Constructs a data point with the specified \a key and OHLC values.
*/
GPFinancialData::GPFinancialData(double key, double open, double high, double low, double close) :
  key(key),
  open(open),
  high(high),
  low(low),
  close(close)
{
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// GPErrorBarsData
////////////////////////////////////////////////////////////////////////////////////////////////////

/*! \class GPErrorBarsData
  \brief Holds the data of one single error bar for GPErrorBars.

  The stored data is:
  \li \a errorMinus: how much the error bar extends towards negative coordinates from the data
  point position
  \li \a errorPlus: how much the error bar extends towards positive coordinates from the data point
  position

  The container for storing the error bar information is \ref GPErrorBarsDataContainer. It is a
  typedef for <tt>QVector<\ref GPErrorBarsData></tt>.

  \see GPErrorBarsDataContainer
*/

/*!
  Constructs an error bar with errors set to zero.
*/
GPErrorBarsData::GPErrorBarsData() :
  errorMinus(0),
  errorPlus(0)
{
}

/*!
  Constructs an error bar with equal \a error in both negative and positive direction.
*/
GPErrorBarsData::GPErrorBarsData(double error) :
  errorMinus(error),
  errorPlus(error)
{
}

/*!
  Constructs an error bar with negative and positive errors set to \a errorMinus and \a errorPlus,
  respectively.
*/
GPErrorBarsData::GPErrorBarsData(double errorMinus, double errorPlus) :
  errorMinus(errorMinus),
  errorPlus(errorPlus)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// GPStatisticalBoxData
////////////////////////////////////////////////////////////////////////////////////////////////////

/*! \class GPStatisticalBoxData
\brief Holds the data of one single data point for GPStatisticalBox.

The stored data is:

\li \a key: coordinate on the key axis of this data point (this is the \a mainKey and the \a sortKey)

\li \a minimum: the position of the lower whisker, typically the minimum measurement of the
sample that's not considered an outlier.

\li \a lowerQuartile: the lower end of the box. The lower and the upper quartiles are the two
statistical quartiles around the median of the sample, they should contain 50% of the sample
data.

\li \a median: the value of the median mark inside the quartile box. The median separates the
sample data in half (50% of the sample data is below/above the median). (This is the \a mainValue)

\li \a upperQuartile: the upper end of the box. The lower and the upper quartiles are the two
statistical quartiles around the median of the sample, they should contain 50% of the sample
data.

\li \a maximum: the position of the upper whisker, typically the maximum measurement of the
sample that's not considered an outlier.

\li \a outliers: a QVector of outlier values that will be drawn as scatter points at the \a key
coordinate of this data point (see \ref GPStatisticalBox::setOutlierStyle)

The container for storing multiple data points is \ref GPStatisticalBoxDataContainer. It is a
typedef for \ref GPDataContainer with \ref GPStatisticalBoxData as the DataType template
parameter. See the documentation there for an explanation regarding the data type's generic
methods.

\see GPStatisticalBoxDataContainer
*/

/* start documentation of inline functions */

/*! \fn double GPStatisticalBoxData::sortKey() const

Returns the \a key member of this data point.

For a general explanation of what this method is good for in the context of the data container,
see the documentation of \ref GPDataContainer.
*/

/*! \fn static GPStatisticalBoxData GPStatisticalBoxData::fromSortKey(double sortKey)

Returns a data point with the specified \a sortKey. All other members are set to zero.

For a general explanation of what this method is good for in the context of the data container,
see the documentation of \ref GPDataContainer.
*/

/*! \fn static static bool GPStatisticalBoxData::sortKeyIsMainKey()

Since the member \a key is both the data point key coordinate and the data ordering parameter,
this method returns true.

For a general explanation of what this method is good for in the context of the data container,
see the documentation of \ref GPDataContainer.
*/

/*! \fn double GPStatisticalBoxData::mainKey() const

Returns the \a key member of this data point.

For a general explanation of what this method is good for in the context of the data container,
see the documentation of \ref GPDataContainer.
*/

/*! \fn double GPStatisticalBoxData::mainValue() const

Returns the \a median member of this data point.

For a general explanation of what this method is good for in the context of the data container,
see the documentation of \ref GPDataContainer.
*/

/*! \fn GPRange GPStatisticalBoxData::valueRange() const

Returns a GPRange spanning from the \a minimum to the \a maximum member of this statistical box
data point, possibly further expanded by outliers.

For a general explanation of what this method is good for in the context of the data container,
see the documentation of \ref GPDataContainer.
*/

/* end documentation of inline functions */

/*!
Constructs a data point with key and all values set to zero.
*/
GPStatisticalBoxData::GPStatisticalBoxData() :
  key(0),
  minimum(0),
  lowerQuartile(0),
  median(0),
  upperQuartile(0),
  maximum(0)
{
}

/*!
Constructs a data point with the specified \a key, \a minimum, \a lowerQuartile, \a median, \a
upperQuartile, \a maximum and optionally a number of \a outliers.
*/
GPStatisticalBoxData::GPStatisticalBoxData(double key, double minimum, double lowerQuartile, double median, double upperQuartile, double maximum, const QVector<double> &outliers) :
  key(key),
  minimum(minimum),
  lowerQuartile(lowerQuartile),
  median(median),
  upperQuartile(upperQuartile),
  maximum(maximum),
  outliers(outliers)
{
}

