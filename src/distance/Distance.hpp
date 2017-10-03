#ifndef GENERAL_DISTANCE_H
#define GENERAL_DISTANCE_H

#include <vector>
#include <algorithm>
#include "TimeSeries.hpp"
#include "Exception.hpp"

#define DTW_SUFFIX "_dtw"

#define NEW_DISTANCE(_class, _type) \
  pairwiseDistance<_class, _type>,  \
  warpedDistance<_class, _type>

#define NEW_DISTANCE_NAME(_name) #_name, #_name"_dtw"

using std::min;
using std::make_pair;
using std::string;
using std::vector;
using std::pair;

namespace genex {

typedef data_t (*dist_t)(const TimeSeries&, const TimeSeries&, data_t);

extern double warpingBandRatio;

/**
 *  @brief returns the an object representing a distance metric
 *
 *  @param distance_name name of a distance metric
 *  @return an object containing methods of the requested distance metric
 *  @throw GenexException if no distance with given name is found
 */
const dist_t getDistance(const string& distance_name);

/**
 *  @return a vector of names of available distances
 */
const vector<string>& getAllDistanceName();

/**
 *  Check if a class has the method InverseNorm using compile-time introspection.
 *  https://jguegant.github.io/blogs/tech/sfinae-introduction.html
 */
template <class T> struct hasInverseNorm
{
  template <typename U, U> struct helper{};
  template <typename C>
  static constexpr decltype(helper<data_t (C::*)(data_t, const TimeSeries&, const TimeSeries&) const,
                                   &C::inverseNorm>(),
                            bool()) test(int /* unused */)
  {
      return true;
  }

  template <typename C> static constexpr bool test(...)
  {
      return false;
  }

  // int is used to give the precedence!
  static constexpr bool value = test<T>(int());
};

/**
 *  @brief returns the warped distance between two sets of data
 *
 *  @param metric the distance metric to use
 *  @param a one of the two arrays of data
 *  @param b the other of the two arrays of data
 *  @param dropout drops the calculation of distance if within this
 */
template<typename DM, typename T>
data_t warpedDistance(const TimeSeries& a, const TimeSeries& b, data_t dropout)
{
  int m = a.getLength();
  int n = b.getLength();
  int r = calculateWarpingBandSize(min(m, n), warpingBandRatio);
  
  static DM* metric = new DM();

  // Fastpath for base intervals
  if (m == 1 && n == 1)
  {
    T result = metric->init();
    result = metric->reduce(result, result, a[0], b[0]);
    data_t normalizedResult = metric->normDTW(result, a, b);
    metric->clean(result);
    return normalizedResult;
  }

  // create cost matrix
  vector< vector< T >> cost(m, vector< T >(n));
  vector< vector< data_t >> ncost(m, vector<data_t>(n));

/*
  auto trace = new pair<data_t, data_t>*[m]; // For tracing warping
  for (int i = 0; i < m; i++)
  {
      trace[i] = new pair<data_t, data_t>[n];
  }
*/

  cost[0][0] = metric->init();
  cost[0][0] = metric->reduce(cost[0][0], cost[0][0], a[0], b[0]);
  ncost[0][0] = metric->normDTW(cost[0][0], a, b);

/*
  trace[0][0] = make_pair(-1, -1);
*/

  // calculate first column
  for(int i = 1; i < m; i++)
  {
    cost[i][0] = metric->init();
    cost[i][0] = metric->reduce(cost[i][0], cost[i-1][0], a[i], b[0]);
    ncost[i][0] = metric->normDTW(cost[i][0], a, b);
/*
    trace[i][0] = make_pair(i - 1, 0);
*/
  }

  // calculate first row
  for(int j = 1; j < n; j++)
  {
    cost[0][j] = metric->init();
    cost[0][j] = metric->reduce(cost[0][j], cost[0][j-1], a[0], b[j]);
    ncost[0][j] = metric->normDTW(cost[0][j], a, b);
/*
    trace[0][j] = make_pair(0, j - 1);
*/
  }

  data_t result;
  bool dropped = false;

  for(int i = 1; i < m; i++)
  {
    data_t bestSoFar = ncost[i][0];
    for(int j = 1; j < n; j++)
    {
      T minPrev = cost[i-1][j];
      if (ncost[i-1][j-1] < ncost[i][j-1] && ncost[i-1][j-1] < ncost[i-1][j])
      {
        minPrev = cost[i-1][j-1];
      }
      else if (ncost[i][j-1] < ncost[i-1][j])
      {
        minPrev = cost[i][j-1];
      }
      cost[i][j] = metric->init();
      cost[i][j] = metric->reduce(cost[i][j], minPrev, a[i], b[j]);
      ncost[i][j] = metric->normDTW(cost[i][j], a, b);
      bestSoFar = min(bestSoFar, ncost[i][j]);
    }

    if (bestSoFar > dropout)
    {
      dropped = true;
      break;
    }
  }

  if (dropped)
  {
    result = INF;
  }
  else
  {
    result = ncost[m - 1][n - 1];
  }

  for(int i = 1; i < m; i++)
  {
    for(int j = 1; j < n; j++)
    {
      metric->clean(cost[i][j]);
    }
  }

  return result;
}

/**
 * Calculates pairwise distance between two time series. This function is enabled if the given
 * distance metric class DM does not have the 'hasInverseNorm' function.
 */
template<typename DM, typename T>
typename std::enable_if<!hasInverseNorm<DM>::value, data_t>::type
pairwiseDistance(const TimeSeries& x_1, const TimeSeries& x_2, data_t dropout)
{
  if (x_1.getLength() != x_2.getLength())
  {
    throw GenexException("Two time series must have the same length for pairwise distance");
  }

  static DM* metric = nullptr;
  if (metric == nullptr)
  {
    metric = new DM();
  }

  T total = metric->init();

  bool dropped = false;

  for(int i = 0; i < x_1.getLength(); i++)
  {
    total = metric->reduce(total, total, x_1[i], x_2[i]);
    if (metric->norm(total, x_1, x_2) >= dropout)
    {
      dropped = true;
      break;
    }
  }

  data_t result = dropped ? INF : metric->norm(total, x_1, x_2);
  metric->clean(total);

  return result;
}

/**
 * Calculates pairwise distance between two time series. This function is enabled if the given
 * distance metric class DM has the 'hasInverseNorm' function.
 */
template<typename DM, typename T>
typename std::enable_if<hasInverseNorm<DM>::value, data_t>::type
pairwiseDistance(const TimeSeries& x_1, const TimeSeries& x_2, data_t dropout)
{
  if (x_1.getLength() != x_2.getLength())
  {
    throw GenexException("Two time series must have the same length for pairwise distance");
  }

  static DM* metric = nullptr;
  if (metric == nullptr)
  {
    metric = new DM();
  }

  T total = metric->init();

  bool dropped = false;
  dropout = metric->inverseNorm(dropout, x_1, x_2);

  for(int i = 0; i < x_1.getLength(); i++)
  {
    total = metric->reduce(total, total, x_1[i], x_2[i]);
    if (total >= dropout)
    {
      dropped = true;
      break;
    }
  }

  data_t result = dropped ? INF : metric->norm(total, x_1, x_2);
  metric->clean(total);

  return result;
}

void setWarpingBandRatio(double ratio);

/**
 * ...
 */
data_t keoghLowerBound(TimeSeries& a, TimeSeries& b, data_t dropout);

/**
 * ...
 */
data_t cascadeDistance(TimeSeries& a, TimeSeries& b, data_t dropout);

} // namespace genex

#endif //GENERAL_DISTANCE_H
