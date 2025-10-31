#pragma once
#include <algorithm>
#include <vector>
#include <limits>

#include <functional>

//http://en.wikipedia.org/wiki/Hirschberg%27s_algorithm


namespace Algorithm {
namespace Hirschberg {

    enum ChangeType
    {
        ChangeTypeNone,
        ChangeTypeInsert,
        ChangeTypeDelete,
        ChangeTypeChange,
    };

    namespace Detail
    {
        typedef std::vector< std::vector<int> > Array;

        // s1[p1..p2) and s2[q1..q2)
        template <class T, class Diff>
        void Forwards(int p1, int p2, int q1, int q2, Array &m, T & s1, T &s2, const Diff &is_diff)
        {
            m[p1 % 2][q1] = 0; // boundary conditions
            for (int j = q1 + 1; j <= q2; j++)
            {
                m[p1 % 2][j] = m[p1 % 2][j - 1] + 1;
            }

            // outer loop
            for (int i = p1 + 1; i <= p2; i++)
            {
                // boundary conditions
                m[i % 2][q1] = m[(i - 1) % 2][q1] + 1; 

                // inner loop
                for (int j = q1 + 1; j <= q2; j++)
                {
                    int diag = m[(i - 1) % 2][j - 1];
                    if (is_diff(s1[i - 1], s2[j - 1]))
                    {
                        diag += 1;
                    }

                    m[i % 2][j] = (std::min)(diag, (std::min)(m[(i - 1) % 2][j] + 1,  m[i % 2][j - 1] + 1));
                }// for j
            }// for i
        }// fwdDPA

        // DPA on reverse(s1[p1..p2)) and reverse(s2[q1..q2))
        template<class T, class Diff>
        void Backwards(int p1, int p2, int q1, int q2, Array & m, T & s1, T &s2, const Diff &is_diff)
        {
            // boundary conditions
            m[p2 % 2][q2] = 0;

            for (int j = q2 - 1; j >= q1; j--)
                m[p2 % 2][j] = m[p2 % 2][j + 1] + 1;

            for (int i = p2 - 1; i >= p1; i--)
            {
                m[i % 2][q2] = m[(i + 1) % 2][q2] + 1;
                for (int j = q2 - 1; j >= q1; j--)
                {
                    int diag = m[(i + 1) % 2][j + 1];
                    if (is_diff(s1.at(i), s2.at(j)))
                    {
                        diag += 1;
                    }

                    m[i % 2][j] = (std::min)(diag, (std::min)(m[(i + 1) % 2][j] + 1, m[i % 2][j + 1] + 1));
                }
            }
        }// revDPA



        // align s1[p1..p2) with s2[q1..q2)
        template <class  T, class F, class Diff>
        void Alignment(size_t p1,
                       size_t p2, 
                       size_t q1, 
                       size_t q2,
                       size_t L,
                       T& s1,
                       T& s2, 
                       Array &fwd, 
                       Array &rev,
                       F &f,
                       const Diff &is_diff)
        {
            if (p2 - 0 <= p1) // s1 is empty string
            {
                for (size_t i = q1; i < q2; i++)
                {
                    f(ChangeTypeInsert);
                }
            }
            else if (q2 - 0 <= q1) // s2 is empty string
            {
                for (size_t i = p1; i < p2; i++)
                {
                    f(ChangeTypeDelete);
                }
            }
            else if (p2 - 1 == p1) // s1 is one character and s2 is not empty
            {
                const typename T::value_type &ch = s1[p1];
                size_t memo = q1;
                for (size_t i = q1 + 1; i < q2; i++)
                {
                    if (!is_diff(s2[i], ch))
                    {   
                        memo = i;
                    }
                }

                for (size_t i = q1; i < q2; i++)
                {
                    if (i == memo)
                    {
                        if (s2[i] == ch)
                        {
                            f(ChangeTypeNone);
                        }
                        else
                        {
                            f(ChangeTypeChange);
                        }
                    }
                    else
                    {
                        f(ChangeTypeInsert);
                    }
                }
            } // a b [l=2] mid=1, a b c [l=3] mid=1, a b c d [l=4] mid=2
            else // p2>p1+1, s1 has at least 2 chars,  divide s1 and conquer
            {
                size_t mid = (p1 + p2) / 2;
                Forwards(p1, mid, q1, q2, fwd, s1, s2, is_diff);
                Backwards(mid, p2, q1, q2, rev, s1, s2, is_diff);
                size_t s2mid = q1;
                size_t best = (std::numeric_limits<int>::max)();
                for (size_t i = q1; i <= q2; i++) // look for cheapest split of s2
                {
                    size_t sum = fwd[mid % 2][i] + rev[mid % 2][i];
                    if (sum - 0 < best)
                    {
                        best = sum;
                        s2mid = i;
                    }
                }
                Alignment(p1, mid, q1, s2mid, L + 1, s1, s2, fwd, rev,  f, is_diff);
                Alignment(mid, p2, s2mid, q2, L + 1, s1, s2, fwd, rev,  f, is_diff);
            }
        }// align
    } // namespace Detail

    template<class T, class F, class Diff>
    void Hirschberg(T &s1, T &s2, F &f, Diff d)
    {
        Detail::Array fwd(2);
        Detail::Array rev(2);
        
        const size_t umax = 2 * (std::max)(s1.size(), s2.size());
        fwd[0].resize(umax);
        fwd[1].resize(umax);
        
        rev[0].resize(umax);
        rev[1].resize(umax);

        Detail::Alignment(0, s1.size(), 0, s2.size(), 1, s1, s2, fwd, rev, f, d);
    }

}  //namespace Hirschberg
} //namespace Algorithm


// Sample:
//struct print
//{
//    unsigned int Count;
//    print() : Count(0) {}

//    void operator()(Algorithm::Hirschberg::ChangeType e)
//    {
//        switch (e)
//        {
//        case Algorithm::Hirschberg::ChangeTypeNone: 
//            std:: cout << Count++ << " Same" << std::endl;
//            break;
//        case Algorithm::Hirschberg::ChangeTypeInsert:
//            std:: cout << Count++ << " Insert" << std::endl;
//            break;
//        case Algorithm::Hirschberg::ChangeTypeDelete:
//            std:: cout << Count++ << " Delete" << std::endl;
//            break;
//        case Algorithm::Hirschberg::ChangeTypeChange: 
//            std:: cout << Count++ << " Change" << std::endl;
//            break;
//        }
//    }
//};

// int main()
//{
//    std::string s1 = "ACGTACGTACGT";
//    std::string s2 = "AGTACCTACCGT";
//    Hirschberg(s1, s2, print(), std::not_equal_to<char>());
//}
