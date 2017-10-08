#include "GroupsEqualLengthSet.hpp"
#include "GroupsEqualLength.hpp"
#include <cmath>
#include <sstream>
#include <functional>
#include <queue>
#include <vector>
#include <fstream>
#include <iostream>
#include <boost/algorithm/string.hpp>
#include "TimeSeries.hpp"
#include "TimeSeriesSet.hpp"
#include "distance/Distance.hpp"
#include "Group.hpp"

using std::vector;
using std::max;
using std::min;
using std::cout;
using std::endl;
using std::ofstream;
using std::ifstream;
using std::string;

namespace genex {

void GroupsEqualLengthSet::reset(void)
{
  for (unsigned int i = 0; i < this->groupsEqualLength.size(); i++) {
    delete this->groupsEqualLength[i];
    this->groupsEqualLength[i] = nullptr;
  }
  this->groupsEqualLength.clear();
}

void GroupsEqualLengthSet::loadDistance(const string& distance_name)
{
  this->distanceName = distance_name;
  this->pairwiseDistance = getDistance(distance_name);
  if (distance_name == "euclidean") {
    this->warpedDistance = cascadeDistance;
  }
  else {
    this->warpedDistance = getDistance(distance_name + DTW_SUFFIX);
  }  
}

int GroupsEqualLengthSet::group(const string& distance_name, data_t threshold)
{
  reset();
  this->loadDistance(distance_name);
  this->groupsEqualLength.resize(dataset.getItemLength() + 1, nullptr);

  int numberOfGroups = 0;

  for (unsigned int i = 2; i < this->groupsEqualLength.size(); i++)
  {
    this->groupsEqualLength[i] = new GroupsEqualLength(dataset, i);
    int noOfGenerated = this->groupsEqualLength[i]->generateGroups(this->pairwiseDistance, threshold);
    numberOfGroups += noOfGenerated;
  }
  return numberOfGroups;
}

candidate_time_series_t GroupsEqualLengthSet::getBestMatch(const TimeSeries& query)
{
  if (query.getLength() <= 1) {
    throw GenexException("Length of query must be larger than 1");
  }
  data_t bestSoFarDist = INF;
  const Group* bestSoFarGroup = nullptr;

  vector<int> order (generateTraverseOrder(query.getLength(), this->groupsEqualLength.size() - 1));
  for (unsigned int io = 0; io < order.size(); io++) {
    int i = order[io];
    // this looks through each group of a certain length finding the best of those groups
    candidate_group_t candidate = this->groupsEqualLength[i]->getBestGroup(query, this->warpedDistance, bestSoFarDist);
    if (candidate.second < bestSoFarDist)
    {
      bestSoFarGroup = candidate.first;
      bestSoFarDist = candidate.second;
    }
  }
  return bestSoFarGroup->getBestMatch(query, this->warpedDistance);
}

bool GroupsEqualLengthSet::grouped(void) const
{
  return groupsEqualLength.size() > 0;
}

vector<TimeSeries> GroupsEqualLengthSet::kNN(const TimeSeries& data, int k)
{
  vector<TimeSeries> best;
  vector<group_index_t> bestSoFar;
  int kPrime = k;
  
  // process each group of a certain length keeping top sum-k groups
  for (unsigned int i = 2; i < this->groupsEqualLength.size(); i++)
  {
    kPrime = this->groupsEqualLength[i]->
        interLevelKNN(data, this->warpedDistance, &bestSoFar, kPrime);
  }
  
  // process top group directly
  if (!bestSoFar.empty()) 
  {
    group_index_t g = bestSoFar.front();
    bestSoFar.erase(bestSoFar.begin());
    vector<candidate_time_series_t> intraResults = 
        this->groupsEqualLength[g.length]->
            getGroup(g.index)->intraGroupKNN(data, kPrime+g.members, this->warpedDistance);
    // add all of the worst's best to answer
    for (int i = 0; i < intraResults.size(); ++i) 
    {
      best.push_back(intraResults[i].data);
    }
  }
  
  // add all timeseries in the *better* groups 
  for (int i = 0; i < bestSoFar.size(); i++)
  {
    group_index_t g = bestSoFar[i];  
    vector<TimeSeries> members = 
        this->groupsEqualLength[g.length]->getGroup(g.index)->getMembers();
    best.insert(std::end(best), std::begin(members), std::end(members));  
  }

  // clean up
  return best;
}

void GroupsEqualLengthSet::saveGroups(ofstream &fout, bool groupSizeOnly) const
{
  // Range of lengths and distance name
  fout << 2 << " " << this->groupsEqualLength.size() << endl;
  fout << this->distanceName << endl;
  for (unsigned int i = 2; i < this->groupsEqualLength.size(); i++) {
    this->groupsEqualLength[i]->saveGroups(fout, groupSizeOnly);
  }
}

int GroupsEqualLengthSet::loadGroups(ifstream &fin)
{
  reset();

  int lenFrom, lenTo;
  int numberOfGroups = 0;
  string distance;
  fin >> lenFrom >> lenTo >> distance;
  boost::trim_right(distance);
  this->loadDistance(distance);
  this->groupsEqualLength.resize(dataset.getItemLength() + 1, nullptr);  
  for (unsigned int i = lenFrom; i < lenTo; i++) {
    GroupsEqualLength* gel = new GroupsEqualLength(dataset, i);
    numberOfGroups += gel->loadGroups(fin);
    this->groupsEqualLength[i] = gel;
  }
  return numberOfGroups;
}

vector<int> generateTraverseOrder(int queryLength, int totalLength)
{
  vector<int> order;
  int low = queryLength - 1;
  int high = queryLength + 1;
  bool lowStop = false, highStop = false;

  order.push_back(queryLength);
  while (!(lowStop && highStop)) {
    if (low < 2) lowStop = true;
    if (high > totalLength) highStop = true;

    if (!lowStop) {
      // queryLength is always larger than low
      int r = calculateWarpingBandSize(queryLength);
      if (low + r >= queryLength) {
        order.push_back(low);
        low--;
      }
      else {
        lowStop = true;
      }
    }

    if (!highStop) {
      // queryLength is always smaller than high
      int r = calculateWarpingBandSize(high);
      if (queryLength + r >= high) {
        order.push_back(high);
        high++;
      }
      else {
        highStop = true;
      }
    }
  }
  return order;
}

} // namespace genex
