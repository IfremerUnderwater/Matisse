#include <QDebug>

#include <streams.h> // DirectShow
#include <dxutil.h> // SAFE_RELEASE

#include "sequenceprocessor.h"


SequenceProcessor *procseq = 0;

SequenceProcessor::SequenceProcessor(QString filename, bool display) {

    CoInitialize(NULL);

    pGraph= 0;

    qDebug() << "Create Instance";
    if (!FAILED(

        CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC,
                                 IID_IGraphBuilder, (void **)&pGraph))) {


        qDebug() << "Query interfaces";

        pGraph->QueryInterface(IID_IMediaControl, (void **)&pMediaControl);
        pGraph->QueryInterface(IID_IMediaEvent, (void **)&pEvent);

        WCHAR	*MediaFile= new WCHAR[filename.size()+1];
        MultiByteToWideChar(CP_ACP, 0, filename.toAscii(), -1, MediaFile, filename.size()+1);

        qDebug() << "Render Files";

        pGraph->RenderFile(MediaFile, NULL);

        if (display) {

            pMediaControl->Run();

            qDebug() << "Wait for completion.";
            long evCode;
            pEvent->WaitForCompletion(INFINITE, &evCode);
            qDebug() << "done.";
        }
    }
}

SequenceProcessor::~SequenceProcessor() {

    SAFE_RELEASE(pMediaControl);
    SAFE_RELEASE(pEvent);
    SAFE_RELEASE(pGraph);

    CoUninitialize();
}

QStringList SequenceProcessor::enumFilters() {

    IEnumFilters *pEnum = NULL;
    IBaseFilter *pFilter;
    ULONG cFetched;
    QStringList names;

    pGraph->EnumFilters(&pEnum);

    while(pEnum->Next(1, &pFilter, &cFetched) == S_OK)
    {
        FILTER_INFO FilterInfo;
        char szName[256];
        QString fname;

        pFilter->QueryFilterInfo(&FilterInfo);
        WideCharToMultiByte(CP_ACP, 0, FilterInfo.achName,
                            -1, szName, 256, 0, 0);
        fname= szName;
        names << fname;

        SAFE_RELEASE(FilterInfo.pGraph);
        SAFE_RELEASE(pFilter);
    }

    SAFE_RELEASE(pEnum);

    return names;
}


