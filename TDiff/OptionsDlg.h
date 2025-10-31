#pragma once

#include "win32express.h"
#include "resource.h"

#include <vector>
#include <string>

void ColorCombo_SetColor(HWND hWnd, COLORREF cr)
{
  const int count = ComboBox_GetCount(hWnd);
  for (int i = 0 ; i < count; i++)
  {
    if (ComboBox_GetItemData(hWnd, i) == cr)
    {
      ComboBox_SetCurSel(hWnd, i);
      return;
    }
  }
  
  //custon
  ComboBox_SetItemData(hWnd, count - 1, cr);
  ComboBox_SetCurSel(hWnd, count - 1);
}

COLORREF ColorCombo_GetColor(HWND hWnd)
{
  const int sel = ComboBox_GetCurSel(hWnd);
  return ComboBox_GetItemData(hWnd, sel);
}


inline LPCTSTR GetColorName(COLORREF cr)
{
  switch (cr)
  {
  case RGB( 0xFF, 0xFF, 0xFF ): return _T("White");
  case RGB( 0x00, 0xFF, 0xFF ): return _T("Aqua");
  case RGB( 0xFF, 0x00, 0xFF ): return _T("Pink");
  case RGB( 0x00, 0x00, 0xFF ): return _T("Blue");
  case RGB( 0xFF, 0xFF, 0x00 ): return _T("Yellow");
  case RGB( 0x00, 0xFF, 0x00 ): return _T("Lime");
  case RGB( 0xFF, 0x00, 0x00 ): return _T("Red");
  case RGB( 0xC0, 0xC0, 0xC0 ): return _T("Silver");
  case RGB( 0x80, 0x80, 0x80 ): return _T("Gray") ;
  case RGB( 0x00, 0x80, 0x80 ): return _T("Emerald");
  case RGB( 0x80, 0x00, 0x80 ): return _T("Purple");
  case RGB( 0x00, 0x00, 0x80 ): return _T("Navy blue");
  case RGB( 0x80, 0x80, 0x00 ): return _T("Olive");
  case RGB( 0x00, 0x80, 0x00 ): return _T("Green");
  case RGB( 0x80, 0x00, 0x00 ): return _T("Brown");
  case RGB( 0x00, 0x00, 0x00 ): return _T("Black");
  default:
    break;
  }
  return _T("Custom") ; 
}
/*
inline LRESULT  CALLBACK  ComboSubClass(
HWND hWnd,
UINT uMsg,
WPARAM wParam,
LPARAM lParam,
UINT_PTR uIdSubclass,
DWORD_PTR dwRefData)
{

return DefSubclassProc(hWnd,uMsg,wParam,lParam);
}
*/

inline void MakeColorCombo(HWND hWnd)
{
  // SetWindowSubclass (hWnd, &ComboSubClass, 1, 0);

  static COLORREF colors[] =
  {
    RGB( 0xFF, 0xFF, 0xFF ), //_T( "Branco" ) ),
    RGB( 0x00, 0xFF, 0xFF ), //_T( "Água" ) ),
    RGB( 0xFF, 0x00, 0xFF ), //_T( "Rosa" ) ),
    RGB( 0x00, 0x00, 0xFF ), //_T( "Azul" ) ),
    RGB( 0xFF, 0xFF, 0x00 ), //_T( "Amarelo" ) ),
    RGB( 0x00, 0xFF, 0x00 ), //_T( "Lima" ) ),
    RGB( 0xFF, 0x00, 0x00 ), //_T( "Vermelho" ) ),
    RGB( 0xC0, 0xC0, 0xC0 ), //_T( "Prata" ) ),
    RGB( 0x80, 0x80, 0x80 ), //_T( "Cinza" ) ),
    RGB( 0x00, 0x80, 0x80 ), //_T( "Esmeralda" ) ),
    RGB( 0x80, 0x00, 0x80 ), //_T( "Roxo" ) ),
    RGB( 0x00, 0x00, 0x80 ), //_T( "Marinho" ) ),
    RGB( 0x80, 0x80, 0x00 ), //_T( "Oliva" ) ),
    RGB( 0x00, 0x80, 0x00 ), //_T( "Verde" ) ),
    RGB( 0x80, 0x00, 0x00 ), //_T( "Marrom" ) ),
    RGB( 0x00, 0x00, 0x00 ), //_T( "Preto" ) ),
    RGB( 0x25, 0x8C, 0x68 ), //_T( "<Outra...>") ),
  };

  for (int i = 0 ; i < (sizeof(colors)/ sizeof(colors[0])); i++)
  {
    ComboBox_AddString(hWnd, L"teste");
    ComboBox_SetItemData(hWnd, i, colors[i]);
  }

  assert(!(GetStyle(hWnd) & CBS_SORT));
  assert(GetStyle(hWnd) & CBS_OWNERDRAWFIXED);  
  assert(GetStyle(hWnd) & CBS_DROPDOWNLIST);
  assert(GetStyle(hWnd) & CBS_HASSTRINGS);
}

inline void DrawColorCombo(LPDRAWITEMSTRUCT pDIStruct)
{
  HDC hDC =pDIStruct->hDC;
  COLORREF crColor = 0;
  COLORREF crNormal = GetSysColor(COLOR_WINDOW);
  COLORREF crSelected = GetSysColor(COLOR_HIGHLIGHT);
  COLORREF crText = GetSysColor(COLOR_WINDOWTEXT);

  HWND hItem = pDIStruct->hwndItem;

  std::wstring  strColor;
  RECT    rcItemRect = pDIStruct->rcItem;
  RECT    rcBlockRect = rcItemRect;
  RECT    rcTextRect = rcBlockRect;

  int      iFourthWidth = 0;
  int      iItem = pDIStruct->itemID;
  int      iState = pDIStruct->itemState;


  iFourthWidth = (rcBlockRect.right - rcBlockRect.left) / 4;     // Get 1/4 Of Item Area

  HBRUSH brFrameBrush = (HBRUSH)GetStockObject(BLACK_BRUSH); // Create Black Brush

  if (iState & ODS_SELECTED)
  { 
    // Set Selected Attributes
    SetTextColor(hDC, (0x00FFFFFF & ~(crText))); // Set Inverted Text Color (With Mask)
    SetBkColor(hDC, crSelected);                 // Set BG To Highlight Color
    FillSolidRect(hDC, &rcBlockRect, crSelected); // Erase Item
  }
  else
  { 
    // Set Standard Attributes
    SetTextColor(hDC, crText);                 // Set Text Color
    SetBkColor(hDC, crNormal);                 // Set BG Color
    FillSolidRect(hDC, &rcBlockRect, crNormal); // Erase Item
  }

  if (iState & ODS_FOCUS)
    DrawFocusRect(hDC, &rcItemRect); // Draw Focus Rect

  //
  //  Calculate Text Area...
  //
  SIZE sz;
  GetTextExtentPoint(hDC, _T("I"), 1, &sz);
  rcTextRect.left += (iFourthWidth + 2); // Set Start Of Text
  //vertical center
  rcTextRect.top += ((rcTextRect.bottom - rcTextRect.top) - sz.cy) / 2;                   // Offset A Bit

  //
  //  Calculate Color Block Area..
  //
  InflateRect(&rcBlockRect, -2, -2); // Reduce Color Block Size
  rcBlockRect.right = iFourthWidth;     // Set Width Of Color Block

  //
  //  Draw Color Text And Block...
  //
  if (iItem != -1)
  {
    crColor = ComboBox_GetItemData(hItem, iItem);
    //GetLBText(iItem, strColor);
    strColor = GetColorName(crColor);

    int iTabStops = 50;

    if (iState & ODS_DISABLED)
    {
      crColor = ::GetSysColor(COLOR_GRAYTEXT); // Get Inactive Text Color
      SetTextColor(hDC, crColor);         // Set Text Color
    }
    else
    {
      crColor = ComboBox_GetItemData(hItem, iItem);//RGB(255,0,0);//GetItemData(iItem); // Get Color Value
    }
    SetBkMode(hDC, TRANSPARENT);
    TabbedTextOut(hDC, rcTextRect.left, rcTextRect.top, strColor.c_str(), strColor.size(), 1, &iTabStops, 0); // Draw Color Name
    FillSolidRect(hDC, &rcBlockRect, crColor);   // Draw Color
    FrameRect(hDC, &rcBlockRect, brFrameBrush); // Draw Frame
  }
}

inline int CALLBACK EnumFontFamExProc(ENUMLOGFONTEX *lpelfe, NEWTEXTMETRICEX *, int, LPARAM lParam) 
{
  std::vector<std::wstring> *pStrVect = (std::vector<std::wstring> *)lParam;
  size_t vectSize = pStrVect->size();

  //Search through all the fonts, EnumFontFamiliesEx never states anything about order
  //Start at the end though, that's the most likely place to find a duplicate
  for(int i = vectSize - 1 ; i >= 0 ; i--) 
  {
    if ( !lstrcmp((*pStrVect)[i].c_str(), (const TCHAR*)lpelfe->elfLogFont.lfFaceName) )
      return 1;       //we already have seen this typeface, ignore it
  }

  //We can add the font
  //Add the face name and not the full name, we do not care about any styles
  pStrVect->push_back((TCHAR *)lpelfe->elfLogFont.lfFaceName);
  return 1; // I want to get all fonts
}

void EnumerateFonts(std::vector<std::wstring> & fontlist, HWND hWnd)
{
  LOGFONT lf;
  fontlist.clear();
  //fontlist.push_back(TEXT(""));

  lf.lfCharSet = DEFAULT_CHARSET;
  lf.lfFaceName[0]='\0';
  lf.lfPitchAndFamily = 0;
  HDC hDC = ::GetDC(hWnd);
  ::EnumFontFamiliesEx(hDC, &lf, (FONTENUMPROC) EnumFontFamExProc, (LPARAM) &fontlist, 0);
}

class OptionsDlg : public Dialog<OptionsDlg, IDD_OPTIONS>
{
  HFONT m_hFont;
  LOGFONT m_LogFont;
  


  void ChangeFont(LPCTSTR fontName, int size)
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


public:
    COLORREF m_AddedColor;
  COLORREF m_DeletedColor;
  COLORREF m_SameColor;
  COLORREF m_BackgroundColor;

  std::wstring m_FontName;
  int m_FontSize;
  OptionsDlg(HWND hParent = NULL) : Dialog<OptionsDlg, IDD_OPTIONS>(hParent)
  {
    m_hFont = NULL;
  }

  ~OptionsDlg()
  {
    DeleteObject(m_hFont);
  }

  void Preview()
  {
    HWND hWnd = GetDlgItem(IDC_RICHEDIT21);
    
    RichEdit_SetBackgroundColor(hWnd, 0, m_BackgroundColor);
    Window_SetFont(hWnd, m_hFont);
    RichEdit_ClearAll(hWnd);
    RichEdit_InsertText(hWnd, _T("The "), m_SameColor, FALSE, FALSE);
    RichEdit_InsertText(hWnd, _T("deleted "), m_DeletedColor, FALSE, TRUE);
    RichEdit_InsertText(hWnd, _T("added "), m_AddedColor, TRUE, FALSE);
    RichEdit_InsertText(hWnd, _T("text"), m_SameColor, FALSE, FALSE);    
  }


  bool OnDrawItem( int id,  LPDRAWITEMSTRUCT pDIStruct)
  {
    if (id == IDC_ADDEDCOLOR || id == IDC_DELETEDCOLOR || id == IDC_SAMECOLOR || id == IDC_BACKCOLOR)
    {
      DrawColorCombo(pDIStruct);
      return true;
    }
    return false;
  }

  void InitDialog()
  {
    MakeColorCombo(GetDlgItem(IDC_ADDEDCOLOR));
    MakeColorCombo(GetDlgItem(IDC_DELETEDCOLOR));
    MakeColorCombo(GetDlgItem(IDC_SAMECOLOR));
    MakeColorCombo(GetDlgItem(IDC_BACKCOLOR));

    ColorCombo_SetColor(GetDlgItem(IDC_ADDEDCOLOR), m_AddedColor);
    ColorCombo_SetColor(GetDlgItem(IDC_DELETEDCOLOR), m_DeletedColor);
    ColorCombo_SetColor(GetDlgItem(IDC_SAMECOLOR), m_SameColor);
    ColorCombo_SetColor(GetDlgItem(IDC_BACKCOLOR), m_BackgroundColor);

    SetFocus(GetDlgItem(IDC_FONTS));

    HWND hFontSize = GetDlgItem(IDC_FONTSIZE);
    ComboBox_AddString(hFontSize, L"8");
    ComboBox_AddString(hFontSize, L"9");
    ComboBox_AddString(hFontSize, L"10");
    ComboBox_AddString(hFontSize, L"11");
    ComboBox_AddString(hFontSize, L"12");
    ComboBox_AddString(hFontSize, L"14");
    ComboBox_AddString(hFontSize, L"15");
    ComboBox_AddString(hFontSize, L"16");


    std::vector<std::wstring>  fontlist;
    EnumerateFonts(fontlist, m_hDlg);
    HWND hFonts = GetDlgItem(IDC_FONTS);
    for (size_t i = 0 ; i < fontlist.size(); i++)
    {
      ComboBox_AddString(hFonts, fontlist[i].c_str());
    }

    SetWindowText(GetDlgItem(IDC_FONTS), m_FontName.c_str());

    WCHAR buff[100];
    _itow(m_FontSize, buff, 10);
    SetWindowText(GetDlgItem(IDC_FONTSIZE), buff);

    ChangeFont(m_FontName.c_str(), m_FontSize);

    Preview();
  }

  void DialogToValues()
  {
    GetDlgItemText(IDC_FONTS, m_FontName);
    std::wstring fontsize;
    GetDlgItemText(IDC_FONTSIZE, fontsize);
    m_FontSize = _wtoi(fontsize.c_str());

    m_AddedColor = ColorCombo_GetColor(GetDlgItem(IDC_ADDEDCOLOR));
    m_DeletedColor = ColorCombo_GetColor(GetDlgItem(IDC_DELETEDCOLOR));
    m_SameColor = ColorCombo_GetColor(GetDlgItem(IDC_SAMECOLOR));
    m_BackgroundColor = ColorCombo_GetColor(GetDlgItem(IDC_BACKCOLOR));
  }

  void OnCommand(int hwp, int lwp, HWND)
  {
    if (hwp == IDOK || hwp == IDCANCEL)
    {            
      DialogToValues();
      EndDialog(hwp);
    }
    else if (hwp == IDC_BUTTON1 || 
             hwp == IDC_BUTTON3 ||
             hwp == IDC_BUTTON4 ||
             hwp == IDC_BUTTON5)
    {
      ColorDlg dlg;
      
      if (hwp == IDC_BUTTON1)
        dlg.rgbCurrent = m_AddedColor;
      else if (hwp == IDC_BUTTON3)
        dlg.rgbCurrent = m_DeletedColor;
      else if (hwp == IDC_BUTTON4)
        dlg.rgbCurrent = m_SameColor;
      else if (hwp == IDC_BUTTON5)
        dlg.rgbCurrent =m_BackgroundColor;

      if (dlg.ShowDialog(m_hDlg) == IDOK)
      {
        if (hwp == IDC_BUTTON1)
          ColorCombo_SetColor(GetDlgItem(IDC_ADDEDCOLOR), dlg.rgbCurrent);
        else if (hwp == IDC_BUTTON3)
          ColorCombo_SetColor(GetDlgItem(IDC_DELETEDCOLOR), dlg.rgbCurrent);
        else if (hwp == IDC_BUTTON4)
          ColorCombo_SetColor(GetDlgItem(IDC_SAMECOLOR), dlg.rgbCurrent);
        else if (hwp == IDC_BUTTON5)
          ColorCombo_SetColor(GetDlgItem(IDC_BACKCOLOR), dlg.rgbCurrent);

       DialogToValues();       
       Preview();
      }
    }
    else if (hwp == IDC_FONTS || 
             hwp == IDC_FONTSIZE ||
             hwp == IDC_ADDEDCOLOR ||
             hwp == IDC_DELETEDCOLOR ||
             hwp == IDC_SAMECOLOR ||
             hwp == IDC_BACKCOLOR)
    {
      if (lwp == CBN_SELCHANGE || lwp == CBN_EDITCHANGE)
      {
       DialogToValues();
       ChangeFont(m_FontName.c_str(), m_FontSize);
       Preview();
      }
    }
  }
};


