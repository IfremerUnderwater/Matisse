#ifndef INTERPOLATOR_H_
#define INTERPOLATOR_H_

#include <utility>
#include <vector>

class Interpolator {
public:
  //On construction, we take in a vector of data point pairs
  //that represent the line we will use to interpolate
  Interpolator(const std::vector<std::pair<double, double>>&  points);

  //Computes the corresponding Y value
  //for X using linear interpolation
  double findValue(double x) const;

private:
  //Our container of (x,y) data points
  //std::pair::<double, double>.first = x value
  //std::pair::<double, double>.second = y value
  std::vector<std::pair<double, double>> _points;
};
#endif /* INTERPOLATOR_H_ */
