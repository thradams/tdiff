#pragma once
#include "win32express.h"
#include "resource.h"


class ChooseDlg : public Dialog<ChooseDlg, IDD_CHOOSEFILE>
{

public:
    std::wstring m_file1;
    std::wstring m_file2;

    ChooseDlg(HWND hParent = NULL) : Dialog<ChooseDlg, IDD_CHOOSEFILE>(hParent)
    {
    }

    void InitDialog()
    {
        SetDlgItemText(m_hDlg, IDC_EDIT1,  m_file1.c_str());
        SetDlgItemText(m_hDlg, IDC_EDIT2,  m_file2.c_str());

        SetFocus(GetDlgItem(IDC_EDIT1));
        Edit_SelectAll(GetDlgItem(IDC_EDIT1));
    }

    void OnCommand(int hwp, int lwp, HWND)
    {
        if (hwp == IDOK || hwp == IDCANCEL)
        {            
            GetDlgItemText(IDC_EDIT1,  m_file1);
            GetDlgItemText(IDC_EDIT2,  m_file2);
            EndDialog(hwp);
        }
        else if(hwp == IDC_BUTTON1)
        {
            OpenDialog dlg;//(m_hDlg);
            if (dlg.ShowDialog(m_file1, m_hDlg) == IDOK) 
            {
                SetDlgItemText(m_hDlg , IDC_EDIT1, m_file1.c_str());
            }
        }
        else if (hwp == IDC_BUTTON2)
        {
            OpenDialog dlg;
            if (dlg.ShowDialog(m_file2, m_hDlg) == IDOK) 
            {
                SetDlgItemText(m_hDlg , IDC_EDIT2, m_file2.c_str());
            }
        }
        else if (hwp == IDC_EDIT1 || hwp == IDC_EDIT2)
        {
          if (lwp == EN_CHANGE)
          {
              std::wstring s1;
              std::wstring s2;
              GetDlgItemText(IDC_EDIT1, s1);
              GetDlgItemText(IDC_EDIT2, s2);
              EnableWindow(GetDlgItem(IDOK), !s1.empty() && !s2.empty());
          }
        }
    }
};
