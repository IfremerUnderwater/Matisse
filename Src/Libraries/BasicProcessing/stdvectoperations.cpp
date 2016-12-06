#include "stdvectoperations.h"
#include <algorithm>


double doubleVectorMean(vector<double> & _v )
{
    double sum = 0.0;
    int n = _v.size();

    for ( int i=0; i < n; i++)
    {
        sum += _v[i];
    }

    return ( sum / (double)n);
}


void doubleVectorScalarMult(vector<double> &_v, double _alpha)
{

    for (unsigned int i=0; i < _v.size(); i++)
    {
        _v[i] *= _alpha;
    }

}

vector<int> integerQuantiles(vector<int> _v, vector<double> _quantiles)
{
    // sort vector
    sort(_v.begin(),_v.end());

    vector<int> quantiles_limits;

    for (unsigned int i=0; i<_quantiles.size(); i++)
        quantiles_limits.push_back( _v[round(_quantiles[i]*(double)_v.size())] );

    return quantiles_limits;

}
