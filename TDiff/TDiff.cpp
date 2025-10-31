// TDiff.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "TDiff.h"
#include "win32express.h"
#include "mainwindow.h"
#include <sstream>
#include <shellapi.h>

CDPI g_metrics;

void InitRichEdit()
{
    HMODULE h = LoadLibrary(L"RICHED20.DLL");
}

inline void find_replace(std::wstring& in_this_string,
                         const std::wstring& find,
                         const std::wstring& replace)
{
    std::wstring::size_type pos = 0;
    while( std::wstring::npos != (pos = in_this_string.find(find, pos)) )
    {
        in_this_string.replace(pos, find.length(), replace);
        pos += replace.length();
    }
}


extern int main(int argc, TCHAR* argv[]);

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{   
   int nArgs;
   LPWSTR *szArglist = CommandLineToArgvW(GetCommandLineW(), &nArgs);

   if (szArglist == NULL)
       return 1; //error
   
   const int r = main(nArgs, szArglist);
   LocalFree(szArglist);
   return r;   
}

HWND hDlgCurrent = NULL;

static BOOL IsIdleMessage(MSG* pMsg)
	{
		// These messages should NOT cause idle processing
		switch(pMsg->message)
		{
		case WM_MOUSEMOVE:
#ifndef _WIN32_WCE
		case WM_NCMOUSEMOVE:
#endif // !_WIN32_WCE
		case WM_PAINT:
		case 0x0118:	// WM_SYSTIMER (caret blink)
			return FALSE;
		}

		return TRUE;
	}


std::wstring GetPath(const wchar_t* file)
{
    wchar_t drive[_MAX_DRIVE];
    wchar_t dir[_MAX_DIR];
    wchar_t fname[_MAX_FNAME];
    wchar_t ext[_MAX_EXT];

    _wsplitpath( file, drive, dir, fname, ext ); // C4996

    std::wstring path(drive);
    path += dir;
    return path;
}

int main(int argc, TCHAR* argv[])
{
    InitRichEdit();

    
    MainWindow wnd(GetPath(argv[0]));
    
    

    if (argc >= 3)
    {
      wnd.m_OldFileName = argv[1];
      wnd.m_NewFileName = argv[2];
      find_replace(wnd.m_NewFileName, L"\"", L"");
      find_replace(wnd.m_OldFileName, L"\"", L"");
    }

    
    HACCEL haccel = LoadAccelerators(GetModuleHandle(NULL), MAKEINTRESOURCE(IDC_TDIFF)); 
 
    wnd.Create(L"teste", WS_OVERLAPPEDWINDOW | WS_MAXIMIZE, 0, NULL, IDC_TDIFF, IDI_TDIFF);

    ShowWindow(wnd.m_hWnd, SW_SHOWMAXIMIZED);

    BOOL bDoIdle = TRUE;
    

    MSG msg = { };
    for (;;)//while (GetMessage(&msg, NULL, 0, 0))
    {
        
        while(bDoIdle && !::PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
        {
            wnd.OnIdle();
            bDoIdle = FALSE;            
        }

        BOOL bRet = ::GetMessage(&msg, NULL, 0, 0);

        if(bRet == -1)
        {
            //ATLTRACE2(atlTraceUI, 0, _T("::GetMessage returned -1 (error)\n"));
            continue;   // error, don't process
        }
        else if(!bRet)
        {
            //ATLTRACE2(atlTraceUI, 0, _T("CMessageLoop::Run - exiting\n"));
            break;   // WM_QUIT, exit message loop
        }


        if (!TranslateAccelerator( 
            wnd.m_hWnd,  // handle to receiving window 
            haccel,    // handle to active accelerator table 
            &msg))         // message data 
        {
            //IsDialogMessage é para tratar mensagens do modeless 
            if (NULL == hDlgCurrent || !IsDialogMessage(hDlgCurrent, &msg))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        } 

        if(IsIdleMessage(&msg))
        {
            bDoIdle = TRUE;            
        }
    }
}