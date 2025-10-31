#pragma once

template < class DFATraits >
class Tokenizer
{
  typedef wchar_t CharType;

  const CharType* m_str;    
  const CharType* m_it;
  CharType m_lastChar;

  void PutBack(CharType ch)
  {
    m_lastChar = ch;
  }

  bool NextChar(CharType & ch)
  {
    if (m_lastChar != 0)
    {
      ch = m_lastChar;
      m_lastChar  = 0;
    }
    else
    {
      if (*m_it == 0)
        return false;
      ch = *m_it++;
    }
    return true;
  }

public:

  typedef typename DFATraits::TokenType TokenType;

  Tokenizer(const CharType* psz) : m_str(psz)
  {
    m_lastChar = 0;
    m_it = psz;
  }

  template<class T>
  bool NextToken(T& lexeme, typename DFATraits::TokenType& tk)
  {
    for (;;)
    {
      if (!NextTokenNoInterleave(lexeme, tk))
        return false;

      if (!DFATraits::IsInterleave(tk))
        return true;
    }        
  }

  template<class T>
  bool NextTokenNoInterleave(T& lexeme, typename DFATraits::TokenType& tk)
  {
    lexeme.clear();
    int lastGoodState = -1;
    int currentState = 0;
    CharType ch;
    while (NextChar(ch))
    {
      currentState = DFATraits::GetNext(currentState, ch);
      if (currentState == -1)
      {
        PutBack(ch);
        break;
      }
      typename DFATraits::TokenType tk2;
      if (DFATraits::GetTokenFromState(currentState, tk2))
      {
        tk = tk2;
        lastGoodState = currentState;
      }
      lexeme += ch;
    }
    return (lastGoodState != -1);
  }
};
        