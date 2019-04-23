
// WebCameraViewerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "WebCameraViewer.h"
#include "WebCameraViewerDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CWebCameraViewerDlg dialog

const int ENGINE_WINDOWS_SIZE_CX = 640;
const int ENGINE_WINDOWS_SIZE_CY = 480;

CWebCameraViewerDlg::CWebCameraViewerDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_WEBCAMERAVIEWER_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CWebCameraViewerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_WINDOW_DIRECTSHOW_TO_PAINT, window_direct_show_to_paint);
}

BEGIN_MESSAGE_MAP(CWebCameraViewerDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// CWebCameraViewerDlg message handlers

BOOL CWebCameraViewerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	CRect rcClient(0, 0, ENGINE_WINDOWS_SIZE_CX, ENGINE_WINDOWS_SIZE_CY);

	if
		(
			AdjustWindowRect(&rcClient, DS_SETFONT | DS_MODALFRAME | DS_FIXEDSYS | WS_POPUP | WS_CAPTION, FALSE)
			==
			TRUE
			)
	{
		if (
			SetWindowPos(NULL, 1, 300, rcClient.Width(), rcClient.Height(), SWP_NOZORDER | SWP_NOREDRAW)
			==
			TRUE
			)
		{
			CRect static_client(0, 0, ENGINE_WINDOWS_SIZE_CX, ENGINE_WINDOWS_SIZE_CY);

			if (
				window_direct_show_to_paint.SetWindowPos(NULL, static_client.left, static_client.top, static_client.Width(), static_client.Height(), SWP_NOZORDER | SWP_NOREDRAW)
				==
				TRUE
				)
			{
				window_direct_show_to_paint.SetWindowTextW(CString());
				CString dummy_string("success");
				RedrawWindow();
			}
		}
	}

	HRESULT hr = S_OK;

	if (SUCCEEDED(hr))
	{
		init_capture(window_direct_show_to_paint.m_hWnd);
	}


	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CWebCameraViewerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CWebCameraViewerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void _FreeMediaType(AM_MEDIA_TYPE& mt)
{
	if (mt.cbFormat != 0)
	{
		CoTaskMemFree((PVOID)mt.pbFormat);
		mt.cbFormat = 0;
		mt.pbFormat = NULL;
	}
	if (mt.pUnk != NULL)
	{
		// pUnk should not be used.
		mt.pUnk->Release();
		mt.pUnk = NULL;
	}
}

HRESULT SaveGraphFile(IGraphBuilder *pGraph, WCHAR *wszPath)
{
	const WCHAR wszStreamName[] = L"ActiveMovieGraph";
	HRESULT hr;

	IStorage *pStorage = NULL;
	hr = StgCreateDocfile(
		wszPath,
		STGM_CREATE | STGM_TRANSACTED | STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
		0, &pStorage);
	if (FAILED(hr))
	{
		return hr;
	}

	IStream *pStream;
	hr = pStorage->CreateStream(
		wszStreamName,
		STGM_WRITE | STGM_CREATE | STGM_SHARE_EXCLUSIVE,
		0, 0, &pStream);
	if (FAILED(hr))
	{
		pStorage->Release();
		return hr;
	}

	IPersistStream *pPersist = NULL;
	pGraph->QueryInterface(IID_IPersistStream, (void**)&pPersist);
	hr = pPersist->Save(pStream, TRUE);
	pStream->Release();
	pPersist->Release();
	if (SUCCEEDED(hr))
	{
		hr = pStorage->Commit(STGC_DEFAULT);
	}
	pStorage->Release();
	return hr;
}

void CWebCameraViewerDlg::init_capture(HWND parameter_hwnd)
{
	HRESULT hr;

	{
		//	Create the Capture Graph Builder
		hr = CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC_SERVER, IID_ICaptureGraphBuilder2, (void**)&pBuild);
		if (SUCCEEDED(hr) && pBuild != NULL)
		{
			//	Create the Filter Graph Manager
			hr = CoCreateInstance(CLSID_FilterGraph, 0, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void**)&pGraph);
			if (SUCCEEDED(hr) && pGraph != NULL)
			{
				//Initialize the Capture Graph Builder
				pBuild->SetFiltergraph(pGraph);

				// Find system device enumerator to find a video capture device.
				hr = CoCreateInstance(CLSID_SystemDeviceEnum, 0, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, (LPVOID*)&pDevEnum);

				if (SUCCEEDED(hr) && pDevEnum != NULL)
				{

					hr = pDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnum, 0);

					if (SUCCEEDED(hr) && pEnum != NULL)
					{
						if (pEnum->Next(1, &pVideoMoniker, 0) == S_OK && pVideoMoniker != NULL)
						{
							hr = pVideoMoniker->BindToObject(0, 0, IID_IBaseFilter, (void **)&pVideoCapture);

							if (SUCCEEDED(hr) && pVideoCapture != NULL)
							{
								pGraph->AddFilter(pVideoCapture, L"Video Capture Filter");


								hr = CoCreateInstance(CLSID_VideoRenderer, 0, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&pVideoRenderer);
								if (SUCCEEDED(hr) && pVideoRenderer != NULL)
								{
									pGraph->AddFilter(pVideoRenderer, L"Video Renderer Filter");

									hr = pBuild->RenderStream(NULL, NULL, pVideoCapture, NULL, pVideoRenderer);

									if (SUCCEEDED(hr))
									{
										// Specify the owner window.
										CComPtr<IVideoWindow> pVidWin;
										OAHWND hWnd = (OAHWND)parameter_hwnd;

										pGraph->QueryInterface(IID_IVideoWindow, (void **)&pVidWin);

										pVidWin->put_Owner((OAHWND)hWnd);
										pVidWin->put_WindowStyle(WS_CHILD | WS_CLIPSIBLINGS);

										pVidWin->put_Left(0);
										pVidWin->put_Top(0);
										pVidWin->put_Width(ENGINE_WINDOWS_SIZE_CX);
										pVidWin->put_Height(ENGINE_WINDOWS_SIZE_CY);

										// Set the owner window to receive event notices.
										pGraph->QueryInterface(IID_IMediaEventEx, (void **)&pEvent);
										pEvent->SetNotifyWindow((OAHWND)hWnd, 0, 0);
									}
								}
							}
						}
					}
					
					// Run the graph.
					pGraph->QueryInterface(IID_IMediaControl, (void **)&pMediaControl);
					pMediaControl->Run();

					SaveGraphFile(pGraph, L"c:\\temp\\web_camera_viewer_source.grf");
				}
			}
		}
	}
}

void CWebCameraViewerDlg::OnClose()
{
	StopMedia();

	CDialogEx::OnClose();

	delete this;
}
