//////////////////////////////////////////////////////////////////////
//
//  This class is designed to provide simple interface for 
//  simultaneous Video Capture & Preview using DirectShow
//
//////////////////////////////////////////////////////////////////////
//
//	References: MS DirectShow Samples
//
//		
//////////////////////////////////////////////////////////////////////
//
//	This class was written by Sagar K.R . 
//  Use of this class is not restricted in any
//	way whatsoever.Please report the bugs to krssagar@firsteccom.co.kr
//
//	Special thanks to all the members at The Code Project! 
//	(www.codeproject.com)
//
//////////////////////////////////////////////////////////////////////

// VMR_Capture.h: interface for the CVMR_Capture class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_VMR_CAPTURE_H__186091F3_30FA_4FAA_AC8B_EF25E8463B9A__INCLUDED_)
#define AFX_VMR_CAPTURE_H__186091F3_30FA_4FAA_AC8B_EF25E8463B9A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <atlbase.h>
#include <dshow.h>
#include <d3d9.h>
#include <vmr9.h>
#include <QStringList>

//#define WM_GRAPHNOTIFY  WM_USER+13
enum PLAYSTATE {Stopped, Paused, Running, Init};


class CVMR_Capture  
{
public:

	CVMR_Capture();
    QStringList EnumDevices();
	HRESULT Init(int iDeviceID,HWND hWnd,int iWidth,int iHeight);
	DWORD GetFrame(BYTE ** pFrame);
	BOOL Pause();
	DWORD ImageCapture(LPCTSTR szFile);
	DWORD GrabFrame();
	
	virtual ~CVMR_Capture();


protected:
	
	IGraphBuilder *m_pGB ;
	IMediaControl *m_pMC;
	IMediaEventEx *m_pME ;
	//IMediaEvent *pME ;
	
	
	IVMRWindowlessControl9 *m_pWC;
	IPin * m_pCamOutPin;
    IPin * m_pPinOut;

	IBaseFilter *m_pDF;
    IBaseFilter *m_pAVIDecompressor;
    IBaseFilter *m_pMJPEGCompressor;
    IBaseFilter *m_pFfdshowDecoder;

	PLAYSTATE m_psCurrent;

	int m_nWidth;
	int m_nHeight;

	BYTE *m_pFrame;
	long m_nFramelen;

	
	bool BindFilter(int deviceId, IBaseFilter **pFilter);
	HRESULT InitializeWindowlessVMR(HWND hWnd);
    HRESULT AddFFDShow();
    HRESULT AddFFDShowSony();
	HRESULT InitVideoWindow(HWND hWnd,int width, int height);
	void StopCapture();
	void CloseInterfaces(void);
	
	void DeleteMediaType(AM_MEDIA_TYPE *pmt);
	bool Convert24Image(BYTE *p32Img,BYTE *p24Img,DWORD dwSize32);
    IPin* GetInPin(IBaseFilter *pfilter, int index);
    IPin* GetOutPin(IBaseFilter *pfilter, int index);



private:
	
};

#endif // !defined(AFX_VMR_CAPTURE_H__186091F3_30FA_4FAA_AC8B_EF25E8463B9A__INCLUDED_)
