// Copyright (C) 2010, Thiago Adams (thiago.adams@gmail.com)
// Permission to copy, use, modify, sell and distribute this software
// is granted provided this copyright notice appears in all copies.
// This software is provided "as is" without express or implied
// warranty, and with no claim as to its suitability for any purpose.

//This is a experimental code.


#pragma once
#include <windows.h>
#include <commctrl.h>
#include <Commdlg.h>



//Gradiente
#pragma comment( lib, "msimg32.lib" )
#pragma comment( lib, "comctl32.lib" )
//create image
//#pragma comment( lib, "comctl32.lib" )


#include <vector>
#include <string>
#ifndef ASSERT
#include <cassert>
#define ASSERT assert
#endif
#include <string.h>

//menu
#include <vector>
#include <string>
//
#define WIN32_EXPRESS_USE_MEMORY_DC


#if defined _M_IX86
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif


//local struct used for implementation
#pragma pack(push, 1)
	struct DLGINITSTRUCT
	{
		WORD nIDC;
		WORD message;
		DWORD dwSize;
	};
	struct DLGTEMPLATEEX
	{
		WORD dlgVer;
		WORD signature;
		DWORD helpID;
		DWORD exStyle;
		DWORD style;
		WORD cDlgItems;
		short x;
		short y;
		short cx;
		short cy;

		// Everything else in this structure is variable length,
		// and therefore must be determined dynamically

		// sz_Or_Ord menu;			// name or ordinal of a menu resource
		// sz_Or_Ord windowClass;	// name or ordinal of a window class
		// WCHAR title[titleLen];	// title string of the dialog box
		// short pointsize;			// only if DS_SETFONT is set
		// short weight;			// only if DS_SETFONT is set
		// short bItalic;			// only if DS_SETFONT is set
		// WCHAR font[fontLen];		// typeface name, if DS_SETFONT is set
	};
	struct DLGITEMTEMPLATEEX
	{
		DWORD helpID;
		DWORD exStyle;
		DWORD style;
		short x;
		short y;
		short cx;
		short cy;
		DWORD id;

		// Everything else in this structure is variable length,
		// and therefore must be determined dynamically

		// sz_Or_Ord windowClass;	// name or ordinal of a window class
		// sz_Or_Ord title;			// title string or ordinal of a resource
		// WORD extraCount;			// bytes following creation data
	};
#pragma pack(pop)

class CDPI;
extern CDPI g_metrics;
extern HWND hDlgCurrent;

template<class T>
T* GetWindowDocument(HWND hWnd)
{
  T *pDoc = reinterpret_cast<T *>(static_cast<LONG_PTR>(
    ::GetWindowLongPtrW(hWnd, GWLP_USERDATA)));
  return pDoc;
}

template<class T>
T* SetWindowDocument(HWND hWnd, T* pReceiver)
{
  LONG_PTR lptr = ::SetWindowLongPtrW(
    hWnd,
    GWLP_USERDATA,
    PtrToUlong(pReceiver));
  T *pOldDoc = (T*)(LONG_PTR)(lptr);
  return pOldDoc;
}

template<class T>
LRESULT CALLBACK WindowsProcEx(HWND hWnd,
                               UINT message,
                               WPARAM wParam,
                               LPARAM lParam)
{

    
       T *pThis = NULL;

       if (message == WM_NCCREATE)
       {
           CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
           pThis = (T*)pCreate->lpCreateParams;
           SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pThis);

           pThis->m_hWnd = hWnd;
       }
       else
       {
           pThis = (T*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
       }
       if (pThis)
       {
           //return pThis->HandleMessage(uMsg, wParam, lParam);
           BOOL bHandled = false;

           LRESULT r = Details::SendMessageTo<T>(pThis, hWnd , message, wParam, lParam, bHandled);
           if (bHandled)
           {
               return r;
           }
       }
       
           return DefWindowProc(hWnd, message, wParam, lParam);
       
}


inline bool IsControlKeyPressed()
{
  return (GetKeyState(VK_CONTROL) & 0xf000) == 0xf000;
}

inline bool IsShiftKeyPressed()
{
  return (GetKeyState(VK_SHIFT) & 0xf000) == 0xf000;
}

#ifndef GET_X_LPARAM
#define GET_X_LPARAM(lParam)    ((int)(short)LOWORD(lParam))
#endif
#ifndef GET_Y_LPARAM
#define GET_Y_LPARAM(lParam)    ((int)(short)HIWORD(lParam))
#endif

namespace Details
{
  class MemoryDC
  {
  public:
    HDC m_hDC;
    HDC m_hDCOriginal;
    RECT m_rcPaint;
    HBITMAP m_hBitmap;
    HBITMAP m_hBmpOld;

    MemoryDC(HDC hDC, RECT& rcPaint) : m_hDCOriginal(hDC), m_hBmpOld(NULL)
    {
      m_rcPaint = rcPaint;
      m_hDC = ::CreateCompatibleDC(m_hDCOriginal);
      ASSERT(m_hDC != NULL);
      m_hBitmap = ::CreateCompatibleBitmap(m_hDCOriginal,
        m_rcPaint.right - m_rcPaint.left, m_rcPaint.bottom - m_rcPaint.top);
      ASSERT(m_hBitmap != NULL);
      m_hBmpOld = (HBITMAP)::SelectObject(m_hDC, m_hBitmap);
      ::SetViewportOrgEx(m_hDC, -m_rcPaint.left, -m_rcPaint.top, 0);
      //FillSolidRect(m_hDC, 0,0, m_rcPaint.right-m_rcPaint.left, m_rcPaint.bottom-m_rcPaint.top, RGB(255,
    }

    ~MemoryDC()
    {
      ::BitBlt(m_hDCOriginal, m_rcPaint.left, m_rcPaint.top,
        m_rcPaint.right - m_rcPaint.left, m_rcPaint.bottom - m_rcPaint.top,
        m_hDC, m_rcPaint.left, m_rcPaint.top, SRCCOPY);
      (HBITMAP)::SelectObject(m_hDC, m_hBmpOld);
      ::DeleteObject(m_hBitmap);
      ::DeleteObject(m_hDC);
    }
  };


  template<class TEventReceiver>
  LRESULT SendMessageTo(TEventReceiver* pEventReceiver,
    HWND hWnd,
    UINT uMsg,
    WPARAM wparam,
    LPARAM lparam,
    BOOL& bHandled)
  {
   

    bHandled = FALSE;
    if (pEventReceiver == NULL)
    {
      return FALSE;
    }
    bHandled = TRUE;
    switch (uMsg)
    {

      //notifications
      //case WM_COMMAND:
      //case WM_NOTIFY:
      //{
      //LRESULT lresult;
      //pEventReceiver->ProcessWindowMessage(hWnd, uMsg, wparam, lparam, lresult);
      //}
      //break;

        __if_exists(TEventReceiver::OnCreate)
    {
    case WM_CREATE:
      pEventReceiver->OnCreate();
      break;
    } 	

      __if_exists(TEventReceiver::OnDeactivate)
      {
    case WM_ACTIVATE:
      {
        if (LOWORD(wparam) == WA_INACTIVE)
        {
          pEventReceiver->OnDeactivate();
        }
        break;
      }
      } //OnDeactivate




#ifndef _WIN32_WCE
      __if_exists(TEventReceiver::OnMouseWheel)
      {
    case WM_MOUSEWHEEL:
      {
        int fwKeys = GET_KEYSTATE_WPARAM(wparam );
        short zDelta = GET_WHEEL_DELTA_WPARAM(wparam );                
        pEventReceiver->OnMouseWheel(fwKeys, zDelta);
      }
      break;
      }//OnMouseWheel
#endif

      __if_exists(TEventReceiver::OnMouseMove)
      {
    case WM_MOUSEMOVE:
      {
        int xPos = GET_X_LPARAM(lparam); 
        int yPos = GET_Y_LPARAM(lparam); 
        pEventReceiver->OnMouseMove(xPos, yPos);
      }
      break;
      }//OnMouseMove

      
      __if_exists(TEventReceiver::OnNotify)
      {
    case WM_NOTIFY :
      {
        //BOOL OnNotify(NMHDR* pw, NMHDR* pl);
        return pEventReceiver->OnNotify(  (NMHDR*) wparam, (NMHDR*)lparam);
      }
      break;
      }

      
      __if_exists(TEventReceiver::OnCommand)
      {
    case WM_COMMAND:
        {

            pEventReceiver->OnCommand(  LOWORD(wparam), HIWORD(wparam), (HWND)lparam);

        }
        break;
      }

      __if_exists(TEventReceiver::OnDrawItem)
      {
        //bool OnDrawItem( int id,  LPDRAWITEMSTRUCT);
    case WM_DRAWITEM:
      return pEventReceiver->OnDrawItem( wparam, (LPDRAWITEMSTRUCT) lparam);
      break;
      }


      __if_exists(TEventReceiver::OnClose)
      {
    case WM_CLOSE:
      {
        return pEventReceiver->OnClose();
      }
      break;
      }//OnMouseDown

      __if_exists(TEventReceiver::OnDoubleClick)
      {
    case WM_LBUTTONDBLCLK:
      {
        int xPos = GET_X_LPARAM(lparam); 
        int yPos = GET_Y_LPARAM(lparam); 
        pEventReceiver->OnDoubleClick(xPos, yPos);
      }
      break;
      }//OnMouseDown

      __if_exists(TEventReceiver::OnMouseDown)
      {
    case WM_LBUTTONDOWN:
      {
        int xPos = GET_X_LPARAM(lparam); 
        int yPos = GET_Y_LPARAM(lparam); 
        pEventReceiver->OnMouseDown(xPos, yPos);
      }
      break;
      }//OnMouseDown

      __if_exists(TEventReceiver::OnRMouseDown)
      {
    case WM_RBUTTONDOWN:
      {
        int xPos = GET_X_LPARAM(lparam); 
        int yPos = GET_Y_LPARAM(lparam); 
        pEventReceiver->OnRMouseDown(xPos, yPos);
      }
      break;
      }//OnMouseDown

      __if_exists(TEventReceiver::OnTimer)
      {
    case WM_TIMER:
      {        
        pEventReceiver->OnTimer();
      }
      break;
      }

      __if_exists(TEventReceiver::OnSize)
      {
    case WM_SIZE:
      {        
          pEventReceiver->OnSize((UINT) wparam, LOWORD (lparam), HIWORD (lparam));
      }
      break;
      }

      

      __if_exists(TEventReceiver::OnMinMax)
      {
    case WM_GETMINMAXINFO:
      {        
        //void OnMinMax(MINMAXINFO* pMinMaxInfo);
        pEventReceiver->OnMinMax((MINMAXINFO*) lparam);
        return 0;
      }
      break;
      }//OnMinMax

      __if_exists(TEventReceiver::OnVScroll)
      {
    case WM_VSCROLL:
      {        
        pEventReceiver->OnVScroll(LOWORD(wparam), HIWORD(wparam) , (HWND)lparam);
      }
      break;
      }//OnMouseDown



      __if_exists(TEventReceiver::OnMouseUp)
      {
    case WM_LBUTTONUP:
      {
        int xPos = GET_X_LPARAM(lparam); 
        int yPos = GET_Y_LPARAM(lparam); 
        pEventReceiver->OnMouseUp(xPos, yPos);
      }
      break;
      }//OnMouseDown


      __if_exists(TEventReceiver::OnSetFocus)
      {
    case WM_SETFOCUS:
      {
        pEventReceiver->OnSetFocus();
      }            
      break;
      }

      __if_exists(TEventReceiver::OnSetCursor)
      {
    case WM_SETCURSOR:
      {
        if (!pEventReceiver->OnSetCursor())
        {
          bHandled = FALSE;
        }
      }            
      break;
      }

      __if_exists (TEventReceiver::OnKillFocus)
      {
    case WM_KILLFOCUS:
      {
        pEventReceiver->OnKillFocus();
      }            
      break;
      }//OnKillFocus

      
#ifdef WIN32_EXPRESS_USE_MEMORY_DC
    //  case WM_ERASEBKGND:
    //    return FALSE; //nao pinta o fundo
    //   break;
#endif

      __if_exists (TEventReceiver::OnPaint)
      {
    case WM_PAINT:
      {
        PAINTSTRUCT ps;
        HDC hDC = BeginPaint(hWnd, &ps);
        {
#ifdef WIN32_EXPRESS_USE_MEMORY_DC
          MemoryDC dc(hDC, ps.rcPaint);
          pEventReceiver->OnPaint(dc.m_hDC);
#else
          pEventReceiver->OnPaint(hDC);
#endif
        }
        EndPaint(hWnd, &ps);
      }
      break;
      }

    default:
        {
       
            //http://support.microsoft.com/kb/71450
            /*  {MSG msg;
            msg.message = uMsg;
            if (IsDialogMessage(hWnd, uMsg))
            {
            bHandled = TRUE;
            return TRUE;
            }*/
            bHandled = FALSE;
            return FALSE;
        }
    }
    return TRUE;
  }
} //namespace details

inline int RunMessageLoop()
{
  MSG msg;
  while (GetMessage(&msg, NULL, 0, 0))
  {
    TranslateMessage(&msg); 
    DispatchMessage(&msg);
  }
  return (int) msg.wParam;
}

inline DWORD GetStyle(HWND m_hWnd)
{
  ASSERT(::IsWindow(m_hWnd));
  return (DWORD)::GetWindowLong(m_hWnd, GWL_STYLE);
}

inline BOOL CenterWindow(HWND m_hWnd, HWND hWndCenter = NULL)
{
  ASSERT(::IsWindow(m_hWnd));

  // determine owner window to center against
  DWORD dwStyle = GetStyle(m_hWnd);
  if(hWndCenter == NULL)
  {
    if(dwStyle & WS_CHILD)
      hWndCenter = ::GetParent(m_hWnd);
    else
      hWndCenter = ::GetWindow(m_hWnd, GW_OWNER);
  }

  // get coordinates of the window relative to its parent
  RECT rcDlg;
  ::GetWindowRect(m_hWnd, &rcDlg);
  RECT rcArea;
  RECT rcCenter;
  HWND hWndParent;
  if(!(dwStyle & WS_CHILD))
  {
    // don't center against invisible or minimized windows
    if(hWndCenter != NULL)
    {
      DWORD dwStyleCenter = ::GetWindowLong(hWndCenter, GWL_STYLE);
      if(!(dwStyleCenter & WS_VISIBLE) || (dwStyleCenter & WS_MINIMIZE))
        hWndCenter = NULL;
    }

    // center within screen coordinates
#if WINVER < 0x0500
    ::SystemParametersInfo(SPI_GETWORKAREA, NULL, &rcArea, NULL);
#else
    HMONITOR hMonitor = NULL;
    if(hWndCenter != NULL)
    {
      hMonitor = ::MonitorFromWindow(hWndCenter,
        MONITOR_DEFAULTTONEAREST);
    }
    else
    {
      hMonitor = ::MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTONEAREST);
    }

    //ATLENSURE_RETURN_VAL(hMonitor != NULL, FALSE);

    MONITORINFO minfo;
    minfo.cbSize = sizeof(MONITORINFO);
    BOOL bResult = ::GetMonitorInfo(hMonitor, &minfo);
    //ATLENSURE_RETURN_VAL(bResult, FALSE);

    rcArea = minfo.rcWork;
#endif
    if(hWndCenter == NULL)
      rcCenter = rcArea;
    else
      ::GetWindowRect(hWndCenter, &rcCenter);
  }
  else
  {
    // center within parent client coordinates
    hWndParent = ::GetParent(m_hWnd);
    ASSERT(::IsWindow(hWndParent));

    ::GetClientRect(hWndParent, &rcArea);
    ASSERT(::IsWindow(hWndCenter));
    ::GetClientRect(hWndCenter, &rcCenter);
    ::MapWindowPoints(hWndCenter, hWndParent, (POINT*)&rcCenter, 2);
  }

  int DlgWidth = rcDlg.right - rcDlg.left;
  int DlgHeight = rcDlg.bottom - rcDlg.top;

  // find dialog's upper left based on rcCenter
  int xLeft = (rcCenter.left + rcCenter.right) / 2 - DlgWidth / 2;
  int yTop = (rcCenter.top + rcCenter.bottom) / 2 - DlgHeight / 2;

  // if the dialog is outside the screen, move it inside
  if(xLeft + DlgWidth > rcArea.right)
    xLeft = rcArea.right - DlgWidth;
  if(xLeft < rcArea.left)
    xLeft = rcArea.left;

  if(yTop + DlgHeight > rcArea.bottom)
    yTop = rcArea.bottom - DlgHeight;
  if(yTop < rcArea.top)
    yTop = rcArea.top;

  // map screen coordinates to child coordinates
  return ::SetWindowPos(m_hWnd, NULL, xLeft, yTop, -1, -1,
    SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}


// Message handler for about box.
template<class T>
INT_PTR CALLBACK DlgProc(HWND hDlg,
                         UINT message,
                         WPARAM wParam,
                         LPARAM lParam)
{
  BOOL bHandled = FALSE;
  UNREFERENCED_PARAMETER(lParam);

  switch (message)
  {
  case WM_INITDIALOG:
    {
      T * p = (T*)lParam;
      SetWindowLongPtr(hDlg, GWLP_USERDATA, (LONG_PTR) p);
      p->InitEntry(hDlg);
      //If the procedure sets the input focus, it must return FALSE to prevent 
      //the system from setting the default focus
      return (INT_PTR)FALSE; //use setfocus on init
    }  
    break;
  case WM_ACTIVATE:
    if (0 == wParam)             // becoming inactive
      hDlgCurrent = NULL;
    else                         // becoming active
      hDlgCurrent = hDlg;

    return FALSE;
  }

  


  //TODO : all messages?
  T * p = (T*)GetWindowLongPtr(hDlg, GWLP_USERDATA);
  LRESULT r = Details::SendMessageTo<T>(p, hDlg, message, wParam, lParam, bHandled);
  
  
  //Typically, the dialog box procedure should return TRUE if it 
  //processed the message, and FALSE if it did not. 
  //If the dialog box procedure returns FALSE, the dialog manager 
  //performs the default dialog operation in response to the message.

  return bHandled;
}


template<class T>
INT_PTR ShowDialog(LPCWSTR lpTemplateName, T * p, HWND hWndParent)
{
  HINSTANCE hInst = GetModuleHandle(NULL);
  INT_PTR r = DialogBoxParam(hInst, lpTemplateName, hWndParent, &DlgProc<T>,(LPARAM) p);

  //If the function succeeds, the return value is the value of the nResult 
  // parameter specified in the call to the EndDialog function used to 
  // terminate the dialog box.
  //If the function fails because the hWndParent parameter is invalid, 
  //the return value is zero. The function returns zero in this case for compatibility with 
  // previous versions of Microsoft Windows. If the function fails for any other reason,
  //the return value is –1. To get extended error information, call GetLastError

  ASSERT(r != 0); //resource exists?	(1813) use != 0 para enddialog
  return r;
}

template<class T>
HWND ShowModeless(UINT IDD, T* p, HWND hParent)
{
  HWND hWnd = NULL;

  HINSTANCE hInstance = GetModuleHandle(NULL);   

  HRSRC hDlg = FindResource(hInstance, MAKEINTRESOURCE(IDD), RT_DIALOG);
  if (hDlg != NULL)
  {
    DWORD dwLastError = 0;
    HGLOBAL hResource = LoadResource(hInstance, hDlg);
    if (hResource != NULL)
    {
      DLGTEMPLATE* pDlg0 = (DLGTEMPLATE*) LockResource(hResource);
      if (pDlg0 != NULL)
      {
          //2025 FIX we need to make  a copy
          DWORD size = SizeofResource(hInstance, hDlg);
          DLGTEMPLATE* pDlg = (DLGTEMPLATE*)  malloc(size);
          memcpy(pDlg, pDlg0, size);

          {//Changes the font from template to use windows dialog font and size
              std::wstring ws;
              int nPointSize;
              GetSystemIconFont(ws, nPointSize);          
              SetFont(pDlg, ws.c_str(), nPointSize );
          }

        hWnd = CreateDialogIndirectParam(
          hInstance,
          pDlg,
          hParent,
          &DlgProc<T>, 
          (LPARAM)p);

        UnlockResource(hResource);

        free(pDlg);
      }
      else
        dwLastError = ::GetLastError();
    }
    else
      dwLastError = ::GetLastError();

    if (dwLastError != 0)
      SetLastError(dwLastError);
  }
  
  return hWnd;
}



inline BOOL IsDialogEx(const DLGTEMPLATE* pTemplate)
{
    return ((DLGTEMPLATEEX*)pTemplate)->signature == 0xFFFF;
}

/////////////////////////////////////////////////////////////////////////////
// HasFont

inline BOOL HasFont(const DLGTEMPLATE* pTemplate)
{
    return (DS_SETFONT &
        (IsDialogEx(pTemplate) ? ((DLGTEMPLATEEX*)pTemplate)->style :
        pTemplate->style));
}

/////////////////////////////////////////////////////////////////////////////
// FontAttrSize

inline int FontAttrSize(BOOL bDialogEx)
{
    return (int)sizeof(WORD) * (bDialogEx ? 3 : 1);
}

inline WCHAR* _SkipString(__in_z WCHAR* p)
{
    while (*p++);
    return p;
}

inline BYTE*  GetFontSizeField(const DLGTEMPLATE* pTemplate)
{
    BOOL bDialogEx = IsDialogEx(pTemplate);
    WORD* pw;

    if (bDialogEx)
        pw = (WORD*)((DLGTEMPLATEEX*)pTemplate + 1);
    else
        pw = (WORD*)(pTemplate + 1);

    if (*pw == (WORD)-1)        // Skip menu name string or ordinal
        pw += 2; // WORDs
    else
        while(*pw++);

    if (*pw == (WORD)-1)        // Skip class name string or ordinal
        pw += 2; // WORDs
    else
        while(*pw++);

    while (*pw++);          // Skip caption string

    return (BYTE*)pw;
}

inline UINT GetTemplateSize(const DLGTEMPLATE* pTemplate)
{
    BOOL bDialogEx = IsDialogEx(pTemplate);
    BYTE* pb = GetFontSizeField(pTemplate);

    if (::HasFont(pTemplate))
    {
        // Skip font size and name
        pb += FontAttrSize(bDialogEx);  // Skip font size, weight, (italic, charset)
        pb += 2 * (wcslen((WCHAR*)pb) + 1);
    }

    WORD nCtrl = bDialogEx ? (WORD)((DLGTEMPLATEEX*)pTemplate)->cDlgItems :
        (WORD)pTemplate->cdit;

    while (nCtrl > 0)
    {
        pb = (BYTE*)(((DWORD_PTR)pb + 3) & ~DWORD_PTR(3)); // DWORD align

        pb += (bDialogEx ? sizeof(DLGITEMTEMPLATEEX) : sizeof(DLGITEMTEMPLATE));

        if (*(WORD*)pb == (WORD)-1)     // Skip class name string or ordinal
            pb += 2 * sizeof(WORD);
        else
            pb = (BYTE*)_SkipString((WCHAR*)pb);

        if (*(WORD*)pb == (WORD)-1)     // Skip text string or ordinal
            pb += 2 * sizeof(WORD);
        else
            pb = (BYTE*)_SkipString((WCHAR*)pb);

        WORD cbExtra = *(WORD*)pb;      // Skip extra data
        if (cbExtra != 0 && !bDialogEx)
            cbExtra -= 2;
        pb += sizeof(WORD) + cbExtra;
        --nCtrl;
    }

    //IA64: Max dialog template size of 4GB should be fine
    return UINT(pb - (BYTE*)pTemplate);
}

inline void GetSystemIconFont(std::wstring& strFontName,int& nPointSize)
{
  strFontName = L"MS Shell Dlg"; // out
  nPointSize = 8; //out

  OSVERSIONINFO osvi;
  ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
  osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
  GetVersionEx(&osvi);

  NONCLIENTMETRICS metrics;
  if (osvi.dwMajorVersion <= 5) 
  {
    // If an application that is compiled for Windows Server 2008 or Windows 
    // Vista must also run on Windows Server 2003 or Windows XP/2000, 
    // use the GetVersionEx function to check the operating system version at 
    // run time and, if the application is running on Windows Server 2003 or 
    // Windows XP/2000, subtract the size of the iPaddedBorderWidth member 
    // from the cbSize member of the NONCLIENTMETRICS structure before 
    // calling the SystemParametersInfo function.

    metrics.cbSize = sizeof(NONCLIENTMETRICS) - sizeof(metrics.iPaddedBorderWidth); 
  }
  else
  {
    metrics.cbSize = sizeof(NONCLIENTMETRICS);
  }

  if (::SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS),   &metrics, 0) != 0)
  {
    HDC hDC    = ::GetDC(NULL);
    int nLPixY = GetDeviceCaps(hDC, LOGPIXELSY);
    ::ReleaseDC(NULL,hDC);

    // copy font parameters ///nao sei bem pq 72 mas funciona?
    nPointSize  = -MulDiv(metrics.lfMessageFont.lfHeight,72,nLPixY);
    strFontName = metrics.lfMessageFont.lfFaceName;
  }
  else
  {
    assert(false);//
  }
}


inline BOOL SetFont(DLGTEMPLATE* pTemplate, LPCTSTR lpFaceName, WORD nFontSize)
{
    DWORD dwTemplateSize = GetTemplateSize(pTemplate);
    BOOL bDialogEx = IsDialogEx(pTemplate);
    BOOL bHasFont = ::HasFont(pTemplate);
    int cbFontAttr = FontAttrSize(bDialogEx);

    if (bDialogEx)
        ((DLGTEMPLATEEX*)pTemplate)->style |= DS_SETFONT;
    else
        pTemplate->style |= DS_SETFONT;

    int nFaceNameLen = lstrlen(lpFaceName);
    if( nFaceNameLen >= LF_FACESIZE )
    {
        // Name too long
        return FALSE;
    }

#ifdef _UNICODE
    int cbNew = cbFontAttr + ((nFaceNameLen + 1) * sizeof(TCHAR));
    BYTE* pbNew = (BYTE*)lpFaceName;
#else
    WCHAR wszFaceName [LF_FACESIZE];
    int cbNew = cbFontAttr + 2 * MultiByteToWideChar(CP_ACP, 0, lpFaceName, -1, wszFaceName, LF_FACESIZE); 
    BYTE* pbNew = (BYTE*)wszFaceName;
#endif
    if (cbNew < cbFontAttr)
    {
        return FALSE;
    }
    BYTE* pb = GetFontSizeField(pTemplate);
    int cbOld = (int)(bHasFont ? cbFontAttr + 2 * (wcslen((WCHAR*)(pb + cbFontAttr)) + 1) : 0);

    BYTE* pOldControls = (BYTE*)(((DWORD_PTR)pb + cbOld + 3) & ~DWORD_PTR(3));
    BYTE* pNewControls = (BYTE*)(((DWORD_PTR)pb + cbNew + 3) & ~DWORD_PTR(3));

    WORD nCtrl = bDialogEx ? (WORD)((DLGTEMPLATEEX*)pTemplate)->cDlgItems :
        (WORD)pTemplate->cdit;

    if (cbNew != cbOld && nCtrl > 0)
    {
        size_t nBuffLeftSize=(size_t)(dwTemplateSize - (pOldControls - (BYTE*)pTemplate));
        if (nBuffLeftSize > dwTemplateSize)
        { 
            return FALSE;
        }
        memmove_s(pNewControls,nBuffLeftSize, pOldControls, nBuffLeftSize);
    }

    *(WORD*)pb = nFontSize;
    memmove_s(pb + cbFontAttr, cbNew - cbFontAttr, pbNew, cbNew - cbFontAttr);

    //IA64: Max dialog template size of 4GB should be fine
    dwTemplateSize += ULONG(pNewControls - pOldControls);

    return TRUE;
}

void GetSystemIconFont(LOGFONT& lf);

template<class T>
INT_PTR ShowDialog2(UINT IDD, T* p, HWND hParent)
{
  INT_PTR r = 0;

  HINSTANCE hInstance = GetModuleHandle(NULL);   

  HRSRC hDlg = FindResource(hInstance, MAKEINTRESOURCE(IDD), RT_DIALOG);
  if (hDlg != NULL)
  {
    DWORD dwLastError = 0;
    HGLOBAL hResource = LoadResource(hInstance, hDlg);
    if (hResource != NULL)
    {
      DLGTEMPLATE* pDlg0 = (DLGTEMPLATE*) LockResource(hResource);

      //2025 FIX we need to make  a copy
      DWORD size = SizeofResource(hInstance, hDlg);
      DLGTEMPLATE* pDlg = (DLGTEMPLATE*)  malloc(size);
      memcpy(pDlg, pDlg0, size);

      
      {//Changes the font from template to use windows dialog font and size
          std::wstring ws;
          int nPointSize;
          GetSystemIconFont(ws, nPointSize);       
          
          SetFont(pDlg, ws.c_str(), nPointSize );
      }

      if (pDlg != NULL)
      {
          r = DialogBoxIndirectParam(
          hInstance,
          pDlg,
          hParent,
          &DlgProc<T>, 
          (LPARAM)p);

        UnlockResource(hResource);
      }
      else
        dwLastError = ::GetLastError();
    }
    else
      dwLastError = ::GetLastError();

    if (dwLastError != 0)
      SetLastError(dwLastError);
  }
  
  
  return r;
}


template<class T, UINT IDD>
class Dialog
{
protected:
 
  HWND m_hParent;

public:
 HWND m_hDlg;
  int EndDialog(int r)
  {
    return ::EndDialog(m_hDlg, r);
  }

  UINT GetDlgItemText(int nID, std::wstring& s) const
  {
    ASSERT(::IsWindow(m_hDlg));
    HWND hItem = GetDlgItem(nID);
    if (hItem != NULL)
    {
      int nLength = ::GetWindowTextLength(hItem);
      std::vector<wchar_t> buffer(nLength+1);
      wchar_t* pszText = &buffer[0];
      nLength = ::GetWindowText(hItem, pszText, nLength+1);
      s = pszText;
      return nLength;
    }
    else
    {
      s.clear();
      return 0;
    }
  }

  

  HWND GetDlgItem(UINT id) const
  {
    return ::GetDlgItem(m_hDlg, id);
  }

  Dialog(HWND hParent = NULL) : m_hParent(hParent), m_hDlg(NULL)
  {
  }

  //WM_INITDIALOG
  void InitEntry(HWND hDlg)
  {     
    m_hDlg = hDlg;
    CenterWindow(m_hDlg, m_hParent);

    __if_exists (T::InitDialog)
    {
      static_cast<T*>(this)->InitDialog();
    }
  }

  //show modeless
  void Show()
  {
    if (m_hDlg == NULL)
    {
      m_hDlg = ShowModeless<T>(IDD, static_cast<T*>(this), m_hParent);
    }
    ShowWindow(m_hDlg, SW_SHOW);
  }

  //show modal
  int ShowDialog()
  {
      //return ::ShowDialog2(MAKEINTRESOURCE(IDD), static_cast<T*>(this), m_hParent);
    return ::ShowDialog2<T>(IDD, static_cast<T*>(this), m_hParent);
  }
};

template<class T>
class Window
{
public:

  HWND m_hWnd;


  Window() : m_hWnd(NULL)
  {
  }

  void Create(PCWSTR lpWindowName,
              DWORD dwStyle,
              DWORD dwExStyle = 0,       
              HWND hWndParent = 0,
              UINT  MENUIDD = 0,
              UINT  ICON = 0,
        int x =  CW_USEDEFAULT,
       int y = CW_USEDEFAULT,
       int nWidth = CW_USEDEFAULT,
       int nHeight = CW_USEDEFAULT)

  {
      wchar_t ws[1000];
      mbstowcs(ws,typeid(T).name(), 1000);

      HINSTANCE hInstance = GetModuleHandle(NULL);

      WNDCLASSEX wcex;
      wcex.cbSize = sizeof(WNDCLASSEX);
      wcex.style          = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
      wcex.lpfnWndProc    = &WindowsProcEx<T>;
      wcex.cbClsExtra     = 0;
      wcex.cbWndExtra     = 0;
      wcex.hInstance      = hInstance;

      wcex.hIcon            =0;// LoadIcon(hInstance, MAKEINTRESOURCE(IconId));

      wcex.hCursor          = LoadCursor(NULL, IDC_ARROW);
      wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW + 1);

      if (MENUIDD != NULL)
          wcex.lpszMenuName   = MAKEINTRESOURCE(MENUIDD);
      else
          wcex.lpszMenuName   = NULL;

      wcex.lpszClassName  = ws;//T::GetClassName();

      if (ICON)
      {
        wcex.hIconSm        = LoadIcon(hInstance,MAKEINTRESOURCE(ICON));
      }
      else
        wcex.hIconSm = NULL;


       RegisterClassEx(&wcex);


    m_hWnd =    CreateWindowEx(dwExStyle, ws/* T::GetClassName()*/, lpWindowName, dwStyle, x, y,
           nWidth, nHeight, hWndParent, 0, GetModuleHandle(NULL), static_cast<T*>(this)
           );

    //__if_exists(T::OnCreate)
    //{
//      static_cast<T*>(this)->OnCreate();
  //  } 	

    ShowWindow(m_hWnd, TRUE);
    UpdateWindow(m_hWnd);
  }
};


class FontDialog
{

  LOGFONT m_lf;

public:

  FontDialog(const LOGFONT& lf)
  {
    m_lf = lf;
  }

  const LOGFONT& GetLogFont() const 
  {
    return m_lf;
  }

  int ShowDialog(HWND hwnd)
  {
    CHOOSEFONT cf;     // common dialog box structure
    DWORD rgbCurrent = 0;  // current text color

    // Initialize CHOOSEFONT
    ZeroMemory(&cf, sizeof(cf));
    cf.lStructSize = sizeof (cf);
    cf.hwndOwner = hwnd;
    cf.lpLogFont = &m_lf;
    cf.rgbColors = rgbCurrent;
    cf.Flags = CF_SCREENFONTS | CF_EFFECTS;

    if (ChooseFont(&cf)==TRUE)
    {      
      return IDOK;
    }
    return IDCANCEL;
  }
};

class ColorDlg
{
  CHOOSECOLOR cc;                 // common dialog box structure 
  COLORREF acrCustClr[16]; // array of custom colors   
  HBRUSH hbrush;                  // brush handle

public:
  DWORD rgbCurrent;        // initial color selection
  int ShowDialog(HWND hwnd = NULL)
  {
    // Initialize CHOOSECOLOR 
    ZeroMemory(&cc, sizeof(cc));
    cc.lStructSize = sizeof(cc);
    cc.hwndOwner = hwnd;
    cc.lpCustColors = (LPDWORD) acrCustClr;
    cc.rgbResult = rgbCurrent;
    cc.Flags = CC_FULLOPEN | CC_RGBINIT;

    if (ChooseColor(&cc)==TRUE) {
      hbrush = CreateSolidBrush(cc.rgbResult);
      rgbCurrent = cc.rgbResult; 
      return IDOK;
    }
    return IDCANCEL;
  }

};

class OpenDialog
{
public:
  int ShowDialog(std::wstring& ws, HWND hwnd = NULL)
  {
    OPENFILENAME ofn;       // common dialog box structure
    TCHAR szFile[260];       // buffer for file name                 
//    HANDLE hf;              // file handle

    // Initialize OPENFILENAME
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = szFile;
    //
    // Set lpstrFile[0] to '\0' so that GetOpenFileName does not 
    // use the contents of szFile to initialize itself.
    //
    ofn.lpstrFile[0] = L'\0';
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = L"All\0*.*\0Text\0*.TXT\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST;// | OFN_FILEMUSTEXIST;

    // Display the Open dialog box. 

    if (GetOpenFileName(&ofn)==TRUE) 
    {      
        //hf = CreateFile(ofn.lpstrFile, GENERIC_READ,
    //0, (LPSECURITY_ATTRIBUTES) NULL,
    //OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
    //(HANDLE) NULL);
    ws = ofn.lpstrFile;
    return IDOK;
    
    }
    
    

    return IDCANCEL;

  }
};

// Definition: relative pixel = 1 pixel at 96 DPI and scaled based on actual DPI.
class CDPI
{
public:
  CDPI() : _fInitialized(false), _dpiX(96), _dpiY(96) { }

  // Get screen DPI.
  int GetDPIX() { _Init(); return _dpiX; }
  int GetDPIY() { _Init(); return _dpiY; }

  // Convert between raw pixels and relative pixels.
  int ScaleX(int x) { _Init(); return MulDiv(x, _dpiX, 96); }
  int ScaleY(int y) { _Init(); return MulDiv(y, _dpiY, 96); }
  int UnscaleX(int x) { _Init(); return MulDiv(x, 96, _dpiX); }
  int UnscaleY(int y) { _Init(); return MulDiv(y, 96, _dpiY); }

  // Determine the screen dimensions in relative pixels.
  int ScaledScreenWidth() { return _ScaledSystemMetricX(SM_CXSCREEN); }
  int ScaledScreenHeight() { return _ScaledSystemMetricY(SM_CYSCREEN); }

  // Scale rectangle from raw pixels to relative pixels.
  void ScaleRect(__inout RECT *pRect)
  {
    pRect->left = ScaleX(pRect->left);
    pRect->right = ScaleX(pRect->right);
    pRect->top = ScaleY(pRect->top);
    pRect->bottom = ScaleY(pRect->bottom);
  }
  // Determine if screen resolution meets minimum requirements in relative
  // pixels.
  bool IsResolutionAtLeast(int cxMin, int cyMin) 
  { 
    return (ScaledScreenWidth() >= cxMin) && (ScaledScreenHeight() >= cyMin); 
  }

  // Convert a point size (1/72 of an inch) to raw pixels.
  int PointsToPixels(int pt) { _Init(); return MulDiv(pt, _dpiY, 72); }

  // Invalidate any cached metrics.
  void Invalidate() { _fInitialized = false; }

private:
  void _Init()
  {
    if (!_fInitialized)
    {
      HDC hdc = GetDC(NULL);
      if (hdc)
      {
        _dpiX = GetDeviceCaps(hdc, LOGPIXELSX);
        _dpiY = GetDeviceCaps(hdc, LOGPIXELSY);
        ReleaseDC(NULL, hdc);
      }
      _fInitialized = true;
    }
  }

  int _ScaledSystemMetricX(int nIndex) 
  { 
    _Init(); 
    return MulDiv(GetSystemMetrics(nIndex), 96, _dpiX); 
  }

  int _ScaledSystemMetricY(int nIndex) 
  { 
    _Init(); 
    return MulDiv(GetSystemMetrics(nIndex), 96, _dpiY); 
  }
private:
  bool _fInitialized;

  int _dpiX;
  int _dpiY;
};

inline void GetSystemIconFont(LOGFONT& lf)
{
  // get LOGFONT structure for the icon font
  SystemParametersInfo(SPI_GETICONTITLELOGFONT,sizeof(LOGFONT),&lf,0);	
}

class DateTimeDlg
{
  void ShowDialog()
  {

    //    const int DATETIMEPICK_CLASS = 1;
    //  const int IDC_MONTHCALENDAR1 = 2;

    HINSTANCE hInst = GetModuleHandle(0);
    WNDCLASSEX wcex;
    ZeroMemory(&wcex, sizeof wcex);
    wcex.cbSize         = sizeof wcex;
    wcex.hbrBackground  = (HBRUSH)(COLOR_3DFACE + 1);
    wcex.lpszMenuName   = 0;


    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = DefWindowProc;
    wcex.hInstance      = hInst;
    wcex.hIcon          = LoadIcon(0, (LPCTSTR)IDI_APPLICATION);
    wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wcex.lpszClassName  = L"DateTimeDlg";
    RegisterClassEx(&wcex);

    HFONT hfont0 = CreateFont(-13, 0, 0, 0, 400, FALSE, FALSE, FALSE, 1, 400, 0, 0, 0, (L"Ms Shell Dlg"));
    HWND hwnd = CreateWindowEx(0, (L"DateTimeDlg"), (L"Dialog"), WS_CAPTION | WS_VISIBLE | WS_POPUP | WS_SYSMENU, 0, 0, 298, 341, 0, 0, hInst, 0);
    HWND hCtrl0_0 = CreateWindowEx(0, WC_BUTTON, (L"OK"), WS_VISIBLE | WS_CHILD | WS_TABSTOP | 0x00000001, 62, 264, 100, 28, hwnd, (HMENU)IDOK, hInst, 0);
    SendMessage(hCtrl0_0, WM_SETFONT, (WPARAM)hfont0, FALSE);
    HWND hCtrl0_1 = CreateWindowEx(0, WC_BUTTON, (L"Cancel"), WS_VISIBLE | WS_CHILD | WS_TABSTOP, 174, 262, 100, 28, hwnd, (HMENU)IDCANCEL, hInst, 0);
    SendMessage(hCtrl0_1, WM_SETFONT, (WPARAM)hfont0, FALSE);
    HWND hCtrl0_2 = CreateWindowEx(0, DATETIMEPICK_CLASS, 0, WS_VISIBLE | WS_CHILD | WS_TABSTOP | DTS_TIMEFORMAT | DTS_UPDOWN | DTS_RIGHTALIGN, 6, 206, 274, 44, hwnd, (HMENU) 1/*IDC_DATETIMEPICKER1*/, hInst, 0);
    SendMessage(hCtrl0_2, WM_SETFONT, (WPARAM)hfont0, FALSE);
    HWND hCtrl0_3 = CreateWindowEx(0, MONTHCAL_CLASS, 0, WS_VISIBLE | WS_CHILD | WS_TABSTOP | MCS_NOTODAY, 6, 8, 274, 192, hwnd, (HMENU)2 /*IDC_MONTHCALENDAR1*/, hInst, 0);
    SendMessage(hCtrl0_3, WM_SETFONT, (WPARAM)hfont0, FALSE);
  }
};




inline void FillSolidRect(HDC hDC, LPCRECT lpRect, COLORREF clr)
{
  COLORREF clrOld = ::SetBkColor(hDC, clr);

  if (clrOld != CLR_INVALID)
  {
    ::ExtTextOut(hDC, 0, 0, ETO_OPAQUE, lpRect, NULL, 0, NULL);
    ::SetBkColor(hDC, clrOld);
  }
}

inline void FillSolidRect(HDC hdc, int left, int top, int right, int bottom, COLORREF clr)
{
  RECT rc = {left, top, right, bottom};
  FillSolidRect(hdc, &rc, clr);
}

inline void Combo_DropDown(HWND hCombo)
{
  SendMessage(hCombo, (UINT) CB_SHOWDROPDOWN, TRUE, 0);  
}

inline void Combo_AddString(HWND hCombo, LPCTSTR psz)
{
  SendMessage(hCombo, (UINT) CB_ADDSTRING, 0, (LPARAM) psz );  
}

inline void Combo_ResetContent(HWND hWndControl)
{
  SendMessage(hWndControl, (UINT) CB_RESETCONTENT,      0,0 );  
}    

inline int Combo_FindStringExact(HWND hWndControl, int nIndexStart, LPCTSTR lpszFind)
{
  return (int)::SendMessage(hWndControl, CB_FINDSTRINGEXACT, nIndexStart, (LPARAM)lpszFind);
}

inline int Combo_SetCurSel(HWND hWnd, int nSelect)
{
  assert(::IsWindow(hWnd));
  return (int)::SendMessage(hWnd, CB_SETCURSEL, nSelect, 0L);
}

inline int Combo_GetCurSel(HWND hWndControl)
{		
  return (int)::SendMessage(hWndControl, CB_GETCURSEL, 0, 0L);
}


inline int Combo_GetLBTextLen(HWND hWnd, int nIndex)
{
  assert(::IsWindow(hWnd));
  return (int)::SendMessage(hWnd, CB_GETLBTEXTLEN, nIndex, 0L);
}

inline int Combo_GetLBText(HWND hWnd, int nIndex, LPTSTR lpszText)
{
  assert(::IsWindow(hWnd));
  return (int)::SendMessage(hWnd, CB_GETLBTEXT, nIndex, (LPARAM)lpszText);
}

inline int Combo_GetLBText(HWND hWnd, int nIndex, std::wstring& strText)
{
  assert(::IsWindow(hWnd));
  int cchLen = Combo_GetLBTextLen(hWnd, nIndex);
  if(cchLen == CB_ERR)
    return CB_ERR;
  int nRet = CB_ERR;
  strText.resize(cchLen);
  LPTSTR lpstr = &strText[0];
  if(lpstr != NULL)
  {
    nRet = Combo_GetLBText(hWnd, nIndex, lpstr);
  }
  return nRet;
}


/////////////////////

class SaveClipRgn
{
  HDC  m_hDC;
  HRGN m_hOldRgn;

public:
  SaveClipRgn()
  {
    m_hDC = NULL;
    m_hOldRgn = NULL;
  }

  ~SaveClipRgn()
  {
    Restore();
    ASSERT(m_hDC == NULL);
    ASSERT(m_hOldRgn == NULL);
  }

  SaveClipRgn(HDC hdc)
  {
    ASSERT(hdc != NULL);
    m_hDC = NULL;
    m_hOldRgn = NULL;
    Save(hdc);
  }

  HRGN GetOldRgn()
  {
    if (m_hDC == NULL)
    {
      ASSERT(FALSE); //lint !e527
      return NULL;
    }
    ASSERT(m_hOldRgn != NULL);
    return m_hOldRgn;
  }

  BOOL Save(HDC hdc)
  {
    ASSERT(hdc != NULL);
    ASSERT(m_hOldRgn == NULL);
    ASSERT(m_hDC == NULL);
    m_hOldRgn = ::CreateRectRgn(0,0,0,0);
    if (m_hOldRgn != NULL)
    {
      switch (GetClipRgn(hdc, m_hOldRgn))
      {
      case 1:
        {
          // success, m_hOldRgn contains the current clipping region for the DC
          m_hDC = hdc;
        } break;

      case 0:
        {
          // success, but the DC has no clipping region
          m_hDC = hdc;
          DeleteObject(m_hOldRgn);
          m_hOldRgn = NULL;
        } break;

      default:
      case -1:
        {
          // erro
          DeleteObject(m_hOldRgn);
          m_hOldRgn = NULL;
        } break;
      }
    }

    return (m_hDC != NULL);
  }

  BOOL Restore()
  {
    if (m_hDC == NULL)
      return FALSE;

    // m_hDC only gets stored if the clipping region was saved successfully,
    // but m_hOldRgn may be NULL to indicate that no clipping region was defined
    ::SelectClipRgn(m_hDC, m_hOldRgn);

    if (m_hOldRgn != NULL)
    {
      // free the HRGN object (if any)
      DeleteObject(m_hOldRgn);
      m_hOldRgn = NULL;
    }

    m_hDC = NULL;

    return TRUE;
  }

  HRGN Detach()
  {
    HRGN hRgn = m_hOldRgn;
    m_hOldRgn = NULL;
    m_hDC = NULL;
    return hRgn;
  }
};





inline void DrawRectangle(HDC hdc, int x0, int y0, int x1, int y1, COLORREF cr)
{
  LOGBRUSH lb;
  lb.lbColor = 0;
  lb.lbHatch =HS_CROSS; 
  lb.lbStyle= BS_NULL;

  HBRUSH hBrush = CreateBrushIndirect(&lb);

  HPEN hPen = CreatePen(PS_SOLID, 0, cr);

  HPEN hOldPen = (HPEN) SelectObject(hdc, hPen);
  HBRUSH hOldBrush = (HBRUSH) SelectObject(hdc, hBrush);


  ::Rectangle(hdc, x0, y0, x1, y1);


  SelectObject(hdc, hOldPen);
  SelectObject(hdc, hOldBrush);
  DeleteObject(hPen);
  DeleteObject(hBrush);
}

inline void DrawRectangle(HDC hdc, LPRECT prc, COLORREF cr)
{
  DrawRectangle(hdc, prc->left, prc->top, prc->right, prc->bottom, cr);
}

inline void DrawSolidRectangle(HDC hdc, LPRECT prc, COLORREF fill, COLORREF cr)
{
  FillSolidRect(hdc, prc, fill);
  DrawRectangle(hdc, prc->left, prc->top, prc->right, prc->bottom, cr);
}

inline int ShowPopupMenu(HWND hWnd,
                   int x,
                   int y,
                   const std::vector<std::wstring>& items)
 {
   HMENU hMenu = CreatePopupMenu();

   for(int i = 0; i < (int)items.size(); i++)
   {
     if (items[i] == L"-")
       AppendMenu(hMenu, MF_SEPARATOR, i + 1, _T(""));
     else
       AppendMenu(hMenu, MF_STRING, i + 1, items[i].c_str());
   }

   POINT pt = {x, y};
   ClientToScreen(hWnd, &pt);
   BOOL b =  TrackPopupMenu(hMenu,
     TPM_LEFTALIGN |TPM_RETURNCMD,
     pt.x,
     pt.y,
     0,
     hWnd,
     0);

   DestroyMenu(hMenu);
   return b;
 }
 

///////////////////////////////////////////////////////////////

inline void GradientRectH(HDC hDC, RECT& rect, COLORREF fore, COLORREF bk)
{
   TRIVERTEX v[2];
   GRADIENT_RECT r[1];

   v[0].x     = (rect.left);
   v[0].y     = (rect.top);
   v[0].Red   = GetRValue(bk) << 8;
   v[0].Green = GetGValue(bk) << 8;
   v[0].Blue  = GetBValue(bk) << 8;
   v[0].Alpha = 0x0000;


   v[1].x     = (rect.right);
   v[1].y     = (rect.bottom);
   v[1].Red   = GetRValue(fore) << 8;
   v[1].Green = GetGValue(fore) << 8;
   v[1].Blue  = GetBValue(fore) << 8;
   v[1].Alpha = 0x0000;

   r[0].UpperLeft = 0;
   r[0].LowerRight = 1;

   GradientFill(hDC,v,(ULONG)2,r,(ULONG)1,GRADIENT_FILL_RECT_H);
}

inline void FillGradientRect(HDC hDC, RECT& rect, COLORREF fore, COLORREF bk, ULONG dwMode )
{
   TRIVERTEX v[2];
   GRADIENT_RECT r[1];

   v[0].x     = (rect.left);
   v[0].y     = (rect.top);
   v[0].Red   = GetRValue(bk) << 8;
   v[0].Green = GetGValue(bk) << 8;
   v[0].Blue  = GetBValue(bk) << 8;
   v[0].Alpha = 0x0000;


   v[1].x     = (rect.right);
   v[1].y     = (rect.bottom);
   v[1].Red   = GetRValue(fore) << 8;
   v[1].Green = GetGValue(fore) << 8;
   v[1].Blue  = GetBValue(fore) << 8;
   v[1].Alpha = 0x0000;

   r[0].UpperLeft = 0;
   r[0].LowerRight = 1;

   GradientFill(hDC,v,(ULONG)2,r,(ULONG)1, dwMode);
}

//, GRADIENT_FILL_RECT_H
inline void GradientRectV(HDC hdc, RECT& rect, COLORREF fore, COLORREF bk, ULONG dwMode = GRADIENT_FILL_RECT_V)
{
  FillGradientRect(hdc, rect, fore, bk, GRADIENT_FILL_RECT_V);   
}
inline void GradientRectH(HDC hdc, RECT& rect, COLORREF fore, COLORREF bk, ULONG dwMode = GRADIENT_FILL_RECT_V)
{
  FillGradientRect(hdc, rect, fore, bk, GRADIENT_FILL_RECT_H);   
}

namespace Gdi
{
    inline void DrawBitmap (HDC hdc, HBITMAP hbm, int Left, int Top) 
    {     
        BOOL f;    
        HDC hdcBits;     
        BITMAP bm;      
        hdcBits = CreateCompatibleDC(hdc);
        GetObject (hbm, sizeof(BITMAP), &bm);     
        SelectObject(hdcBits,hbm);     
        f = BitBlt(hdc,Left,Top,bm.bmWidth, bm.bmHeight,hdcBits,0,0,SRCCOPY);     
        DeleteDC(hdcBits); 
    }  


    struct Pen
    {
        HPEN m_hPen;

        Pen(int iStyle = PS_SOLID, int cWidth = 1, COLORREF color = RGB(0,0,0))
        {
            m_hPen = NULL;
            Create(iStyle, cWidth, color);
        }
        ~Pen()
        {
            DeletePen();
        }
        void Create(int iStyle, int cWidth, COLORREF color)
        {
            DeletePen();
            m_hPen = ::CreatePen(iStyle, cWidth, color);
        }

        void DeletePen()
        {
            if (m_hPen)
            {
                DeleteObject(m_hPen);
                m_hPen = NULL;
            }
        }
    };

    struct ScopeSelect
    {
        HDC m_hdc;
        HGDIOBJ m_hOld;
        ScopeSelect(HDC hdc, HGDIOBJ s)
        {
            m_hdc = hdc;
            m_hOld = ::SelectObject(hdc, s);
        }
        ~ScopeSelect()
        {
            ::SelectObject(m_hdc, m_hOld);
        }
    };

    class ClipRect
{
  SaveClipRgn m_old;
  HRGN m_hNewRegionClip;
public:

  ClipRect(HDC hdc, RECT& newClipRect) : m_old(hdc)
  {
    RECT rcClip = newClipRect;
    ::LPtoDP(hdc, (LPPOINT)&rcClip, 2);
    m_hNewRegionClip = CreateRectRgnIndirect(&rcClip);
    //SelectClipRgn(hdc, m_hNewRegionClip);    
    ExtSelectClipRgn(hdc, m_hNewRegionClip, RGN_AND);
  }

  ~ClipRect()
  {
    DeleteObject(m_hNewRegionClip);
    EndClip();
  }

  void EndClip()
  {
    m_old.Restore();
  }
};
////////////////////////
}

inline void Edit_SetSel(HWND hWnd, int nStartChar, int nEndChar, BOOL bNoScroll = TRUE)
{ 
    assert(::IsWindow(hWnd));
    ::SendMessage(hWnd, EM_SETSEL, nStartChar, nEndChar);
	  if (!bNoScroll)
		::SendMessage(hWnd, EM_SCROLLCARET, 0, 0); 
}

inline void Edit_SelectAll(HWND hWnd)
{
    Edit_SetSel(hWnd, 0, -1);
}


//rich edit

#include <Richedit.h>

inline void RichEdit_ReplaceSel(HWND hWnd, LPCTSTR psz)
{
    SendMessage(hWnd, (UINT) EM_REPLACESEL, FALSE, (LPARAM)psz);
}


inline std::wstring RichEdit_GetSelText(HWND hWnd) 
{
	ASSERT(::IsWindow(hWnd));
	CHARRANGE cr;
	cr.cpMin = cr.cpMax = 0;
	::SendMessage(hWnd, EM_EXGETSEL, 0, (LPARAM)&cr);
	std::vector<wchar_t> strText;
    strText.resize((cr.cpMax - cr.cpMin + 1) * 2);
	//LPTSTR lpsz=strText.GetBufferSetLength((cr.cpMax - cr.cpMin + 1) * 2);
	//lpsz[0] = NULL;
	::SendMessage(hWnd, EM_GETSELTEXT, 0, (LPARAM)&strText[0]);
	//strText.ReleaseBuffer();
	return std::wstring(&strText[0]);
}


inline void RichEdit_GetSel(HWND hWnd, long& nStartChar, long& nEndChar)
{
    assert(::IsWindow(hWnd));
    CHARRANGE cr;
    ::SendMessage(hWnd, EM_EXGETSEL, 0, (LPARAM)&cr);
    nStartChar = cr.cpMin;
    nEndChar = cr.cpMax;
}

inline void RichEdit_SetSel(HWND hWnd, long nStartChar, long nEndChar)
{
    assert(::IsWindow(hWnd));
    CHARRANGE cr;
    cr.cpMin = nStartChar;
    cr.cpMax = nEndChar;
    ::SendMessage(hWnd, EM_EXSETSEL, 0, (LPARAM)&cr);
}

inline BOOL RichEdit_LineScroll(HWND hWnd, int nLines, int nChars = 0)
{
    assert(::IsWindow(hWnd));
    return (BOOL)::SendMessage(hWnd, EM_LINESCROLL, nChars, nLines);
}

inline void RichEdit_Clear(HWND hWnd)
{
    assert(::IsWindow(hWnd));
    ::SendMessage(hWnd, WM_CLEAR, 0, 0L);
}

inline BOOL RichEditCtrl_SetReadOnly(HWND hWnd, BOOL bReadOnly)
{
  assert(::IsWindow(hWnd));
  return (BOOL)::SendMessage(hWnd, EM_SETREADONLY, bReadOnly, 0L); 
}


inline int RichEdit_GetLineCount(HWND hWnd);

inline void RichEdit_ClearAll(HWND hWnd)
{    
    RichEditCtrl_SetReadOnly(hWnd, FALSE);
    RichEdit_SetSel(hWnd, 0, -1);
    RichEdit_Clear(hWnd);    
}

inline int RichEdit_GetLineCount(HWND hWnd)
{
  assert(::IsWindow(hWnd));	
  return (int)::SendMessage(hWnd, EM_GETLINECOUNT, 0, 0);
}

inline int RichEdit_LineIndex(HWND hWnd, int nLine = -1)
{
    assert(::IsWindow(hWnd));
    return (int)::SendMessage(hWnd, EM_LINEINDEX, nLine, 0L);
}

inline int RichEdit_LineLength(HWND hWnd,int nLine = -1)
	{
		assert(::IsWindow(hWnd));
		return (int)::SendMessage(hWnd, EM_LINELENGTH, nLine, 0L);
	}

inline int RichEdit_LineFromChar(HWND hWnd,int nIndex = -1)
{
    assert(::IsWindow(hWnd));
    return (int)::SendMessage(hWnd, EM_LINEFROMCHAR, nIndex, 0L);
}

inline DWORD RichEdit_GetSelectionCharFormat(HWND hWnd, CHARFORMAT &cf)
{
    assert(::IsWindow(hWnd));
    cf.cbSize = sizeof(CHARFORMAT);
    return (DWORD)::SendMessage(hWnd, EM_GETCHARFORMAT, 1, (LPARAM)&cf);
}

inline DWORD RichEdit_GetSelectionCharFormat(HWND hWnd, CHARFORMAT2 &cf)
{
    assert(::IsWindow(hWnd));
    cf.cbSize = sizeof(CHARFORMAT2);
    return (DWORD)::SendMessage(hWnd, EM_GETCHARFORMAT, 1, (LPARAM)&cf);
}

inline BOOL RichEdit_SetSelectionCharFormat(HWND hWnd, CHARFORMAT &cf)
{
    assert(::IsWindow(hWnd));
    cf.cbSize = sizeof(CHARFORMAT);
    return (BOOL)::SendMessage(hWnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
}

inline BOOL RichEdit_SetSelectionCharFormat(HWND hWnd, CHARFORMAT2 &cf)
{
    assert(::IsWindow(hWnd));
    cf.cbSize = sizeof(CHARFORMAT2);
    return (BOOL)::SendMessage(hWnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
}
inline int RichEdit_GetFirstVisibleLine(HWND hWnd) 
{
    assert(::IsWindow(hWnd));
    return (int)::SendMessage(hWnd, EM_GETFIRSTVISIBLELINE, 0, 0L);
}

inline int RichEdit_SetEventMask(HWND hWnd, DWORD dwMask)  
{
    //ENM_MOUSEEVENTS
    assert(::IsWindow(hWnd));
    return (int)::SendMessage(hWnd, EM_SETEVENTMASK, 0, dwMask);
}

inline void RichEdit_Copy(HWND hWnd)
{
    assert(::IsWindow(hWnd));
    ::SendMessage(hWnd, WM_COPY, 0, 0L);
}

inline LONG RichEdit_FindText(HWND hWnd, DWORD dwFlags, FINDTEXT& ft)
	{
		assert(::IsWindow(hWnd));
#if (_RICHEDIT_VER >= 0x0200) && defined(_UNICODE)
		return (LONG)::SendMessage(hWnd, EM_FINDTEXTW, dwFlags, (LPARAM)&ft);
#else
		return (LONG)::SendMessage(m_hWnd, EM_FINDTEXT, dwFlags, (LPARAM)&ft);
#endif
	}

inline COLORREF RichEdit_SetBackgroundColor(HWND hWnd, BOOL bSysColor, COLORREF cr)
{ 
  assert(::IsWindow(hWnd));
  return (COLORREF)::SendMessage(hWnd, EM_SETBKGNDCOLOR, bSysColor, cr); 
}


inline void RichEdit_InsertText(HWND edit, 
                                LPCTSTR psz,
                                COLORREF color, 
                                BOOL bold, 
                                BOOL strikeout)
{
    CHARFORMAT cf0;
    RichEdit_GetSelectionCharFormat(edit, cf0);

    RichEdit_ReplaceSel(edit, psz);

    LONG s, e;
    RichEdit_GetSel(edit, s, e);
    RichEdit_SetSel(edit, e  - _tcslen(psz), e + 1);



    CHARFORMAT cf;

    // Modify the selection format so that the selected text is 
    // displayed in bold and not striked out.
    cf.cbSize = sizeof(cf);
    cf.crTextColor = color;

    cf.dwMask = CFM_COLOR  |CFM_BOLD;  
    cf.dwEffects = 0;

    if (bold)
    {
        cf.dwMask |= CFM_BOLD;  
        cf.dwEffects |= CFE_BOLD;
    }

    if (strikeout)
    {
        cf.dwEffects |= CFE_STRIKEOUT;
    }
    cf.dwMask |= CFM_STRIKEOUT;  

    RichEdit_SetSelectionCharFormat(edit, cf);
    RichEdit_SetSel(edit, e+1,e+1);
    RichEdit_SetSelectionCharFormat(edit, cf0);
}

inline UINT Button_GetState(HWND hWnd)
{
    assert(::IsWindow(hWnd)); 
    return (UINT)::SendMessage(hWnd, BM_GETSTATE, 0, 0); 
}


inline int ComboBox_AddString(HWND hWnd, LPCTSTR lpszString)
	{ 
    assert(::IsWindow(hWnd));
    return (int)::SendMessage(hWnd, CB_ADDSTRING, 0, (LPARAM)lpszString); 
}

inline void Window_SetFont(HWND hWnd, HFONT hFont, BOOL bRedraw = TRUE)
{ 
  assert(::IsWindow(hWnd)); 
  ::SendMessage(hWnd, WM_SETFONT, (WPARAM)hFont, bRedraw); 
}

inline DWORD_PTR ComboBox_GetItemData(HWND hWnd, int nIndex)
 { 
   assert(::IsWindow(hWnd)); 
   return ::SendMessage(hWnd, CB_GETITEMDATA, nIndex, 0); 
}

inline int ComboBox_SetItemData(HWND hWnd, int nIndex, DWORD_PTR dwItemData)
{
  assert(::IsWindow(hWnd)); 
  return (int)::SendMessage(hWnd, CB_SETITEMDATA, nIndex, (LPARAM)dwItemData); 
}


inline int ComboBox_GetCount(HWND hWnd) 
{
  assert(::IsWindow(hWnd)); 
  return (int)::SendMessage(hWnd, CB_GETCOUNT, 0, 0); 
}
inline int ComboBox_GetCurSel(HWND hWnd) 
{ 
  assert(::IsWindow(hWnd));
  return (int)::SendMessage(hWnd, CB_GETCURSEL, 0, 0); 
}
inline int ComboBox_SetCurSel(HWND hWnd,int nSelect)
{
  assert(::IsWindow(hWnd));
  return (int)::SendMessage(hWnd, CB_SETCURSEL, nSelect, 0); 
}
