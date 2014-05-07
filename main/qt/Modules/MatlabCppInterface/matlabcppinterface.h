#ifndef MATLABCPPINTERFACE_H
#define MATLABCPPINTERFACE_H

#include "MatisseParameters.h"
#include "MosaicContext.h"
#include "mclmcrrt.h"
#include "mclcppclass.h"

using namespace MatisseCommon;

class MatlabCppInterface
{

public:

    static void MatlabCppInterface::mosaicDataMatlabToCpp (mwArray &mosaicData, MosaicData &mosaicDataCpp);
    static void MatlabCppInterface::mosaicDataCppToMatlab (MosaicData &mosaicDataCpp, mwArray &mosaicData);
    static void MatlabCppInterface::matchingDataMatlabToCpp (mwArray &matchingData, MatchingData &matchingDataCpp);
    static void MatlabCppInterface::matchingDataCppToMatlab (MatchingData &matchingDataCpp, mwArray &matchingData);
    static void MatlabCppInterface::generate_algo_param (MatisseCommon::MatisseParameters *mosaicParameters, mwArray &algo_param);

};

#endif // MATLABCPPINTERFACE_H
