//
// MATLAB Compiler: 4.18.1 (R2013a)
// Date: Mon May 12 09:34:26 2014
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

#include <stdio.h>
#define EXPORTING_MatisseCppLib 1
#include "MatisseCppLib.h"

static HMCRINSTANCE _mcr_inst = NULL;


#if defined( _MSC_VER) || defined(__BORLANDC__) || defined(__WATCOMC__) || defined(__LCC__)
#ifdef __LCC__
#undef EXTERN_C
#endif
#include <windows.h>

static char path_to_dll[_MAX_PATH];

BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, void *pv)
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        if (GetModuleFileName(hInstance, path_to_dll, _MAX_PATH) == 0)
            return FALSE;
    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {
    }
    return TRUE;
}
#endif
#ifdef __cplusplus
extern "C" {
#endif

static int mclDefaultPrintHandler(const char *s)
{
  return mclWrite(1 /* stdout */, s, sizeof(char)*strlen(s));
}

#ifdef __cplusplus
} /* End extern "C" block */
#endif

#ifdef __cplusplus
extern "C" {
#endif

static int mclDefaultErrorHandler(const char *s)
{
  int written = 0;
  size_t len = 0;
  len = strlen(s);
  written = mclWrite(2 /* stderr */, s, sizeof(char)*len);
  if (len > 0 && s[ len-1 ] != '\n')
    written += mclWrite(2 /* stderr */, "\n", sizeof(char));
  return written;
}

#ifdef __cplusplus
} /* End extern "C" block */
#endif

/* This symbol is defined in shared libraries. Define it here
 * (to nothing) in case this isn't a shared library. 
 */
#ifndef LIB_MatisseCppLib_C_API
#define LIB_MatisseCppLib_C_API /* No special import/export declaration */
#endif

LIB_MatisseCppLib_C_API 
bool MW_CALL_CONV MatisseCppLibInitializeWithHandlers(
    mclOutputHandlerFcn error_handler,
    mclOutputHandlerFcn print_handler)
{
    int bResult = 0;
  if (_mcr_inst != NULL)
    return true;
  if (!mclmcrInitialize())
    return false;
  if (!GetModuleFileName(GetModuleHandle("MatisseCppLib"), path_to_dll, _MAX_PATH))
    return false;
    {
        mclCtfStream ctfStream = 
            mclGetEmbeddedCtfStream(path_to_dll);
        if (ctfStream) {
            bResult = mclInitializeComponentInstanceEmbedded(   &_mcr_inst,
                                                                error_handler, 
                                                                print_handler,
                                                                ctfStream);
            mclDestroyStream(ctfStream);
        } else {
            bResult = 0;
        }
    }  
    if (!bResult)
    return false;
  return true;
}

LIB_MatisseCppLib_C_API 
bool MW_CALL_CONV MatisseCppLibInitialize(void)
{
  return MatisseCppLibInitializeWithHandlers(mclDefaultErrorHandler, 
                                             mclDefaultPrintHandler);
}

LIB_MatisseCppLib_C_API 
void MW_CALL_CONV MatisseCppLibTerminate(void)
{
  if (_mcr_inst != NULL)
    mclTerminateInstance(&_mcr_inst);
}

LIB_MatisseCppLib_C_API 
void MW_CALL_CONV MatisseCppLibPrintStackTrace(void) 
{
  char** stackTrace;
  int stackDepth = mclGetStackTrace(&stackTrace);
  int i;
  for(i=0; i<stackDepth; i++)
  {
    mclWrite(2 /* stderr */, stackTrace[i], sizeof(char)*strlen(stackTrace[i]));
    mclWrite(2 /* stderr */, "\n", sizeof(char)*strlen("\n"));
  }
  mclFreeStackTrace(&stackTrace, stackDepth);
}


LIB_MatisseCppLib_C_API 
bool MW_CALL_CONV mlxMosaic_draw_and_write(int nlhs, mxArray *plhs[], int nrhs, mxArray 
                                           *prhs[])
{
  return mclFeval(_mcr_inst, "mosaic_draw_and_write", nlhs, plhs, nrhs, prhs);
}

LIB_MatisseCppLib_C_API 
bool MW_CALL_CONV mlxMosaic_init_and_matching(int nlhs, mxArray *plhs[], int nrhs, 
                                              mxArray *prhs[])
{
  return mclFeval(_mcr_inst, "mosaic_init_and_matching", nlhs, plhs, nrhs, prhs);
}

LIB_MatisseCppLib_C_API 
bool MW_CALL_CONV mlxMosaic_optimization(int nlhs, mxArray *plhs[], int nrhs, mxArray 
                                         *prhs[])
{
  return mclFeval(_mcr_inst, "mosaic_optimization", nlhs, plhs, nrhs, prhs);
}

LIB_MatisseCppLib_CPP_API 
void MW_CALL_CONV mosaic_draw_and_write(const mwArray& M, const mwArray& algo_param)
{
  mclcppMlfFeval(_mcr_inst, "mosaic_draw_and_write", 0, 0, 2, &M, &algo_param);
}

LIB_MatisseCppLib_CPP_API 
void MW_CALL_CONV mosaic_init_and_matching(int nargout, mwArray& M, mwArray& algo_param, 
                                           mwArray& matching_data, const mwArray& M_in1, 
                                           const mwArray& algo_param_in1)
{
  mclcppMlfFeval(_mcr_inst, "mosaic_init_and_matching", nargout, 3, 2, &M, &algo_param, &matching_data, &M_in1, &algo_param_in1);
}

LIB_MatisseCppLib_CPP_API 
void MW_CALL_CONV mosaic_optimization(int nargout, mwArray& M, mwArray& algo_param, const 
                                      mwArray& M_in1, const mwArray& algo_param_in1, 
                                      const mwArray& matching_data)
{
  mclcppMlfFeval(_mcr_inst, "mosaic_optimization", nargout, 2, 3, &M, &algo_param, &M_in1, &algo_param_in1, &matching_data);
}

