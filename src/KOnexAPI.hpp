#ifndef GENEX_API_H
#define GENEX_API_H

#include <vector>
#include <string>

#include "GroupableTimeSeriesSet.hpp"
#include "TimeSeries.hpp"

using std::string;
using std::vector;

namespace konex {

/**
 * A struct holding general information of a dataset
 */
struct dataset_info_t
{
  dataset_info_t() : name(""), id(-1), itemCount(0),
    itemLength(0), isGrouped(false), isNormalized(false) {}
  dataset_info_t(int id, string name, int itemCount, int itemLength,
                 bool isGrouped, bool isNormalized) :
    name(name), id(id), itemCount(itemCount), itemLength(itemLength),
    isGrouped(isGrouped), isNormalized(isNormalized) {}

  string name;
  int id;
  int itemCount;
  int itemLength;
  bool isGrouped;
  bool isNormalized;
};

/**
 * A struct holding general information of a distance metric
 */
struct distance_info_t
{
  distance_info_t(string name, string description) :
    name(name), description(description) {}
  string name;
  string description;
};

class KOnexAPI
{
public:

  /**
   *  @brief destructor
   */
  ~KOnexAPI();

  /**
   *  @brief loads data from a text file to the memory
   *
   *  Values in the text file have to be arranged in a table format. The number of
   *  values (a.k.a columns) is inferred from the first line. All lines must have
   *  the same number of columns. If the number of lines exceeds maxNumRow, only
   *  maxNumRow lines are read and the rest is discarded. On the other hand, if
   *  maxNumRow is larger than or equal to the actual number of lines, or maxNumRow is
   *  not positive all lines are read.
   *
   *  @param filePath path to a text file
   *  @param maxNumRow maximum number of rows to be read. If this value is not positive,
   *         all lines are read
   *  @param separator a string containings possible separator characters for values
   *         in a line
   *  @param startCol columns before startCol are discarded
   *  @return an index used to refer to the just loaded dataset
   *
   *  @throw KOnexException if cannot read from the given file
   */
  dataset_info_t loadDataset(const string& filePath, int maxNumRow,
                             int startCol, const string& separators);

  void saveDataset(int index, const string& filePath, char separator);                           

  /**
   *  @brief unloads a dataset at given index
   *
   *  @param index index of the dataset being unloaded
   */
  void unloadDataset(int index);

  /**
   *  @brief unloads all datasets
   */
  void unloadAllDataset();

  /**
   *  @return number of loaded datasets
   */
  int getDatasetCount();

  /**
   *  @brief gets information of a dataset
   *
   *  @param index index of the dataset
   *  @return a dataset_info_t struct containing information about the dataset
   */
  dataset_info_t getDatasetInfo(int index);

  /**
   *  @return a vector of information of loaded dataset
   */
  vector<dataset_info_t> getAllDatasetInfo();

  /**
   *  @brief normalizes the dataset
   *
   *  Each value in the dataset is transformed by the following formula:
   *    d = (d - min) / (max - min)
   *  Where min and max are respectively the minimum and maximum values
   *  across the whole dataset.
   *
   *  @param the index of the dataset to be normalized
   *  @return a pair (min, max) - the minimum and maximum value across
   *          the whole dataset before being normalized.
   */
  std::pair<data_t, data_t> normalizeDataset(int idx);

  /**
   *  @brief groups the dataset
   *
   *  @param the index of the dataset to be grouped
   *  @param threshold the threshold to use when creating the group
   *  @param distance_name the distance to use when grouping the data
   *  @return the number of groups created
   */
  int groupDataset(int idx, data_t threshold, const string& distance_name, int numThreads = 1);

  void saveGroup(int idx, const string& path, bool groupSizeOnly);
  int loadGroup(int idx, const string& path);

  void setWarpingBandRatio(double ratio);

  /**
   *  @brief gets the best match in a dataset
   *
   *  @param result_idx the index of the result dataset
   *  @param query_idx the index of the query dataset
   *  @param index the index of the timeseries in the query dataset
   *  @param start the start of the index
   *  @param end the end of the index
   *  @return best match in the dataset
   */
  candidate_time_series_t getBestMatch(
      int result_idx, int query_idx, int index, int start = -1, int end = -1);


  /**
   *  @brief gets k similar TimeSeries to the query. Provides a bound of dist
   *  to the query for each result. The distance is <= the dist provided.
   * 
   *  @param k the number of similar time series to find
   *  @param h the number of time series to examine
   *  @param result_idx the index of the result dataset
   *  @param query_idx the index of the query dataset
   *  @param index the index of the timeseries in the query dataset
   *  @param start the start of the index
   *  @param end the end of the index
   *  @param approx if true, return the approximated distance, otherwise return the exact distance
   *  @return k similar time series
   */
  std::vector<candidate_time_series_t> kSim(
    int k, int h, int result_idx, int query_idx, int index, int start = -1, int end = -1);

 /**
   *  @brief gets k similar TimeSeries to the query, exhaustively.
   *  Provides the exact distance.
   * 
   *  @param k the number of similar time series to find
   *  @param result_idx the index of the result dataset
   *  @param query_idx the index of the query dataset
   *  @param index the index of the timeseries in the query dataset
   *  @param start the start of the index
   *  @param end the end of the index
   *  @return k similar time series
   */
  std::vector<candidate_time_series_t> kSimRaw(
    int k, int result_idx, int query_idx, int index, int start = -1, int end = -1, int PAABlockSize = 0);

  dataset_info_t PAA(int idx, int n);

  data_t distanceBetween(int ds1, int idx1, int start1, int end1,
                         int ds2, int idx2, int start2, int end2,
                         const std::string& distance_name);
  
  void printTS(int ds, int idx, int start, int end);

private:
  void _checkDatasetIndex(int index);

  vector<GroupableTimeSeriesSet*> loadedDatasets;
  int datasetCount = 0;
};

} // namespace konex

#endif // GENEX_API_H