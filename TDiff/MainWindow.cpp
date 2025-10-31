#include "StdAfx.h"
#include "MainWindow.h"

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <set>
#include <ctype.h>
#include <map>
#include "diff/Levenshtein.h" 
#include "stopwatch.h"
#include "diff/Myers.h"
#include "AboutDlg.h"
#include "ChooseDlg.h"
#include "OptionsDlg.h"
#include <codecvt> //utf8

using namespace std;
using namespace Algorithm::Hirschberg;


#define TDIFF_CAPTION _T("TDiff")

#define IDC_RICHEDIT1 10
#define IDC_FIRST 20
#define IDC_PREVIOUS 30
#define IDC_NEWCODE 40
#define IDC_NEXT 50
#define IDC_LAST 60
#define IDC_OLDCODE 70
#define IDC_EXIT 80

//////////////////////////////////////////


const bool s_DoMergeInLines = true;

/////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////

MainWindow::MainWindow(const std::wstring& fullpathname)
{
    m_Path = fullpathname;
    m_FontSize = 11;
    m_FontName = L"Courier New";
    m_hFont = NULL;
    m_hFirst = NULL;
    m_hPrev = NULL;
    m_hNext = NULL;
    m_hLast = NULL;
    m_PopupMenuActivated = FALSE;
    m_DiffIndex = 0;
    m_BackgroundColor = RGB(255,255,255);
    m_AddedColor = RGB(0,0,255);
    m_DeletedColor = RGB(255,0,0);;
    m_SameColor= RGB(128,128,128);

    LoadOptions();
}

MainWindow::~MainWindow()
{
  DeleteObject(m_hFont);
}



void MainWindow::InsertText(HWND edit, wchar_t ch, COLORREF color, BOOL bold, BOOL strikeout)
{
    wchar_t psz[2] = {0, 0 };
    psz[0] = ch;
    RichEdit_InsertText(edit, psz, color, bold, strikeout);
}

void MainWindow::InsertText(HWND edit, wchar_t ch, COLORREF color)
{
    wchar_t psz[2] = {0, 0 };
    psz[0] = ch;
    RichEdit_InsertText(edit, psz, color, FALSE, FALSE);  
}

void MainWindow::InsertText(HWND edit, LPCTSTR psz, COLORREF color)
{
    RichEdit_InsertText(edit, psz, color, FALSE, FALSE);  
}


void MainWindow::InsertSameText(HWND edit, LPCTSTR psz)
{
    RichEdit_InsertText(edit, psz, m_SameColor, FALSE, FALSE);  
}

void MainWindow::InsertSameText(HWND edit, wchar_t ch)
{
    wchar_t psz[2] = {0, 0 };
    psz[0] = ch;    
    InsertSameText(edit, psz);
}

void MainWindow::InsertAddedText(HWND edit, LPCTSTR psz)
{
    RichEdit_InsertText(edit, psz, m_AddedColor, TRUE, FALSE);  
}

void MainWindow::InsertAddedText(HWND edit, wchar_t ch)
{
    wchar_t psz[2] = {0, 0 };
    psz[0] = ch;    

    InsertAddedText(edit, psz);
}

void MainWindow::InsertDeletedText(HWND edit, LPCTSTR psz)
{
    RichEdit_InsertText(edit, psz, m_DeletedColor, FALSE, TRUE);  
}

void MainWindow::InsertDeletedText2(HWND edit)
{
  ASSERT(FALSE);
    //InsertText(edit, _T("..."), RGB(255,0,0), FALSE, TRUE);
    //InsertDeletedText(edit, L"\n"); 
}

void MainWindow::InsertDeletedText(HWND edit, wchar_t ch)
{
    wchar_t psz[2] = {0, 0 };
    psz[0] = ch;    

    InsertDeletedText(edit, psz);
}

void MainWindow::Next()
{
    long start, end;
    RichEdit_GetSel(m_hRichEdit, start, end);

    //indice baseado em zero
    int currentLine = RichEdit_LineFromChar(m_hRichEdit, start);

    if(currentLine < 0)
        currentLine = 0;

    if (m_Changes.empty() || currentLine >= m_Changes.size())
    {
        RichEdit_SetSel(m_hRichEdit, start, end);
        SetFocus(m_hRichEdit);
        return;
    }

    
    Algorithm::Hirschberg::ChangeType changetype = m_Changes[currentLine];

    for (size_t i = currentLine + 1; i < m_Changes.size(); i++)
    {
        //Quando mudar a primeira vez ja fica como none
        if (m_Changes[i] != changetype)
        {
            changetype = Algorithm::Hirschberg::ChangeTypeNone;
        }

        if (m_Changes[i] != changetype && 
            m_Changes[i] != Algorithm::Hirschberg::ChangeTypeNone)
        {                
            int ci = RichEdit_LineIndex(m_hRichEdit, i);
            assert(ci >= 0);

            RichEdit_SetSel(m_hRichEdit, ci, ci+1);
            SetFocus(m_hRichEdit);
            return; //break;

        }
    }

    //nao se move
    RichEdit_SetSel(m_hRichEdit, start, end);
    SetFocus(m_hRichEdit);
}

void MainWindow::Previous()
{
    long start, end;
    RichEdit_GetSel(m_hRichEdit, start, end);
    int currentLine = RichEdit_LineFromChar(m_hRichEdit, start);

    if (currentLine <= 0 || m_Changes.empty() || currentLine >= m_Changes.size())
    {
        RichEdit_SetSel(m_hRichEdit, start, end);
        SetFocus(m_hRichEdit);
        return;
    }


    Algorithm::Hirschberg::ChangeType changetype = m_Changes[currentLine];

    for (int i = currentLine - 1; i >= 0; i--)
    {
        //Quando mudar a primeira vez ja fica como none
        if (m_Changes[i] != changetype)
        {
            changetype = Algorithm::Hirschberg::ChangeTypeNone;
        }

        if (m_Changes[i] != changetype &&
            m_Changes[i] != Algorithm::Hirschberg::ChangeTypeNone)
        {                
            int ci = RichEdit_LineIndex(m_hRichEdit, i);
            assert(ci >= 0);

            RichEdit_SetSel(m_hRichEdit, ci, ci+1);
            SetFocus(m_hRichEdit);
            return;//break;
        }
    }

    //nao se move
    RichEdit_SetSel(m_hRichEdit, start, end);
    SetFocus(m_hRichEdit);
}


void MainWindow::MoveFirst()
{
    long start, end;
    RichEdit_GetSel(m_hRichEdit, start, end);


    if (m_Changes.empty())
    {
        RichEdit_SetSel(m_hRichEdit, start, end);
        SetFocus(m_hRichEdit);
        return;
    }

    int currentLine = 0;
    Algorithm::Hirschberg::ChangeType changetype = m_Changes[currentLine];
    for (size_t i = currentLine; i < m_Changes.size(); i++)
    {
        if (m_Changes[i] != Algorithm::Hirschberg::ChangeTypeNone)
        {                
            int ci = RichEdit_LineIndex(m_hRichEdit, i);
            RichEdit_SetSel(m_hRichEdit, ci, ci+1);
            SetFocus(m_hRichEdit);
            return; //break;
        }
    }
    //nao se move
    RichEdit_SetSel(m_hRichEdit, start, end);
    SetFocus(m_hRichEdit);
}

void MainWindow::MoveLast()
{
    long start, end;
    RichEdit_GetSel(m_hRichEdit, start, end);

    if (m_Changes.empty())
    {
        RichEdit_SetSel(m_hRichEdit, start, end);
        SetFocus(m_hRichEdit);
        return;
    }

    int currentLine = m_Changes.size()-1;
    Algorithm::Hirschberg::ChangeType changetype = m_Changes[currentLine];

    for (int i = currentLine; i >= 0; i--)
    {
        if (m_Changes[i] != Algorithm::Hirschberg::ChangeTypeNone)
        {                
            int ci = RichEdit_LineIndex(m_hRichEdit, i);
            assert(ci >= 0);
            
            RichEdit_SetSel(m_hRichEdit, ci, ci+1);
            SetFocus(m_hRichEdit);
            
            return; //break;
        }
    }
    //nao se move
    RichEdit_SetSel(m_hRichEdit, start, end);
    SetFocus(m_hRichEdit);
}

BOOL MainWindow::OnNotify(NMHDR* pw, NMHDR* pl)
{
  
  if (pl != 0 && pl->hwndFrom == m_hRichEdit)
  {
     MSGFILTER *pFilter = (MSGFILTER*) pl;   
     if (pFilter->msg == WM_RBUTTONDOWN)
     {
       POINT pt ;
       pt.x = GET_X_LPARAM(pFilter->lParam); 
       pt.y = GET_Y_LPARAM(pFilter->lParam); 

        m_PopupMenuActivated = TRUE;
        HMENU hMenu = GetMenu(m_hWnd);
        HMENU hMenuEdit = GetSubMenu(hMenu, 1);
        
        ClientToScreen(m_hRichEdit, &pt);
        int i =  TrackPopupMenu(hMenuEdit,
            TPM_LEFTALIGN |TPM_RETURNCMD,
            pt.x,
            pt.y,
            0,
            m_hRichEdit,
            0);

       m_PopupMenuActivated = FALSE;

       switch (i)
       {
       case ID_EDIT_COPY: CopySelection();break;
       
       case ID_EDIT_FIND: Find();break;
       case ID_EDIT_FINDNEXT: 
           OnFindNextEvent(m_FindDlg.m_TextToFind, m_FindDlg.m_MatchCase, m_FindDlg.m_MatchWholeWord, m_FindDlg.m_SearchUp);
           break;

       case ID_EDIT_MOVENEXT: Next();break;
       case ID_EDIT_MOVEPREVIOUS: Previous();break;
       case ID_EDIT_MOVEFIRST: MoveFirst();break;
       case ID_EDIT_MOVELAST: MoveLast();break;
       } 
       
       return TRUE;
     }
  }
  return FALSE;
}

static void CopyToClipboard(const std::wstring& ws)
{
  // Open and empty the clipboard
  if (::OpenClipboard(0))
  {
    if (::EmptyClipboard())
    {
      // Calculate number of bytes to be written (includes '\0' terminator)
      const int nBytes = ( ws.size() + 1 ) * sizeof(wchar_t);
      // Allocate global memory
      HGLOBAL hMem = ::GlobalAlloc(GMEM_MOVEABLE, nBytes);
      if (hMem != NULL)
      {
        // Get a pointer to the memory
        LPVOID pMem = ::GlobalLock(hMem);
        if (pMem != NULL)
        {
          // Copy string to memory
          ::memcpy(pMem, &ws[0], nBytes);
          ::GlobalUnlock(hMem);      
          // Copy memory to clipboard
          ::SetClipboardData(CF_UNICODETEXT, hMem);
        }
      }
    }
    // Close the clipboard
    ::CloseClipboard();
  }
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
void MainWindow::CopySelection()
{
  //Não funciona se eu fecho o programa ele perde o clip
  //RichEdit_Copy(m_hRichEdit);  
  std::wstring ws = RichEdit_GetSelText(m_hRichEdit);
  find_replace(ws, L"\r", L"\r\n");
  CopyToClipboard(ws);
}

void MainWindow::OnFindNextEvent(const std::wstring& str, bool matchcase, bool matchword, bool searchUp) 
{
    long s, e;
    RichEdit_GetSel(m_hRichEdit, s, e);

    FINDTEXT ftxt;
    ftxt.lpstrText = m_FindDlg.m_TextToFind.c_str();

    if (m_IsFirstFound)
      ftxt.chrg.cpMin = e;
    else
      ftxt.chrg.cpMin = s;

    ftxt.chrg.cpMax = -1;
    DWORD flags = searchUp ? FR_NOUPDOWN : FR_DOWN;
        
        if (matchcase)
            flags |= FR_MATCHCASE;

        if (matchword)
            flags |= FR_WHOLEWORD;

    long pos = -1;
    for (int k = 0; k < 2; k++)
    {
        pos = RichEdit_FindText(m_hRichEdit, flags, ftxt);
        
        if (pos >= 0)
        {
            if (!m_IsFirstFound)
            {                
                m_FindStart = pos;
                m_IsFirstFound = true;
                
                //estava em cima da 1 palavra encontrada entao pular
               // if (pos == s)
                //{
                  //  ftxt.chrg.cpMin = pos + str.size();
                    //continue;
                //}               
            }
            else if (m_FindStart == pos)
            {
                m_FindStart = pos;
                m_IsFirstFound = false;
                RichEdit_SetSel(m_hRichEdit, pos, pos + m_FindDlg.m_TextToFind.size());                
                MessageBox(m_FindDlg.m_hDlg, L"Find reached the starting point of the search.", L"Find", MB_OK | MB_ICONINFORMATION);
                //SetFocus(m_hRichEdit);
                return;
            }
           
            RichEdit_SetSel(m_hRichEdit, pos, pos + m_FindDlg.m_TextToFind.size());
           // SetFocus(m_hRichEdit);
            break;
        }
        
        //nao achou vai pro zero
        //TODO considerar up dai tem q ir pro fim
        ftxt.chrg.cpMin = 0;        
    }

    if (pos < 0 )
    {
        //if (!m_IsFirstFound)
        {
            std::wstring msg;
            msg += L"The following specified text was not found:\n\n";
            msg += str;
            MessageBox(m_FindDlg.m_hDlg, msg.c_str(), L"Find", MB_OK | MB_ICONINFORMATION);
        }
        //else
        {
          //  MessageBox(m_FindDlg.m_hDlg, L"Find reached the starting point of the search.", L"Find", MB_OK | MB_ICONINFORMATION);
        }
    }

}

void MainWindow::Find()
{    
    m_FindDlg.Show();
    long s, e;
    RichEdit_GetSel(m_hRichEdit, s, e);
    
    m_FindStart  = -1;
    m_IsFirstFound = false;
    
    std::wstring ss = RichEdit_GetSelText(m_hRichEdit);
    
    m_FindDlg.SetText(ss);    
}

bool MainWindow::SaveOptions()
{
  std::wstring filename = m_Path + L"tdiffconfig.txt";
  std::wofstream file(filename);
  if (file.is_open())
  {
    file << m_FontName << std::endl;
    file << m_FontSize << std::endl;
    file << GetRValue(m_SameColor) << L" " << GetGValue(m_SameColor) << L" " << GetBValue(m_SameColor)  << std::endl;
    file << GetRValue(m_AddedColor) << L" " << GetGValue(m_AddedColor) << L" " << GetBValue(m_AddedColor)  << std::endl;
    file << GetRValue(m_DeletedColor) << L" " << GetGValue(m_DeletedColor) << L" " << GetBValue(m_DeletedColor)  << std::endl;
    file << GetRValue(m_BackgroundColor) << L" " << GetGValue(m_BackgroundColor) << L" " << GetBValue(m_BackgroundColor)  << std::endl;
    return true;
  }
  return false;
}

COLORREF ParseColor(const std::wstring s)
{
  std::wstringstream ss(s);
  int r, g, b;
  ss >> r >> g >> b;
  return RGB(r,g,b);
}

void MainWindow::LoadOptions()
{
  std::wstring filename = m_Path + L"tdiffconfig.txt";
  std::wifstream file(filename);

  //optional
  if (file.is_open())
  {
    std::getline(file, m_FontName);
    std::wstring fontSizeString;
    std::getline(file, fontSizeString);
    m_FontSize = _wtoi(fontSizeString.c_str());  

    std::wstring colorline;

    if (std::getline(file, colorline))
    {
      m_SameColor = ParseColor(colorline);
    }

    if (std::getline(file, colorline))
    {
      m_AddedColor= ParseColor(colorline);
    }

    if (std::getline(file, colorline))
    {
      m_DeletedColor = ParseColor(colorline);
    }
    
    if (std::getline(file, colorline))
    {
      m_BackgroundColor = ParseColor(colorline);
    }
  }
}

void MainWindow::OnCommand(int hwp, int lwp, HWND)
{
    if (hwp == IDM_EXIT || hwp == IDC_EXIT)
    {
        CloseWindow(m_hWnd);
        PostQuitMessage(0);
    }
    else if (hwp == ID_EDIT_COPY)
    {
        CopySelection();
    }
    else if (hwp == ID_EDIT_MOVEFIRST)
    {
        MoveFirst();
    }
    else if (hwp == ID_EDIT_MOVENEXT)
    {
        Next();
    }
    else if (hwp == ID_EDIT_MOVEPREVIOUS)
    {
        Previous();
    }
    else if (hwp == ID_EDIT_MOVELAST)
    {
        MoveLast();
    }
    else if (hwp == ID_EDIT_FIND)
    {
        Find();
    }
    else if (hwp == ID_EDIT_FINDNEXT)
    {
        OnFindNextEvent(m_FindDlg.m_TextToFind, m_FindDlg.m_MatchCase, m_FindDlg.m_MatchWholeWord, m_FindDlg.m_SearchUp);
    }
    else if (hwp == ID_FILE_OPTIONS)
    {
      OptionsDlg dlg(m_hWnd);
      
      dlg.m_FontName = m_FontName;
      dlg.m_FontSize = m_FontSize;

      dlg.m_AddedColor= m_AddedColor;
      dlg.m_DeletedColor= m_DeletedColor;
      dlg.m_SameColor= m_SameColor;
      dlg.m_BackgroundColor = m_BackgroundColor;

      if (dlg.ShowDialog() == IDOK)
      {
        m_FontName = dlg.m_FontName;
        m_FontSize = dlg.m_FontSize;

        m_AddedColor = dlg.m_AddedColor ;
        m_DeletedColor = dlg.m_DeletedColor ;
        m_SameColor = dlg.m_SameColor ;
        m_BackgroundColor = dlg.m_BackgroundColor;

        ChangeFont(m_FontName.c_str(), m_FontSize);
        
        RichEdit_SetBackgroundColor(m_hRichEdit, 0, m_BackgroundColor);
        Window_SetFont(m_hRichEdit, m_hFont);

        if (!m_OldFileName.empty() && !m_NewFileName.empty())
        {
          Run();
          MoveFirst();
        }

        SaveOptions();
      }
    }
    else if (hwp == ID_FILE_REFRESH)
    {
      if (!m_OldFileName.empty() && !m_NewFileName.empty())
      {
        Run();
        MoveFirst();
      }     
    }
    else if(hwp == ID_ABOUT_ABOUT)
    {
        AboutDlg dlg(m_hWnd);
        dlg.ShowDialog();

    }
    else if (hwp == ID_FILE_OPEN)
    {
        ChooseDlg dlg(m_hWnd);
        dlg.m_file1=m_OldFileName;
        dlg.m_file2=m_NewFileName;

        if (dlg.ShowDialog() == IDOK)
        {
            m_OldFileName = dlg.m_file1;
            m_NewFileName = dlg.m_file2;
            Run();
            MoveFirst();
            //SetSel(m_hRichEdit,0 ,0);
            //Next();
        }
    }

    else if (hwp == IDC_FIRST)
    {
        MoveFirst();

    }
    else if (hwp == IDC_PREVIOUS)
    {
        Previous();

    }
    else if (hwp == IDC_NEXT)
    {
        Next();
    }
    else if (hwp == IDC_LAST)
    {

        MoveLast();     
    }
}

BOOL MainWindow::OnClose()
{    
    CloseWindow(m_hWnd);
    PostQuitMessage(0);
    return TRUE;
}

//Este aqui nao pode desenhar direto.. pq ainda eh decidido depois como vai ser.
struct PrintWord
{
    unsigned int Count;
    const std::wstring &s1;
    const std::wstring &s2;
    int line1;
    int line2;
    bool m_onlyNew;

    std::wstring deleted;
    std::wstring added;

    HWND m_file;
    MainWindow & mainWindow;

    ~PrintWord()
    {
      printChanged();
    }

    void printChanged()
    {
        if (!deleted.empty())
        {
            //TODO nao pode desenhar ainda
            if (!m_onlyNew)
            {
                mainWindow.InsertDeletedText(m_file, deleted.c_str());
            }
            deleted.clear();
        }
        if (!added.empty())
        {
            mainWindow.InsertAddedText(m_file, added.c_str());
            added.clear();
        }
    }

    PrintWord(const std::wstring &ss1, const std::wstring &ss2, MainWindow& mw) :  s1(ss1), s2(ss2) , mainWindow(mw)
    {
        m_onlyNew = false;
        line1 = 0;
        line2 = 0;
    }

    void operator()(Algorithm::Hirschberg::ChangeType e)
    {
        switch (e)
        {
        case Algorithm::Hirschberg::ChangeTypeNone: 
            {
                printChanged();
                mainWindow.InsertSameText(m_file, s1[line1]);
                line1++;
                line2++;
            }
            break;
        case Algorithm::Hirschberg::ChangeTypeInsert:
            {
                printChanged();
                mainWindow.InsertAddedText(m_file, s2[line2++]);
            }
            break;

        case Algorithm::Hirschberg::ChangeTypeDelete:
            {
                printChanged();
                if (!m_onlyNew)
                {
                    mainWindow.InsertDeletedText(m_file, s1[line1++]);
                }
            }
            break;

        case Algorithm::Hirschberg::ChangeTypeChange: 
            {
                wchar_t c1= s1[line1++];
                deleted.append(&c1, 1);
                wchar_t c2 = s2[line2++];
                added.append(&c2, 1);
            }
            break;
        }
    }
};


//Este aqui nao pode desenhar direto.. pq ainda eh decidido depois como vai ser.
struct MyersPrintWord
{
  int m_a;
  int m_b;
   const std::wstring &s1;
    const std::wstring & s2;

    HWND m_file;
    MainWindow& mainWindow;

    MyersPrintWord(MainWindow& mw, const std::wstring &ss1, const std::wstring &ss2) :  s1(ss1), s2(ss2) , mainWindow(mw)
    {
       m_a = 0;
       m_b = 0;
    }

  
    void operator()(Algorithm::Myers::Operation op, size_t i)
    {
      switch (op)
      {
      case Algorithm::Myers::RemoveFromB:
        {
          int k = 0;
          for ( ; k < i; k++)
          {
              mainWindow.InsertDeletedText(m_file, s1[m_b + k]);
          }
          m_b += k;
        }

        break;
      case Algorithm::Myers::InsertFromA:
        {
           int k = 0;
           for ( ; k < i; k++)
           {
              mainWindow.InsertAddedText(m_file, s2[m_a + k]);
           }
           m_a += k;
        }
        break;
      case Algorithm::Myers::EqualAB:
        {
          int k = 0;
          for ( ; k < i; k++)
          {
              mainWindow.InsertSameText(m_file, s1[m_b + k]);
          }
          m_a += k;
          m_b += k;
        }
        break;
      }
    }
};


    template<class T>
void CallOut(MemoOfChanges& changes, T& out)
{
    for (size_t i = 0 ; i < changes.size(); i++)
    {
        out(changes[i]);
    }
}

template<class T>
void CallOut(TMyersMemoOfChanges& changes, T& out)
{
    for (size_t i = 0 ; i < changes.size(); i++)
    {
        out(changes[i].m_Operation, changes[i].m_Count);
    }
}


// Imprime palavaras de uma frase
struct PrintWords
{
    unsigned int Count;
    unsigned int m_line1;
    unsigned int m_line2;
    const Lexemes& m_v1;
    const Lexemes& m_v2;
    HWND m_hedit;
    MainWindow& mainWindow;

    PrintWords(MainWindow& mw, HWND h, const Lexemes& v1, const Lexemes& v2)
        : m_hedit(h), m_v1(v1), m_v2(v2), Count(0), m_line1(0), m_line2(0), mainWindow(mw)
    {
    }

    void operator()(Algorithm::Hirschberg::ChangeType e)
    {
        switch (e)
        {
        case Algorithm::Hirschberg::ChangeTypeNone: 
            mainWindow.InsertSameText(m_hedit, m_v1[m_line1].m_lexeme.c_str());
            m_line1++;
            m_line2++;
            break;
        case Algorithm::Hirschberg::ChangeTypeInsert:
            mainWindow.InsertAddedText(m_hedit, m_v2[m_line2].m_lexeme.c_str());
            m_line2++;
            break;
        case Algorithm::Hirschberg::ChangeTypeDelete:
            mainWindow.InsertDeletedText(m_hedit, m_v1[m_line1].m_lexeme.c_str());
            m_line1++;
            break;
        case Algorithm::Hirschberg::ChangeTypeChange: 
            {
                //Talvez seja o caso de so fazer se for identificador
                //ou string
                const std::wstring & s1 = m_v1[m_line1].m_lexeme;
                const std::wstring & s2 = m_v2[m_line2].m_lexeme;
                MyersCountAndKeppChanges mf;
                Algorithm::Myers::MyersDiff(s1, s2, std::equal_to<wchar_t>(), mf);

                if (mf.m_Added == 0 && mf.m_Removed > 0)
                {
                    //so add
                  MyersPrintWord ps(mainWindow, s1, s2);
                    ps.m_file = m_hedit;
                    CallOut(mf.m_Changes, ps);
                }
                else if (mf.m_Added > 0 && mf.m_Removed == 0)
                {
                    //so del
                    MyersPrintWord ps(mainWindow, s1, s2);
                    ps.m_file = m_hedit;
                    CallOut(mf.m_Changes, ps);
                }
                else if ((mf.m_Added + mf.m_Removed) / 2 < (mf.m_Same + mf.m_Added + mf.m_Removed) / 3)
                {
                    //so del
                    MyersPrintWord ps(mainWindow, s1, s2);
                    ps.m_file = m_hedit;
                    CallOut(mf.m_Changes, ps);
                }
                else
                {
                    mainWindow.InsertDeletedText(m_hedit, s1.c_str());
                    mainWindow.InsertAddedText(m_hedit, s2.c_str());
                }
                /*
                CountAndKeepChanges fc;
                Hirschberg(m_v1[m_line1].m_lexeme, 
                    m_v2[m_line2].m_lexeme, fc, std::not_equal_to<wchar_t>());

                if (fc.changed_count <= 1)
                {
                    PrintWord ps(m_v1[m_line1].m_lexeme, m_v2[m_line2].m_lexeme);
                    ps.m_file = m_hedit;
                    CallOut(fc.m_Changes, ps);
                }
                else
                {
                    InsertDeletedText(m_hedit, m_v1[m_line1].m_lexeme.c_str());
                    InsertAddedText(m_hedit, m_v2[m_line2].m_lexeme.c_str());
                }
                */
                m_line1++;
                m_line2++;

            }
            break;
        }
    }
};




struct MyersPrintLines2
{
    unsigned int Count;    
    TextLines &m_s1;
    TextLines &m_s2;
    int m_a;
    int m_b;
    int lineFinal;
    bool m_onlyNew;
    MemoOfChanges m_Changes;
    HWND m_file;
    std::vector<int> m_deleted;

    MainWindow& mainWindow;

    MyersPrintLines2(MainWindow& mw, HWND edit,        
        TextLines &ss1,
        TextLines &ss2,
        int a,
        int b)
        : Count(0),  m_s1(ss1), m_s2(ss2),mainWindow(mw)
    {
        m_file = edit;
        m_a = a;
        m_b = b;
        lineFinal = 0;
        m_onlyNew = false;
    }

    void PrintDeletedLines(int start = 0)
    {
      //coloca os deletados
      for (int k = start; k < m_deleted.size(); k++)
      {
        m_Changes.push_back(Algorithm::Hirschberg::ChangeTypeDelete);
        mainWindow.InsertDeletedText(m_file, m_s1[m_deleted[k]].m_text.c_str()); 
        mainWindow.InsertDeletedText(m_file, L"\n"); 

        /* se quiser apenas 1 lniha
        m_Changes.push_back(Algorithm::Hirschberg::ChangeTypeDelete);
        mainWindow.InsertDeletedText2(m_file);         
        break;
        */
      }
      
   

      m_deleted.clear();
    }

    ~MyersPrintLines2()
    {
      PrintDeletedLines();
    }

    void PrintRemaning()
    {
      PrintDeletedLines();
    }

    void operator()(Algorithm::Myers::Operation op, size_t i)
    {
      switch (op)
      {
      case Algorithm::Myers::RemoveFromB:
        {
          int k = 0;
          for ( ; k < i; k++)
          {
            m_deleted.push_back(m_b + k);
          }
          m_b += k;
        }

        break;
      case Algorithm::Myers::InsertFromA:
        {
          int k = 0;
          int j = 0;
         
           PrintDeletedLines(j);

           for ( ; k < i; k++)
           {
             m_Changes.push_back(Algorithm::Hirschberg::ChangeTypeInsert);
             mainWindow.InsertAddedText(m_file,  m_s2[m_a + k].m_text.c_str());
             mainWindow.InsertAddedText(m_file,  L"\n");
           }
          
          m_a += i;//k;

          m_deleted.clear();
        }
        break;
      case Algorithm::Myers::EqualAB:
        {
          PrintDeletedLines();

          int k = 0;
          for ( ; k < i; k++)
          {
            const Lexemes & s1 = m_s1[m_b + k].Lexemes();
            const Lexemes & s2 = m_s2[m_a + k].Lexemes();

            if (m_s2[m_a + k].m_pszSetEntry != m_s1[m_b + k].m_pszSetEntry)
            { 
              m_Changes.push_back(Algorithm::Hirschberg::ChangeTypeChange);

              CountAndKeepChanges f;
              Algorithm::Hirschberg::Hirschberg(s1, s2, f, std::not_equal_to<Lexeme>());

              PrintWords p(mainWindow, m_file, s1, s2);
              CallOut(f.m_Changes, p);
              mainWindow.InsertSameText(m_file,  L"\n");
            }
            else
            {
              m_Changes.push_back(Algorithm::Hirschberg::ChangeTypeNone);
              mainWindow.InsertSameText(m_file,   m_s2[m_a + k].m_text.c_str());
              mainWindow.InsertSameText(m_file,  L"\n");
            }
          }
          m_a += k;
          m_b += k;
          m_deleted.clear();
        }
        break;
      }
    }
};

static void OrderedInsert(std::vector<int>& v, int val)
{
  for (int i = v.size() -1 ; i >= 0; i--)
  {
    if (val > v[i])
    {                  
      v.insert(v.begin() + i + 1, val);
      return;
    }
  }  
  
  //insere bem na frente
  v.insert(v.begin(), val);  
}

struct MyersPrintLines
{
    unsigned int Count;    
    TextLines &m_s1;
    TextLines &m_s2;
    int m_a;
    int m_b;
    int lineFinal;
    bool m_onlyNew;
    MemoOfChanges m_Changes;
    HWND m_file;
    std::vector<int> m_deleted;
    std::vector<int> m_inserted;
    Algorithm::Myers::Operation m_Previous;

    MainWindow& mainWindow;

    void PrintDeletedAndInserted()
    {
      if (m_deleted.size() > 0)
      {
        for (int k = 0; k < m_deleted.size(); k++)
        {
          m_Changes.push_back(Algorithm::Hirschberg::ChangeTypeDelete);
          mainWindow.InsertDeletedText(m_file, m_s1[m_deleted[k]].m_text.c_str()); 
          mainWindow.InsertDeletedText(m_file, L"\n"); 
          
          //se quiser apenas 1 linha
          /*m_Changes.push_back(Algorithm::Hirschberg::ChangeTypeDelete);
          mainWindow.InsertDeletedText2(m_file);           
          break;*/

        }
        

        m_deleted.clear();

      }

      if (m_inserted.size() > 0)
      {
        //imprime direto
        for ( int k = 0; k < m_inserted.size(); k++)
        {
          m_Changes.push_back(Algorithm::Hirschberg::ChangeTypeInsert);
          mainWindow.InsertAddedText(m_file,  m_s2[m_inserted[k]].m_text.c_str());
          mainWindow.InsertAddedText(m_file,  L"\n");             
        }
        m_inserted.clear();  
      }
    }

    void PrintRemaning()
    {
      
        //faz uma comparacao mais flexivel agora entre as linha deletadas e incluidas
          // para fazer o merge entre elas
          if (s_DoMergeInLines && m_deleted.size() > 0 && m_inserted.size() > 0)
          {
          MyersPrintLines2 myersPrintLines(mainWindow, 
                                           m_file,
                                           m_s1, 
                                           m_s2,
                                           m_inserted.front(), 
                                           m_deleted.front());

          Algorithm::Myers::MyersLCS(m_s1.begin() + m_deleted.front(),
                                     m_s1.begin() + m_deleted.front() + m_deleted.size() ,
                                     m_s2.begin() + m_inserted.front(), 
                                     m_s2.begin() + m_inserted.front() + m_inserted.size(), 
                                     IsTheSameLineFlexibe,
                                     myersPrintLines);//, std::not_equal_to<LineOfText>());
          
          //tem que completar antes de fazer insert no changes
          myersPrintLines.PrintRemaning();

          //tranfere as trocas
          m_Changes.insert(m_Changes.end(),
                          myersPrintLines.m_Changes.begin(), 
                           myersPrintLines.m_Changes.end());
          m_inserted.clear();  
          m_deleted.clear();
          }
          
          PrintDeletedAndInserted();
    }

    MyersPrintLines(MainWindow& mw, HWND edit, TextLines &ss1, TextLines &ss2)
        : Count(0),  m_s1(ss1), m_s2(ss2), mainWindow(mw)
    {
        m_Previous = Algorithm::Myers::EqualAB;
        m_file = edit;
        m_a = 0;
        m_b = 0;
        lineFinal = 0;
        m_onlyNew = false;
    }

    void PrintDeletedLines(int start = 0)
    {
      //coloca os deletados
      for (int k = start; k < m_deleted.size(); k++)
      {
        m_Changes.push_back(Algorithm::Hirschberg::ChangeTypeDelete);
        mainWindow.InsertDeletedText(m_file, m_s1[m_deleted[k]].m_text.c_str()); 
        mainWindow.InsertDeletedText(m_file, L"\n"); 
      }
      m_deleted.clear();
    }

    ~MyersPrintLines()
    {
      PrintRemaning();
    }

    void operator()(Algorithm::Myers::Operation op, size_t i)
    {
      switch (op)
      {
      case Algorithm::Myers::RemoveFromB:
        {
          if (m_Previous != Algorithm::Myers::RemoveFromB)
          {
            PrintRemaning();
          }

          int k = 0;
          for ( ; k < i; k++)
          {
            m_deleted.push_back(m_b + k);
          }
          m_b += k;
        }

        break;
      case Algorithm::Myers::InsertFromA:
        {          
          for (int k = 0; k < i; k++)
          {              
            m_inserted.push_back(m_a + k);              
          }
          m_a += i;//k;
        }
        break;
      case Algorithm::Myers::EqualAB:
        {
          if (i == 1)
         {
           
             //pode acontecer de 1 linha igual
              // quebrar um bom bloco de merge, 
              // entao se for 1 so eu vou juntar mais.
              // para isso adicinar esta linha em cada bloco

             //enfia a linha na posicao correta
             OrderedInsert(m_deleted, m_b);
             OrderedInsert(m_inserted, m_a);

              m_a += 1;
              m_b += 1;
          }
          else
          
          {
            PrintRemaning();

            int k = 0;
            for ( ; k < i; k++)
            {
              const Lexemes & s1 = m_s1[m_b + k].Lexemes();
              const Lexemes & s2 = m_s2[m_a + k].Lexemes();

              if (m_s2[m_a + k].m_pszSetEntry != m_s1[m_b + k].m_pszSetEntry)
              { 
                m_Changes.push_back(Algorithm::Hirschberg::ChangeTypeChange);

                CountAndKeepChanges f;
                Algorithm::Hirschberg::Hirschberg(s1, s2, f, std::not_equal_to<Lexeme>());

                PrintWords p(mainWindow, m_file, s1, s2);
                CallOut(f.m_Changes, p);
                mainWindow.InsertSameText(m_file,  L"\n");
              }
              else
              {
                m_Changes.push_back(Algorithm::Hirschberg::ChangeTypeNone);
                mainWindow.InsertSameText(m_file,   m_s2[m_a + k].m_text.c_str());
                mainWindow.InsertSameText(m_file,  L"\n");
              }
            }
            m_a += k;
            m_b += k;
          }
         
        }
        break;
      }
      m_Previous = op;
    }
};



bool IsUtf8(const std::wstring &filename)
{  
  std::wifstream f(filename.c_str(), std::ios::binary);    
  if (!f)
  {
    return false; //throw std::exception("File not found");
  }
  wchar_t ch0, ch1, ch2;
  f.read(&ch0, 1);
  f.read(&ch1, 1);
  f.read(&ch2, 1);
  
  //acho que o header utf8 eh este
  //EF BB BF
  return ch0 == 0xEF && ch1 == 0xBB && ch2 == 0xBF;
}



//Otimizacao importante
//Carrega cada linha adicionando no mapa se ela nao esta lah
//depois passa o ponteiro da string para o vector.
static void Load(SetOfStrings &map,
                 const std::wstring &filename,
                 TextLines &v)
{

     //leitura utf8
    std::locale ulocale(locale(), new codecvt_utf8<wchar_t,0x10ffff, std::consume_header>) ;
    std::wifstream file;
    if (IsUtf8(filename))
    {
      file.imbue(ulocale);    
    }
  
    file.open(filename.c_str());
    if (!file)
    {
        throw std::exception("File not found");
    }

    std::wstring str;
    while (std::getline(file, str))
    {
        v.push_back(LineOfText(map.insert(str).first->c_str()));
        v.back().m_text.swap(str);

        //v.push_back(LineOfText(NULL, str));        
    }
}


void MainWindow::Run()
{
    SendMessage( m_hRichEdit, WM_SETREDRAW, (WPARAM) FALSE, (LPARAM) 0);

    SetCursor(LoadCursor(NULL, IDC_WAIT));
    
    std::wstring str = L"TDiff - ";
    str += m_NewFileName;
    SetWindowText(m_hWnd, str.c_str());
    try
    {

        Stopwatch stopwatch(true);
        std::wstring file1 = m_OldFileName;
        std::wstring file2 = m_NewFileName;        

        m_Changes.clear();

        //Transfere todas as strings para um mapa
        //e todos os ponteiros para os vectors. 
        //A comparação depois direto no ponteiro será mais rápida.

        SetOfStrings map;

        //vou comecar pelo 2 que eh mais novo eh oq fica no set.        
        TextLines s2;
        s2.reserve(1000); //linhas reservadas        
        Load(map, file2, s2);
       
        TextLines s1;
        s1.reserve(s2.size() > 1000 ? s2.size() : 1000); //minimo 1000 reservado
        Load(map, file1, s1);

        RichEdit_ClearAll(m_hRichEdit);
#ifdef H
        //Output do algoritmo
        PrintLines p(m_hRichEdit, s1, s2);
        Hirschberg(s1, s2, p, std::not_equal_to<LineOfText>());
        m_Changes.swap(p.m_Changes);
#else
        {
          MyersPrintLines myersPrintLines(*this, m_hRichEdit, s1, s2);
          Algorithm::Myers::MyersDiff(s1, s2, IsTheSameLine, myersPrintLines);//, std::not_equal_to<LineOfText>());

          //tem que transferir antes de fazer o swap
          myersPrintLines.PrintRemaning();

          m_Changes.swap(myersPrintLines.m_Changes);
        }
#endif

        std::wstringstream ss;
        ss << L"(" << stopwatch.GetElapsedMilliseconds() << L" ms) \r\n";

        InsertText(m_hRichEdit,ss.str().c_str(), RGB(0,128,0) );

    }
    catch (const std::exception &e)
    {
        MessageBoxA(m_hWnd, e.what(),"Error", MB_ICONEXCLAMATION | MB_OK);
    }
    SendMessage( m_hRichEdit, WM_SETREDRAW, (WPARAM) TRUE, (LPARAM) 0);
    RedrawWindow(m_hRichEdit, NULL, NULL, RDW_ERASE | RDW_FRAME | RDW_INVALIDATE | RDW_ALLCHILDREN);

    SetCursor(LoadCursor(NULL, IDC_ARROW));
}

void MainWindow::OnMinMax(MINMAXINFO* pMinMaxInfo)
{
    pMinMaxInfo->ptMinTrackSize.x = 500;
    pMinMaxInfo->ptMinTrackSize.y = 300;
}

BOOL MainWindow::OnSetCursor()
{
  if (m_PopupMenuActivated)
  {
    SetCursor(LoadCursor(NULL, IDC_ARROW));
    return TRUE;
  }
  return FALSE;
}

void MainWindow::OnPaint(HDC hdc)
{
    RECT rc;
    GetClientRect(m_hWnd, &rc);

    FillSolidRect(hdc, &rc, GetSysColor(COLOR_3DFACE));//RGB(188,199,216));//GetSysColor(COLOR_3DFACE));

    RECT rc2 = rc;
    rc2.bottom = 50;
    GetClientRect(m_hWnd, &rc); //GetSysColor(COLOR_3DFACE)  //RGB(220, 231, 245)
    GradientRectV(hdc, rc2,  GetSysColor(COLOR_3DFACE) , RGB(255,255,255));
    //Gdi::Pen pen(RGB(0,0,0));
    //Gdi::ScopeSelect s(hdc, pen.m_hPen);
    //::MoveToEx(hdc, 0, 49, 0);
    //::LineTo(hdc, rc.right, 49);
}


void MainWindow::ChangeFont(LPCTSTR fontName, int size)
{
    DeleteObject(m_hFont);
    m_hFont = NULL;
    m_LogFont.lfHeight=-g_metrics.PointsToPixels(size);
    m_LogFont.lfWidth=0;
    m_LogFont.lfEscapement=0;
    m_LogFont.lfOrientation = 0;
    m_LogFont.lfWeight = 400;
    m_LogFont.lfItalic = 0;
    m_LogFont.lfUnderline = 0;
    m_LogFont.lfStrikeOut = 0;
    m_LogFont.lfCharSet = 0;
    m_LogFont.lfOutPrecision = 3;
    m_LogFont.lfClipPrecision = 2;
    m_LogFont.lfQuality = 1;
    m_LogFont.lfPitchAndFamily = 49;
    wcscpy(m_LogFont.lfFaceName , fontName);
    m_hFont = CreateFontIndirect(&m_LogFont);
}

void MainWindow::OnCreate()
{
    m_FindDlg.m_pIFindNextEvent = this;

    ChangeFont(m_FontName.c_str(), m_FontSize);

    SetWindowText(m_hWnd, TDIFF_CAPTION);
    HINSTANCE hInst = GetModuleHandle(0);

    m_hRichEdit = CreateWindowEx(0, RICHEDIT_CLASS, 0, ES_READONLY | WS_VISIBLE | WS_CHILD | WS_TABSTOP |  ES_AUTOHSCROLL | ES_MULTILINE | ES_WANTRETURN | ES_AUTOVSCROLL | WS_VSCROLL | WS_HSCROLL | ES_NOHIDESEL, 82, 144, 232, 158, m_hWnd, (HMENU)IDC_RICHEDIT1, hInst, 0);

    RichEdit_SetEventMask(m_hRichEdit, ENM_MOUSEEVENTS);
    SendMessage(m_hRichEdit, WM_SETFONT, (WPARAM)m_hFont, FALSE);

    RichEdit_SetBackgroundColor(m_hRichEdit, 0, m_BackgroundColor);

    LONG lfHeight = -13;//-g_metrics.PointsToPixels(8); 
    HFONT hfont0 = CreateFont(lfHeight, 0, 0, 0, 400, FALSE, FALSE, FALSE, 1, 400, 0, 0, 0, (L"Ms Shell Dlg 2"));
    

    RECT rc1 = {12, 10, 12 + 88, 10 + 28};
    //g_metrics.ScaleRect(&rc1);
    m_hFirst = CreateWindowEx(0, WC_BUTTON, (L"|< First"), WS_DISABLED | WS_VISIBLE | WS_CHILD | WS_TABSTOP, rc1.left, rc1.top, rc1.right  - rc1.left, rc1.bottom - rc1.top, m_hWnd, (HMENU)IDC_FIRST, hInst, 0);
    SendMessage(m_hFirst, WM_SETFONT, (WPARAM)hfont0, FALSE);

    RECT rc2 = {103, 10, 103 + 88, 10 + 28};
    //g_metrics.ScaleRect(&rc2);
    m_hPrev = CreateWindowEx(0, WC_BUTTON, (L"< Previous"), WS_DISABLED | WS_VISIBLE | WS_CHILD | WS_TABSTOP,  rc2.left, rc2.top, rc2.right  - rc2.left, rc2.bottom - rc2.top, m_hWnd, (HMENU)IDC_PREVIOUS, hInst, 0);
    SendMessage(m_hPrev, WM_SETFONT, (WPARAM)hfont0, FALSE);

    RECT rc3 = {194, 10, 194 + 88, 10 + 28};
    //g_metrics.ScaleRect(&rc3);
    m_hNext = CreateWindowEx(0, WC_BUTTON, (L"Next >"), WS_DISABLED | WS_VISIBLE | WS_CHILD | WS_TABSTOP,  rc3.left, rc3.top, rc3.right  - rc3.left, rc3.bottom - rc3.top, m_hWnd, (HMENU)IDC_NEXT, hInst, 0);
    SendMessage(m_hNext, WM_SETFONT, (WPARAM)hfont0, FALSE);

    RECT rc4 = {285, 10, 285 + 88, 10 + 28};
    //g_metrics.ScaleRect(&rc4);
    m_hLast = CreateWindowEx(0, WC_BUTTON, (L"Last >|"), WS_DISABLED | WS_VISIBLE | WS_CHILD | WS_TABSTOP,  rc4.left, rc4.top, rc4.right  - rc4.left, rc4.bottom - rc4.top, m_hWnd, (HMENU)IDC_LAST, hInst, 0);
    SendMessage(m_hLast, WM_SETFONT, (WPARAM)hfont0, FALSE);

    m_hExitButton = CreateWindowEx(0, WC_BUTTON, (L"Exit"), WS_VISIBLE | WS_CHILD | WS_TABSTOP, 473,14,79,20, m_hWnd, (HMENU)IDC_EXIT, hInst, 0);
    SendMessage(m_hExitButton, WM_SETFONT, (WPARAM)hfont0, FALSE);


    if (!m_OldFileName.empty() && !m_NewFileName.empty())
    {
        Run();
        SetFocus(m_hRichEdit);
        RichEdit_SetSel(m_hRichEdit,0 ,0);
        Next();
    }
}

void MainWindow::OnDeactivate()
{
    //m_FindDlg.Hide();
}

void MainWindow::OnSize(UINT type, int w, int h)
{
    if (type == SIZE_MINIMIZED)
    {
        m_FindDlg.Hide();
        return;
    }

    int l = 5;
    int t = 50;
    RECT rc1 = {w - 88 - 11, 10, (w - 88 - 11) + 88, 10 + 28};
    //g_metrics.ScaleRect(&rc1);
    int W = rc1.right-rc1.left;
    rc1.left = w - W - 11;
    rc1.right = rc1.left + W;

    MoveWindow(m_hExitButton, rc1.left, rc1.top, rc1.right-rc1.left,rc1.bottom-rc1.top, TRUE);

    RECT rc2 = {l, t,  w-5, t + h-t-5};
    //g_metrics.ScaleRect(&rc2);

    MoveWindow(m_hRichEdit, rc2.left, rc2.top, rc2.right-rc2.left,rc2.bottom-rc2.top,TRUE);  
}




/*
struct PrintLines
{
    unsigned int Count;    
    TextLines &s1;
    TextLines &s2;
    int line1;
    int line2;
    int lineFinal;
    bool m_onlyNew;
    MemoOfChanges m_Changes;
    HWND m_file;

    PrintLines(HWND edit,        
        TextLines &ss1,
        TextLines &ss2)
        : Count(0),  s1(ss1), s2(ss2)
    {
        m_file = edit;
        line1 = 0;
        line2 = 0;
        lineFinal = 0;
        m_onlyNew = false;
    }

    ~PrintLines()
    {
    }

    void operator()(Algorithm::Hirschberg::ChangeType e)
    {
        switch (e)
        {
        case Algorithm::Hirschberg::ChangeTypeNone: 
            {
                m_Changes.push_back(Algorithm::Hirschberg::ChangeTypeNone);

                //vou pegar da linha nova s2, pq o algoritmo de comparacao
                //pode usar trim, e as string ditas iguais, nao sao exatamente iguais.
                //se o compare considerasse outras aqui aqui teria q ser feito o diff
                //tal qual eh feito no caso de ser diferente

                InsertText(m_file, s2[line2].m_text.c_str() , m_SameColor);
                InsertText(m_file, _T(" \r\n") , m_SameColor);
                line1++;
                line2++;
                lineFinal++;
            }
            break;
        case Algorithm::Hirschberg::ChangeTypeInsert:
            {
                m_Changes.push_back(Algorithm::Hirschberg::ChangeTypeInsert);

                InsertAddedText(m_file, s2[line2++].m_text.c_str());
                InsertAddedText(m_file, _T(" \r\n"));
                lineFinal++;
            }

            break;
        case Algorithm::Hirschberg::ChangeTypeDelete:
            {
                m_Changes.push_back(Algorithm::Hirschberg::ChangeTypeDelete);

                if (!m_onlyNew)
                {
                    InsertDeletedText(m_file, s1[line1++].m_text.c_str());
                    InsertDeletedText(m_file, _T(" \r\n"));
                    lineFinal++;
                }
            }
            break;
        case Algorithm::Hirschberg::ChangeTypeChange: 
            {
                Lexemes v1;
                Fill(s1[line1].m_text, v1);

                Lexemes v2;
                Fill(s2[line2].m_text, v2);

                CountAndKeepChanges f;
                Algorithm::Hirschberg::Hirschberg(v1, v2, f, std::not_equal_to<Lexeme>());

                bool useone = false;
                if (f.changed_count == 0)
                {
                    //so adicionado
                    //so deletado

                    //adicionado e deletado
                    useone = true;
                }
                else if (f.changed_count == 1)
                {
                    //so mudou 1 caractere                        
                    useone = true;
                }
                else
                {
                    //regra de antes da versao ate 1.3
                    useone =  f.same_count > f.deleted_count + f.added_count + f.changed_count;
                }

                if (useone || m_onlyNew)
                {
                    m_Changes.push_back(Algorithm::Hirschberg::ChangeTypeChange);

                    PrintWords p(m_file, v1,v2);
                    CallOut(f.m_Changes, p);                                       
                    InsertText(m_file, L" \r\n", m_SameColor);
                    lineFinal++;
                }
                else
                {
                    if (!m_onlyNew)
                    {
                        m_Changes.push_back(Algorithm::Hirschberg::ChangeTypeDelete);
                        InsertDeletedText(m_file, s1[line1].m_text.c_str());
                        InsertDeletedText(m_file, _T(" \r\n"));
                    }

                    m_Changes.push_back(Algorithm::Hirschberg::ChangeTypeInsert);
                    lineFinal++;
                    InsertAddedText(m_file,  s2[line2].m_text.c_str());
                    InsertAddedText(m_file, _T(" \r\n"));
                    lineFinal++;
                }
            }

            line1++;
            line2++;

            break;
        }
    }
};
*/

void MainWindow::OnIdle()
{
    HMENU hMenu = GetMenu(m_hWnd);
    HMENU hMenuEdit = GetSubMenu(hMenu, 1);
    
    int lineCount = RichEdit_GetLineCount(m_hRichEdit);
    int firstLineCount = RichEdit_LineLength(m_hRichEdit, 0);
    BOOL emptyRichEdit = lineCount == 1 && firstLineCount == 0;

    long start, end;
    RichEdit_GetSel(m_hRichEdit, start, end);

    EnableMenuItem(hMenuEdit, ID_EDIT_COPY, end != start ? MF_ENABLED : MF_GRAYED );

    EnableMenuItem(hMenuEdit, ID_EDIT_FIND, emptyRichEdit ? MF_GRAYED : MF_ENABLED );
    EnableMenuItem(hMenuEdit, ID_EDIT_FINDNEXT, ( !emptyRichEdit && !m_FindDlg.m_TextToFind.empty()) ? MF_ENABLED : MF_GRAYED );
    BOOL bHasChanges = !m_Changes.empty();
    EnableMenuItem(hMenuEdit, ID_EDIT_MOVEFIRST, bHasChanges ? MF_ENABLED : MF_GRAYED );
    EnableMenuItem(hMenuEdit, ID_EDIT_MOVENEXT, bHasChanges ? MF_ENABLED : MF_GRAYED );
    EnableMenuItem(hMenuEdit, ID_EDIT_MOVEPREVIOUS, bHasChanges ? MF_ENABLED : MF_GRAYED );
    EnableMenuItem(hMenuEdit, ID_EDIT_MOVELAST, bHasChanges ? MF_ENABLED : MF_GRAYED );
    
    //botoes da tela
    EnableWindow(m_hFirst, bHasChanges);
    EnableWindow(m_hNext, bHasChanges);
    EnableWindow(m_hPrev, bHasChanges);
    EnableWindow(m_hLast, bHasChanges);
    
    HMENU hMenuFile = GetSubMenu(hMenu, 0);
    EnableMenuItem(hMenuFile, ID_FILE_REFRESH, !m_Changes.empty() ? MF_ENABLED : MF_GRAYED );

}

