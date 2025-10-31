#pragma once
#include "win32express.h"
#include "resource.h"

class AboutDlg : public Dialog<AboutDlg, IDD_ABOUTBOX>
{
public:
    AboutDlg(HWND hParent = NULL) : Dialog<AboutDlg, IDD_ABOUTBOX>(hParent)
    {
    }

    void InitDialog()
    {
      SetFocus(m_hDlg);
    }

    void OnCommand(int hwp, int lwp, HWND)
    {
        if (hwp == IDOK || hwp == IDCANCEL)
        {            
            EndDialog(hwp);
        }
    }
};