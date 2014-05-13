//
// MATLAB Compiler: 4.18.1 (R2013a)
// Date: Tue May 13 15:19:24 2014
// Arguments: "-B" "macro_default" "-W" "cpplib:MatisseCppLib" "-T" "link:lib"
// "-d"
// "D:\Projects\Matlab_MosaicTool_and_Lib\MosaicTool\MatisseCppLib\MatisseCppLib
// \src" "-w" "enable:specified_file_mismatch" "-w" "enable:repeated_file" "-w"
// "enable:switch_ignored" "-w" "enable:missing_lib_sentinel" "-w"
// "enable:demo_license" "-v"
// "D:\Projects\Matlab_MosaicTool_and_Lib\MosaicTool\MatisseCppLib\mosaic_draw_a
// nd_write.m"
// "D:\Projects\Matlab_MosaicTool_and_Lib\MosaicTool\MatisseCppLib\mosaic_init_a
// nd_matching.m"
// "D:\Projects\Matlab_MosaicTool_and_Lib\MosaicTool\MatisseCppLib\mosaic_optimi
// zation.m" 
//

#ifndef __MatisseCppLib_h
#define __MatisseCppLib_h 1

#if defined(__cplusplus) && !defined(mclmcrrt_h) && defined(__linux__)
#  pragma implementation "mclmcrrt.h"
#endif
#include "mclmcrrt.h"
#include "mclcppclass.h"
#ifdef __cplusplus
extern "C" {
#endif

#if defined(__SUNPRO_CC)
/* Solaris shared libraries use __global, rather than mapfiles
 * to define the API exported from a shared library. __global is
 * only necessary when building the library -- files including
 * this header file to use the library do not need the __global
 * declaration; hence the EXPORTING_<library> logic.
 */

#ifdef EXPORTING_MatisseCppLib
#define PUBLIC_MatisseCppLib_C_API __global
#else
#define PUBLIC_MatisseCppLib_C_API /* No import statement needed. */
#endif

#define LIB_MatisseCppLib_C_API PUBLIC_MatisseCppLib_C_API

#elif defined(_HPUX_SOURCE)

#ifdef EXPORTING_MatisseCppLib
#define PUBLIC_MatisseCppLib_C_API __declspec(dllexport)
#else
#define PUBLIC_MatisseCppLib_C_API __declspec(dllimport)
#endif

#define LIB_MatisseCppLib_C_API PUBLIC_MatisseCppLib_C_API


#else

#define LIB_MatisseCppLib_C_API

#endif

/* This symbol is defined in shared libraries. Define it here
 * (to nothing) in case this isn't a shared library. 
 */
#ifndef LIB_MatisseCppLib_C_API 
#define LIB_MatisseCppLib_C_API /* No special import/export declaration */
#endif

extern LIB_MatisseCppLib_C_API 
bool MW_CALL_CONV MatisseCppLibInitializeWithHandlers(
       mclOutputHandlerFcn error_handler, 
       mclOutputHandlerFcn print_handler);

extern LIB_MatisseCppLib_C_API 
bool MW_CALL_CONV MatisseCppLibInitialize(void);

extern LIB_MatisseCppLib_C_API 
void MW_CALL_CONV MatisseCppLibTerminate(void);



extern LIB_MatisseCppLib_C_API 
void MW_CALL_CONV MatisseCppLibPrintStackTrace(void);

extern LIB_MatisseCppLib_C_API 
bool MW_CALL_CONV mlxMosaic_draw_and_write(int nlhs, mxArray *plhs[], int nrhs, mxArray 
                                           *prhs[]);

extern LIB_MatisseCppLib_C_API 
bool MW_CALL_CONV mlxMosaic_init_and_matching(int nlhs, mxArray *plhs[], int nrhs, 
                                              mxArray *prhs[]);

extern LIB_MatisseCppLib_C_API 
bool MW_CALL_CONV mlxMosaic_optimization(int nlhs, mxArray *plhs[], int nrhs, mxArray 
                                         *prhs[]);


#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

/* On Windows, use __declspec to control the exported API */
#if defined(_MSC_VER) || defined(__BORLANDC__)

#ifdef EXPORTING_MatisseCppLib
#define PUBLIC_MatisseCppLib_CPP_API __declspec(dllexport)
#else
#define PUBLIC_MatisseCppLib_CPP_API __declspec(dllimport)
#endif

#define LIB_MatisseCppLib_CPP_API PUBLIC_MatisseCppLib_CPP_API

#else

#if !defined(LIB_MatisseCppLib_CPP_API)
#if defined(LIB_MatisseCppLib_C_API)
#define LIB_MatisseCppLib_CPP_API LIB_MatisseCppLib_C_API
#else
#define LIB_MatisseCppLib_CPP_API /* empty! */ 
#endif
#endif

#endif

extern LIB_MatisseCppLib_CPP_API void MW_CALL_CONV mosaic_draw_and_write(const mwArray& M, const mwArray& algo_param);

extern LIB_MatisseCppLib_CPP_API void MW_CALL_CONV mosaic_init_and_matching(int nargout, mwArray& M, mwArray& algo_param, mwArray& matching_data, const mwArray& M_in1, const mwArray& algo_param_in1);

extern LIB_MatisseCppLib_CPP_API void MW_CALL_CONV mosaic_optimization(int nargout, mwArray& M, mwArray& algo_param, const mwArray& M_in1, const mwArray& algo_param_in1, const mwArray& matching_data);

#endif
#endif
