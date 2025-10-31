#pragma once

#include "resource.h"
#include "win32express.h"
#include "hirschberg.h"
#include <vector>
#include <set>
#include "tokenizer.h"

#include "Myers.h"
#include <unordered_set> 
#include "FindDlg.h"

#define TRIMLINES

typedef std::vector<Algorithm::Hirschberg::ChangeType> MemoOfChanges;

struct MyersMemo
{
    Algorithm::Myers::Operation m_Operation;
    int m_Count;
    //MyersMemo();//
    MyersMemo(Algorithm::Myers::Operation Operation,
              int Count)
    {
        m_Operation = Operation;
        m_Count = Count;
    }
};

typedef std::vector<MyersMemo> TMyersMemoOfChanges;





// trim from start
inline std::wstring &ltrim(std::wstring &s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(isspace))));
        return s;
}

// trim from end
inline std::wstring &rtrim(std::wstring &s) {
        s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(isspace))).base(), s.end());
        return s;

}

// trim from both ends
inline std::wstring &trim(std::wstring &s) {
        return ltrim(rtrim(s));
}


//ignora todos espacos antes e depois, tem que dar o mesmo resultado que trim
// se todos os espacos do meio forem iguais
//a difernete que 2 ou mais espacoes no meio ficcam como sendo 1
inline int Strcmp(const wchar_t* s1, const wchar_t* s2)
{
  //std::wstring ss1(s1);
  //std::wstring ss2(s2);

  const wchar_t* s1e = s1 + wcslen(s1) -1;
  const wchar_t* s2e = s2 + wcslen(s2) -1;

  //trim da esquerda
  while (*s1 == L' ')
    ++s1;
  while (*s2 == L' ')
    ++s2;

  while (*s1e == ' ')
    --s1e;
  while (*s2e  == ' ')
    --s2e;

  if  (s1e < s1)
    s1e = s1;
  if  (s2e < s2)
    s2e = s2;



  while (s1 <= s1e && s2 <= s2e)
  {
    if (*s1 == ' ')
    {
      while (*(s1 + 1) == ' ')
        ++s1;    
    }
    if (*s2 == ' ')
    {
      while (*(s2+1) == ' ')
        ++s2;      
    }

    if (*s1 != *s2)
    {
      return *s1 < *s2 ? -1 : 1;
    }
    ++s1;
    ++s2;
  }

  //assert(Strcmp(L" ", L" ") == 0);
  //assert(Strcmp(L"", L"") == 0);
  //assert(Strcmp(L"    ", L" ") == 0);
  //assert(Strcmp(L" A   B ", L"   A B   ") == 0);
  //assert(Strcmp(L" } ", L"   }  ") == 0);
  //assert(Strcmp(L" } ", L"   }") == 0);

  //assert(Strcmp(L" A   B ", L"AB ") == 1);
  //assert(Strcmp(L" A   B ", L"A  B ") == 0);
  //assert(Strcmp(L" A   B ", L"A B C") == -1);
  //assert(Strcmp(L"A B C", L" A   B ") == 1);

  size_t s1_size = s1e - s1 + 1;
  size_t s2_size = s2e - s2 + 1;
  if (*s1 == 0)
    s1_size =  0;
  if (*s2 == 0)
    s2_size =  0;
  int r =  (s1_size == s2_size) ? 0 : ( s1_size < s2_size ) ? -1 : 1;


  // bool b = trim(ss1) < trim(ss2);
  //assert(b == (r == -1));
  return r;
}

// Usado no set que "junta" as strings
struct SetCompareLess
{
    bool operator ()(const std::wstring& s1, const std::wstring& s2) const
    {
       return Strcmp(s1.c_str(), s2.c_str()) == -1;
    }
};
// Usado no set que "junta" as strings
struct SetEqual
{
    bool operator ()(const std::wstring& s1, const std::wstring& s2) const
    {
       return Strcmp(s1.c_str(), s2.c_str()) == 0;
    }
};

typedef Tokenizer<MGrammar::MGrammar> Scanner;



struct Lexeme
{
    MGrammar::Tokens m_Token; 
    std::wstring m_lexeme;

    Lexeme(MGrammar::Tokens t, const std::wstring& l) : m_Token(t), m_lexeme(l)
    {
    }

    bool operator ==(const Lexeme& other) const
    {
        if (m_Token != other.m_Token)
            return false;
        return m_lexeme == other.m_lexeme;
    }
    bool operator !=(const Lexeme& other) const
    {
        return !operator==(other);
    }
};

typedef std::vector<Lexeme> Lexemes;

inline void Fill(const std::wstring &s1, Lexemes& v)
{
    Scanner scanner(s1.c_str());

    MGrammar::Tokens token; 
    std::wstring lexeme;

    while (scanner.NextToken(lexeme , token))
    {
        v.push_back(Lexeme(token, lexeme));
    }
}

struct CountAndKeepChanges
{
    int same_count;
    int added_count;
    int deleted_count;
    int changed_count;

    int lineFinal;

    CountAndKeepChanges()
    {
        same_count = 0;
        added_count = 0;
        deleted_count = 0;
        changed_count = 0;
        lineFinal = 0;
    }

    MemoOfChanges m_Changes;

    void operator()(Algorithm::Hirschberg::ChangeType e)
    {

        switch (e)
        {
        case Algorithm::Hirschberg::ChangeTypeNone: 
            same_count++;
            m_Changes.push_back(e);
            lineFinal++;
            break;
        case Algorithm::Hirschberg::ChangeTypeInsert:
            added_count++;
            m_Changes.push_back(e);
            lineFinal++;
            break;

        case Algorithm::Hirschberg::ChangeTypeDelete:
            deleted_count++;
            m_Changes.push_back(e);
            lineFinal++;
            break;

        case Algorithm::Hirschberg::ChangeTypeChange: 
            changed_count++;
            m_Changes.push_back(e);
            lineFinal++;
            break;
        }
    }
};

struct MyersCountAndKeppChanges
{
    TMyersMemoOfChanges m_Changes;
    int m_Same;
    int m_Added;
    int m_Removed;

    MyersCountAndKeppChanges()
    {
        m_Same = 0;
        m_Added= 0;
        m_Removed= 0;
    }

    ~MyersCountAndKeppChanges()
    {      
    }

    void operator()(Algorithm::Myers::Operation op, size_t i)
    {
      switch (op)
      {
      case Algorithm::Myers::RemoveFromB:
        {
          m_Changes.push_back(MyersMemo(Algorithm::Myers::RemoveFromB, i));
          m_Removed += i;
        }
        break;
      case Algorithm::Myers::InsertFromA:
        {
          m_Changes.push_back(MyersMemo(Algorithm::Myers::InsertFromA, i));
           m_Added += i;
        }
        break;
      case Algorithm::Myers::EqualAB:
        {
          m_Changes.push_back(MyersMemo(Algorithm::Myers::EqualAB, i));
          m_Same += i;
        }
        break;
      }
    }
};
struct LineOfText;
inline bool IsTheSameLine(const LineOfText& a, const LineOfText& b);




struct LineOfText
{
private:
  mutable Lexemes m_lexemes;
public:

  //String que aponta para um set
  const wchar_t* m_pszSetEntry;

  // String original lida no texto
  // Dependendo da comparação do Set(SetCompareLess) pode diferir
  // de m_pszSetEntry
  // Se a comparação SetCompareLess for exata pode até ser removido
  std::wstring m_text;

  const Lexemes&  Lexemes()  const
  {
    
    if (!m_text.empty() && m_lexemes.empty())
    {
      Fill(m_text, m_lexemes);
    }

    return m_lexemes; 
  }
                

  LineOfText(const wchar_t* psz)
    :
    m_text(psz),
    m_pszSetEntry(psz)
  {
    
  }


  LineOfText(const wchar_t* psz, const std::wstring& text)
    :
    m_pszSetEntry(psz),
    m_text(text)
  {
    // Fill(psz, m_lexemes);
  }

/*  bool operator == (const LineOfText& line) const
  {
    //aqui tem que ser bastante conservador..
    // se o ponteiros forem diferentes tem que dizer que é igual
    // so se for muito parecido. Ver
    //quando sao diferentes eh tratado em:
    //Algorithm::Myers::EqualAB
    return IsTheSameLine(*this, line);
  }

  bool operator != (const LineOfText& line) const
  {
    return !operator ==(line);
  }*/
};

inline bool IsTheSameLine(const LineOfText& a, const LineOfText& b)
{
  return a.m_pszSetEntry == b.m_pszSetEntry;

  //Exatamente igual ou só difere por espaços
  if (a.m_pszSetEntry == b.m_pszSetEntry)
  {
    return true; 
  }

  CountAndKeepChanges f;
  Algorithm::Hirschberg::Hirschberg(a.m_text, b.m_text, f, std::not_equal_to<wchar_t>());
  
  if (f.added_count > 0 &&
      f.deleted_count == 0 &&
      f.changed_count == 0)
  {
    if (a.m_text.size() + b.m_text.size() > 20)
    {
      //para ter uma margem de seguranca vou considerar um certo tamanho min
      return true;
    }
    //So adicionado
  }

  //so deletado
  if (f.added_count ==  0 &&
      f.deleted_count > 0 &&
      f.changed_count == 0)
  {
    if (a.m_text.size() + b.m_text.size() > 20)
    {
      //para ter uma margem de seguranca vou considerar um certo tamanho min
      return true;
    }    
  }


  //1/3 mudancas
  if (f.added_count + f.deleted_count + f.changed_count < f.same_count / 4)
  {
    return true;
  }

  return false;
}  


inline bool IsTheSameLineFlexibe(const LineOfText& a, const LineOfText& b)
{
  //Exatamente igual ou só difere por espaços
  if (a.m_pszSetEntry == b.m_pszSetEntry)
  {
    return true; 
  }

  //o insert do set ja deve cuidar disso
  ASSERT(Strcmp(a.m_pszSetEntry, b.m_pszSetEntry) != 0);

  CountAndKeepChanges f;
  Algorithm::Hirschberg::Hirschberg(a.Lexemes(), b.Lexemes(), f, std::not_equal_to<Lexeme>());

    
  //So adicionado?
  if (f.added_count > 0 &&
      f.deleted_count == 0 &&
      f.changed_count == 0)
  {
    if (a.m_text.size() + b.m_text.size() > 20)
    {
      //para ter uma margem de seguranca vou considerar um certo tamanho min
      return true;
    }
  }

  //so deletado?
  if (f.added_count ==  0 &&
      f.deleted_count > 0 &&
      f.changed_count == 0)
  {
    //tem que cuidar para linhas vazias nao deram certo com outras 

    if (a.m_text.size() > 5 && //nenhuma string pode ser muito pequena
        b.m_text.size()  > 5 && //nenhuma string pode ser muito pequena
        a.m_text.size() + b.m_text.size() > 20) //e a soma + 20
    {
      //para ter uma margem de seguranca vou considerar um certo tamanho min
      return true;
    }    
  }

  //so trocou 1 e no minimo 2 estao iguais?
  if (f.added_count ==  0 &&
      f.deleted_count == 0 &&
      f.same_count > 2 &&
      f.changed_count == 1)
  {
      return true;
  }


  //Menos de 1/3 de mudancas?
  if (f.added_count + f.deleted_count + f.changed_count < (f.added_count + f.deleted_count + f.changed_count + f.same_count) / 3)
  {
    return true;
  }


  CountAndKeepChanges f2;
  Algorithm::Hirschberg::Hirschberg(a.m_text, b.m_text, f2, std::not_equal_to<wchar_t>());
  //Menos de 1/4 de mudancas de caracteres?
  if (f2.added_count + f2.deleted_count + f2.changed_count < (f2.added_count + f2.deleted_count + f2.changed_count + f2.same_count) / 4)
  {
    return true;
  }

  return false;
}  

typedef std::vector<LineOfText> TextLines;

//parece que ficou mais rapido que o set normal
//typedef std::tr1::unordered_set<std::wstring, std::hash<std::wstring>, SetEqual> SetOfStrings;
//
typedef std::set<std::wstring, SetCompareLess> SetOfStrings;

class MainWindow : public Window<MainWindow>, public IFindNextEvent
{
    MemoOfChanges m_Changes;

    HWND m_hRichEdit;
    HWND m_hExitButton;
    std::wstring m_Path;
    int  m_FontSize;
    std::wstring m_FontName;
    HFONT m_hFont;
    LOGFONT m_LogFont;

    HWND m_hFirst;
    HWND m_hPrev ;
    HWND m_hNext ;
    HWND m_hLast ;
    
      COLORREF m_AddedColor;
  COLORREF m_DeletedColor;
  COLORREF m_SameColor;
  COLORREF m_BackgroundColor;

    int m_DiffIndex;
    long m_FindStart;
    long m_IsFirstFound;

    void Next();
    void Previous();
    void MoveLast();
    void MoveFirst();
    void CopySelection();
    bool m_PopupMenuActivated;

    void Find();
    void FindNext();
    
    virtual void OnFindNextEvent(const std::wstring& s, bool matchcase, bool matchword, bool searchUp) ;
    
    void ChangeFont(LPCTSTR fontName, int size);
    FindDlg m_FindDlg;

    bool SaveOptions();
    void LoadOptions();

public:

  
    MainWindow(const std::wstring& fullpathname);
    ~MainWindow();

    
    std::wstring m_OldFileName;
    std::wstring m_NewFileName;

    void Run();
    void OnIdle();

    void OnMinMax(MINMAXINFO* pMinMaxInfo);
    void OnPaint(HDC);
    void OnCommand(int , int, HWND lparam);
    BOOL OnNotify(NMHDR* pw, NMHDR* pl);
    void OnDeactivate();
    BOOL OnClose();
    void OnCreate();
    void OnSize(UINT type, int w, int h);
    BOOL OnSetCursor();


    
    void InsertText(HWND edit, wchar_t ch, COLORREF color, BOOL bold, BOOL strikeout);
    void InsertText(HWND edit, wchar_t ch, COLORREF color);
    void InsertText(HWND edit, LPCTSTR psz, COLORREF color);
    
    void InsertSameText(HWND edit, LPCTSTR psz);
    void InsertSameText(HWND edit, wchar_t ch);
    void InsertAddedText(HWND edit, LPCTSTR psz);
    void InsertAddedText(HWND edit, wchar_t ch);
    void InsertDeletedText(HWND edit, LPCTSTR psz);
    void InsertDeletedText2(HWND edit);
    void InsertDeletedText(HWND edit, wchar_t ch);

};
