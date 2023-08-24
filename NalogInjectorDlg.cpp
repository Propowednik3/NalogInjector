// NalogInjectorDlg.cpp : implementation file
//

#include "stdafx.h"
#include "NalogInjector.h"
#include "NalogInjectorDlg.h"
#include <curl.h>
#include "AzWindows.h"
#include "Shlwapi.h"
#include <process.h>
#include "unzip.h"
#include "..\AZ\AzOci.h"

#pragma comment (lib, "shlwapi.lib")
#pragma comment (lib, "libcurl.lib")

#define		MYMSG_NOTIFYICON		(WM_APP + 100)

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

struct DataList 
{
	char Dir[128];
	char Name[128];
	char Path[128];
	char Status;
};

struct TimeList 
{
	char Hour;
	char Min;
	char Status;
};

int LoadSettings(char *Buff);
int ParsePage(char* pBuffer, unsigned int uiDataLen, DataList **ppList, unsigned int *puiItemCount, char *cDir);
int SaveSettings(char *Buff);
int DownloadFile(char *cPath, char *cSavePath);
int UnzipFile(char *pPath);
void OnAction(int iSort, char cUseLoaded);
void AllAction(void* pData);
int DownloadAddress(char* cPath, char**pBuffer, unsigned int *uiLen);
void ListPrint(LPTSTR txt, ...);
VOID AnimateIcon(HINSTANCE hInstance, HWND hWnd, DWORD dwMsgType,UINT nIndexOfIcon, char * cCaption);
	

CURL *curl;	
char cStatus;
char cAutoMode;
char cUseExistedFiles;
char cServer[64];
char cUlPath[64];
char cIpPath[64];
char cLogin[64];
char cDebugPath[256];
char cDebug;
char cPassword[64];
char cClntCert[64];
char cCaCert[64];
char cKeyCert[64];
char cKeyPass[64];
char cWorkDir[256];
char cArchDir[256];
char cErrorDir[256];
char cOraDataBase[64], cOraLogin[64], cOraPassword[64];
//unsigned int iDirCnt;
//DataList *pDirList;
unsigned int iTimeCnt;
TimeList *pTimeList;
char cActionExec;
HWND hWndList;

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	public:
	virtual BOOL DestroyWindow();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
	ON_WM_SIZE()	
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNalogInjectorDlg dialog

CNalogInjectorDlg::CNalogInjectorDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CNalogInjectorDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CNalogInjectorDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CNalogInjectorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNalogInjectorDlg)
	DDX_Control(pDX, IDC_CHECK2, m_Scroll);
	DDX_Control(pDX, IDC_CHECK1, m_UseLoaded);
	DDX_Control(pDX, IDC_LIST1, m_List);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CNalogInjectorDlg, CDialog)
	//{{AFX_MSG_MAP(CNalogInjectorDlg)
	ON_WM_TIMER()
	ON_MESSAGE(MYMSG_NOTIFYICON, OnMsgIcon)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_SIZE()
	ON_LBN_DBLCLK(IDC_LIST1, OnDblclkList1)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNalogInjectorDlg message handlers

BOOL CNalogInjectorDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	hWndList = m_List.m_hWnd;
	AnimateIcon(AfxGetInstanceHandle(), m_hWnd, NIM_ADD, 1, "Nalog Injector");

	char buffpath[MAX_PATH];
	char bufffile[MAX_PATH];	
	
	GetModuleFileName(NULL, buffpath, MAX_PATH);
	PathRemoveFileSpec(buffpath);
	SetCurrentDirectory(buffpath);
	
	cAutoMode = 0;
	cUseExistedFiles = 0;
	iTimeCnt = 0;
	pTimeList = NULL;
	cDebug = 0;
	memset(cDebugPath, 0, 256);
	memset(cLogin, 0, 64);
	memset(cPassword, 0, 64);
	memset(cServer, 0, 64);
	memset(cUlPath, 0, 64);
	memset(cIpPath, 0, 64);
	memset(cClntCert, 0, 64);
	memset(cCaCert, 0, 64);
	memset(cKeyCert, 0, 64);
	memset(cKeyPass, 0, 64);
	memset(cWorkDir, 0, 256);
	memset(cArchDir, 0, 256);
	memset(cErrorDir, 0, 256);
	memset(cOraDataBase, 0, 64);
	memset(cOraLogin, 0, 64);
	memset(cOraPassword, 0, 64);
	
	
	memset(bufffile, 0, MAX_PATH);
	strcpy(bufffile, buffpath);
	strcat(bufffile, "\\");
	strcat(bufffile, "config.ini");
	ListPrint("LoadSettings:'%s'", bufffile);
	LoadSettings(bufffile);
	ListPrint("LoadSettings:Done");
	memset(bufffile, 0, MAX_PATH);
	strcpy(bufffile, buffpath);
	strcat(bufffile, "\\");
	strcat(bufffile, "skin.bmp");
	
	ListPrint("Debug:'%i'", cDebug);
	ListPrint("DebugPath:'%s'", cDebugPath);
	ListPrint("Login:'%s'", cLogin);
	ListPrint("Password:'%s'", cPassword);
	ListPrint("Server:'%s'", cServer);
	ListPrint("UlPath:'%s'", cUlPath);
	ListPrint("IpPath:'%s'", cIpPath);
	ListPrint("AutoMode:%i", cAutoMode);
	ListPrint("ClientCert:%s", cClntCert);
	ListPrint("CACert:%s", cCaCert);
	ListPrint("KeyCert:%s", cKeyCert);
	ListPrint("KeyPassword:%s", cKeyPass);
	ListPrint("WorkDir:%s", cWorkDir);	
	ListPrint("ArchDir:%s", cArchDir);	
	ListPrint("ErrorDir:%s", cErrorDir);	
	ListPrint("OraDataBase:%s", cOraDataBase);
	ListPrint("OraLogin:%s", cOraLogin);
	ListPrint("OraPassword:%s", cOraPassword);	
	ListPrint("UseLoadedFiles:%i", cUseExistedFiles);	
	for (unsigned int i = 0; i < iTimeCnt; i++) ListPrint("Time:%i:%i", pTimeList[i].Hour, pTimeList[i].Min);
	
	/*if (cAutoMode == 0) AnimateIcon(AfxGetInstanceHandle(), m_hWnd, NIM_ADD, 1, "Disable");
		else AnimateIcon(AfxGetInstanceHandle(), m_hWnd, NIM_ADD, 5, "Wait");
	uiTimeCount = 0;*/
	
	m_UseLoaded.SetCheck(cUseExistedFiles);

	curl = curl_easy_init();

	cActionExec = 0;
	
	/*SetTimer(0, uiScanTime, NULL);*/
	if (cAutoMode) SetTimer(2, 30000, NULL);
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CNalogInjectorDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CNalogInjectorDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

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
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CNalogInjectorDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CNalogInjectorDlg::OnMsgIcon(WPARAM wParam, LPARAM lParam)
{
	switch(lParam)
	{
	case WM_RBUTTONDOWN:
		{
			break ;
		}
	case WM_LBUTTONDOWN:
		{	
			break ;
		}
	case WM_LBUTTONDBLCLK:
		{
			ShowWindow(SW_SHOW);
			break ;
		}
	case WM_RBUTTONDBLCLK:
		{
			ExitProcess(0);
			break ;
		}
	}		
}


void ListPrint(LPTSTR txt, ...)
{
	char szBuffer[MAXSIZEMESSAGE];
	char buff[MAXSIZEMESSAGE];
	DWORD tick = GetTickCount();
	memset(buff, 0, MAXSIZEMESSAGE);
	itoa(tick, buff,10);
	strcat(buff, ":>> ");
	memset(szBuffer, 0, MAXSIZEMESSAGE);
	//strcpy(szBuffer, ">>>>>>>>> ");
	va_list valist;
	va_start(valist, txt);		
	int nResult = wvsprintf(szBuffer, txt,	valist);		
	va_end(valist);
	strcat(buff, szBuffer);
	SendMessage(hWndList, LB_ADDSTRING, 0, (LPARAM)buff);
	lPrint(buff);
/*	if (CNalogInjectorDlg::m_Scroll.GetCheck()) 
	{
		SendMessage(hWndList, LB_SETCURSEL, 0, (LPARAM)0);
	}	*/
}

void CNalogInjectorDlg::OnTimer(UINT nIDEvent)
{
	SYSTEMTIME st, lt;

	while (m_List.GetCount() > 1000) m_List.DeleteString(0);

	GetSystemTime(&st);
    GetLocalTime(&lt);
	
	//ListPrint("Current time:%i:%i", lt.wHour, lt.wMinute);
	
	unsigned int i;
	for (i = 0; i < iTimeCnt; i++)
	{
		if ((lt.wHour == pTimeList[i].Hour) && (lt.wMinute == pTimeList[i].Min))
		{
			if (pTimeList[i].Status == 1)
			{
				pTimeList[i].Status = 0;
				ListPrint("Action time:%i:%i", lt.wHour, lt.wMinute);
				if (cActionExec == 0) AllAction(NULL);				
			}
		} else if (pTimeList[i].Status == 0) pTimeList[i].Status = 1;
	}
}

void CNalogInjectorDlg::OnOK()
{
	if (cActionExec == 0) _beginthread(AllAction, 0, (void*)m_UseLoaded.GetCheck());
}

void AllAction(void* pData)
{
	OnAction(0, (char)pData);
	OnAction(1, (char)pData);
}

void OnAction(int iSort, char cUseLoaded) 
{
	ListPrint("Start Scan Address %i", iSort);
	cActionExec = 1;
	char *pBuffer;
	unsigned int uiDataLen;
	char cPath[256];
	char cPath2[256];
	char cRequest[1024];
	unsigned int i, n;
	DataList *pList = NULL;
	unsigned int uiItemCount = 0;
	HRESULT hRes;
	char ErrorBuff[MAX_PATH];
	char cOciInit = 0;
	char cOciInit2 = 0;
	Oci azOci;
	Oci exOci;
	
	if (cOciInit == 0)
	{
		hRes = azOci.Init((PUCHAR)cOraDataBase, (PUCHAR)cOraLogin, (PUCHAR)cOraPassword);
		if (hRes == 1) cOciInit = 1;
		else
		{
			azOci.GetTextError(ErrorBuff);
			ListPrint("Error init OCI %i: %s", hRes, ErrorBuff);
		}
	}

	if (cOciInit2 == 0)
	{
		hRes = exOci.Init((PUCHAR)cOraDataBase, (PUCHAR)cOraLogin, (PUCHAR)cOraPassword);
		if (hRes == 1) cOciInit2 = 1;
		else
		{
			exOci.GetTextError(ErrorBuff);
			ListPrint("Error init OCI %i: %s", hRes, ErrorBuff);
		}
	}


	memset(cPath, 0, 256);
	strcpy(cPath, cServer);
	if (iSort == 0) 
	{
		strcat(cPath, cUlPath);
		ListPrint("Download UL Address:%s", cPath);
	}	
	if (iSort == 1) 
	{
		strcat(cPath, cIpPath);
		ListPrint("Download IP Address:%s", cPath);
	}
	DownloadAddress(cPath, &pBuffer, &uiDataLen);
	if (uiDataLen)
	{
		ListPrint("Downloaded datalen: '%i'", uiDataLen);
				
		if (cDebug && (cDebugPath[0] > 0)) 
		{
			memset(cRequest, 0, 1024);
			strcpy(cRequest, cDebugPath);
			strcat(cRequest, "\\");
			if (iSort == 0) strcat(cRequest, "\\egrul_page.html");
			if (iSort == 1) strcat(cRequest, "\\egrip_page.html");
			SaveData(pBuffer, uiDataLen, cRequest);
		}

		pList = NULL;
		uiItemCount = 0;
		ParsePage(pBuffer, uiDataLen, &pList, &uiItemCount, NULL);
		free(pBuffer);

		if (uiItemCount)
		{
			for (i = 0; i < uiItemCount; i++)
			{	
				memset(cRequest, 0, 1024);
				sprintf(cRequest, "select count(*) from ORGR_FILENAMES where upper(name) = upper('%s') and upper(path) = upper('%s') and type = 0 and sort = %i and path like '%s%%'", pList[i].Name, pList[i].Path, iSort, iSort ? cIpPath : cUlPath);
				lPrint("Test exist DIR '%s' in base", pList[i].Name);
				lPrint("ExecuteRequest: '%s'", cRequest);
				n = -1;			
				hRes = azOci.ExecuteRequestIntRet((unsigned char*)cRequest, (int*)&n);
				if (hRes < 0)
				{
					azOci.GetTextError(ErrorBuff);
					ListPrint("Error test exist DIR '%s' in base %i: %s", pList[i].Name, hRes, ErrorBuff);
					n = -1;
				}
				
				if (n == 0)
				{
					ListPrint("Add Dir Name: %s, Path: %s", pList[i].Name, pList[i].Path);
					memset(cRequest, 0, 1024);
					sprintf(cRequest, "insert into ORGR_FILENAMES (path, name, type, sort, status) values ('%s', '%s', %i, %i, %i)", pList[i].Path, pList[i].Name, 0, iSort, 0);
					lPrint("ExecuteRequest: '%s'", cRequest);
					hRes = azOci.ExecuteRequest((unsigned char*)cRequest);
					if (hRes < 0)
					{
						azOci.GetTextError(ErrorBuff);
						ListPrint("Error test exist DIR '%s' in base %i: %s", pList[i].Name, hRes, ErrorBuff);
					}
				}
				
			}
			free(pList);
		}			
	} 
	else
	{
		lPrint("DownloadAddress Failed");
		ListPrint("DownloadAddress Failed");
	}	
	pList = NULL;
	uiItemCount = 0;
	
	n = 0;
	unsigned int prev_val;
	
	exOci.CreateParamList(0);
	memset(cRequest, 0, 1024);
	sprintf(cRequest, "select distinct to_char(o.id), o.path, o.name from ORGR_FILENAMES o \
								left join ORGR_FILENAMES f on f.parent = o.id \
								where o.type = 0 and o.sort = %i and ((f.status = 0) or (o.status = 0)) \
								and o.path like '%s%%'", iSort, iSort ? cIpPath : cUlPath);
	lPrint("ExecuteRequest : '%s'", cRequest);
	hRes = exOci.ExecuteRequestEx((unsigned char*)cRequest);
	if (!hRes)	
	{
		if (hRes != 0)
		{
			exOci.GetTextError(ErrorBuff);
			ListPrint("Error : %s", ErrorBuff);
		}
	}
	else
	{
		ListPrint("Load File List");
		do
		{
			memset(cPath, 0, 256);
			strcpy(cPath, cServer);
			strcat(cPath, (char*)exOci.RowList[1].Data);
			ListPrint("Download File List:%s", cPath);
			DownloadAddress(cPath, &pBuffer, &uiDataLen);
			if (uiDataLen)
			{
				ListPrint("Downloaded datalen: '%i'", uiDataLen);
				if (cDebug && (cDebugPath[0] > 0)) 
				{
					memset(cRequest, 0, 1024);
					strcpy(cRequest, cDebugPath);
					strcat(cRequest, "\\");
					if (iSort == 0) strcat(cRequest, "\\egrul_list.html");
					if (iSort == 1) strcat(cRequest, "\\egrip_list.html");
					SaveData(pBuffer, uiDataLen, cRequest);
				}
				
				memset(cRequest, 0, 1024);
				sprintf(cRequest, "update ORGR_FILENAMES set status = 1 where id = %s", (UINT)((char*)exOci.RowList[0].Data));
				lPrint("ExecuteRequest : '%s'", cRequest);
				hRes = azOci.ExecuteRequest((unsigned char*)cRequest);
				prev_val = uiItemCount;
				ParsePage(pBuffer, uiDataLen, &pList, &uiItemCount, (char*)exOci.RowList[2].Data);
				free(pBuffer);
				if (prev_val == uiItemCount) ListPrint("No files in: '%s'", cPath);
			} else ListPrint("Error download address: '%s'", cPath);
		} while(exOci.GetNextData());
	}

	if ((uiItemCount) && (cOciInit != 0) && (cOciInit2 != 0))
	{
		int iErrors = 0;
		for (i = 0; i < uiItemCount; i++)
		{
			iErrors = 0;
			memset(cRequest, 0, 1024);
			sprintf(cRequest, "select count(*) from ORGR_FILENAMES where upper(name) = upper('%s') and type = 1 and status = 1", pList[i].Name);
			ListPrint("Test exist ZIP file '%s' in base", pList[i].Name);
			lPrint("ExecuteRequest: '%s'", cRequest);
			n = -1;			
			hRes = azOci.ExecuteRequestIntRet((unsigned char*)cRequest, (int*)&n);
			if (hRes < 0)
			{
				azOci.GetTextError(ErrorBuff);
				ListPrint("Error test exist ZIP file '%s' in base %i: %s", pList[i].Name, hRes, ErrorBuff);
				n = -1;
			}
			if ((hRes >= 0) && (n > 0)) ListPrint("Exist ZIP file '%s' in base: SKIP", pList[i].Name);
			if ((hRes >= 0) && (n == 0))
			{				
				memset(cPath, 0, 256);
				strcpy(cPath, cServer);
				strcat(cPath, pList[i].Path);
				memset(cPath2, 0, 256);
				strcpy(cPath2, cArchDir);
				strcat(cPath2, pList[i].Dir);
				if (CreateDirectory(cPath2, NULL) != 0) ListPrint("Create Directory:'%s'", cPath2);;
				strcat(cPath2, "\\");
				strcat(cPath2, pList[i].Name);
					
				if ((cUseLoaded == 0) || (PathFileExists(cPath2) == 0))
				{
					ListPrint("DownloadFile:%s in: %s", cPath, cPath2);
					DownloadFile(cPath, cPath2);
					ListPrint("DownloadFile DONE");
				}
				else ListPrint("Skip Download File:'%s' EXIST", cPath2);
				
				HZIP hz = OpenZip(cPath2,0);
				if (hz == 0)
				{
					ListPrint("Error OpenZip '%s'", cPath2);
					iErrors++;					
				}
				ZIPENTRY ze; 
				ZRESULT zr;
				zr = GetZipItem(hz,-1,&ze);
				if (zr != ZR_OK)
				{
					ListPrint("Error GetZipItem '%i'", zr);
					iErrors++;
				}
				//ListPrint("GetZipItem %i", zr);
				int numitems=ze.index;
				for (int b=0; b < numitems; b++)
				{
					zr = GetZipItem(hz,b,&ze);
					if (zr != ZR_OK)
					{
						ListPrint("Error GetZipItem '%i'", zr);
						iErrors++;
					}
					memset(cRequest, 0, 1024);
					sprintf(cRequest, "select count(*) from ORGR_FILENAMES where upper(name) = upper('%s') and type = 2 and status = 1", ze.name);
					ListPrint("Test exist XML file '%s' in base", ze.name);
					lPrint("ExecuteRequest: '%s'", cRequest);
					n = -1;			
					hRes = azOci.ExecuteRequestIntRet((unsigned char*)cRequest, (int*)&n);
					if (hRes < 0)
					{
						iErrors++;
						azOci.GetTextError(ErrorBuff);
						ListPrint("Error test exist XML file '%s' in base %i: %s", ze.name, hRes, ErrorBuff);
						n = -1;
					}
					if ((hRes >= 0) && (n > 0)) ListPrint("Exist XML file '%s' in base: SKIP", ze.name);
					if ((hRes >= 0) && (n == 0))
					{
						memset(cPath, 0, 256);
						strcpy(cPath, cWorkDir);
						strcat(cPath, ze.name);
						ListPrint("Unzip:%s", ze.name);
						if (UnzipItem(hz,b,cPath) == ZR_OK)
						{						
							memset(cRequest, 0, 1024);
							sprintf(cRequest, "begin load_org_reestr('%s', '%s', '%s', %i, '%s'); end;", pList[i].Dir, pList[i].Name, ze.name, iSort, iSort ? cIpPath : cUlPath);
							lPrint("ExecuteRequest: '%s'", cRequest);
							//ListPrint("Load file in table: '%s'", ze.name);
							hRes = azOci.ExecuteRequest((unsigned char*)cRequest);
							if (hRes < 0)
							{
								iErrors++;
								azOci.GetTextError(ErrorBuff);
								ListPrint("Error load file: '%s' to Base %i: %s", ze.name, hRes, ErrorBuff);
								memset(cPath2, 0, 256);
								strcpy(cPath2, cErrorDir);
								strcat(cPath2, ze.name);				
								CopyFile(cPath, cPath2, TRUE);
							} 
							else 
							{
								ListPrint("Loaded file: '%s' to Base", ze.name);
								memset(cRequest, 0, 1024);
								sprintf(cRequest, "update ORGR_FILENAMES set status = 1 where upper(name) = upper('%s') and type = 2 and status = 0", ze.name);
								lPrint("ExecuteRequest : '%s'", cRequest);
								hRes = azOci.ExecuteRequest((unsigned char*)cRequest);				
								if (hRes < 0)
								{
									azOci.GetTextError(ErrorBuff);
									ListPrint("Error set new Status '%s' in base %i: %s", pList[i].Name, hRes, ErrorBuff);
									n = -1;
								}
							}	
							lPrint("Delete file: '%s'", ze.name);
							DeleteFile(cPath);
						}
					}					
				}
				CloseZip(hz);
			}
			if (iErrors)
			{
				memset(cRequest, 0, 1024);
				sprintf(cRequest, "update ORGR_FILENAMES set status = 0 where upper(name) = upper('%s') and Sort = %i and type = 0 and path like '%s%%'", pList[i].Dir, iSort, iSort ? cIpPath : cUlPath);
				lPrint("ExecuteRequest : '%s'", cRequest);
				hRes = azOci.ExecuteRequest((unsigned char*)cRequest);				
				if (hRes < 0)
				{
					azOci.GetTextError(ErrorBuff);
					ListPrint("Error set new Status '%s' in base %i: %s", pList[i].Name, hRes, ErrorBuff);
					n = -1;
				}
				memset(cRequest, 0, 1024);
				sprintf(cRequest, "update ORGR_FILENAMES set status = 0 where upper(name) = upper('%s') and Sort = %i and type = 1", pList[i].Name, iSort);
				lPrint("ExecuteRequest : '%s'", cRequest);
				hRes = azOci.ExecuteRequest((unsigned char*)cRequest);				
				if (hRes < 0)
				{
					azOci.GetTextError(ErrorBuff);
					ListPrint("Error set new Status '%s' in base %i: %s", pList[i].Name, hRes, ErrorBuff);
					n = -1;
				}
			}
			else
			{
				memset(cRequest, 0, 1024);
				sprintf(cRequest, "update ORGR_FILENAMES set status = 1 where upper(name) = upper('%s') and Sort = %i and type = 1", pList[i].Name, iSort);
				lPrint("ExecuteRequest : '%s'", cRequest);
				hRes = azOci.ExecuteRequest((unsigned char*)cRequest);				
				if (hRes < 0)
				{
					azOci.GetTextError(ErrorBuff);
					ListPrint("Error set new Status '%s' in base %i: %s", pList[i].Name, hRes, ErrorBuff);
					n = -1;
				}
			}
		}
		
		free(pList);		
	}
	
	if (cOciInit) azOci.Release();
	if (cOciInit2) exOci.Release();
	cActionExec = 0;
	ListPrint("Done Scan Address %i", iSort);
}

void CNalogInjectorDlg::OnCancel() 
{
	// TODO: Add extra cleanup here
	ShowWindow(SW_HIDE);
	//CDialog::OnCancel();
}

LRESULT CNalogInjectorDlg::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	// TODO: Add your specialized code here and/or call the base class
	
	return CDialog::DefWindowProc(message, wParam, lParam);
}

struct MiscData 
{
	char *DataBody;
	unsigned int DataLen;
	unsigned int BufferSize;
};

int curl_writer(char *data, size_t size, size_t nmemb, void *pData)
{
	int result = size * nmemb;
	MiscData *UserData = (MiscData*)pData;
	//lPrint("%s", data);
	if ((UserData->DataLen + result) > UserData->BufferSize)
	{
		if (UserData->BufferSize < 500000000)
		{
			UserData->BufferSize += 5000000;
			UserData->DataBody = (char*)realloc(UserData->DataBody, UserData->BufferSize);
			//lPrint("Downloaded %i", UserData->DataLen + result);
		} else result = 0;
	}
	if (result)	
	{
		memcpy(&UserData->DataBody[UserData->DataLen], data, result);
		UserData->DataLen += result;
	}

	return result;
}

int DownloadAddress(char *cPath, char **pBuffer, unsigned int *uiLen)
{
	CURLcode res = CURLE_OK;
	MiscData UserData;

	UserData.DataBody = NULL;
	UserData.DataLen = 0;
	UserData.BufferSize = 0;

	*uiLen = 0;
	*pBuffer = NULL;
		
	//curl = curl_easy_init();
	if(curl) 
	{
		curl_easy_setopt(curl, CURLOPT_USERNAME, cLogin);
		curl_easy_setopt(curl, CURLOPT_PASSWORD, cPassword);		
		curl_easy_setopt(curl, CURLOPT_URL, cPath);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
		
		curl_easy_setopt(curl, CURLOPT_CAPATH, cCaCert);
		curl_easy_setopt(curl, CURLOPT_SSLCERT, cClntCert);
		curl_easy_setopt(curl, CURLOPT_SSLCERTTYPE, "PEM");
		curl_easy_setopt(curl, CURLOPT_SSLKEY, cKeyCert);
		curl_easy_setopt(curl, CURLOPT_SSLKEYTYPE, "PEM");
		curl_easy_setopt(curl, CURLOPT_SSLKEYPASSWD, cKeyPass);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_writer);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &UserData);
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 0);
		
		res = curl_easy_perform(curl);
		if(res != CURLE_OK)	
		{
			ListPrint("curl_easy_perform() failed: %s\n",	curl_easy_strerror(res));
			//MessageBox(0, curl_easy_strerror(res), "curl_easy_perform() failed:", MB_ICONERROR);
		}
		else
		{
			*uiLen = UserData.DataLen;
			*pBuffer = UserData.DataBody;
		}	
		
		//curl_easy_cleanup(curl);
	} 
	else 
	{
		ListPrint("curl_easy_init failed: %s\n",	curl_easy_strerror(res));
		//MessageBox(0, "curl_easy_init error", curl_easy_strerror(res), MB_ICONERROR);
	}	
	
	return 1;
}

int curl_writer_file(char *data, size_t size, size_t nmemb, void *pData)
{
	int result = size * nmemb;
	
	if (fwrite(data, result, 1, (FILE*)pData) != 1)
		ListPrint("Ошибка записи в файл");
//	ListPrint("%i %i", fwrite(data, result, 1, (FILE*)pData), result);
//	ListPrint("Downloaded %i", ftell((FILE*)pData));
	return result;
}

int DownloadFile(char *cPath, char *cSavePath)
{
	CURLcode res = CURLE_OK;
	
	FILE *f;
	if ((f = fopen(cSavePath,"wb+")) == NULL)
	{
		ListPrint("Ошибка создания файла %s", cSavePath);
		return 0;
	}
	
	//curl = curl_easy_init();
	if(curl) 
	{
		curl_easy_setopt(curl, CURLOPT_USERNAME, cLogin);
		curl_easy_setopt(curl, CURLOPT_PASSWORD, cPassword);		
		curl_easy_setopt(curl, CURLOPT_URL, cPath);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
		
		curl_easy_setopt(curl, CURLOPT_CAPATH, cCaCert);
		curl_easy_setopt(curl, CURLOPT_SSLCERT, cClntCert);
		curl_easy_setopt(curl, CURLOPT_SSLCERTTYPE, "PEM");
		curl_easy_setopt(curl, CURLOPT_SSLKEY, cKeyCert);
		curl_easy_setopt(curl, CURLOPT_SSLKEYTYPE, "PEM");
		curl_easy_setopt(curl, CURLOPT_SSLKEYPASSWD, cKeyPass);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_writer_file);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, f);
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 0);
		
		res = curl_easy_perform(curl);
		if(res != CURLE_OK)	
		{
			ListPrint("curl_easy_perform() failed: %s\n",	curl_easy_strerror(res));
			//MessageBox(0, curl_easy_strerror(res), "curl_easy_perform() failed:", MB_ICONERROR);
		}
		
		//curl_easy_cleanup(curl);
	} 
	else 
	{
		ListPrint("curl_easy_init failed: %s\n",	curl_easy_strerror(res));
		//MessageBox(0, "curl_easy_init error", curl_easy_strerror(res), MB_ICONERROR);
	}
	
	fclose(f);
	return 1;
}

BOOL CAboutDlg::DestroyWindow() 
{
	curl_easy_cleanup(curl);
	return CDialog::DestroyWindow();
}

int GetParamSetting(unsigned int uiNum, char cParamKey, char *cBuffIn, unsigned int uiBuffInSize, char *cBuffOut, unsigned int uiBuffOutSize)
{
	memset(cBuffOut, 0, uiBuffOutSize);
	uiBuffOutSize--;
	unsigned int n, m;
	unsigned int Clk = 0;
	int PrevPos = 0;
	uiBuffInSize--;
	for (n = 0; n <= uiBuffInSize; n++)
	{
		if ((cBuffIn[n] == cParamKey) || (n == uiBuffInSize))
		{
			if (Clk == uiNum)
			{
				if (cBuffIn[n] != cParamKey) n++;
				m = n - PrevPos;
				if (m > uiBuffOutSize)
				{
					memcpy(cBuffOut, &cBuffIn[PrevPos], uiBuffOutSize);
					return 2;
				}
				else 
				{
					memcpy(cBuffOut, &cBuffIn[PrevPos], m);
					return 1;
				}
			}
			PrevPos = n + 1;
			Clk++;
		}
	}
	return 0;	
}

int Str2Int(char *cString)
{
	int n,i;
	int ret;
	char cStr[32];
	if (strlen(cString) > 31) return - 2;
	memset(cStr, 0, 32);
	strcpy(cStr,cString);
	ret = 0;
	i = 1;
	for(n = 0; n != (int)strlen(cString); n++)
	{
		if ((cStr[n] > 47) && (cStr[n] < 58))
		{
			ret *= 10;
			cStr[n] -= 48;			
			ret += cStr[n];	
		}
		if (cStr[n] == 45) i *= -1;
	}
	return ret*i;
}

void UpperTextLimit(char *cText, int iLen)
{
	int n;
	int m = iLen;
	for (n = 0; n != m; n++) if ((cText[n] > 96) && (cText[n] < 123)) cText[n] = cText[n] - 32;
}

int SaveSettings(char *Buff)
{
	FILE *f;
	if ((f = fopen(Buff,"w")) == NULL)
	{
		ListPrint("Error save settings:%s\n", Buff);
		return 0;
	}

	char Buff1[1024];

	memset(Buff1, 0, 1024);
	sprintf(Buff1, "Debug=%i\n", cDebug);
	fputs(Buff1, f);
	
	memset(Buff1, 0, 1024);
	sprintf(Buff1, "DebugPath=%s\n", cDebugPath);
	fputs(Buff1, f);
	
	memset(Buff1, 0, 1024);
	sprintf(Buff1, "Login=%s\n", cLogin);
	fputs(Buff1, f);
	
	memset(Buff1, 0, 1024);
	sprintf(Buff1, "Password=%s\n", cPassword);
	fputs(Buff1, f);

	memset(Buff1, 0, 1024);
	sprintf(Buff1, "URL=%s\n", cServer);
	fputs(Buff1, f);
	
	memset(Buff1, 0, 1024);
	sprintf(Buff1, "UlPath=%s\n", cUlPath);
	fputs(Buff1, f);

	memset(Buff1, 0, 1024);
	sprintf(Buff1, "IpPath=%s\n", cIpPath);
	fputs(Buff1, f);

	memset(Buff1, 0, 1024);
	sprintf(Buff1, "Auto=%i\n", cAutoMode);
	fputs(Buff1, f);

	memset(Buff1, 0, 1024);
	sprintf(Buff1, "ClientCert=%s\n", cClntCert);
	fputs(Buff1, f);

	memset(Buff1, 0, 1024);
	sprintf(Buff1, "CaCert=%s\n", cCaCert);
	fputs(Buff1, f);

	memset(Buff1, 0, 1024);
	sprintf(Buff1, "KeyCert=%s\n", cKeyCert);
	fputs(Buff1, f);
	
	memset(Buff1, 0, 1024);
	sprintf(Buff1, "KeyPassword=%s\n", cKeyPass);
	fputs(Buff1, f);

	memset(Buff1, 0, 1024);
	sprintf(Buff1, "WorkDir=%s\n", cWorkDir);
	fputs(Buff1, f);
	
	memset(Buff1, 0, 1024);
	sprintf(Buff1, "ArchDir=%s\n", cArchDir);
	fputs(Buff1, f);
	
	memset(Buff1, 0, 1024);
	sprintf(Buff1, "ErrorDir=%s\n", cErrorDir);
	fputs(Buff1, f);
	
	memset(Buff1, 0, 1024);
	sprintf(Buff1, "OraDataBase=%s\n", cOraDataBase);
	fputs(Buff1, f);

	memset(Buff1, 0, 1024);
	sprintf(Buff1, "OraLogin=%s\n", cOraLogin);
	fputs(Buff1, f);

	memset(Buff1, 0, 1024);
	sprintf(Buff1, "OraPassword=%s\n", cOraPassword);
	fputs(Buff1, f);
	
	memset(Buff1, 0, 1024);
	sprintf(Buff1, "UseLoadedFiles=%s\n", cUseExistedFiles);
	fputs(Buff1, f);
	
	unsigned int i;

	for (i = 0; i < iTimeCnt; i++)
	{
		memset(Buff1, 0, 1024);
		sprintf(Buff1, "Time=%i;%i;\n", pTimeList[i].Hour, pTimeList[i].Min);
		fputs(Buff1, f);
	}

	fclose(f);
	return 1;
}

int LoadSettings(char *Buff)
{	
	FILE *f;
	if ((f = fopen(Buff,"r")) == NULL)
	{
		ListPrint("Error load settings:%s\n", Buff);
		return 0;
	}
	
	char Buff1[1024];
	char Buff2[1024];
	char Buff3[1024];
	char Buff4[256];
	unsigned int n, m, len2, len3;
	
	memset(Buff1, 0, 1024);
	while (fgets(Buff1, 1024, f) != NULL)
	{
		if ((Buff1[0] != 35) && (Buff1[0] > 32))
		{
			memset(Buff2, 0, 1024);	
			m = 0;
			for (n = 0; n < strlen(Buff1); n++) if (Buff1[n] > 31) {Buff2[m] = Buff1[n]; m++;}					
			//for (n = 0; n < strlen(Buff2); n++) if (Buff2[n] < 32) Buff2[n] = 0;		
			for (n = 0; n < strlen(Buff2); n++) if (Buff2[n] == 61) break;  // "="
			if ((strlen(Buff2) != n) && n)
			{
				UpperTextLimit(Buff2, n);
				n++;
				memset(Buff3, 0, 1024);
				memcpy(Buff3,&Buff2[n],strlen(Buff2)-n);
				len2 = strlen(Buff2);				
				len3 = strlen(Buff3);	
				ListPrint("Load sett: %s", Buff2);
				
				if ((SearchStrInData(Buff2, len2, 0, "URL=") == 1)
					&& (GetParamSetting(0, 59, Buff3, len3, Buff4, 64) == 1))
				{
					memset(cServer, 0, 64);
					strcpy(cServer, Buff4);
				}

				if ((SearchStrInData(Buff2, len2, 0, "ULPATH=") == 1)
					&& (GetParamSetting(0, 59, Buff3, len3, Buff4, 64) == 1))
				{
					memset(cUlPath, 0, 64);
					strcpy(cUlPath, Buff4);
				}

				if ((SearchStrInData(Buff2, len2, 0, "IPPATH=") == 1)
					&& (GetParamSetting(0, 59, Buff3, len3, Buff4, 64) == 1))
				{
					memset(cIpPath, 0, 64);
					strcpy(cIpPath, Buff4);
				}

				if ((SearchStrInData(Buff2, len2, 0, "DEBUG=") == 1)
					&& (GetParamSetting(0, 59, Buff3, len3, Buff4, 10) == 1))
				{
					cDebug = Str2Int(Buff4);
				}

				if ((SearchStrInData(Buff2, len2, 0, "DEBUGPATH=") == 1)
					&& (GetParamSetting(0, 59, Buff3, len3, Buff4, 256) == 1))
				{
					memset(cDebugPath, 0, 256);
					strcpy(cDebugPath, Buff4);
				}
				
				if ((SearchStrInData(Buff2, len2, 0, "LOGIN=") == 1)
					&& (GetParamSetting(0, 59, Buff3, len3, Buff4, 64) == 1))
				{
					memset(cLogin, 0, 64);
					strcpy(cLogin, Buff4);
				}
				
				if ((SearchStrInData(Buff2, len2, 0, "PASSWORD=") == 1)
					&& (GetParamSetting(0, 59, Buff3, len3, Buff4, 64) == 1))
				{
					memset(cPassword, 0, 64);
					strcpy(cPassword, Buff4);	
				}
				
				if ((SearchStrInData(Buff2, len2, 0, "AUTO=") == 1)
					&& (GetParamSetting(0, 59, Buff3, len3, Buff4, 10) == 1)) cAutoMode = Str2Int(Buff4);
				
				
				if ((SearchStrInData(Buff2, len2, 0, "CLIENTCERT=") == 1)
					&& (GetParamSetting(0, 59, Buff3, len3, Buff4, 64) == 1))
				{
					memset(cClntCert, 0, 64);
					strcpy(cClntCert, Buff4);	
				}

				if ((SearchStrInData(Buff2, len2, 0, "CACERT=") == 1)
					&& (GetParamSetting(0, 59, Buff3, len3, Buff4, 64) == 1))
				{
					memset(cCaCert, 0, 64);
					strcpy(cCaCert, Buff4);	
				}

				if ((SearchStrInData(Buff2, len2, 0, "KEYCERT=") == 1)
					&& (GetParamSetting(0, 59, Buff3, len3, Buff4, 64) == 1))
				{
					memset(cKeyCert, 0, 64);
					strcpy(cKeyCert, Buff4);	
				}

				if ((SearchStrInData(Buff2, len2, 0, "KEYPASSWORD=") == 1)
					&& (GetParamSetting(0, 59, Buff3, len3, Buff4, 64) == 1))
				{
					memset(cKeyPass, 0, 64);
					strcpy(cKeyPass, Buff4);	
				}

				if ((SearchStrInData(Buff2, len2, 0, "WORKDIR=") == 1)
					&& (GetParamSetting(0, 59, Buff3, len3, Buff4, 256) == 1))
				{
					memset(cWorkDir, 0, 256);
					strcpy(cWorkDir, Buff4);	
				}

				if ((SearchStrInData(Buff2, len2, 0, "ARCHDIR=") == 1)
					&& (GetParamSetting(0, 59, Buff3, len3, Buff4, 256) == 1))
				{
					memset(cArchDir, 0, 256);
					strcpy(cArchDir, Buff4);	
				}

				if ((SearchStrInData(Buff2, len2, 0, "ERRORDIR=") == 1)
					&& (GetParamSetting(0, 59, Buff3, len3, Buff4, 256) == 1))
				{
					memset(cErrorDir, 0, 256);
					strcpy(cErrorDir, Buff4);	
				}

				if ((SearchStrInData(Buff2, len2, 0, "ORADATABASE=") == 1)
					&& (GetParamSetting(0, 59, Buff3, len3, Buff4, 64) == 1))
				{
					memset(cOraDataBase, 0, 64);
					strcpy(cOraDataBase, Buff4);	
				}

				if ((SearchStrInData(Buff2, len2, 0, "ORALOGIN=") == 1)
					&& (GetParamSetting(0, 59, Buff3, len3, Buff4, 64) == 1))
				{
					memset(cOraLogin, 0, 64);
					strcpy(cOraLogin, Buff4);	
				}

				if ((SearchStrInData(Buff2, len2, 0, "ORAPASSWORD=") == 1)
					&& (GetParamSetting(0, 59, Buff3, len3, Buff4, 64) == 1))
				{
					memset(cOraPassword, 0, 64);
					strcpy(cOraPassword, Buff4);	
				}
				
				if ((SearchStrInData(Buff2, len2, 0, "USELOADEDFILES=") == 1)
					&& (GetParamSetting(0, 59, Buff3, len3, Buff4, 10) == 1)) cUseExistedFiles = Str2Int(Buff4);
								
				if (SearchStrInData(Buff2, len2, 0, "TIME=") == 1)
				{
					iTimeCnt++;
					pTimeList = (TimeList*)realloc(pTimeList, sizeof(TimeList)*iTimeCnt);
					memset(&pTimeList[iTimeCnt-1], 0, sizeof(TimeList));
					
					if (GetParamSetting(0, 59, Buff3, len3, Buff4, 5) == 1)
						pTimeList[iTimeCnt-1].Hour = (char)Str2Int(Buff4);
					if (GetParamSetting(1, 59, Buff3, len3, Buff4, 5) == 1)
						pTimeList[iTimeCnt-1].Min = (char)Str2Int(Buff4);
					pTimeList[iTimeCnt-1].Status = 1;							
				}
			}
		}
	}
	fclose(f);
	
	return 1;
}

int ParsePage(char* pBuffer, unsigned int uiDataLen, DataList **ppList, unsigned int *puiItemCount, char *cDir)
{
	int iPos = 0;
	int iPos1, iPos2;
	int iLen;

	unsigned int uiItemCnt = *puiItemCount;
	DataList *pList = *ppList;

	iPos = SearchStrInDataCaseIgn(pBuffer, uiDataLen, iPos, "<li ");

	while(iPos >= 0)
	{		
		iLen = SearchStrInData(pBuffer, uiDataLen, iPos, ">");		
		if (iLen < 0) break;

		iPos1 = SearchStrInDataCaseIgn(pBuffer, iLen, iPos, "data-name=\"..\"");
		if (iPos1 < 0)
		{
			uiItemCnt++;
			pList = (DataList*)realloc(pList, uiItemCnt*sizeof(DataList));
			memset(&pList[uiItemCnt-1], 0, sizeof(DataList));
			
			*puiItemCount = uiItemCnt;
			*ppList = pList;
			
			if (cDir)
			{
				memset(pList[uiItemCnt-1].Dir, 0, 128);
				strcpy(pList[uiItemCnt-1].Dir, cDir);
			}

			iPos1 = SearchStrInDataCaseIgn(pBuffer, iLen, iPos, "data-name=\"");
			if (iPos1 >= 0)
			{
				iPos1 += 10;
				iPos2 = SearchStrInDataCaseIgn(pBuffer, iLen, iPos1, "\"");
				if (iPos2 >= 0)
				{
					iPos2 -= iPos1 + 1;
					if (iPos2 < 128)
					{
						memcpy(pList[uiItemCnt-1].Name, &pBuffer[iPos1], iPos2);
						//ListPrint("parameter1: '%s'", pList[uiItemCnt-1].Name);
					} 
					else 
					{
						memcpy(pList[uiItemCnt-1].Name, &pBuffer[iPos1], 127);
						ListPrint("Big length parameter: '%s'", pList->Name);
						//MessageBox(0, (char*)pList[uiItemCnt-1].Name, "Big length parameter", MB_ICONERROR);
					}	
				}	
			}

			iPos1 = SearchStrInDataCaseIgn(pBuffer, iLen, iPos, "data-href=\"");
			if (iPos1 >= 0)
			{
				iPos1 += 10;
				iPos2 = SearchStrInDataCaseIgn(pBuffer, iLen, iPos1, "\"");
				if (iPos2 >= 0)
				{
					iPos2 -= iPos1 + 1;
					if (iPos2 < 128)
					{
						memcpy(pList[uiItemCnt-1].Path, &pBuffer[iPos1], iPos2);
						//ListPrint("parameter2: '%s'", pList[uiItemCnt-1].Path);
					} 
					else 
					{
						memcpy(pList[uiItemCnt-1].Name, &pBuffer[iPos1], 127);
						ListPrint("Big length parameter: '%s'", pList[uiItemCnt-1].Path);
						//MessageBox(0, (char*)pList[uiItemCnt-1].Path, "Big length parameter", MB_ICONERROR);
					}	
				}	
			}
		}	
		iPos = SearchStrInDataCaseIgn(pBuffer, uiDataLen, iPos, "<li ");
	}
	return 1;
}

int UnzipFile(char *pPath)
{
	HZIP hz = OpenZip(pPath,0);
	ZIPENTRY ze; 
	GetZipItem(hz,-1,&ze); 
	int numitems=ze.index;
	for (int i=0; i<numitems; i++)
	{ 
		GetZipItem(hz,i,&ze);
		ListPrint("Unzip:%s", ze.name);
		UnzipItem(hz,i,ze.name);
	}
	CloseZip(hz);
	return 1;
}


BOOL CAboutDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	return CDialog::OnCommand(wParam, lParam);
}

void CAboutDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
}

void CNalogInjectorDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);	
	//if (nType == 1)	ShowWindow(SW_HIDE);
}

void AnimateIcon(HINSTANCE hInstance, HWND hWnd, DWORD dwMsgType,UINT nIndexOfIcon, char * cCaption)
{
	//HICON hIconAtIndex = LoadIcon(hInstance, (LPCTSTR) MAKEINTRESOURCE(IconResourceArray[nIndexOfIcon]));
	if ((nIndexOfIcon == 5) && (cStatus == 0)) 
	{
		nIndexOfIcon = 0;
		cCaption = "Error";
	}
	CBitmap	m_BMP;
	m_BMP.LoadBitmap(IDB_BMP1);
	ICONINFO icInfo;	
	icInfo.fIcon	= TRUE;
	icInfo.hbmMask	= (HBITMAP) m_BMP;
	icInfo.xHotspot = 0;
	icInfo.yHotspot = 0;
	icInfo.hbmColor	= (HBITMAP) m_BMP;
	HICON hIconAtIndex = CreateIconIndirect(&icInfo);
	
	NOTIFYICONDATA IconData;
	
	IconData.cbSize = sizeof(NOTIFYICONDATA);
	IconData.hIcon  = hIconAtIndex;
	IconData.hWnd   = hWnd;
	lstrcpyn(IconData.szTip,cCaption, (int) strlen(cCaption)+1);
	IconData.uCallbackMessage = MYMSG_NOTIFYICON;
	IconData.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
	
	Shell_NotifyIcon(dwMsgType, &IconData);
	SendMessage(hWnd, WM_SETICON, NULL, (long) hIconAtIndex);
	
	if(hIconAtIndex) DestroyIcon(hIconAtIndex);
	m_BMP.DeleteObject();
}

void CNalogInjectorDlg::OnDblclkList1() 
{
	m_List.SetCurSel(m_List.GetCount()-1);
}

void CNalogInjectorDlg::OnButton1() 
{
	
	
}


