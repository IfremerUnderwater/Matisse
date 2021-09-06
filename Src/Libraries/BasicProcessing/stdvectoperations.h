#ifndef BASIC_PROCESSING_STDVECTOPERATIONS_H_
#define BASIC_PROCESSING_STDVECTOPERATIONS_H_

#include <vector>


namespace basic_processing {

///
/// \brief doubleVector Mean compute the vector mean
/// \param _v input vector
/// \return mean value
///
double doubleVectorMean(std::vector<double> & _v );

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
void doubleVectorScalarMult(std::vector<double> & _v, double _alpha );

///
/// \brief integerQuantiles compute quantiles for integer vectors
/// \param _v input vector
/// \param _quantiles required quantiles list
/// \return return quantiles limits
///
std::vector<int> integerQuantiles(std::vector<int> _v, std::vector<double> _quantiles);

///
/// \brief doubleQuantiles compute quantiles for double vectors
/// \param _v input vector
/// \param _quantiles required quantiles list
/// \return return quantiles limits
///
std::vector<double> doubleQuantiles(std::vector<double> _v, std::vector<double> _quantiles);

} // namespace basic_processing

#endif // BASIC_PROCESSING_STDVECTOPERATIONS_H_

