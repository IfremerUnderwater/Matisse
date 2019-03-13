#ifndef STDVECTOPERATIONS
#define STDVECTOPERATIONS

#include <vector>


using namespace std;

///
/// \brief doubleVector Mean compute the vector mean
/// \param v_p input vector
/// \return mean value
///
double doubleVectorMean( vector<double> & _v );

///
/// \brief doubleVectorScalarMult Multiply a vector with a double
/// \param v_p input vector modified on output
/// \param m_p input double
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

