#pragma once
#include "win32express.h"
#include "resource.h"

struct IFindNextEvent
{
    virtual void OnFindNextEvent(const std::wstring& s, 
        bool matchcase, 
        bool matchword,
        bool searchUp)

    {
    }
};

class FindDlg: public Dialog<FindDlg, IDD_FIND>
{
public:
    IFindNextEvent* m_pIFindNextEvent;

    std::wstring m_TextToFind;
    BOOL m_MatchCase;
    BOOL m_MatchWholeWord;
    BOOL m_SearchUp;

    FindDlg(HWND hParent = NULL) : Dialog<FindDlg, IDD_FIND>(hParent)
    {
        m_pIFindNextEvent = 0;
    }

    void InitDialog()
    {
        
        //SetDlgItemText(m_hDlg, IDC_EDIT1,  m_file1.c_str());
        //SetDlgItemText(m_hDlg, IDC_EDIT2,  m_file2.c_str());
    }
    
    void Hide()
    {
        ShowWindow(m_hDlg, SW_HIDE);
    }
    void SetText(const std::wstring& s)
    {
        if (!s.empty())
          m_TextToFind = s;

        SetDlgItemText(m_hDlg, IDC_EDIT1, m_TextToFind.c_str() );
        Edit_SelectAll(GetDlgItem(IDC_EDIT1));
        SetFocus(GetDlgItem(IDC_EDIT1));
    }

    void OnCommand(int hwp, int lwp, HWND w)
    {
        if (hwp == IDOK )
        {  
            m_MatchCase = Button_GetState(GetDlgItem(IDC_MATCHCASE)) == BST_CHECKED;
            m_MatchWholeWord = Button_GetState(GetDlgItem(IDC_MATCHWORD)) == BST_CHECKED;

            m_SearchUp = false; //utton_GetState(GetDlgItem(IDC_SEARCHUP)) == BST_CHECKED;
            

            GetDlgItemText(IDC_EDIT1, m_TextToFind);
            if (m_pIFindNextEvent)
              m_pIFindNextEvent->OnFindNextEvent(m_TextToFind, m_MatchCase, m_MatchWholeWord, m_SearchUp);
        }
        else if (hwp == IDCANCEL)
        { 
            Hide();
        }
        else if (hwp == IDC_EDIT1 )
        {
          if (lwp == EN_CHANGE)
          {
              GetDlgItemText(IDC_EDIT1, m_TextToFind);
              EnableWindow(GetDlgItem(IDOK), !m_TextToFind.empty());
          }
        }
    }
     
};

