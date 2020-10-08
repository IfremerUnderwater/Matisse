#ifndef STDVECTOPERATIONS
#define STDVECTOPERATIONS

#include <vector>


using namespace std;

///
/// \brief doubleVector Mean compute the vector mean
/// \param _v input vector
/// \return mean value
///
double doubleVectorMean( vector<double> & _v );

///
/// \brief doubleVectorMedian compute the vector's median
/// \param _vec input vector
/// \return mean value
///
double doubleVectorMedian(std::vector<double> _vec);

///
/// \brief doubleVectorScalarMult Multiply a vector with a double
/// \param _v input vector modified on output
/// \param _alpha input double
///
void doubleVectorScalarMult(vector<double> & _v, double _alpha );

///
/// \brief integerQuantiles compute quantiles for integer vectors
/// \param _v input vector
/// \param _quantiles required quantiles list
/// \return return quantiles limits
///
vector<int> integerQuantiles(vector<int> _v, vector<double> _quantiles);

#endif // STDVECTOPERATIONS

