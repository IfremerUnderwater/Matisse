#include "matlabcppinterface.h"

///
/// \brief MatlabCppInterface::mosaicDataMatlabToCpp converts matlab MosaicData to its C++ equivalent
/// \param mosaicData Matlab MosaicData structure
/// \param mosaicDataCpp C++ MosaicData structure
///
/*static*/ void MatlabCppInterface::mosaicDataMatlabToCpp (mwArray &mosaicData, MosaicData &mosaicDataCpp){

    // ********************* Complete mosaicData structure from matlab object *************************


    //Fill Init field
    mosaicDataCpp.init.mosaicOrigin.x = mosaicData.Get("init",1,1).Get("mosaic_origin",1,1).Get("x",1,1);
    mosaicDataCpp.init.mosaicOrigin.y = mosaicData.Get("init",1,1).Get("mosaic_origin",1,1).Get("y",1,1);

    mosaicDataCpp.init.pixelSize.x = mosaicData.Get("init",1,1).Get("pixel_size",1,1).Get("x",1,1);
    mosaicDataCpp.init.pixelSize.y = mosaicData.Get("init",1,1).Get("pixel_size",1,1).Get("y",1,1);

    mosaicDataCpp.init.mosaicSize.x = mosaicData.Get("init",1,1).Get("mosaic_size",1,1).Get("x",1,1);
    mosaicDataCpp.init.mosaicSize.y = mosaicData.Get("init",1,1).Get("mosaic_size",1,1).Get("y",1,1);

    double mosaic_ullr_value[4];
    mosaicData.Get("init",1,1).Get("mosaic_ullr",1,1).GetData(mosaic_ullr_value,4);
    mosaicDataCpp.init.mosaic_ullr = Mat(1,4,CV_64F,mosaic_ullr_value).clone();


    //Fill Nodes field
    double matrix3x3[9];
    double matrix1x6[6];
    Mat tempMat,tempMat2;
    std::string tempString;
    mwSize nodesNb;

    nodesNb = mosaicData.Get("nodes",1,1).NumberOfElements();
    MosaicNode currentNode;



    qDebug() << "There is " << nodesNb << "nodes in this mosaic";

    // Clear old data before pushing new data
    mosaicDataCpp.nodes.clear();

    for (int i=1; i<=nodesNb; i++){

        currentNode.imageIndex = mosaicData.Get("nodes",1,1).Get("index",nodesNb,i);

        // Fill homography field
        mosaicData.Get("nodes",1,1).Get("homo",nodesNb,i).Get("matrix",1,1).GetData(matrix3x3,9);
        tempMat= Mat(3,3,CV_64F,matrix3x3);

        transpose(tempMat, tempMat2); // Transpose for row-major conversion
        currentNode.homo.matrix = tempMat2.clone();

        tempString = mosaicData.Get("nodes",1,1).Get("homo",nodesNb,i).Get("model",1,1).ToString();
        currentNode.homo.model = QString(tempString.c_str());

        tempString = mosaicData.Get("nodes",1,1).Get("homo",nodesNb,i).Get("type",1,1).ToString();
        currentNode.homo.type = QString(tempString.c_str());

        // Fill pose field
        mosaicData.Get("nodes",1,1).Get("pose",nodesNb,i).Get("matrix",1,1).GetData(matrix1x6,6);
        currentNode.pose.matrix = Mat(1,6,CV_64F,matrix1x6).clone();

        tempString = mosaicData.Get("nodes",1,1).Get("pose",nodesNb,i).Get("type",1,1).ToString();
        currentNode.pose.type = QString(tempString.c_str());

        // Get image field
        tempString = mosaicData.Get("nodes",1,1).Get("image",nodesNb,i).ToString();
        currentNode.pose.type = QString(tempString.c_str());
        currentNode.dataSetImageName = QString(tempString.c_str());

        // Fill node with current node
        mosaicDataCpp.nodes.push_back(currentNode);
        //qDebug() << "pose type = " << mosaicDataCpp.nodes.at(i-1).pose.type;

    }

    // ********************* Ending : Complete mosaicData structure from matlab object ************

}

///
/// \brief MatlabCppInterface::mosaicDataCppToMatlab converts C++ MosaicData to its Matlab equivalent
/// \param mosaicDataCpp C++ MosaicData structure
/// \param mosaicData Matlab MosaicData structure
///
/*static*/ void MatlabCppInterface::mosaicDataCppToMatlab (MosaicData &mosaicDataCpp, mwArray &mosaicData){

    // Generate input mosaicData *****************************************************************

    const char *mfields[] = {"init", "nodes"};
    mwSize mdims[2] = {1, 1};
    mosaicData = mwArray(2,mdims, 2, mfields);

    // Generate init struct **********************************************************************
    const char *initfields[] = {"mosaic_origin", "pixel_size", "mosaic_size", "mosaic_ullr"};
    mwSize idims[2] = {1, 1};
    mwArray init(2,idims, 4, initfields);

    // Generate mosaic_origin struct
    const char *xyzfields[] = {"x", "y", "z"};
    mwArray mosaic_origin(2,idims, 3, xyzfields);

    mosaic_origin.Get(xyzfields[0],1,1).Set(mwArray(mosaicDataCpp.init.mosaicOrigin.x));
    mosaic_origin.Get(xyzfields[1],1,1).Set(mwArray(mosaicDataCpp.init.mosaicOrigin.y));
    mosaic_origin.Get(xyzfields[2],1,1).Set(mwArray(mosaicDataCpp.init.mosaicOrigin.z));

    // Generate mosaic_origin struct
    const char *xyfields[] = {"x", "y"};
    mwArray pixel_size(2,idims, 2, xyfields);

    pixel_size.Get(xyfields[0],1,1).Set(mwArray(mosaicDataCpp.init.pixelSize.x));
    pixel_size.Get(xyfields[1],1,1).Set(mwArray(mosaicDataCpp.init.pixelSize.y));

    // Generate mosaic_origin struct
    mwArray mosaic_size(2,idims, 2, xyfields);

    mosaic_size.Get(xyfields[0],1,1).Set(mwArray(mosaicDataCpp.init.mosaicSize.x));
    mosaic_size.Get(xyfields[1],1,1).Set(mwArray(mosaicDataCpp.init.mosaicSize.y));

    // Generate mosaic_ullr matrix
    mwArray mosaic_ullr(1, 4, mxDOUBLE_CLASS);
    for (int i=0; i<4; i++){
        mosaic_ullr(1,i+1) = mosaicDataCpp.init.mosaic_ullr.at<double>(0,i);
    }

    // Set init fields
    init.Get(initfields[0],1,1).Set(mosaic_origin);
    init.Get(initfields[1],1,1).Set(pixel_size);
    init.Get(initfields[2],1,1).Set(mosaic_size);
    init.Get(initfields[3],1,1).Set(mosaic_ullr);


    // Generate nodes struct *********************************************************************

    // Construct nodes struct
    const char *nodesfields[] = {"index", "homo", "pose", "image"};
    mwSize ndims[2] = {1, mosaicDataCpp.nodes.count()};
    mwArray nodes(2,ndims, 4, nodesfields);

    // Construct homo struct
    const char *homofields[] = {"model", "type", "matrix"};
    mwSize hdims[2] = {1, 1};
    mwArray homo(2,hdims, 3, homofields);

    // Construct pose struct
    const char *posefields[] = {"type", "matrix"};
    mwSize pdims[2] = {1, 1};
    mwArray pose;
    mwArray tempMatlabMat;

    for (int i=0; i<mosaicDataCpp.nodes.count(); i++){

        nodes.Get("index",ndims[1],i+1).Set(mwArray(mosaicDataCpp.nodes.at(i).imageIndex));

        // Fill homography field
        tempMatlabMat = mwArray(3,3,mxDOUBLE_CLASS);
        for (int m=0; m<3; m++){
            for (int n=0; n<3; n++){
                // Note : the opencv '.at<T>(m,n)' function works as if the matrix
                // was stored in column-major convention and hence no transposition is needed
                tempMatlabMat(m+1,n+1) = mosaicDataCpp.nodes.at(i).homo.matrix.at<double>(m,n);
            }
        }

        // The .Clone() is needed to create a copy of the current object
        homo.Get("matrix",1,1).Set(tempMatlabMat.Clone());

        homo.Get("model",1,1)
                .Set(mwArray(mosaicDataCpp.nodes.at(i).homo.model.toLocal8Bit().data()));

        homo.Get("type",1,1)
                .Set(mwArray(mosaicDataCpp.nodes.at(i).homo.type.toLocal8Bit().data()));

        nodes.Get("homo",ndims[1],i+1).Set(homo.Clone());

        // Fill pose field
        pose = mwArray(2,pdims, 2, posefields);
        tempMatlabMat = mwArray(1,6,mxDOUBLE_CLASS);
        for (int m=0; m<6; m++){
            tempMatlabMat(1,m+1) = mosaicDataCpp.nodes.at(i).pose.matrix.at<double>(0,m);

        }

        pose.Get("matrix",1,1).Set(tempMatlabMat.Clone());

        pose.Get("type",1,1)
                .Set(mwArray(mosaicDataCpp.nodes.at(i).pose.type.toLocal8Bit().data()));

        nodes.Get("pose",ndims[1],i+1).Set(pose.Clone());

        // Fill image field
        QString tempString;
        tempString = mosaicDataCpp.nodes.at(i).dataSetImageName;
        tempString = tempString.split('\n').at(0); // there seem to be a bug (in matlab api ?) and filename end with \n to be removed
        nodes.Get("image",ndims[1],i+1).Set(mwArray(tempString.toLocal8Bit().data()));

    }

    // Set mosaicData fields
    mosaicData.Get(mfields[0],1,1).Set(init);
    mosaicData.Get(mfields[1],1,1).Set(nodes);

}

///
/// \brief MatlabCppInterface::matchingDataMatlabToCpp converts matlab MatchingData to its C++ equivalent
/// \param matchingData C++ MatchingData structure
/// \param matchingDataCpp Matlab MatchingData structure
///
/*static*/ void MatlabCppInterface::matchingDataMatlabToCpp (mwArray &matchingData, MatchingData &matchingDataCpp){

    int mdim[2];
    int pdim[2];
    Point2f tempPoint;

    matchingData.GetDimensions().GetData(mdim,2);
    qDebug() << "matchingDataSize = (" << mdim[0] << ", " << mdim[1] << ")";

    Matching2Images CurrentMatching;

    for (int i=0; i<mdim[0]; i++){

        // Get images indexes
        matchingData(i+1,1).GetData(CurrentMatching.indexes, 2);

        // Get pointsImage1
        matchingData(i+1,2).GetDimensions().GetData(pdim,2);
        for (int m=0; m<pdim[1]; m++){
            tempPoint.x = (matchingData(i+1,2))(1,m+1);
            tempPoint.y = (matchingData(i+1,2))(2,m+1);
            CurrentMatching.pointsImage1.push_back(tempPoint);
        }

        // Get pointsImage2
        matchingData(i+1,3).GetDimensions().GetData(pdim,2);
        for (int m=0; m<pdim[1]; m++){
            tempPoint.x = (matchingData(i+1,3))(1,m+1);
            tempPoint.y = (matchingData(i+1,3))(2,m+1);
            CurrentMatching.pointsImage2.push_back(tempPoint);
        }

        // Push result to matchingDataCpp
        matchingDataCpp.push_back(CurrentMatching);

        // Free vectors for next loop
        CurrentMatching.pointsImage1.clear();
        CurrentMatching.pointsImage2.clear();

    }

}


///
/// \brief MatlabCppInterface::matchingDataMatlabToCpp converts C++ MatchingData to its Matlab equivalent
/// \param matchingDataCpp Matlab MatchingData structure
/// \param matchingData C++ MatchingData structure
///
/*static*/ void MatlabCppInterface::matchingDataCppToMatlab (MatchingData &matchingDataCpp, mwArray &matchingData){

    if (matchingDataCpp.count() > 0){
        // Allocate the corresponding size
        matchingData = mwArray(matchingDataCpp.count(), 3, mxCELL_CLASS);

        mwArray indexes(1, 2, mxDOUBLE_CLASS);
        mwArray matchesImg1;
        mwArray matchesImg2;

        for (int i=0; i<matchingDataCpp.count(); i++){

            // Fill indexes
            indexes(1,1) = matchingDataCpp.at(i).indexes[0];
            indexes(1,2) = matchingDataCpp.at(i).indexes[1];

            matchingData(i+1,1) = indexes.Clone();

            // Fill matchesImg1
            matchesImg1 = mwArray(2, matchingDataCpp.at(i).pointsImage1.count() , mxDOUBLE_CLASS);
            for (int m=0; m<matchingDataCpp.at(i).pointsImage1.count(); m++){
                matchesImg1(1,m+1) = matchingDataCpp.at(i).pointsImage1.at(m).x;
                matchesImg1(2,m+1) = matchingDataCpp.at(i).pointsImage1.at(m).y;
            }

            // Fill matchesImg2
            matchesImg2 = mwArray(2, matchingDataCpp.at(i).pointsImage2.count() , mxDOUBLE_CLASS);
            for (int m=0; m<matchingDataCpp.at(i).pointsImage2.count(); m++){
                matchesImg2(1,m+1) = matchingDataCpp.at(i).pointsImage2.at(m).x;
                matchesImg2(2,m+1) = matchingDataCpp.at(i).pointsImage2.at(m).y;
            }


            // Affect matchesImg values to matchingData
            matchingData(i+1,2) = matchesImg1.Clone();
            matchingData(i+1,3) = matchesImg2.Clone();

        }
    }


}



///
/// \brief MatlabCppInterface::generate_algo_param converts Cpp parameters to algo_param matlab structure
/// \param algo_param Matlab structure for algorithms parameters
///
/*static*/ void MatlabCppInterface::generate_algo_param (MatisseCommon::MatisseParameters *mosaicParameters, mwArray &algo_param){

    bool Ok;

    const char *paramfields[] = {"sift_code", "scale_factor", "maxdist_centers", "min_matches", "max_matches", "filter_overlap", "max_overlap", "min_overlap",
                                 "MaxIter", "ba_method", "cosmetic_stretch", "expo_comp_method", "seam_method", "blending_method", "no_plot", "WriteGeoTiff",
                                 "reproj_std", "X_std", "Y_std", "alt_std", "Roll_std", "Pitch_std", "Yaw_std", "K", "V_Pose_C", "dataset_dir", "output_dir",
                                 "output_filename", "navFile" , "First_processed_image", "Last_processed_image", "step_im", "utm_hemisphere", "utm_zone",
                                 "block_drawing", "block_width", "block_height", "max_Roll", "max_Pitch"};

    mwSize pdims[2] = {1, 1};
    algo_param = mwArray(2,pdims, 39, paramfields);

    QString tempString;
    double tempDouble;
    int tempInt;
    bool tempBool;

    tempString =mosaicParameters->getStringParamValue("algo_param", "sift_code");
    qDebug() << "sift_code = " << tempString;
    algo_param.Get(paramfields[0],1,1).Set(mwArray(tempString.toLocal8Bit().data()));

    tempDouble = mosaicParameters->getDoubleParamValue("algo_param", "scale_factor", Ok);
    qDebug() << "scale_factor = " << tempDouble;
    algo_param.Get(paramfields[1],1,1).Set(mwArray(tempDouble));

    tempDouble = mosaicParameters->getDoubleParamValue("algo_param", "maxdist_centers", Ok);
    qDebug() << "maxdist_centers = " << tempDouble;
    algo_param.Get(paramfields[2],1,1).Set(mwArray(tempDouble));

    tempInt =  mosaicParameters->getIntParamValue("algo_param", "min_matches", Ok);
    qDebug() << "min_matches = " << tempInt;
    algo_param.Get(paramfields[3],1,1).Set(mwArray(tempInt));

    tempInt = mosaicParameters->getIntParamValue("algo_param", "max_matches", Ok);
    qDebug() << "max_matches = " <<  tempInt;
    algo_param.Get(paramfields[4],1,1).Set(mwArray(tempInt));

    tempBool = mosaicParameters->getBoolParamValue("algo_param", "filter_overlap", Ok);
    qDebug() << "filter_overlap = " << tempBool;
    algo_param.Get(paramfields[5],1,1).Set(mwArray(tempBool));

    tempDouble = mosaicParameters->getDoubleParamValue("algo_param", "max_overlap", Ok);
    qDebug() << "max_overlap = " << tempDouble;
    algo_param.Get(paramfields[6],1,1).Set(mwArray(tempDouble));

    tempDouble = mosaicParameters->getDoubleParamValue("algo_param", "min_overlap", Ok);
    qDebug() << "min_overlap = " << tempDouble;
    algo_param.Get(paramfields[7],1,1).Set(mwArray(tempDouble));

    tempInt = mosaicParameters->getIntParamValue("algo_param", "MaxIter", Ok);
    qDebug() << "MaxIter = " << tempInt;
    algo_param.Get(paramfields[8],1,1).Set(mwArray(tempInt));

    tempString = mosaicParameters->getStringParamValue("algo_param", "ba_method");
    qDebug() << "ba_method = " << tempString;
    algo_param.Get(paramfields[9],1,1).Set(mwArray(tempString.toLocal8Bit().data()));

    tempBool = mosaicParameters->getBoolParamValue("algo_param", "cosmetic_stretch", Ok);
    qDebug() << "cosmetic_stretch = " << tempBool;
    algo_param.Get(paramfields[10],1,1).Set(mwArray(tempBool));

    tempString = mosaicParameters->getStringParamValue("algo_param", "expo_comp_method");
    qDebug() << "expo_comp_method = " << tempString;
    algo_param.Get(paramfields[11],1,1).Set(mwArray(tempString.toLocal8Bit().data()));

    tempString = mosaicParameters->getStringParamValue("algo_param", "seam_method");
    qDebug() << "seam_method = " << tempString;
    algo_param.Get(paramfields[12],1,1).Set(mwArray(tempString.toLocal8Bit().data()));

    tempString = mosaicParameters->getStringParamValue("algo_param", "blending_method");
    qDebug() << "blending_method = " << tempString;
    algo_param.Get(paramfields[13],1,1).Set(mwArray(tempString.toLocal8Bit().data()));

    tempBool = mosaicParameters->getBoolParamValue("algo_param", "no_plot", Ok);
    qDebug() << "no_plot = " << tempBool;
    algo_param.Get(paramfields[14],1,1).Set(mwArray(tempBool));

    tempBool = mosaicParameters->getBoolParamValue("algo_param", "WriteGeoTiff", Ok);
    qDebug() << "WriteGeoTiff = " << tempBool;
    algo_param.Get(paramfields[15],1,1).Set(mwArray(tempBool));

    tempDouble = mosaicParameters->getDoubleParamValue("vehic_param", "reproj_std", Ok);
    qDebug() << "reproj_std = " << tempDouble;
    algo_param.Get(paramfields[16],1,1).Set(mwArray(tempDouble));

    tempDouble = mosaicParameters->getDoubleParamValue("vehic_param", "X_std", Ok);
    qDebug() << "X_std = " << tempDouble;
    algo_param.Get(paramfields[17],1,1).Set(mwArray(tempDouble));

    tempDouble = mosaicParameters->getDoubleParamValue("vehic_param", "Y_std", Ok);
    qDebug() << "Y_std = " << tempDouble;
    algo_param.Get(paramfields[18],1,1).Set(mwArray(tempDouble));

    tempDouble = mosaicParameters->getDoubleParamValue("vehic_param", "alt_std", Ok);
    qDebug() << "alt_std = " << tempDouble;
    algo_param.Get(paramfields[19],1,1).Set(mwArray(tempDouble));

    tempDouble = mosaicParameters->getDoubleParamValue("vehic_param", "Roll_std", Ok);
    qDebug() << "Roll_std = " << tempDouble;
    algo_param.Get(paramfields[20],1,1).Set(mwArray(tempDouble));

    tempDouble = mosaicParameters->getDoubleParamValue("vehic_param", "Pitch_std", Ok);
    qDebug() << "Pitch_std = " << tempDouble;
    algo_param.Get(paramfields[21],1,1).Set(mwArray(tempDouble));

    tempDouble = mosaicParameters->getDoubleParamValue("vehic_param", "Yaw_std", Ok);
    qDebug() << "Yaw_std = " << tempDouble;
    algo_param.Get(paramfields[22],1,1).Set(mwArray(tempDouble));

    QMatrix3x3 K = mosaicParameters->getMatrix3x3ParamValue("cam_param",  "K",  Ok);
    if (Ok){
        qDebug() << "K Ok =" << Ok;
        mwArray Kmat(3,3,mxDOUBLE_CLASS);

        for (int i=0; i<3; i++){
            for(int j=0; j<3; j++){
                Kmat(i+1,j+1) = K(i,j);
            }
        }

        algo_param.Get(paramfields[23],1,1).Set(Kmat.Clone());
    }else{
        qDebug() << "K Ok =" << Ok;
    }

    Matrix6x1 V_Pose_C = mosaicParameters->getMatrix6x1ParamValue("cam_param",  "V_Pose_C",  Ok);
    if (Ok){
        mwArray V_Pose_C_mat(1,6,mxDOUBLE_CLASS);

        for (int i=0; i<6; i++){
            V_Pose_C_mat(1,i+1) = V_Pose_C(0,i);
        }
        algo_param.Get(paramfields[24],1,1).Set(V_Pose_C_mat.Clone());
    }

    tempString = mosaicParameters->getStringParamValue("dataset_param", "dataset_dir");
    qDebug() << "dataset_dir = " << tempString;
    algo_param.Get(paramfields[25],1,1).Set(mwArray(tempString.toLocal8Bit().data()));



    // ********************************Define output_filename *************************************************
    QString datasetDirnameStr = mosaicParameters->getStringParamValue("dataset_param", "dataset_dir");
    QString outputDirnameStr = mosaicParameters->getStringParamValue("dataset_param", "output_dir");
    QString outputFilename = mosaicParameters->getStringParamValue("dataset_param", "output_filename");

    if (outputDirnameStr.isEmpty()
     || datasetDirnameStr.isEmpty()
     || outputFilename.isEmpty())
        return;

    QFileInfo outputDirInfo(outputDirnameStr);
    QFileInfo datasetDirInfo(datasetDirnameStr);

    bool isRelativeDir = outputDirInfo.isRelative();

    if (isRelativeDir) {
        outputDirnameStr = QDir::cleanPath( datasetDirInfo.absoluteFilePath() + QDir::separator() + outputDirnameStr);
    }

    qDebug() << "output_dir = " << outputDirnameStr;
    algo_param.Get(paramfields[26],1,1).Set(mwArray(outputDirnameStr.toLocal8Bit().data()));
    qDebug() << "output_filename = " << outputDirnameStr;
    algo_param.Get(paramfields[27],1,1).Set(mwArray(outputFilename.toLocal8Bit().data()));

    // ********************************************************************************************************

    tempString = mosaicParameters->getStringParamValue("dataset_param", "navFile");
    QFileInfo navFile(tempString);
    if (navFile.isAbsolute()){
        tempString = navFile.fileName();
    }
    qDebug() << "navFile = " << tempString;
    algo_param.Get(paramfields[28],1,1).Set(mwArray(tempString.toLocal8Bit().data()));

    tempInt = mosaicParameters->getIntParamValue("algo_param", "First_processed_image", Ok);
    qDebug() << "First_processed_image = " << tempInt;
    algo_param.Get(paramfields[29],1,1).Set(mwArray(tempInt));

    tempInt = mosaicParameters->getIntParamValue("algo_param", "Last_processed_image", Ok);
    qDebug() << "Last_processed_image = " << tempInt;
    algo_param.Get(paramfields[30],1,1).Set(mwArray(tempInt));

    tempInt = mosaicParameters->getIntParamValue("algo_param", "step_im", Ok);
    qDebug() << "step_im = " << tempInt;
    algo_param.Get(paramfields[31],1,1).Set(mwArray(tempInt));

    tempBool = mosaicParameters->getBoolParamValue("algo_param", "block_drawing", Ok);
    qDebug() << "block_drawing = " << tempBool;
    algo_param.Get(paramfields[34],1,1).Set(mwArray(tempBool));

    tempInt = mosaicParameters->getIntParamValue("algo_param", "block_width", Ok);
    qDebug() << "block_width = " << tempInt;
    algo_param.Get(paramfields[35],1,1).Set(mwArray(tempInt));

    tempInt = mosaicParameters->getIntParamValue("algo_param", "block_height", Ok);
    qDebug() << "block_height = " << tempInt;
    algo_param.Get(paramfields[36],1,1).Set(mwArray(tempInt));

    tempDouble = mosaicParameters->getDoubleParamValue("algo_param", "max_Roll", Ok);
    qDebug() << "max_Roll = " << tempDouble;
    algo_param.Get(paramfields[37],1,1).Set(mwArray(tempDouble));

    tempDouble = mosaicParameters->getDoubleParamValue("algo_param", "max_Pitch", Ok);
    qDebug() << "max_Pitch = " << tempDouble;
    algo_param.Get(paramfields[38],1,1).Set(mwArray(tempDouble));

}
