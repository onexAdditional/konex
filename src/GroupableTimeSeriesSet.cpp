#include "GroupableTimeSeriesSet.hpp"
#include "GlobalGroupSpace.hpp"
#include "Exception.hpp"
#include "distance/Distance.hpp"

#include <fstream>

using std::ofstream;
using std::ifstream;
using std::cout;
using std::endl;

namespace genex {

GroupableTimeSeriesSet::~GroupableTimeSeriesSet()
{
  this->reset();
}

int GroupableTimeSeriesSet::groupAllLengths(const std::string& distance_name, data_t threshold)
{
  if (!this->isLoaded())
  {
    throw GenexException("No data to group");
  }

  // clear old groups
  reset();

  this->groupsAllLengthSet = new GroupsEqualLengthSet(*this);
  int cntGroups = this->groupsAllLengthSet->group(distance_name, threshold);
  this->threshold = threshold;
  return cntGroups;
}

bool GroupableTimeSeriesSet::isGrouped() const
{
  return this->groupsAllLengthSet != nullptr;
}

void GroupableTimeSeriesSet::reset()
{
  delete this->groupsAllLengthSet;
  this->groupsAllLengthSet = nullptr;
}

void GroupableTimeSeriesSet::saveGroups(const string& path, bool groupSizeOnly) const
{
  if (!this->isGrouped()) {
    throw GenexException("No group found");
  }

  ofstream fout(path);
  if (fout)
  {
    // Version of the file format, the threshold and the required dataset dimensions
    fout << GROUP_FILE_VERSION << " " 
         << this->threshold << " "
         << this->getItemCount() << " "
         << this->getItemLength() << endl;
    this->groupsAllLengthSet->saveGroups(fout, groupSizeOnly);
  }
  else
  {
    throw GenexException("Cannot open file");
  }
}

int GroupableTimeSeriesSet::loadGroups(const string& path)
{
  int numberOfGroups = 0;
  ifstream fin(path);
  if (fin)
  {
    int version, grpItemCount, grpItemLength;
    data_t threshold;
    fin >> version >> threshold >> grpItemCount >> grpItemLength;
    if (version != GROUP_FILE_VERSION)
    {
      throw GenexException("Incompatible file version");
    }
    if (grpItemCount != this->getItemCount())
    {
      throw GenexException("Incompatible item count");
    }
    if (grpItemLength != this->getItemLength())
    {
      throw GenexException("Incompatible item length");
    }
    cout << "Saved groups are compatible with the dataset" << endl;
    reset();
    this->threshold = threshold;
    this->groupsAllLengthSet = new GroupsEqualLengthSet(*this);
    numberOfGroups = this->groupsAllLengthSet->loadGroups(fin);
  }
  else
  {
    throw GenexException("Cannot open file");
  }
  return numberOfGroups;
}

candidate_time_series_t GroupableTimeSeriesSet::getBestMatch(const TimeSeries& query) const
{
  if (this->groupsAllLengthSet) //not nullptr
  {
    return this->groupsAllLengthSet->getBestMatch(query);
  }
  throw GenexException("Dataset is not grouped");
}

std::vector<TimeSeries> GroupableTimeSeriesSet::kNN(const TimeSeries& query, int k)
{
  if (this->groupsAllLengthSet) //not nullptr
  {
    return this->groupsAllLengthSet->kNN(query, k);
  }
  throw GenexException("Dataset is not grouped");
}


} // namespace genex