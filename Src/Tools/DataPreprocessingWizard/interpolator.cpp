#include "interpolator.h"
#include <algorithm>
#include <stdexcept>

Interpolator::Interpolator(const std::vector<std::pair<double, double>>& points)
  : _points(points) {
  //Defensive programming. Assume the caller has not sorted the table in
  //in ascending order
  std::sort(_points.begin(), _points.end());

  //Ensure that no 2 adjacent x values are equal,
  //lest we try to divide by zero when we interpolate.
  const double EPSILON{1.0E-8};
  for(std::size_t i=1; i<_points.size(); ++i) {
    double deltaX{std::abs(_points[i].first - _points[i-1].first)};
    if(deltaX < EPSILON ) {
      std::string err{"Potential Divide By Zero: Points " +
        std::to_string(i-1) + " And " +
        std::to_string(i) + " Are Too Close In Value"};
      throw std::range_error(err);
    }
  }
}

double Interpolator::findValue(double x) const {
  //Define a lambda that returns true if the x value
  //of a point pair is < the caller's x value
  auto lessThan =
      [](const std::pair<double, double>& point, double x)
      {return point.first < x;};

  //Find the first table entry whose value is >= caller's x value
  auto iter =
      std::lower_bound(_points.cbegin(), _points.cend(), x, lessThan);

  //If the caller's X value is greater than the largest
  //X value in the table, we can't interpolate.
  if(iter == _points.cend()) {
    return (_points.cend() - 1)->second;
  }

  //If the caller's X value is less than the smallest X value in the table,
  //we can't interpolate.
  if(iter == _points.cbegin() and x <= _points.cbegin()->first) {
    return _points.cbegin()->second;
  }

  //We can interpolate!
  double upperX{iter->first};
  double upperY{iter->second};
  double lowerX{(iter - 1)->first};
  double lowerY{(iter - 1)->second};

  double deltaY{upperY - lowerY};
  double deltaX{upperX - lowerX};

  return lowerY + ((x - lowerX)/ deltaX) * deltaY;
}
