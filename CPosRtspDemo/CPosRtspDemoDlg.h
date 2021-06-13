
// CPosRtspDemoDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include"afxmt.h"//同步操作时需要包含的头文件
using namespace std;
// CCPosRtspDemoDlg dialog
class CCPosRtspDemoDlg : public CDialogEx
{
// Construction
public:
	CCPosRtspDemoDlg(CWnd* pParent = NULL);	// standard constructor
	char FAR* sendbuf;
	int nread;
	int threadCreated = 0;
// Dialog Data
	CString m_strUrl;//视频URL
	int m_exit_rtsp;//控制是否退出线程
	DWORD m_dwPlayerThreadId; //播放线程ID
	DWORD m_dwSenderThreadId; //播放线程ID

	DWORD_PTR m_callbackFun;//回调函数
	DWORD_PTR m_callbackPara;//回调函数的参数

	int RtspThreadStart(CString strUrl, DWORD_PTR callbackFun, DWORD_PTR callbackPara);//播放视频线程开始
	static DWORD WINAPI PlayerWorkThread(LPVOID lpParam);//视频工作线程
	static DWORD WINAPI SendFrameThread(LPVOID lpParam);//

	static int Rtsp_Init();//RTSP初始化, 在程序启动时必须先初始化
	static void Rtsp_Release();//释放RTSP内在,程序结束时释放

	char *m_dib_buffer;//用来存储抓取的图像
	unsigned int m_dib_picture_size;//图像大小

	CCriticalSection g_memCriticalSection;

	int DibBufferWrite(char *data, int width, int height, int bpp);//把每一帧图像实时保存到RAM中

	int CaptureBmpToRam(char *pBuf, int len);

	// int m_iCapture;//抓取图像的张数

// Dialog Data
	enum { IDD = IDD_CPOSRTSPDEMO_DIALOG };

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
public:
	CEdit m_edit_server_url;
	CEdit m_edit_server_port;
	CEdit m_edit_timeout;
	afx_msg void SendFrame(
		LPVOID lpParam
	);
	void CCPosRtspDemoDlg::sendFrameLoop(CString strUrl, CString strPort
		//, CString strFileName
		, CString strTimeOut, LPVOID lpParam);
	afx_msg void OnBnClickedOk();


	//CEdit m_edit_path_name;
	CEdit m_edit_receive;

	CStatic m_static_video;
	CEdit m_edit_num;
	CEdit m_edit_url;

	//afx_msg void OnBnClickedBLoad();
	afx_msg void OnBnClickedBCapture();
	//afx_msg void OnBnClickedBPreview();
	afx_msg void OnClose();
	//afx_msg void OnBnClickedBFrame();
	//afx_msg void OnBnClickedCancel4();
	afx_msg void OnBnClickedCancel();
};
