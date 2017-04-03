#include "GroupableTimeSeriesSet.hpp"
#include "GroupsEqualLengthSet.hpp"
#include "Exception.hpp"

namespace genex {

int GroupableTimeSeriesSet::groupAllLengths(const DistanceMetric* metric, data_t threshold)
{
  if (!this->valid()) 
  {
    throw GenexException("No data to group.");
  }

  // clear old groups
  resetGrouping();

  this->groupsAllLengthSet = new GroupsEqualLengthSet(*this);
  int cntGroups = this->groupsAllLengthSet->group(metric, threshold);
    
  return cntGroups;
}

void GroupableTimeSeriesSet::resetGrouping(void)
{
  delete this->groupsAllLengthSet;
  this->groupsAllLengthSet = nullptr;
}

candidate_time_series_t GroupableTimeSeriesSet::getBestMatch(const TimeSeries& query, DistanceMetric* metric)
{
  if (this->groupsAllLengthSet) //not nullptr
  {
    return this->groupsAllLengthSet->getBestMatch(query, metric);
  }
  throw GenexException("Dataset is not grouped.");
}

} // namespace genex