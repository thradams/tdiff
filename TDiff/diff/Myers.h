#pragma once

//E. Myers, An O(ND) Difference Algorithm and Its Variations,''
//Algorithmica 1, 2 (1986), 251-266.

namespace Algorithm {
namespace Myers {


enum Operation
{
  EqualAB,
  InsertFromA,
  RemoveFromB
};


// An edit script for A and B is a set of insertion and deletion
//commands that transform A into B.
// The delete command "xD" deletes the symbol a x from A.
// The insert command "x I b1 ,b2 , . . . bt" inserts the sequence of symbols
// b1 . . . bt immediately after a x.
// Script commands refer to symbol positions within A before any
//commands have been
// performed.


template<class Iterator, class TCompare,  class F>
int MyersLCS(Iterator aBeginIt, Iterator aEndIt, Iterator bBeginIt, Iterator bEndIt, TCompare isEqual, F& f)
{
  // Fast walkthrough equal lines at the start
  int equalBeginCount = 0;
  while (aBeginIt != aEndIt &&  bBeginIt != bEndIt &&  isEqual(*aBeginIt, *bBeginIt))
  {
    aBeginIt++;
    bBeginIt++;
    equalBeginCount++;
  }

  // Fast walkthrough equal lines at the end
  int equalEndCount = 0;
  while (aBeginIt != aEndIt &&
    bBeginIt != bEndIt &&
    isEqual(*(aEndIt - 1) , *(bEndIt - 1)))
  {
    aEndIt--;
    bEndIt--;
    equalEndCount++;
  }

  if (equalBeginCount)
  {
    f(EqualAB, equalBeginCount);
  }

  const int aSize = std::distance(aBeginIt, aEndIt);
  const int bSize = std::distance(bBeginIt, bEndIt);
  const int abSize = aSize + bSize;

  // TRIVIAL CASES
  if (!aSize || !bSize)
  {
    if (aSize || bSize)
    {
      f(aSize ? RemoveFromB : InsertFromA, aSize ? aSize : bSize);
    }

    if (equalEndCount)
    {
      f(EqualAB, equalEndCount);
    }

    return aSize ? aSize : bSize;
  }

  int meet = -1;

  int aCut;
  int bCut;

  { // Shortest Middle Snake

    //http://citeseer.ist.psu.edu/myers86ond.html
    vector<int> front(abSize + 1, -abSize);
    front[aSize] = 0;

    vector<int> tnorf(abSize + 1, -abSize);
    tnorf[bSize] = 0;

    for (int chng = 0; chng <= abSize && meet == -1; ++chng)
    {
      const bool isOdd = chng & 1; // impar?

      if (isOdd)
      {
        const int s = aSize - chng / 2;
        for (int d = s < 0 ? -s % 2 : s;
          d <= aSize + chng / 2 && d <= abSize;
          d += 2)
        {
          int& adv = front[d];
          if (adv < 0)
          {
            continue;
          }

          // find the end of the furthest reaching forwardD-path in diagonal k.
          while (d + adv - aSize < bSize &&
            adv < aSize &&
            isEqual(*(aBeginIt + adv) , *(bBeginIt + (adv + d - aSize))))
          {
            adv++;
          }

          if (adv + tnorf[d] >= aSize)
          {
            meet = d;
            break;
          }

          if (adv + d < abSize && front[d + 1] < adv)
          {
            front[d + 1] = adv;
          }

          if (adv < aSize && front[d - 1] <= adv)
          {
            front[d - 1] = adv + 1;
          }
        }
      }
      else // pares
      {
        const int s = bSize - chng / 2;
        for (int d = s < 0 ? -s % 2 : s;
          d <= bSize + chng / 2 && d <= abSize;
          d += 2)
        {
          int& adv = tnorf[d];
          if (adv < 0)
          {
            continue;
          }

          while (adv < aSize && adv < d &&
            isEqual(*(aBeginIt + (aSize - adv - 1)) , *(bBeginIt + (d - adv - 1))))
          {
            adv++;
          }

          if (front[d] + adv >= aSize)
          {
            meet = d;
            break;
          }

          if (adv < aSize && tnorf[d + 1] <= adv)
          {
            tnorf[d + 1] = adv + 1;
          }

          if (adv < d && tnorf[d - 1] < adv)
          {
            tnorf[d - 1] = adv;
          }
        } // for
      } // final pares
    } // chng loop

    aCut = aSize - tnorf[meet];
    bCut = meet - tnorf[meet];
  } // Shortest Middle Snake

  // HIRSCHBERG-MYERS' DIVIDE-AND-CONQUER
  int dff = MyersLCS(aBeginIt, aBeginIt + aCut, bBeginIt, bBeginIt + bCut, isEqual, f);
  dff += MyersLCS(aBeginIt + aCut, aEndIt, bBeginIt + bCut, bEndIt, isEqual, f);

  //assert(dff == chng - 1);
  if (equalEndCount)
  {
    f(EqualAB, equalEndCount);
  }

  return dff;
}

template<class T, class TCompare, class F>
int MyersDiff(const T& a, const T& b, TCompare isEqual, F &f)
{
  return MyersLCS(a.begin(), a.end(), b.begin(), b.end(), isEqual, f);
}

}
}


/*
Exemplo:


template<class T>
struct DetailedScript
{
  T& m_s1;
  T& m_s2;
  int m_b;
  int m_a;

  DetailedScript(T& s1, T&s2) : m_s1(s1), m_s2(s2)
  {
    m_a = 0;
    m_b = 0;
  }

  void operator()(Algorithm::Myers::Operation op, size_t i)
  {
    using namespace console;

    switch (op)
    {
    case Algorithm::Myers::RemoveFromB:
      {
        int k = 0;
        for ( ; k < i; k++)
        {
          console::textcolor(console::LIGHTRED);
          std::wcout << m_s1[m_b + k];
        }
        m_b += k;
      }

      break;
    case Algorithm::Myers::InsertFromA:
      {
        console::textcolor(console::LIGHTBLUE);
        int k = 0;
        for ( ; k < i; k++)
        {
          std::wcout << m_s2[m_a + k];
        }
        m_a += k;        
      }
      break;
    case Algorithm::Myers::EqualAB:
      {
        console::textcolor(console::WHITE);
        int k = 0;
        for ( ; k < i; k++)
        {
          std::wcout << m_s2[m_a + k];
        }
        m_a += k;
        m_b += k;
      }
      break;
    }
  }
};

*/