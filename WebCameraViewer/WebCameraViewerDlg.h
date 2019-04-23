
// WebCameraViewerDlg.h : header file
//

#pragma once


#include <Dshow.h>
#pragma comment(lib, "Strmiids.lib")

#include "afxwin.h"

// CWebCameraViewerDlg dialog
class CWebCameraViewerDlg : public CDialogEx
{
// Construction
public:
	CWebCameraViewerDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_WEBCAMERAVIEWER_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

	afx_msg void OnClose();

	void StopMedia()
	{
		HRESULT hr = S_OK;

		if (SUCCEEDED(hr) && pMediaControl != NULL)
		{
			hr = pMediaControl->Stop();
		}
	}

public:
	void init_capture(HWND parameter_hwnd);

	CComPtr<IGraphBuilder> pGraph;
	CComPtr<ICaptureGraphBuilder2> pBuild;

	CComPtr<IBaseFilter> pVideoCapture;
	CComPtr<IBaseFilter> pVideoRenderer;

	CComPtr<IBaseFilter> pAudioCapture;
	CComPtr<IBaseFilter> pAudioRenderer;

	CComPtr<ICreateDevEnum> pDevEnum;
	CComPtr<IEnumMoniker> pEnum;
	CComPtr<IEnumMoniker> pAudioEnum;
	CComPtr<IMoniker> pVideoMoniker;
	CComPtr<IMoniker> pAudioMoniker;

	CComPtr<IMediaControl> pMediaControl;
	CComPtr<IMediaEventEx> pEvent;

	CStatic window_direct_show_to_paint;
};
