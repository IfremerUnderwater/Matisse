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

// VMR_Capture.cpp: implementation of the CVMR_Capture class.
//
//////////////////////////////////////////////////////////////////////

#include "VMR_Capture.h"

#include <QDebug>
#include <uuids.h>

#define MY_DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
        EXTERN_C const GUID DECLSPEC_SELECTANY name \
                = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }

//979520BE-4C52-4BA7-A534-B145EDCFDF21
MY_DEFINE_GUID(CLSID_DecklinkMJPEGEncoderFilter,
0x979520BE, 0x4C52, 0x4BA7, 0xA5, 0x34, 0xB1, 0x45, 0xED, 0xCF, 0xDF, 0x21);

//1E003B41-B606-4AE4-B2BB-C35E133575A5
MY_DEFINE_GUID(CLSID_DecklinkMJPEGCompressor,
0x1E003B41, 0xB606, 0x4AE4, 0xB2, 0xBB, 0xC3, 0x5E, 0x13, 0x35, 0x75, 0xA5);

// {04FE9017-F873-410e-871E-AB91661A4EF7}
MY_DEFINE_GUID(CLSID_FFDSHOW,
0x04fe9017, 0xf873, 0x410e, 0x87, 0x1e, 0xab, 0x91, 0x66, 0x1a, 0x4e, 0xf7);

//0B390488-D80F-4A68-8408-48DC199F0E97
MY_DEFINE_GUID(CLSID_FFDSHOW_RAW,
0x0B390488, 0xD80F, 0x4A68, 0x84, 0x08, 0x48, 0xDC, 0x19, 0x9F, 0x0E, 0x97);

//C1F400A4-3F08-11D3-9F0B-006008039E37
MY_DEFINE_GUID(CLSID_NULL_RENDERER,
0xC1F400A4, 0x3F08, 0x11D3, 0x9F, 0x0B, 0x00, 0x60, 0x08, 0x03, 0x9E, 0x37);


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
bool CVMR_Capture::configure()
{
  //  Init( );
    return false;
}

bool CVMR_Capture::start()
{
    return false;
}

void CVMR_Capture::stop()
{
    StopCapture();
    CloseInterfaces();
    CoUninitialize();
}

bool CVMR_Capture::isValid()
{
    return false;
}



CVMR_Capture::CVMR_Capture():
    m_pGB(NULL),
    m_pMC (NULL),
    m_pME(NULL),
    m_pWC(NULL),
    m_pDF(NULL),
    m_pCamOutPin(NULL),
    m_pFrame(NULL),
    m_nFramelen(0),
    m_pAVIDecompressor(NULL),
    m_pMJPEGCompressor(NULL),
    m_pFfdshowDecoder(NULL),
    m_pPinOut(NULL),
    m_psCurrent(Stopped)
{
	CoInitialize(NULL);

}

CVMR_Capture::~CVMR_Capture()
{
    CloseInterfaces();
    CoUninitialize( );
}

HRESULT CVMR_Capture::Init(int iDeviceID,HWND hWnd, int iWidth, int iHeight)
{
	HRESULT hr;
	// Get the interface for DirectShow's GraphBuilder
    hr=CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, 
                         IID_IGraphBuilder, (void **)&m_pGB);

    if(SUCCEEDED(hr))
    {
        // Create the Video Mixing Renderer and add it to the graph
        if (hWnd!=NULL) {
            InitializeWindowlessVMR(hWnd);
        }
		// Bind Device Filter.  We know the device because the id was passed in
        if(!BindFilter(iDeviceID, &m_pDF))
            return S_FALSE;

		hr=m_pGB->AddFilter(m_pDF, L"Video Capture");
		if (FAILED(hr))
            return hr;
        m_pCamOutPin = GetOutPin(m_pDF, 0);
		
        // QueryInterface for DirectShow interfaces
        hr = m_pGB->QueryInterface(IID_IMediaControl, (void **)&m_pMC);
        hr = m_pGB->QueryInterface(IID_IMediaEventEx, (void **)&m_pME);     

        if (hWnd!=NULL) {
            hr = InitVideoWindow(hWnd,iWidth, iHeight);
        }
		m_nFramelen=iWidth*iHeight*3;
		m_pFrame=(BYTE*) new BYTE[m_nFramelen];		

        m_pPinOut = m_pCamOutPin;
		// Run the graph to play the media file
        AddFFDShow();
        //Sony();

		m_psCurrent=Stopped;
        
        hr = m_pGB->Render(m_pPinOut);
		hr = m_pMC->Run();
		m_psCurrent=Running;

        
	}
	return hr;

}


HRESULT CVMR_Capture::AddFFDShow()
{
    HRESULT hr;
    // Create decoder instance
    hr = CoCreateInstance(CLSID_FFDSHOW_RAW, NULL,
                                  CLSCTX_INPROC, IID_IBaseFilter, (void**)&m_pFfdshowDecoder);
    if( !m_pFfdshowDecoder )
    {
        qDebug() << "Could not create FFDSHOW Video Decoder";
        return hr;
    }

    // Add decoder to graph
    CComQIPtr< IBaseFilter, &IID_IBaseFilter > pProcess( m_pFfdshowDecoder );
    m_pGB->AddFilter(pProcess,  L"ffdshow Video Decoder");
    if( SUCCEEDED(hr))
    {
        qDebug() << "ffdshow Video Decoder added to graph";
    }
    else
    {
        qDebug() << "Could not add ffdshow Video Decoder";
        return hr;
    }

    IPin * pFilterIn = GetInPin( pProcess, 0 );
    IPin * pFilterOut = GetOutPin( pProcess, 0 );

    hr = m_pGB->Connect( m_pCamOutPin, pFilterIn );
    if( FAILED( hr ) )
    {
         qDebug() <<"Could not connect FilterIn";
        return hr;
    }
    m_pPinOut = pFilterOut;

}


void  CVMR_Capture::showFFdShowProperties( HWND hWnd){

/* Obtain the filter's IBaseFilter interface. (Not shown) */
ISpecifyPropertyPages *pProp;
HRESULT hr = m_pFfdshowDecoder->QueryInterface(IID_ISpecifyPropertyPages, (void **)&pProp);
if (SUCCEEDED(hr))
{
    // Get the filter's name and IUnknown pointer.
    FILTER_INFO FilterInfo;
    hr = m_pFfdshowDecoder->QueryFilterInfo(&FilterInfo);
    IUnknown *pFilterUnk;
    m_pFfdshowDecoder->QueryInterface(IID_IUnknown, (void **)&pFilterUnk);

    // Show the page.
    CAUUID caGUID;
    pProp->GetPages(&caGUID);
    pProp->Release();
    OleCreatePropertyFrame(
        hWnd,                   // Parent window
        0, 0,                   // Reserved
        FilterInfo.achName,     // Caption for the dialog box
        1,                      // Number of objects (just the filter)
        &pFilterUnk,            // Array of object pointers.
        caGUID.cElems,          // Number of property pages
        caGUID.pElems,          // Array of property page CLSIDs
        0,                      // Locale identifier
        0, NULL                 // Reserved
    );

    // Clean up.
    pFilterUnk->Release();
    FilterInfo.pGraph->Release();
    CoTaskMemFree(caGUID.pElems);
}

}

HRESULT CVMR_Capture::AddFFDShowSony()
{
    qDebug() << "AddFFDShow";

    HRESULT hr;
    IPin * pFilterIn;
    IPin * pFilterOut;


    // Create AVI Decompressor  instance
    hr = CoCreateInstance(CLSID_AVIDec, NULL,
                                  CLSCTX_INPROC, IID_IBaseFilter, (void**)&m_pAVIDecompressor);

    if( !m_pAVIDecompressor )
    {
        qDebug() << "Could not create AVI Decompressor";
        return hr;
    }

    // Add decoder to graph
    CComQIPtr< IBaseFilter, &IID_IBaseFilter > pAviDec( m_pAVIDecompressor );
    m_pGB->AddFilter(pAviDec,  L"AVI Decompressor");
    if( SUCCEEDED(hr))
    {
        qDebug() << "AVI Decompressor added to graph";
    }
    else
    {
        qDebug() << "Could not add AVI Decompressor";
        return hr;
    }

    // Create MJPEG Compressor instance
    hr = CoCreateInstance(CLSID_DecklinkMJPEGCompressor, NULL,
                                  CLSCTX_INPROC, IID_IBaseFilter, (void**)&m_pMJPEGCompressor);


    if( !m_pMJPEGCompressor )
    {
        qDebug() << "Could not create MJPEG Compressor (Please install decklink driver)";
        return hr;
    }

    // Add encoder to graph
    CComQIPtr< IBaseFilter, &IID_IBaseFilter > pMJPEG( m_pMJPEGCompressor );
    m_pGB->AddFilter(pMJPEG,  L"Decklink MJPEG Compressor");
    if( SUCCEEDED(hr))
    {
        qDebug() << "Decklink MJPEG Compressor added to graph";
    }
    else
    {
        qDebug() << "Could not add Decklink MJPEG Compressor";
        return hr;
    }



    // Create FFDShow Decoder instance
    hr = CoCreateInstance(CLSID_FFDSHOW, NULL,
                                  CLSCTX_INPROC, IID_IBaseFilter, (void**)&m_pFfdshowDecoder);

    if( !m_pFfdshowDecoder )
    {
        qDebug() << "Could not create FFDSHOW Video Decoder";
        return hr;
    }

    // Add decoder to graph
    CComQIPtr< IBaseFilter, &IID_IBaseFilter > pProcess( m_pFfdshowDecoder );
    m_pGB->AddFilter(pProcess,  L"ffdshow Video Decoder");
    if( SUCCEEDED(hr))
    {
        qDebug() << "ffdshow Video Decoder added to graph";
    }
    else
    {
        qDebug() << "Could not add ffdshow Video Decoder";
        return hr;
    }

    // Connect filters

    pFilterIn = GetInPin( pAviDec, 0 );
    hr = m_pGB->Connect( m_pCamOutPin, pFilterIn );
    if( FAILED( hr ) )
    {
         qDebug() <<"Could not connect CAM with AVI";
        return hr;
    }

    pFilterOut = GetOutPin( pAviDec, 0 );
    pFilterIn = GetInPin( pMJPEG, 0 );


    hr = m_pGB->Connect( pFilterOut, pFilterIn );
    if( FAILED( hr ) )
    {
         qDebug() <<"Could not connect AVI with MJPEG";
        return hr;
    }

    pFilterOut = GetOutPin( pMJPEG, 0 );
    pFilterIn = GetInPin( pProcess, 0 );

    hr = m_pGB->Connect( pFilterOut, pFilterIn );
    if( FAILED( hr ) )
    {
         qDebug() <<"Could not connect MJPEG with FFDSHOW";
        return hr;
    }

    pFilterOut = GetOutPin( pProcess, 0 );


    m_pPinOut = pFilterOut;

}


HRESULT CVMR_Capture::InitializeWindowlessVMR(HWND hWnd)
{
    IBaseFilter* pVmr = NULL;

    // Create the VMR and add it to the filter graph.
    HRESULT hr = CoCreateInstance(CLSID_VideoMixingRenderer, NULL,
                                  CLSCTX_INPROC, IID_IBaseFilter, (void**)&pVmr);
    if (SUCCEEDED(hr)) 
    {
        qDebug() << "Creation du Video renderer ";
        hr = m_pGB->AddFilter(pVmr, L"Video Mixing Renderer");
        if (SUCCEEDED(hr)) 
        {
            // Set the rendering mode and number of streams.  
            IVMRFilterConfig* pConfig;

            hr = pVmr->QueryInterface(IID_IVMRFilterConfig, (void**)&pConfig);
            if( SUCCEEDED(hr)) 
            {
                pConfig->SetRenderingMode(VMRMode_Windowless);
                pConfig->Release();
            }
            hr = pVmr->QueryInterface(IID_IVMRWindowlessControl, (void**)&m_pWC);
            if( SUCCEEDED(hr))
            {
                qDebug() << "SetClipping Video ";
                m_pWC->SetVideoClippingWindow(hWnd);
            }
        }
        pVmr->Release();
    }

    return hr;
}


bool CVMR_Capture::BindFilter(int deviceId, IBaseFilter **pFilter)
{
	if (deviceId < 0)
		return false;
	
    // enumerate all video capture devices
	CComPtr<ICreateDevEnum> pCreateDevEnum;
    HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
			  IID_ICreateDevEnum, (void**)&pCreateDevEnum);
    if (hr != NOERROR)
	{
		return false;
	}

    CComPtr<IEnumMoniker> pEm;
    hr = pCreateDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory,
								&pEm, 0);
    if (hr != NOERROR) 
	{
		return false;
    }

    pEm->Reset();
    ULONG cFetched;
    IMoniker *pM;
	int index = 0;
    while(hr = pEm->Next(1, &pM, &cFetched), hr==S_OK, index <= deviceId)
    {
		IPropertyBag *pBag;
		hr = pM->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pBag);
		if(SUCCEEDED(hr)) 
		{
			VARIANT var;
			var.vt = VT_BSTR;
			hr = pBag->Read(L"FriendlyName", &var, NULL);
			if (hr == NOERROR) 
			{
				if (index == deviceId)
				{
					pM->BindToObject(0, 0, IID_IBaseFilter, (void**)pFilter);
				}
				SysFreeString(var.bstrVal);
			}
			pBag->Release();
		}
		pM->Release();
		index++;
    }
	return true;
}

HRESULT CVMR_Capture::InitVideoWindow(HWND hWnd,int width, int height)
{

	// Set the grabbing size
    // First we iterate through the available media types and 
    // store the first one that fits the requested size.
    // If we have found one, we set it.
    // In any case we query the size of the current media type
    // to have this information for clients of this class.
    //     Gerhard Reitmayr <reitmayr@i ...............>

	HRESULT hr;
	RECT rcDest;
	
    CComPtr<IAMStreamConfig> pConfig;
    IEnumMediaTypes *pMedia;
    AM_MEDIA_TYPE *pmt = NULL, *pfnt = NULL;

    hr = m_pCamOutPin->EnumMediaTypes( &pMedia );
    if(SUCCEEDED(hr))
    {

        while(pMedia->Next(1, &pmt, 0) == S_OK)
        {
            if( pmt->formattype == FORMAT_VideoInfo )
            {
                VIDEOINFOHEADER *vih = (VIDEOINFOHEADER *)pmt->pbFormat;
                if( vih->bmiHeader.biWidth == width && vih->bmiHeader.biHeight == height )
                {
                    pfnt = pmt;
                    break;
                }
                DeleteMediaType( pmt );
            }                        
        }
        pMedia->Release();
    }
    hr = m_pCamOutPin->QueryInterface( IID_IAMStreamConfig, (void **) &pConfig );
    if(SUCCEEDED(hr))
    {
        if( pfnt != NULL )
        {
            hr=pConfig->SetFormat( pfnt );
            DeleteMediaType( pfnt );
        }
        hr = pConfig->GetFormat( &pfnt );
        if(SUCCEEDED(hr))
        {
            m_nWidth = ((VIDEOINFOHEADER *)pfnt->pbFormat)->bmiHeader.biWidth;
            m_nHeight = ((VIDEOINFOHEADER *)pfnt->pbFormat)->bmiHeader.biHeight;
            qDebug() << "Cam width" << m_nWidth;
            DeleteMediaType( pfnt );
        }
    }
	::GetClientRect (hWnd,&rcDest);
    hr = m_pWC->SetVideoPosition(NULL, &rcDest);
    return hr;
}

void CVMR_Capture::StopCapture()
{
    HRESULT hr;

	if((m_psCurrent == Paused) || (m_psCurrent == Running))
    {
        hr = m_pMC->Stop();
        m_psCurrent = Stopped;		
        // Display the first frame to indicate the reset condition
        hr = m_pMC->Pause();
    }
}

void CVMR_Capture::CloseInterfaces(void)
{
    HRESULT hr;

    
	// Stop media playback
    if(m_pMC)
        hr = m_pMC->Stop();	    
    m_psCurrent = Stopped;  

//	SAFE_RELEASE(pME);	

    // Release and zero DirectShow interfaces
    if(m_pCamOutPin)
    m_pCamOutPin->Disconnect ();


    SAFE_RELEASE(m_pPinOut);
    SAFE_RELEASE(m_pAVIDecompressor);
    SAFE_RELEASE(m_pFfdshowDecoder);
    SAFE_RELEASE(m_pMJPEGCompressor);
    SAFE_RELEASE(m_pMC);    
    SAFE_RELEASE(m_pGB);    
    SAFE_RELEASE(m_pWC);	
	SAFE_RELEASE(m_pDF);


	
	
	
//delete allocated memory 
	if(m_pFrame!=NULL)
		delete []m_pFrame;

}

//Capture RAW IMAGE BITS 24bits/pixel
DWORD CVMR_Capture::ImageCapture(LPCTSTR szFile)
{
	BYTE *pImage;
	DWORD dwSize,dwWritten;
	dwSize=this->GrabFrame ();
	this->GetFrame (&pImage);
				
	HANDLE hFile = CreateFile(szFile, GENERIC_WRITE, FILE_SHARE_READ, NULL,
					  CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

	if (hFile == INVALID_HANDLE_VALUE)
	return FALSE;

	WriteFile(hFile, (LPCVOID)pImage , m_nFramelen, &dwWritten, 0);
	// Close the file
	CloseHandle(hFile);

	return dwWritten;
}

void CVMR_Capture::DeleteMediaType(AM_MEDIA_TYPE *pmt)
{
    // allow NULL pointers for coding simplicity

    if (pmt == NULL) {
        return;
    }

    if (pmt->cbFormat != 0) {
        CoTaskMemFree((PVOID)pmt->pbFormat);

        // Strictly unnecessary but tidier
        pmt->cbFormat = 0;
        pmt->pbFormat = NULL;
    }
    if (pmt->pUnk != NULL) {
        pmt->pUnk->Release();
        pmt->pUnk = NULL;
    }

    CoTaskMemFree((PVOID)pmt);
}


DWORD CVMR_Capture::GrabFrame()
{
	long lOut=-1;

	if(m_pWC ) 
    {          

        // Read the current video frame into a byte buffer.  The information
        // will be returned in a packed Windows DIB and will be allocated
        // by the VMR.
        if(m_pWC->GetCurrentImage(&lpCurrImage) == S_OK)
        {	
			LPBITMAPINFOHEADER  pdib = (LPBITMAPINFOHEADER) lpCurrImage;
            if(m_pFrame!=NULL)
                    delete []m_pFrame;
            qDebug() << "height,width" << pdib->biHeight << pdib->biWidth;
            qDebug() << "planes"<< pdib->biPlanes;
            qDebug() << "bitCount"<< pdib->biBitCount;
            qDebug() << "sizeImage"<< pdib->biSizeImage;
            qDebug() << "compression"<<pdib->biCompression;

            m_nFramelen=pdib->biHeight * pdib->biWidth * 3;
            m_pFrame=new BYTE [pdib->biHeight * pdib->biWidth * 3] ;

            if(pdib->biBitCount ==32)
            {
                BYTE *pTemp32;
                pTemp32=lpCurrImage + sizeof(BITMAPINFOHEADER);
                qDebug() << "change from 32 to 24 bit /pixel";
                this->Convert24Image(pTemp32, m_pFrame, pdib->biSizeImage);		
            }
		}
		else
		{
            qDebug() << "GetCurrentImage NOK";
			return lOut;
		}
	}
	else
	{
         CoTaskMemFree(lpCurrImage);	//free the imageIls veinnent
		return lOut;
	}	
    return lOut=m_nFramelen;

}

bool CVMR_Capture::Convert24Image(BYTE *p32Img, BYTE *p24Img,DWORD dwSize32)
{

	if(p32Img != NULL && p24Img != NULL && dwSize32>0)
	{
		DWORD dwSize24;
		dwSize24=(dwSize32 * 3)/4;

		BYTE *pTemp,*ptr;
		pTemp=p32Img;

		ptr=p24Img + dwSize24-1 ;

		for (DWORD index = 0; index < dwSize32/4 ; index++)
		{									
            unsigned char b = *(pTemp++);
			unsigned char g = *(pTemp++);
            unsigned char r = *(pTemp++);
			(pTemp++);//skip alpha
						
            *(ptr--) = r;
			*(ptr--) = g;
            *(ptr--) = b;

		}	
	}
	else
	{
		return false;
	}

    return true;
}


// Query whether a pin has a specified direction (input / output)
HRESULT IsPinDirection(IPin *pPin, PIN_DIRECTION dir, BOOL *pResult)
{
    PIN_DIRECTION pinDir;
    HRESULT hr = pPin->QueryDirection(&pinDir);
    if (SUCCEEDED(hr))
    {
        *pResult = (pinDir == dir);
    }
    return hr;
}


IPin *CVMR_Capture::GetInPin(IBaseFilter *pfilter, int index)
{
    CComPtr<IEnumPins> pEnum;
    pfilter->EnumPins(&pEnum);
    IPin* result = NULL;

    pEnum->Reset();

    int count=0;
    while (SUCCEEDED(pEnum->Next(1, &result, NULL))) {
        BOOL isInput;
        IsPinDirection(result, PINDIR_INPUT, &isInput);
        if (isInput) {
            if (index == count) {
                return result;
            }
            count++;
        }
    }
    return NULL;
}

IPin *CVMR_Capture::GetOutPin(IBaseFilter *pfilter, int index)
{
    CComPtr<IEnumPins> pEnum;
    pfilter->EnumPins(&pEnum);
    IPin* result = NULL;

    pEnum->Reset();

    int count=0;
    while (SUCCEEDED( pEnum->Next(1, &result, NULL))) {
        BOOL isInput;
        IsPinDirection(result, PINDIR_OUTPUT, &isInput);
        if (isInput) {
            if (index == count) {
                return result;
            }
            count++;
        }
    }
    return NULL;
}

BOOL CVMR_Capture::Pause()
{	
    if (!m_pMC)
        return FALSE;
  

    if(((m_psCurrent == Paused) || (m_psCurrent == Stopped)) )
    {
		this->StopCapture();
        if (SUCCEEDED(m_pMC->Run()))
            m_psCurrent = Running;
		
    }
    else
    {
        if (SUCCEEDED(m_pMC->Pause()))
            m_psCurrent = Paused;
    }

	return TRUE;
}



DWORD CVMR_Capture::GetFrame(BYTE **pFrame)
{
	if(m_pFrame && m_nFramelen)
	{
        *pFrame=m_pFrame;
    }
	

	return m_nFramelen;
}

QStringList  CVMR_Capture::EnumDevices()
{

    QStringList result;
	
    // enumerate all video capture devices
	CComPtr<ICreateDevEnum> pCreateDevEnum;

    HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
			  IID_ICreateDevEnum, (void**)&pCreateDevEnum);
    if (hr != NOERROR)
	{
        return result;
	}

    CComPtr<IEnumMoniker> pEm;
    hr = pCreateDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory,&pEm, 0);
    if (hr != NOERROR) 
	{
        return result ;
    }
    pEm->Reset();
    ULONG cFetched;
    IMoniker *pM;
    while(hr = pEm->Next(1, &pM, &cFetched), hr==S_OK)
    {
		IPropertyBag *pBag;
		hr = pM->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pBag);
		if(SUCCEEDED(hr)) 
		{
			VARIANT var;
			var.vt = VT_BSTR;
			hr = pBag->Read(L"FriendlyName", &var, NULL);
			if (hr == NOERROR) 
			{
				TCHAR str[2048];		
                WideCharToMultiByte(CP_ACP,0,var.bstrVal, -1, (LPSTR)str, 2048, NULL, NULL);
                QString qstr = (LPSTR)str;
                result << qstr;
				SysFreeString(var.bstrVal);
			}
			pBag->Release();
		}
		pM->Release();
    }
    return result;
}


