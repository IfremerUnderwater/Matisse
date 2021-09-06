#include "stdvectoperations.h"
#include <algorithm>
#include <limits>
#include <math.h>

namespace basic_processing {

#ifdef WIN32
#ifndef round
inline double round(double number)
{
    return number < 0.0 ? ceil(number - 0.5) : floor(number + 0.5);
}
#endif
#endif

double doubleVectorMean(std::vector<double> & _v )
{
    double sum = 0.0;
    int n = _v.size();

    for ( int i=0; i < n; i++)
    {
        sum += _v[i];
    }

    return ( sum / (double)n);
}

double doubleVectorMedian(std::vector<double> _vec)
{
    if (_vec.size() < 1)
        return std::numeric_limits<double>::signaling_NaN();

    const auto alpha = _vec.begin();
    const auto omega = _vec.end();

    // Find the two middle positions (they will be the same if size is odd)
    const auto i1 = alpha + (_vec.size() - 1) / 2;
    const auto i2 = alpha + _vec.size() / 2;

    // Partial sort to place the correct elements at those indexes (it's okay to modify the vector,
    // as we've been given a copy; otherwise, we could use std::partial_sort_copy to populate a
    // temporary vector).
    std::nth_element(alpha, i1, omega);
    std::nth_element(i1, i2, omega);

    return 0.5 * (*i1 + *i2);
}

void doubleVectorScalarMult(std::vector<double> &_v, double _alpha)
{

    for (unsigned int i=0; i < _v.size(); i++)
    {
        _v[i] *= _alpha;
    }

}

std::vector<int> integerQuantiles(std::vector<int> _v, std::vector<double> _quantiles)
{
    // sort vector
    sort(_v.begin(),_v.end());

    std::vector<int> quantiles_limits;

    for (unsigned int i=0; i<_quantiles.size(); i++)
        quantiles_limits.push_back( _v[round(_quantiles[i]*(double)_v.size())] );

    return quantiles_limits;

}

std::vector<double> doubleQuantiles(std::vector<double> _v, std::vector<double> _quantiles)
{
    // sort vector
    sort(_v.begin(), _v.end());

    std::vector<double> quantiles_limits;

    for (unsigned int i = 0; i < _quantiles.size(); i++)
        quantiles_limits.push_back(_v[round(_quantiles[i] * (double)_v.size())]);

    return quantiles_limits;

}

} // namespace basic_processing
