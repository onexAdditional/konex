#define BOOST_TEST_MODULE "Test General Distance Function"

#include <boost/test/unit_test.hpp>
#include <cmath>

#include "distance/Euclidean.hpp"
#include "distance/Chebyshev.hpp"
#include "distance/Manhattan.hpp"

#include "distance/Distance.hpp"
#include "Exception.hpp"

using namespace genex;

#define TOLERANCE 1e-9
struct MockData
{
  Euclidean* dist_1 = new Euclidean();
  Manhattan* dist_2 = new Manhattan();
  Chebyshev* dist_3 = new Chebyshev();

  data_t dat_1[5] = {1, 2, 3, 4, 5};
  data_t dat_2[5] = {11, 2, 3, 4, 5};

  data_t dat_3[2] = {2, 4};
  data_t dat_4[5] = {2, 2, 2, 4, 4};

  data_t dat_5[4] = {1, 2, 2, 4};
  data_t dat_6[4] = {1, 2, 4, 5};

  data_t dat_7[4] = {2, 2, 2, 2};
  data_t dat_8[4] = {20, 20, 20, 15};

  data_t dat_9[6]  = {2, 2, 2, 2, 2, 2};
  data_t dat_10[6] = {4, 3, 3, 3, 3, 3};

  data_t dat_11[7] = {4, 3, 5, 3, 5, 3, 4};
  data_t dat_12[7] = {4, 3, 3, 1, 1, 3, 4};
};

BOOST_AUTO_TEST_CASE( general_distance, *boost::unit_test::tolerance(TOLERANCE) )
{
  MockData data;
  TimeSeries ts_1(data.dat_1, 0, 0, 5);
  TimeSeries ts_2(data.dat_2, 0, 0, 5);

  // data_t total_1 = distance::generalDistance(data.dist_1, ts_1, ts_2);
  // BOOST_TEST( total_1, 2.0 );

  data_t total_2 = distance::generalDistance(data.dist_2, ts_1, ts_2);
  BOOST_TEST( total_2, 2.0 );

  data_t total_3 = distance::generalDistance(data.dist_3, ts_1, ts_2);
  BOOST_TEST( total_3, 10.0 );
}

BOOST_AUTO_TEST_CASE( easy_general_warped_distance, *boost::unit_test::tolerance(TOLERANCE) )
{
  MockData data;
  TimeSeries ts_3{data.dat_3, 0, 0, 2};
  TimeSeries ts_4{data.dat_4, 0, 0, 5};

  TimeSeries ts_5{data.dat_5, 0, 0, 4};
  TimeSeries ts_6{data.dat_6, 0, 0, 4};

  TimeSeries ts_11{data.dat_11, 0, 0, 7};
  TimeSeries ts_12{data.dat_12, 0, 0, 7};

  Euclidean* dist_1 = new Euclidean();
  Manhattan* dist_2 = new Manhattan();
  Chebyshev* dist_3 = new Chebyshev();

  data_t total_1 = distance::generalWarpedDistance(data.dist_1, ts_3, ts_4, INF);
  BOOST_TEST( total_1 == 0.0 );

  data_t total_2 = distance::generalWarpedDistance(data.dist_2, ts_3, ts_4, INF);
  BOOST_TEST( total_2 == 0.0 );

  data_t total_3 = distance::generalWarpedDistance(data.dist_3, ts_3, ts_4, INF);
  BOOST_TEST( total_3 == 0.0 );

  data_t total_4 = distance::generalWarpedDistance(data.dist_1, ts_5, ts_6, INF);
  BOOST_TEST( total_4 == 1.0/4.0 );

  data_t total_5 = distance::generalWarpedDistance(data.dist_2, ts_5, ts_6, INF);
  BOOST_TEST( total_5 == 1.0/4.0 );

  data_t total_6 = distance::generalWarpedDistance(data.dist_3, ts_5, ts_6, INF);
  BOOST_TEST( total_6 == 1.0 );

  gDebug = true;
  data_t total_7 = distance::generalWarpedDistance(data.dist_1, ts_11, ts_12, INF);
  data_t result_7 = (sqrt(12.0)/7.0);
  BOOST_TEST( total_7 == result_7 );
  gDebug = false;
  data_t total_8 = distance::generalWarpedDistance(data.dist_2, ts_11, ts_12, INF);
  BOOST_TEST( total_8 == (8.0/7.0) );

  data_t total_9 = distance::generalWarpedDistance(data.dist_3, ts_11, ts_12, INF);
  BOOST_TEST( total_9 == (2.0) );
}

BOOST_AUTO_TEST_CASE( easy_gwd_dropout, *boost::unit_test::tolerance(TOLERANCE) )
{
  MockData data;
  TimeSeries ts_3{data.dat_3, 0, 0, 2};
  TimeSeries ts_4{data.dat_4, 0, 0, 5};

  TimeSeries ts_7{data.dat_7, 0, 0, 4};
  TimeSeries ts_8{data.dat_8, 0, 0, 4};

  data_t total_1 = distance::generalWarpedDistance(data.dist_1, ts_3, ts_4, 5);
  BOOST_TEST( total_1 == 0.0 );

  data_t total_2 = distance::generalWarpedDistance(data.dist_2, ts_3, ts_4, 5);
  BOOST_TEST( total_2 == 0.0 );

  data_t total_3 = distance::generalWarpedDistance(data.dist_3, ts_3, ts_4, 5);
  BOOST_TEST( total_3 == 0.0 );

  data_t total_4 = distance::generalWarpedDistance(data.dist_1, ts_7, ts_8, 5);
  BOOST_TEST( total_4 == INF );

  data_t total_5 = distance::generalWarpedDistance(data.dist_2, ts_7, ts_8, 5);
  BOOST_TEST( total_5 == INF );

  data_t total_6 = distance::generalWarpedDistance(data.dist_3, ts_7, ts_8, 5);
  BOOST_TEST( total_6 == INF );
}

BOOST_AUTO_TEST_CASE( gwd_different_distances, *boost::unit_test::tolerance(TOLERANCE) )
{
  MockData data;
  TimeSeries ts_9{data.dat_9, 0, 0, 6};
  TimeSeries ts_10{data.dat_10, 0, 0, 6};

  data_t total_1 = distance::generalWarpedDistance(data.dist_1, ts_9, ts_10, INF);
  BOOST_TEST( total_1 == 3.0/6.0 );

  data_t total_2 = distance::generalWarpedDistance(data.dist_2, ts_9, ts_10, INF);
  BOOST_TEST( total_2 == 7.0/6.0 );

  data_t total_3 = distance::generalWarpedDistance(data.dist_3, ts_9, ts_10, INF);
  BOOST_TEST( total_3 == 2.0 );
}

BOOST_AUTO_TEST_CASE( get_distance_metric, *boost::unit_test::tolerance(TOLERANCE) )
{
   const DistanceMetric * d = distance::getDistanceMetric("euclidean");
   Cache* cache = d->init();
   data_t a = d->dist(100.0, 110.0);
   BOOST_TEST( a == 100 );

   const DistanceMetric * d_2 = distance::getDistanceMetric("manhattan");
   Cache* cache_2 = d_2->init();
   data_t e = d_2->dist(100.0, 110.0);

   BOOST_TEST( e == 10 );
   delete cache_2;
}

BOOST_AUTO_TEST_CASE( distance_metric_not_found )
{
   BOOST_CHECK_THROW( distance::getDistanceMetric("oracle"), GenexException );
}