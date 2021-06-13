
// CPosRtspDemoDlg.cpp : implementation file
//
#include <iostream>
#include <cstdio>
#include "stdafx.h"
#include "CPosRtspDemo.h"
#include "CPosRtspDemoDlg.h"
#include "afxdialogex.h"

#include "Logger.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif




#define USE_SDL

#ifdef USE_SDL
#include "SDL2/SDL.h"
#endif // USE_SDL
#define  _WINSOCK_DEPRECATED_NO_WARNINGS 1


extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavutil/imgutils.h"
};


#pragma comment(lib, "video/lib/avcodec.lib")
#pragma comment(lib, "video/lib/avformat.lib")
#pragma comment(lib, "video/lib/avutil.lib")
#pragma comment(lib, "video/lib/avdevice.lib")
#pragma comment(lib, "video/lib/avfilter.lib")
#pragma comment(lib, "video/lib/postproc.lib")
#pragma comment(lib, "video/lib/swresample.lib")
#pragma comment(lib, "video/lib/swscale.lib")

#ifdef USE_SDL
#pragma comment(lib, "video/lib/SDL2.lib")
#pragma comment(lib, "video/lib/SDL2main.lib")
#endif



#define DIB_BUFFER_SIZE 10000000 //¶¨Òå×¥È¡Í¼ÏñµÄÄÚÔÚ´óÐ¡, 10MB
using namespace std;

std::wstring s2ws(const std::string& s)
{
	int len;
	int slength = (int)s.length() + 1;
	len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
	std::wstring buf;
	buf.resize(len);
	MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength,
		const_cast<wchar_t*>(buf.c_str()), len);
	return buf;
}

// CCPosRtspDemoDlg dialog

CCPosRtspDemoDlg::CCPosRtspDemoDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CCPosRtspDemoDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_dwPlayerThreadId = 0;
	m_dwSenderThreadId = 0;

	m_dib_buffer = new char[DIB_BUFFER_SIZE];
	memset(m_dib_buffer, 0, DIB_BUFFER_SIZE * sizeof(unsigned char));
	m_dib_picture_size = 0;

	// m_iCapture = 0;


}

void CCPosRtspDemoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_VIDEO, m_static_video);
	DDX_Control(pDX, IDC_EDIT_NUM, m_edit_num);
	DDX_Control(pDX, IDC_EDIT_URL, m_edit_url);

	DDX_Control(pDX, IDC_EDIT_RECEIVE, m_edit_receive);

	DDX_Control(pDX, IDC_EDIT_SERVER_URL, m_edit_server_url);
	DDX_Control(pDX, IDC_EDIT_SERVER_PORT, m_edit_server_port);
	DDX_Control(pDX, IDC_EDIT_TIMEOUT, m_edit_timeout);
}

BEGIN_MESSAGE_MAP(CCPosRtspDemoDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	// ON_BN_CLICKED(IDC_B_LOAD, &CCPosRtspDemoDlg::OnBnClickedBLoad)
	ON_BN_CLICKED(IDC_B_CAPTURE, &CCPosRtspDemoDlg::OnBnClickedBCapture)
	// ON_BN_CLICKED(IDC_B_PREVIEW, &CCPosRtspDemoDlg::OnBnClickedBPreview)
	ON_WM_CLOSE()
	// ON_BN_CLICKED(IDC_B_FRAME, &CCPosRtspDemoDlg::OnBnClickedBFrame)
	ON_BN_CLICKED(IDCANCEL, &CCPosRtspDemoDlg::OnBnClickedCancel)
END_MESSAGE_MAP()


BOOL CCPosRtspDemoDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// https://renenyffenegger.ch/notes/development/libraries/cpp/read-configuration-files/
	 // 
	/*

	std::string ipv4 = config("App", "IPv4");
	std::string port = config("App", "Port");
	std::string timeout = config("App", "Timeout");
	std::string rtsp_url = config("App", "RTSP_URL");
	std::string framePerSecond = config("App", "FramePerSecond");

	*/
	char buff[100];

	int nArgs = 0;

	LPWSTR* szAarglist = CommandLineToArgvW(GetCommandLineW(), &nArgs);
	CString strArg1(szAarglist[0]);
	CString strArg2(szAarglist[1]);
	CString strArg3(szAarglist[2]);
	CString strArg4(szAarglist[3]);
	CString strArg5(szAarglist[4]);
	CString strArg6(szAarglist[5]);

	CT2CA psz_app(strArg1);
	CT2CA psz_ipv4(strArg2);
	CT2CA psz_port(strArg3);
	CT2CA psz_rtsp_url(strArg4);
	CT2CA psz_timeout(strArg5);
	CT2CA psz_framePerSecond(strArg6);

	std::string app(psz_app);
	std::string ipv4(psz_ipv4);
	std::string port(psz_port);
	std::string rtsp_url(psz_rtsp_url);
	std::string timeout(psz_timeout);
	std::string framePerSecond(psz_framePerSecond);

	m_edit_server_url.SetWindowTextW(s2ws(ipv4).c_str());
	m_edit_server_port.SetWindowTextW(s2ws(port).c_str());
	m_edit_url.SetWindowTextW(s2ws(rtsp_url).c_str());
	m_edit_timeout.SetWindowTextW(s2ws(timeout).c_str());
	m_edit_num.SetWindowTextW(s2ws(framePerSecond).c_str());

	sprintf(buff, "szAarglist[0]=%s %s %s %s %s %s %d\n", strArg1, strArg2, strArg3, strArg4, strArg5, strArg6, nArgs);
	OutputDebugStringW(s2ws(buff).c_str());

	std::string str = GetCommandLineA();



	//³õÊ¼»¯ÊÓÆµ¿â
	Rtsp_Init();


	OnBnClickedBCapture();
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CCPosRtspDemoDlg::OnPaint()
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
HCURSOR CCPosRtspDemoDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


/*
void CCPosRtspDemoDlg::OnBnClickedBLoad()
{

	CFileDialog dlgOpen(TRUE, _T("*.*"), _T(""), NULL, _T(""));//"*.dat|*.dat|All Files|*.*|"

	if (dlgOpen.DoModal() == IDOK)
	{
		CString strFilePath = dlgOpen.GetPathName();
		m_edit_url.SetWindowTextW(strFilePath);
		//m_edit_path_name.SetWindowTextW(strFilePath);
	}

}

*/

void CCPosRtspDemoDlg::OnBnClickedBCapture()
{
	if (!threadCreated) {
		CString strNum;
		m_edit_num.GetWindowTextW(strNum);

		OutputDebugStringW(L"\nCapture");

		// LOGGER->Log(" Message is:%s Number is:%d", "Capture", _wtoi(strNum));
		//m_iCapture = _wtoi(strNum);
		/*
		// int i = 0;
		for (int i = m_iCapture; i >0; i--)
		{
			if (m_iCapture < 0)
			{
				MessageBox(_T("Capture Complete"));
				break;
			}
			Sleep(200);
		}
		*/

		CString strUrl;
		m_edit_url.GetWindowTextW(strUrl);

		//ÕýÔÚÔËÐÐ, ÏÈÍ£Ö¹
		if (m_dwPlayerThreadId)
		{
			m_exit_rtsp = 1;

			int i = 0;

			for (i = 0; i < 300; i++)
			{
				if (m_dwPlayerThreadId == 0)
				{
					break;
				}
				Sleep(10);
			}
		}

		RtspThreadStart(strUrl, 0, (DWORD_PTR)m_static_video.m_hWnd);
		threadCreated = 1;
	}
}


//»ñÈ¡³ÌÐòÔËÐÐÄ¿Â¼
CString GetAppPath()
{
	CString strPath;
	GetModuleFileName(NULL, strPath.GetBufferSetLength(MAX_PATH + 1), MAX_PATH);
	strPath.ReleaseBuffer();
	int iPos;
	iPos = strPath.ReverseFind(_T('\\'));
	strPath = strPath.Left(iPos + 1);
	return strPath;
}


//¸ù¾ÝDIBÍ¼ÏñµÄÊý¾ÝÉú³ÉBMPÎÄ¼þ
//unsigned char *data Í¼ÏñÊý¾Ý
//int width Í¼Ïñ¿í
//int height Í¼Ïñ¸ß¶È
//int bpp ÑÕÉ«Î»Êý
//CString strName ±£´æÎÄ¼þÃû
/*
void SaveBitmap(unsigned char *data, int width, int height, int bpp, CString strName)
{
	//BMPÎÄ¼þÍ·ÐÅÏ¢
	BITMAPFILEHEADER bmpHeader = { 0 };
	bmpHeader.bfType = ('M' << 8) | 'B';
	bmpHeader.bfReserved1 = 0;
	bmpHeader.bfReserved2 = 0;
	bmpHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	bmpHeader.bfSize = bmpHeader.bfOffBits + width*height*bpp / 8; //BMPÎÄ¼þµÄ´óÐ¡

	BITMAPINFO bmpInfo = { 0 };
	bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmpInfo.bmiHeader.biWidth = width;
	bmpInfo.bmiHeader.biHeight = -height;  // ·´×ªÍ¼Æ¬
	bmpInfo.bmiHeader.biPlanes = 1;
	bmpInfo.bmiHeader.biBitCount = bpp;
	bmpInfo.bmiHeader.biCompression = 0;
	bmpInfo.bmiHeader.biSizeImage = 0;
	bmpInfo.bmiHeader.biXPelsPerMeter = 100;
	bmpInfo.bmiHeader.biYPelsPerMeter = 100;
	bmpInfo.bmiHeader.biClrUsed = 0;
	bmpInfo.bmiHeader.biClrImportant = 0;

	if (strName.IsEmpty()) return;

	//±£´æµ½ÎÄ¼þÖÐ
	CFile f;
	BOOL r = FALSE;
	if (f.Open(strName, CFile::modeCreate | CFile::modeWrite))
	{
		f.Write(&bmpHeader, sizeof(BITMAPFILEHEADER));
		f.Write(&bmpInfo.bmiHeader, sizeof(BITMAPINFOHEADER));
		f.Write(data, width * height * bpp / 8);
		f.Close();
	}


}

*/


//´íÎó»Øµ÷
static int ctx_open_timeout_interrupt_cb(LPVOID lpParam)
{
	CCPosRtspDemoDlg* pThis = (CCPosRtspDemoDlg*)lpParam;
	//return 1; ·
	return 0;//
}


void CCPosRtspDemoDlg::SendFrame(LPVOID lpParam)
{
	CCPosRtspDemoDlg* pThis = (CCPosRtspDemoDlg*)lpParam;
	CString strUrl;
	CString strPort;
	CString strTimeOut;
	m_edit_server_url.GetWindowTextW(strUrl);
	m_edit_server_port.GetWindowTextW(strPort);
	m_edit_timeout.GetWindowTextW(strTimeOut);
	//CString strReceive = 
	pThis->sendFrameLoop(strUrl, strPort, strTimeOut, lpParam);
	//m_edit_receive.SetWindowTextW(strReceive);
}

DWORD WINAPI CCPosRtspDemoDlg::SendFrameThread(LPVOID lpParam)
{
	CCPosRtspDemoDlg* pThis = (CCPosRtspDemoDlg*)lpParam;
	while (1) {
		pThis->SendFrame(lpParam);
	}
	return 1;
}

DWORD WINAPI CCPosRtspDemoDlg::PlayerWorkThread(LPVOID lpParam)
{
	CCPosRtspDemoDlg* pThis = (CCPosRtspDemoDlg*)lpParam;
	int res = 0;
	while (1) {
		// LOGGER->Log(" Message is:%s Number is:%d", "Capture", _wtoi(strNum));
		LOGGER->Log("PlayerWorkThread create 1\n");

		do
		{
			AVFormatContext* pFormatCtx = NULL;	//ÊÓÆµ¸ñÊ½·ÖÎöÆ÷
			int i = 0;
			int videoindex = -1; //ÊÓÆµÍ¨»°(ÒòÎªÒ»¸öÉãÏñÍ·ÖÐ¿ÉÄÜÓÐ¶à¸öRTSPÊÓÆµÍ¨µÀ,ÀýÈç¸ßÇåÍ¨µÀ,±êÇåÍ¨µÀµÈ)
			AVCodecContext* pCodecCtx = NULL;//½âÂëÆ÷
			AVCodec* pCodec = NULL;//
			AVFrame* pFrame = NULL;//ÊÓÆµÖ¡
			AVFrame* pFrameBGR = NULL;//RGB½âÂëÖ¡
			unsigned char* out_buffer = NULL;//½âÂë»º³å,ÓÃÓÚÊµÊ±½âÂëÍ¼Ïñ
			AVPacket* packet = NULL;//ÊÓÆµÍøÂçÊý¾Ý°ü
			int ret = 0;
			int got_picture = 0;//±ê¼Ç½âÂëÊÇ·ñÍê³É
			AVDictionary* opts = NULL;//½âÂëÏà¹Ø²ÎÊýÉèÖÃ


			struct SwsContext* img_convert_ctx = NULL;//Í¼Ïñ×ª»»Æ÷,ÓÃÓÚ°ÑÊÓÆµÊý¾Ý×ª»»³ÉRGBÍ¼Ïñ¸ñÊ½

			char chUrl[256] = { 0 };//ÊÓÆµurl
			int size = 0;
			size = WideCharToMultiByte(CP_ACP, 0, pThis->m_strUrl, -1, chUrl, sizeof(chUrl) - 1, NULL, NULL);  //°ÑCString×ª³ÉcharÀàÐÍ,·µ»Ø³¤¶È,°üÀ¨\0
			chUrl[size] = 0;


#ifdef USE_SDL
			//SDLÍ¼ÏñÏÔÊ¾¿â, ÓÃ´Ë¿â¿ÉÒÔ´ó·ù¶ÈÌáÉýÊÓÆµÏÔÊ¾µÄÐ§ÂÊ
			//------------SDL----------------
			int screen_w = 0;
			int screen_h = 0;
			SDL_Window* screen = NULL;
			SDL_Renderer* sdlRenderer = NULL;
			SDL_Texture* sdlTexture = NULL;
			SDL_Rect sdlRect;
			SDL_Thread* video_tid = NULL;
#endif


			//ÊÓÆµ¸ñÊ½·ÖÎöÆ÷³õÊ¼»¯
			pFormatCtx = avformat_alloc_context();
			pFormatCtx->interrupt_callback.callback = ctx_open_timeout_interrupt_cb;
			pFormatCtx->interrupt_callback.opaque = pThis;
			//pFormatCtx->flags |= AVFMT_FLAG_NONBLOCK; //ÉèÖÃÎª·Ç×èÈûÐÍ



			av_dict_set(&opts, "stimeout", "3000000", 0); // ÉèÖÃ³¬Ê±3Ãë
			av_dict_set(&opts, "rtsp_transport", "tcp", 0);  //ÒÔtcp·½Ê½´ò¿ª, Ò²¿ÉÒÔÊ¹ÓÃudp
			//av_dict_set(&opts, "timeout", "3000000", 0);//ÉèÖÃ³¬Ê±3Ãë


			LOGGER->Log("PlayerWorkThread create 2\n");

			LOGGER->Log("PlayerWorkThread create 2.1\n");
			// ´ò¿ªÊÓÆµÁ÷£¬¶ÁÈ¡ÎÄ¼þÍ·ÐÅÏ¢µ½ pFormatCtx ½á¹¹ÌåÖÐ
			res = avformat_open_input(&pFormatCtx, chUrl, NULL, &opts);
			if (res < 0)
			{
				//	LOGGER->Log("\nCouldn't open input stream.\n");
				continue;
			}
			if (opts) av_dict_free(&opts);

			// ¶ÁÈ¡Á÷ÐÅÏ¢µ½ pFormatCtx->streams ÖÐ
			// pFormatCtx->streams ÊÇÒ»¸öÊý×é£¬Êý×é´óÐ¡ÊÇ pFormatCtx->nb_streams
			res = avformat_find_stream_info(pFormatCtx, NULL);
			if (res < 0)
			{
				//	LOGGER->Log("\nCouldn't find stream information.\n");
				continue;
			}
			LOGGER->Log("PlayerWorkThread create 2.2\n");

			//´ÓÐÅÏ¢Êý×éÖÐ²éÑ¯ÊÓÆµÍ¨µÀ
			//´Ë´¦Ö±½ÓÊ¹ÓÃÁËµÚÒ»¸öÊÓÆµÁ÷Í¨µÀ, ¿ÉÒÔ×Ô¼º¸ù¾ÝÐèÒªÑ¡ÔñÆäËûÊÓÆµÍ¨µÀ
			videoindex = -1;
			for (i = 0; i < pFormatCtx->nb_streams; i++)//¼ì²éÕÒµ½µÄËùÓÐÊÓÆµÁ÷Í¨µÀ
			{
				if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)// ÕÒµ½µÚÒ»¸öÊÓÆµÍ¨µÀ, Ö±½ÓÊ¹ÓÃ´ËÊÓÆµÁ÷Í¨µÀ
				{
					videoindex = i;
					break;
				}
			}
			if (videoindex == -1)//ËùÓÐÍ¨µÀ¶¼²»ÊÇvideo, Ö±½ÓÍË³ö
			{
				//LOGGER->Log("\nDidn't find a video stream.\n");
				res = -1;
				continue;
			}

			LOGGER->Log("PlayerWorkThread create 2.3\n");
			// »ñÈ¡½âÂëÆ÷ÉÏÏÂÎÄ
			pCodecCtx = pFormatCtx->streams[videoindex]->codec;
			// »ñÈ¡½âÂëÆ÷
			pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
			if (pCodec == NULL)
			{
				//LOGGER->Log("\nCodec not found.\n");
				res = -1;
				continue;
			}
			LOGGER->Log("PlayerWorkThread create 2.4\n");

			// ´ò¿ª½âÂëÆ÷
			res = avcodec_open2(pCodecCtx, pCodec, NULL);
			if (res < 0)
			{
				//LOGGER->Log("\nCould not open codec.\n");
				continue;
			}
			LOGGER->Log("PlayerWorkThread create 2.5\n");

			packet = (AVPacket*)av_malloc(sizeof(AVPacket));//ÊÓÆµÍøÂçÊý¾Ý°ü
			//ÎªÊÓÆµÖ¡½âÂëÉêÇëÄÚ´æ¿Õ¼ä
			pFrame = av_frame_alloc();


			//////////////////////////////////////////////////////////////////////////////////////

					//°ÑÊÓÆµ¸ñÊ½µÄÊý¾Ý×ª»»ÎªBMPÍ¼Æ¬Ê¹ÓÃµÄRGB¸ñÊ½ËùÐèÒª×öµÄ×¼±¸¹¤×÷

					//ÎªÊÓÆµÊý¾Ý×ª»»ÎªRGBÊý¾ÝµÄ×ª»»Æ÷ÉêÇëÄÚ´æ
			pFrameBGR = av_frame_alloc();
			int numBytes = av_image_get_buffer_size(AV_PIX_FMT_BGR24, pCodecCtx->width, pCodecCtx->height, 1);//»ñÈ¡×ª»»ÎªRGBËùÐèÒªµÄÄÚ´æ´óÐ¡

			//ÉêÇëÒª±£´æRGBÊý¾ÝµÄ´æ´¢Çø
			out_buffer = (unsigned char*)av_malloc(numBytes * sizeof(unsigned char));
			if (out_buffer == NULL)
			{
				res = 0;
				continue;
			}

			LOGGER->Log("PlayerWorkThread create 2.6\n");
			// Ìî³ä pFrameBGR ÖÐµÄ×Ö¶Î(data¡¢linsizeµÈ)(³õÊ¼»¯RGB¸ñÊ½×ª»»²ÎÊý)
			av_image_fill_arrays(pFrameBGR->data, pFrameBGR->linesize, out_buffer,
				AV_PIX_FMT_BGR24, pCodecCtx->width, pCodecCtx->height, 1);

			// »ñÈ¡Í¼Ïñ×ª»»Æ÷
			img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
				pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_BGR24, SWS_BICUBIC, NULL, NULL, NULL);
			if (img_convert_ctx == NULL)
			{
				res = -1;
				continue;
			}

			LOGGER->Log("PlayerWorkThread create 2.7\n");
			//////////////////////////////////////////////////////////////////////////////////////



#ifdef USE_SDL

		//ÉèÖÃÏÔÊ¾Í¼ÏñÊ±SDL¿âµÄ²ÎÊý

		//Èç¹û¸Õ¿ªÊ¼Á¬½ÓÊ±ÍË³ö, ÒòÎª´°ÌåÏú»Ù, µ¼ÖÂ³öÀ´´°Ìå´íÎó
			if (pThis->m_exit_rtsp)
			{
				continue;
			}
			LOGGER->Log("PlayerWorkThread create 2.8\n");

			//ÉèÖÃÁËÊÓÆµÏÔÊ¾´°¿Ú, ÏÔÊ¾ÊÓÆµµ½´°¿ÚÖÐ, ÉèÖÃÏÔÊ¾²ÎÊý
			if (pThis->m_callbackPara && pCodecCtx->width > 0)	//ÐèÒªÏÔÊ¾
			{

				screen_w = pCodecCtx->width;
				screen_h = pCodecCtx->height;

				//Èç¹ûÐèÒª¶ÀÁ¢µÄ´°¿ÚÏÔÊ¾ÊÓÆµ
				//screen = SDL_CreateWindow("Simplest ffmpeg player's Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
				//	screen_w, screen_h,SDL_WINDOW_OPENGL);

				//²»ÐèÒª¶ÀÁ¢´°¿Ú,Ê¹ÓÃwindows½çÃæÖÐµÄ´°¿Ú
				screen = SDL_CreateWindowFrom((void*)(pThis->m_callbackPara));
				if (!screen)
				{
					//LOGGER->Log("\nSDL: could not create window - exiting:%s\n", SDL_GetError());
					res = -1;
					continue;
				}
				LOGGER->Log("PlayerWorkThread create 2.8.1\n");
				sdlRenderer = SDL_CreateRenderer(screen, -1, 0);
				//ÉèÖÃÏÔÊ¾µÄÍ¼Ïñ¸ñÊ½ÎªBRG24¸ñÊ½(ÒòÎªÇ°Ãæ½âÂëÊ±Ê¹ÓÃµÄÊÇBRG24)
				sdlTexture = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_BGR24, SDL_TEXTUREACCESS_STREAMING, pCodecCtx->width, pCodecCtx->height);//SDL_PIXELFORMAT_IYUV
				if (!sdlTexture)
				{
					res = -1;
					continue;
				}

				LOGGER->Log("PlayerWorkThread create 2.8.2\n");
				sdlRect.x = 0;
				sdlRect.y = 0;
				sdlRect.w = screen_w;
				sdlRect.h = screen_h;
				//------------SDL End------------
				::ShowWindow((HWND)pThis->m_callbackPara, SW_SHOWNORMAL);//ÏÔÊ¾windowsÖÐµÄ´°¿Ú
			}

#endif

			CString strFileName;
			CString strExePath;

			strExePath = GetAppPath();
			int read_frame_result = 0;
			// int frame_count = 0;//ÊµÊ±½âÂëÖ¡ÊýÍ³¼Æ

			memset(pThis->m_dib_buffer, 0, DIB_BUFFER_SIZE * sizeof(unsigned char));
			pThis->m_dib_picture_size = 0;


			//LOGGER->Log("\nPlayerWorkThread create 2.9");
			//ÒÔÉÏ×¼±¸¹¤×÷Íê³É, ×¼±¸½âÂëÊÓÆµ,×ª»»RGBÍ¼Ïñ¸ñÊ½,±£´æµ½ÎÄ¼þ
			LOGGER->Log("\nWhile begin");
			while (1)
			{
				//Sleep(1000);
				/*
				Sleep(1000);

				//ÓÃ»§¿ªÊ¼×¥Í¼ > 0±íÊ¾¿ªÊ¼×¥È¡, ==0 ±íÊ¾ÕýÔÚ×¥È¡, < 0 ±íÊ¾Íê³É
				if (pThis->m_iCapture > 0)
				{
					capture_amount = pThis->m_iCapture;
					pThis->m_iCapture = 0;
					capture_cnt = 0;

				}
				*/
				LOGGER->Log("\nWhile e1");
				while (1)
				{
					LOGGER->Log("\nWhile e2.1");
					read_frame_result = av_read_frame(pFormatCtx, packet);
					if (read_frame_result < 0)
					{
						break;
					}
					if (packet->stream_index == videoindex)
					{
						break;
					}
					else
					{
						av_free_packet(packet);
					}
					if (pThis->m_exit_rtsp)
					{
						break;
					}
					LOGGER->Log("\nWhile e2.2");
				}
				LOGGER->Log("\nWhile e1.1");
				if (read_frame_result < 0 || pThis->m_exit_rtsp)
				{
					break;
				}
				LOGGER->Log("\nWhile e1.2");

				//Sleep(1000);
				res = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);
				//LOGGER->Log("\nWhile e1.2.1");

				//Sleep(1000);
				if (res < 0)//½âÂë´íÎó
				{
					LOGGER->Log("\nDecode Error.\n");
					res = -1;
					break;
				}
				// frame_count++;
				LOGGER->Log("\nWhile e1.3");
				if (got_picture)
				{
					LOGGER->Log("\nGot picture");
					//Sleep(1000);
					sws_scale(img_convert_ctx, pFrame->data, pFrame->linesize, 0, pCodecCtx->height, pFrameBGR->data, pFrameBGR->linesize);

					pThis->DibBufferWrite((char*)pFrameBGR->data[0], pCodecCtx->width, pCodecCtx->height, 24);
					/*
					if (capture_amount && capture_cnt < capture_amount)
					{
					*/
					LOGGER->Log("\nWhile e1.3.1");
					//Sleep(1000);
					//LOGGER->Log("\nWhile e1.3.2");
					//COleDateTime timeNow;
					//timeNow = COleDateTime::GetCurrentTime();
					// capture_cnt++;
					// strFileName.Format(_T("%s%s_%d.bmp"), strExePath, timeNow.Format(_T("%Y-%m-%d %H.%M.%S")), capture_cnt);
					// pThis->SendFrame((char*)pFrameBGR->data[0], pCodecCtx->width* pCodecCtx->height* 24 / 8, lpParam);
					//
					// 
					pThis->sendbuf = (char*)pFrameBGR->data[0];
					pThis->nread = pCodecCtx->width * pCodecCtx->height * 24 / 8;
					// SaveBitmap(pFrameBGR->data[0], pCodecCtx->width, pCodecCtx->height, 24, strFileName);
					/*
					if (capture_cnt >= capture_amount)pThis->m_iCapture = -1; //×îºóÒ»ÕÅ±£´æºó½áÊø
					}
					else
					{
						capture_amount = capture_cnt = 0;

					}
						*/
					LOGGER->Log("\nWhile e1.3.3");
#ifdef USE_SDL
					if (pThis->m_callbackPara && pCodecCtx->width > 0)
					{
						LOGGER->Log("\nWhile e1.3.3.1");
						//SDL---------------------------
						SDL_UpdateTexture(sdlTexture, NULL, pFrameBGR->data[0], pFrameBGR->linesize[0]);
						SDL_RenderClear(sdlRenderer);
						//SDL_RenderCopy( sdlRenderer, sdlTexture, &sdlRect, &sdlRect );  
						SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, NULL);
						SDL_RenderPresent(sdlRenderer);
						LOGGER->Log("\nWhile e1.3.3.2");
					}
					//SDL End-----------------------
#endif
					LOGGER->Log("\nWhile e1.3.4");
					/*

					if (chUrl[1] == ':')
					{
						Sleep(33);
					}
					*/
					LOGGER->Log("\nWhile e1.3.5");
				}
				//Sleep(1000);
				LOGGER->Log("\nWhile e1.4");
				av_free_packet(packet);
				//LOGGER->Log("\nWhile e1.5");
				if (pThis->m_exit_rtsp)
				{
					break;
				}
				LOGGER->Log("\nWhile e1.6");

				/*

				if (pCodecCtx != nullptr)
				{
					free(pCodecCtx);
				}
				if (pFrame != nullptr)
				{
					delete[] pFrame;
				}

				if (packet != nullptr)
				{
					delete[] packet;
				}
				*/
				//			delete[] pFrame;
							// delete got_picture;
							//delete[] packet;
							// free(pFrameBGR->data);
			}
			//Sleep(1000);
			LOGGER->Log("\nWhile e5");
#ifdef USE_SDL
			if (screen_w)
			{
				if (sdlTexture) SDL_DestroyTexture(sdlTexture);
				if (sdlRenderer) SDL_DestroyRenderer(sdlRenderer);
				if (screen)
				{
					SDL_DestroyWindow(screen);
					::ShowWindow((HWND)pThis->m_callbackPara, SW_SHOWNORMAL);
				}
			}
#endif

			if (img_convert_ctx) sws_freeContext(img_convert_ctx);
			if (out_buffer) av_free(out_buffer);
			if (packet) av_free(packet);
			if (pFrameBGR) av_frame_free(&pFrameBGR);
			if (pFrame) av_frame_free(&pFrame);
			if (pCodecCtx) avcodec_close(pCodecCtx);
			if (pFormatCtx) avformat_close_input(&pFormatCtx);
		} while (1);
		LOGGER->Log("\nWhile e5.1");

		memset(pThis->m_dib_buffer, 0, DIB_BUFFER_SIZE * sizeof(unsigned char));
		pThis->m_dib_picture_size = 0;
		pThis->m_dwPlayerThreadId = NULL;
		pThis->m_dwSenderThreadId = NULL;

	}
	return res;
}



int CCPosRtspDemoDlg::RtspThreadStart(CString strUrl, DWORD_PTR callbackFun, DWORD_PTR callbackPara)
{
	OutputDebugStringW(L"\nRtspThreadStart create 1");
	if (strUrl.IsEmpty()) return 0;


	// ´´½¨¹¤×÷ÕßÏß³Ì  
	if (m_dwPlayerThreadId == 0)
	{

		m_strUrl = strUrl;//Òª²¥·ÅÊÓÆµµÄURL
		m_callbackFun = callbackFun;
		m_callbackPara = callbackPara;
		m_exit_rtsp = FALSE;

		CreateThread(NULL, 0, PlayerWorkThread, this, 0, &m_dwPlayerThreadId);
		CreateThread(NULL, 0, SendFrameThread, this, 0, &m_dwSenderThreadId);
	}

	return 1;

}

int CCPosRtspDemoDlg::Rtsp_Init()
{
	int res = 0;
	av_register_all();
	avcodec_register_all();
	avformat_network_init();
#ifdef USE_SDL
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER))
	{
		res = -1;
	}
#endif
	return res;
}

void CCPosRtspDemoDlg::Rtsp_Release()
{
#ifdef USE_SDL

	SDL_Quit();
#endif
}

int CCPosRtspDemoDlg::DibBufferWrite(char* data, int width, int height, int bpp)
{
	if (data == NULL)
	{
		m_dib_picture_size = 0;
		return 0;
	}

	int buf_size = DIB_BUFFER_SIZE;
	int len = 0;
	int dib_size = width * height * bpp / 8;
	BITMAPFILEHEADER bmpHeader = { 0 };
	bmpHeader.bfType = ('M' << 8) | 'B';
	bmpHeader.bfReserved1 = 0;
	bmpHeader.bfReserved2 = 0;
	bmpHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	bmpHeader.bfSize = bmpHeader.bfOffBits + width * height * bpp / 8;

	BITMAPINFO bmpInfo = { 0 };
	bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmpInfo.bmiHeader.biWidth = width;
	bmpInfo.bmiHeader.biHeight = -height;  // ·´×ªÍ¼Æ¬
	bmpInfo.bmiHeader.biPlanes = 1;
	bmpInfo.bmiHeader.biBitCount = bpp;
	bmpInfo.bmiHeader.biCompression = 0;
	bmpInfo.bmiHeader.biSizeImage = 0;
	bmpInfo.bmiHeader.biXPelsPerMeter = 100;
	bmpInfo.bmiHeader.biYPelsPerMeter = 100;
	bmpInfo.bmiHeader.biClrUsed = 0;
	bmpInfo.bmiHeader.biClrImportant = 0;

	g_memCriticalSection.Lock();
	memcpy(m_dib_buffer + len, (&bmpHeader), sizeof(BITMAPFILEHEADER));
	len += sizeof(BITMAPFILEHEADER);

	memcpy(m_dib_buffer + len, (&bmpInfo.bmiHeader), sizeof(BITMAPINFOHEADER));
	len += sizeof(BITMAPINFOHEADER);
	if (len + dib_size <= buf_size)
	{
		//Sleep(1000);
		memcpy(m_dib_buffer + len, data, dib_size);
		len += dib_size;
	}
	m_dib_picture_size = len;
	g_memCriticalSection.Unlock();
	return len;

}

int CCPosRtspDemoDlg::CaptureBmpToRam(char* pBuf, int len)
{

	if (m_dib_picture_size)
	{
		if (len > m_dib_picture_size)
		{
			g_memCriticalSection.Lock();
			memcpy(pBuf, m_dib_buffer, m_dib_picture_size);
			g_memCriticalSection.Unlock();
		}
		else
		{
			return 0;
		}
	}
	else
	{
		return 0;
	}

	return m_dib_picture_size;
}






void CCPosRtspDemoDlg::OnClose()
{
	/*


	*/
	m_exit_rtsp = 1;//ÍË³öÏß³Ì

	int i = 0;

	//µÈÏß³ÌÍË³ö
	for (i = 0; i < 300; i++)
	{
		if (m_dwPlayerThreadId == 0)
		{
			break;
		}
		Sleep(10);
	}



	delete[]m_dib_buffer;


	CDialogEx::OnClose();
}

/*
//×¥È¡Ò»Ö¡Í¼Ïñ, ±£´æµ½ÎÄ¼þÖÐ
void CCPosRtspDemoDlg::OnBnClickedBFrame()
{
	char *buf = 0;
	buf = new char[DIB_BUFFER_SIZE];
	int size = 0;
	size = CaptureBmpToRam(buf, DIB_BUFFER_SIZE);

	if (size)
	{
		CString str = _T("bmp(*.bmp)|*.bmp|All Files (*.*)|*.*||");
		CFileDialog dlgOpen(FALSE, _T(".bmp"), _T(""), NULL, str);

		if (dlgOpen.DoModal() == IDOK)
		{
			CString strFile = dlgOpen.GetPathName();
			//±£´æµ½ÎÄ¼þÖÐ
			CFile f;
			BOOL r = FALSE;
			if (f.Open(strFile, CFile::modeCreate | CFile::modeWrite))
			{

				f.Write(buf, size);
				f.Close();
			}

		}
	}


	delete []buf;

}


*/
void CCPosRtspDemoDlg::sendFrameLoop(CString strUrl, CString strPort
	, CString strTimeOut, LPVOID lpParam)
{
	CCPosRtspDemoDlg* pThis = (CCPosRtspDemoDlg*)lpParam;
	char buff[100];
	while (true) {
		SOCKET sockClient;
		SOCKADDR_IN addrServer;
		sockClient = socket(AF_INET, SOCK_STREAM, 0);
		char chServer[256] = { 0 };
		char chClientId[256] = { 0 };
		WideCharToMultiByte(CP_ACP, 0, strUrl, -1, chServer, sizeof(chServer) - 1, NULL, NULL);
		int timeout = _wtoi(strTimeOut);
		if (timeout < 1) timeout = 1;
		addrServer.sin_addr.s_addr = inet_addr((const char*)chServer);
		addrServer.sin_family = AF_INET;
		addrServer.sin_port = htons(_wtoi(strPort));
		struct timeval timeoutRecv = { 0 };
		timeoutRecv.tv_sec = timeout * 1000;
		setsockopt(sockClient, SOL_SOCKET, SO_SNDTIMEO, (const char*)&timeoutRecv, sizeof(timeoutRecv));
		setsockopt(sockClient, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeoutRecv, sizeof(timeoutRecv));
		int iResult = connect(sockClient, (SOCKADDR*)&addrServer, sizeof(SOCKADDR));
		if (iResult == NO_ERROR)
		{
			WSADATA wsaData;
			WSAStartup(MAKEWORD(2, 2), &wsaData);
			int rc = 0;
			size_t count = 0;
			g_memCriticalSection.Lock();
			rc = send(sockClient, pThis->m_dib_buffer, pThis->nread, 0);
			g_memCriticalSection.Unlock();
			if (rc < 0)
			{
				//OutputDebugStringW(L"Sent complete\n");
			}
			char* ptr = 0;
			do {
				if (nread <= 0)
				{
					break;
				}
				ptr = sendbuf;
				int zero_cnt = 0;
				do {
					rc = send(sockClient, ptr, nread, 0);
					if (rc < 0)
					{
						break;
					}
					if (rc == 0)
					{
						zero_cnt++;
						if (zero_cnt >= 3) break;
					}
					else
					{
						zero_cnt = 0;
					}

					ptr += rc;
					count += nread;
					nread -= rc;
				} while (nread);
			} while (rc > 0);
			rc = send(sockClient, sendbuf, 1, 0);
			/*
			char recvBuf[32] = { 0 };
			int rs = 0;
			int recv_len = 0;
			CString strReceive;
			do
			{
				memset(recvBuf, 0, sizeof(recvBuf));
				recv_len = recv(sockClient, recvBuf, 31, 0);
				if (recv_len < 0)
				{
					break;
				}
				else if (recv_len == 0)
				{
				}
				if (recv_len != 31)
				{
					rs = 0;
				}
				else
				{
					rs = 1;
				}
				strReceive += CString(recvBuf);
			} while (rs);
			snprintf(buff, sizeof(buff), "strReceive %s\n", strReceive);
			OutputDebugStringW(s2ws(buff).c_str());
			*/
		}
		closesocket(sockClient);
		WSACleanup();
		shutdown(sockClient, SD_SEND);
	}
}

/*
void CCPosRtspDemoDlg::OnBnClickedCancel4()
{
	// TODO: Add your control notification handler code here
	CDialogEx::OnCancel();
}

*/

//¼ÆËãÐ£ÑéÂë£¬³¤¶È²»°üº¬Ð£ÑéÂë±¾Éí
unsigned char checksum(unsigned char* str, int len)
{
	int i;
	unsigned char mod = 0;
	for (i = 0; i < len; i++)
	{
		mod += *(str + i);
	}
	return -mod;
}


void CCPosRtspDemoDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	CDialogEx::OnOK();
}


void CCPosRtspDemoDlg::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	CDialogEx::OnCancel();
}
