// ----------------------------------------------------------------------------------------------
// Copyright 2015 Mårten Rånge
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// ----------------------------------------------------------------------------------------------

#include "stdafx.h"

#include <algorithm>
#include <cstring>
#include <initializer_list>
#include <iostream>
#include <sstream>
#include <tuple>
#include <vector>

#include "../tsprintf/tsprintf.hpp"


#define TEST_CASE() TS_PRINTF("%s(%d) : TEST_CASE - %s\n", __FILE__, static_cast<int> (__LINE__), __FUNCTION__)
#define TEST_EQ(expected, actual) test_eq (__FILE__, __LINE__, expected, #expected, actual, #actual)

namespace tests
{
  using namespace typesafe_printf::details;

  std::uint32_t errors = 0;

  template<typename T>
  std::ostream & operator<< (std::ostream & os, std::vector<T> const & vs)
  {
    os << '[';

    auto sz = vs.size ();
    for (auto iter = 0U; iter < sz; ++iter)
    {
      if (iter > 0)
      {
        os << ", ";
      }

      os << vs[iter];
    }

    os << ']';

    return os;
  }

  template<typename T>
  struct add_const_to_pointee
  {
    using type = T;
  };

  template<typename T>
  struct add_const_to_pointee<T *>
  {
    using type = T const *;
  };

  template<typename T>
  using add_const_to_pointee_t = typename add_const_to_pointee<T>::type;

  template<typename T>
  struct fixup_type
  {
    // using type = add_const_to_pointee_t<std::decay<T>_t::type>;
    // std::decay_t doesn't exist in GCC 4.8.1, use std::decay instead
    using type = add_const_to_pointee_t<typename std::decay<T>::type>;
  };

  template<typename T>
  using fixup_type_t = typename fixup_type<T>::type;

  template<typename TExpected, typename TActual>
  struct comparer
  {
    static bool is_equal (TExpected const & expected, TActual const & actual)
    {
      return expected == actual;
    }
  };

  template<>
  struct comparer<char const *, char const *>
  {
    static bool is_equal (char const * expected, char const * actual)
    {
      return std::strcmp (expected, actual) == 0;
    }
  };

  template<typename TExpected, typename TActual>
  bool test_eq (
      char const *  file_name
    , int           line_no
    , TExpected &&  expected
    , const char *  sexpected
    , TActual &&    actual
    , const char *  sactual
    )
   {
    TYPESAFE_PRINTF__ASSERT (file_name);
    TYPESAFE_PRINTF__ASSERT (sexpected);
    TYPESAFE_PRINTF__ASSERT (sactual);

    using expected_t  = fixup_type_t<TExpected>       ;
    using actual_t    = fixup_type_t<TActual>         ;
    using comparer_t  = comparer<expected_t, actual_t>;

    if (comparer_t::is_equal (expected, actual))
    {
      return true;
    }
    else
    {
      ++errors;
      std::cout
        << file_name
        << "("
        << line_no
        << "): EQ - "
        << sexpected
        << "{"
        << std::forward<TExpected> (expected)
        << "} == "
        << sactual
        << "{"
        << std::forward<TActual> (actual)
        << "}"
        << std::endl
        ;

      return false;
    }
  }

  std::vector<type_id> decode (encoded_types_t encoded_types)
  {
    std::vector<type_id> result;

    while (encoded_types != 0)
    {
      result.push_back (static_cast<type_id> (encoded_types & type_id__mask));
      encoded_types >>= type_id__bits;
    }

    return result;
  }

  template<size_type N>
  bool copy_to_buffer (char (&arr) [N], std::string const & s)
  {
    std::fill_n (arr, N, 0);

    if (s.size () + 1 < N)
    {
      std::copy (
          s.begin ()
        , s.end ()
        , arr
        );
      return true;
    }
    else
    {
      return false;
    }
  }

  template<std::size_t N>
  bool is_sorted_string (char const (&arr) [N])
  {
    if (N > 0)
    {
      auto begin  = arr             ;
      auto end    = begin + (N  - 1);

      TYPESAFE_PRINTF__ASSERT (*end == '\0');

      return std::is_sorted (begin, end);
    }
    else
    {
      return true;
    }
  }

  template<std::size_t N>
  bool any_of (char ch, char const (&arr) [N])
  {
    if (N > 0)
    {
      auto begin  = arr             ;
      auto end    = begin + (N  - 1);

      TYPESAFE_PRINTF__ASSERT (*end == '\0');

      return std::find (begin, end, ch) != end;
    }
    else
    {
      return false;
    }
  }

  void test__scanner_literals ()
  {
    TEST_CASE ();

    {
      auto expected = true;
      auto actual   = is_sorted_string (scanner::union_of_cs_at);

      TEST_EQ (expected, actual);
    }

    {
      auto expected = true;
      auto actual   = is_sorted_string (scanner::union_of_signed_ints);

      TEST_EQ (expected, actual);
    }

    {
      auto expected = true;
      auto actual   = is_sorted_string (scanner::union_of_unsigned_ints);

      TEST_EQ (expected, actual);
    }

    {
      auto expected = true;
      auto actual   = is_sorted_string (scanner::union_of_floats);

      TEST_EQ (expected, actual);
    }
  }

  void test__scanner_any_of ()
  {
    TEST_CASE ();

    for (auto ch = '\0'; ch <= 'z'; ++ch)
    {
      {
        auto expected = any_of (ch, scanner::union_of_cs_at);
        auto actual   = scanner::any_of (ch, scanner::union_of_cs_at);
        auto actualb  = scanner::binary_any_of (ch, scanner::union_of_cs_at);

        TEST_EQ (expected, actual);
        TEST_EQ (expected, actualb);
      }

      {
        auto expected = any_of (ch, scanner::union_of_signed_ints);
        auto actual   = scanner::any_of (ch, scanner::union_of_signed_ints);
        auto actualb  = scanner::binary_any_of (ch, scanner::union_of_signed_ints);

        TEST_EQ (expected, actual);
        TEST_EQ (expected, actualb);
      }

      {
        auto expected = any_of (ch, scanner::union_of_unsigned_ints);
        auto actual   = scanner::any_of (ch, scanner::union_of_unsigned_ints);
        auto actualb  = scanner::binary_any_of (ch, scanner::union_of_unsigned_ints);

        TEST_EQ (expected, actual);
        TEST_EQ (expected, actualb);
      }

      {
        auto expected = any_of (ch, scanner::union_of_floats);
        auto actual   = scanner::any_of (ch, scanner::union_of_floats);
        auto actualb  = scanner::binary_any_of (ch, scanner::union_of_floats);

        TEST_EQ (expected, actual);
        TEST_EQ (expected, actualb);
      }
    }

  }

  void test__scanner ()
  {
    TEST_CASE ();

    constexpr auto max_size = 64;

    {
      using test_case_t = std::tuple<std::string, std::vector<type_id>>;

      // Some manually chosen test-cases
      std::vector<test_case_t> test_cases =
        {
          test_case_t {"Hello"      , {                           }},
          test_case_t {"%%"         , {                           }},
          test_case_t {"%d"         , {tid__int                   }},
          test_case_t {"Hello %lld" , {tid__long_long             }},
          test_case_t {"%+0.0f,%d%%", {tid__double    , tid__int  }},
        };

      for (auto && test_case : test_cases)
      {
        char buffer[max_size] = {};

        auto format_specifier = std::get<0> (test_case);
        auto types            = std::get<1> (test_case);

        if (TEST_EQ (true, copy_to_buffer (buffer, format_specifier)))
        {
          auto expected = types;
          auto actual   = decode (scanner::encode (buffer));

          TEST_EQ (expected, actual);
        }
      }
    }

    {
      std::vector<std::string> argument_type_table
        {
          "hh"  ,
          "h"   ,
          ""    ,
          "l"   ,
          "ll"  ,
          "j"   ,
          "z"   ,
          "t"   ,
          "L"   ,
        };

      std::vector<std::string> conversion_specifiers_table
        {
          "c"         ,
          "s"         ,
          "di"        ,
          "oxXu"      ,
          "fFeEaAgG"  ,
          "n"         ,
          "p"         ,
        };

      // Generates all variants of argument_types and conversion specifiers
      //  and verifies the mapping towards the type_ids table
      for (auto cs = 0U; cs < scanner::conversion_specifier__count; ++cs)
      {
        for (auto at = 0U; at < scanner::argument_type__count; ++at)
        {
          auto conversion_specifiers  = conversion_specifiers_table[cs];
          auto argument_type          = argument_type_table[at];

          for (auto && conversion_specifier : conversion_specifiers)
          {
            char buffer[max_size] = {};
            std::ostringstream ss;
            ss << "%" << argument_type << conversion_specifier;

            auto format_specifier = ss.str ();

            if (TEST_EQ (true, copy_to_buffer (buffer, format_specifier)))
            {
              std::vector<type_id>  expected  { scanner::type_ids[cs][at] };
              auto                  actual    = decode (scanner::encode (buffer));

              TEST_EQ (expected, actual);
            }
          }
        }
      }
    }
  }

  template<typename T>
  auto unconst (T const * p)
  {
    return const_cast<T*> (p);
  }

  void test__printf_variants ()
  {
    TEST_CASE ();

    char buffer[1024] {};

    TS_PRINTF   (             "TS_PRINTF\n" );

    TS_SPRINTF  (buffer,      "TS_SPRINTF"  );
    TEST_EQ     ("TS_SPRINTF" ,buffer);

    TS_SNPRINTF (buffer, 20,  "TS_SNPRINTF" );
    TEST_EQ     ("TS_SNPRINTF",buffer);

    TS_FPRINTF  (stdout,      "TS_FPRINTF\n");

    TS_SPRINTF  (buffer,      "TS_SPRINTF: %d\n%s\n",  3, "Hello");
    TEST_EQ     ("TS_SPRINTF: 3\nHello\n" ,buffer);

    TS_SPRINTF  (
        buffer
      , "This is a long format %s, the purpose is to make sure we don't hit any funny compiler limits.\n"
        "This is a long format %s, the purpose is to make sure we don't hit any funny compiler limits.\n"
        "This is a long format %s, the purpose is to make sure we don't hit any funny compiler limits.\n"
        "This is a long format %s, the purpose is to make sure we don't hit any funny compiler limits.\n"
        "This is a long format %s, the purpose is to make sure we don't hit any funny compiler limits.\n"
      , "string"
      , "string"
      , "string"
      , "string"
      , "string"
      );

    using signed_size_t       = typesafe_printf::details::ssize_t   ;
    using unsigned_ptrdiff_t  = typesafe_printf::details::uptrdiff_t;

    char const *            v_char_p                 = "02"                 ;
    double                  v_double                 = 0x03                 ;
    int                     v_int                    = 0x04                 ;
    int                     nv_int_p                 = 0x05                 ;
    int *                   v_int_p                  = &nv_int_p            ;
    std::intmax_t           v_intmax_t               = 0x06                 ;
    std::intmax_t           nv_intmax_t              = 0x07                 ;
    std::intmax_t *         v_intmax_t_p             = &nv_intmax_t         ;
    long                    v_long                   = 0x08                 ;
    long double             v_long_double            = 0x09                 ;
    long long               v_long_long              = 0x0A                 ;
    long long               nv_long_long_p           = 0x0B                 ;
    long long *             v_long_long_p            = &nv_long_long_p      ;
    long                    nv_long_p                = 0x0C                 ;
    long *                  v_long_p                 = &nv_long_p           ;
    std::ptrdiff_t          v_ptrdiff_t              = 0x0D                 ;
    std::ptrdiff_t          nv_ptrdiff_t_p           = 0x0E                 ;
    std::ptrdiff_t *        v_ptrdiff_t_p            = &nv_ptrdiff_t_p      ;
    short                   v_short                  = 0x0F                 ;
    short                   nv_short_p               = 0x10                 ;
    short *                 v_short_p                = &nv_short_p          ;
    char signed             v_signed_char            = 0x11                 ;
    char signed             nv_signed_char_p         = 0x12                 ;
    char signed *           v_signed_char_p          = &nv_signed_char_p    ;
    signed_size_t           v_signed_size_t          = 0x13                 ;
    signed_size_t           nv_signed_size_t_p       = 0x14                 ;
    signed_size_t *         v_signed_size_t_p        = &nv_signed_size_t_p  ;
    std::size_t             v_size_t                 = 0x15                 ;
    std::uintmax_t          v_uintmax_t              = 0x16                 ;
    char unsigned           v_unsigned_char          = 0x17                 ;
    int unsigned            v_unsigned_int           = 0x18                 ;
    long unsigned           v_unsigned_long          = 0x19                 ;
    long long unsigned      v_unsigned_long_long     = 0x1A                 ;
    unsigned_ptrdiff_t      v_unsigned_ptrdiff_t     = 0x1B                 ;
    short unsigned          v_unsigned_short         = 0x1C                 ;
    void const *            v_void_p                 = (void const *)0x1D   ;
    wchar_t const *         v_wchar_t_p              = L"1E"                ;
    std::wint_t             v_wint_t                 = 0x1F                 ;

    // These are positive test cases making sure valid code compiles.
    //  As the actual runtime just forwards it to sprintf there's not
    //  much value in checking the actual sprintf:ing.

    // char formatters
    TS_SPRINTF  (buffer,  "%%c: %c, %lc", v_int, v_wint_t);
    // string formatters
    TS_SPRINTF  (buffer,  "%%s: %s, %ls", v_char_p, v_wchar_t_p);
    // integer formatters
    TS_SPRINTF  (buffer,  "%%d: %hhd, %hd, %d, %ld, %lld, %jd, %zd, %td", v_signed_char, v_short, v_int, v_long, v_long_long, v_intmax_t, v_signed_size_t, v_ptrdiff_t);
    TS_SPRINTF  (buffer,  "%%i: %hhi, %hi, %i, %li, %lli, %ji, %zi, %ti", v_signed_char, v_short, v_int, v_long, v_long_long, v_intmax_t, v_signed_size_t, v_ptrdiff_t);
    TS_SPRINTF  (buffer,  "%%o: %hho, %ho, %o, %lo, %llo, %jo, %zo, %to", v_unsigned_char, v_unsigned_short, v_unsigned_int, v_unsigned_long, v_unsigned_long_long, v_uintmax_t, v_size_t, v_unsigned_ptrdiff_t);
    TS_SPRINTF  (buffer,  "%%x: %hhx, %hx, %x, %lx, %llx, %jx, %zx, %tx", v_unsigned_char, v_unsigned_short, v_unsigned_int, v_unsigned_long, v_unsigned_long_long, v_uintmax_t, v_size_t, v_unsigned_ptrdiff_t);
    TS_SPRINTF  (buffer,  "%%X: %hhX, %hX, %X, %lX, %llX, %jX, %zX, %tX", v_unsigned_char, v_unsigned_short, v_unsigned_int, v_unsigned_long, v_unsigned_long_long, v_uintmax_t, v_size_t, v_unsigned_ptrdiff_t);
    TS_SPRINTF  (buffer,  "%%u: %hhu, %hu, %u, %lu, %llu, %ju, %zu, %tu", v_unsigned_char, v_unsigned_short, v_unsigned_int, v_unsigned_long, v_unsigned_long_long, v_uintmax_t, v_size_t, v_unsigned_ptrdiff_t);
    // floating point formatters
    TS_SPRINTF  (buffer,  "%%f: %f, %lf, %Lf", v_double, v_double, v_long_double);
    TS_SPRINTF  (buffer,  "%%F: %F, %lF, %LF", v_double, v_double, v_long_double);
    TS_SPRINTF  (buffer,  "%%e: %e, %le, %Le", v_double, v_double, v_long_double);
    TS_SPRINTF  (buffer,  "%%E: %E, %lE, %LE", v_double, v_double, v_long_double);
    TS_SPRINTF  (buffer,  "%%a: %a, %la, %La", v_double, v_double, v_long_double);
    TS_SPRINTF  (buffer,  "%%A: %A, %lA, %LA", v_double, v_double, v_long_double);
    TS_SPRINTF  (buffer,  "%%g: %g, %lg, %Lg", v_double, v_double, v_long_double);
    TS_SPRINTF  (buffer,  "%%G: %G, %lG, %LG", v_double, v_double, v_long_double);
    // %n formatters
    TS_SPRINTF  (buffer,  "%%n: %hhn, %hn, %n, %ln, %lln, %jn, %zn, %tn", v_signed_char_p, v_short_p, v_int_p, v_long_p, v_long_long_p, v_intmax_t_p, v_signed_size_t_p, v_ptrdiff_t_p);
    // pointer formatters
    TS_SPRINTF  (buffer,  "%%p: %p", v_void_p);

    // non-const pointee check
    TS_SPRINTF  (buffer,  "non-const: %s, %ls, %p", unconst (v_char_p), unconst (v_wchar_t_p), unconst (v_void_p));
  /*
  TODO: Figure out a good way to test negative test-cases like below:

  TS_PRINTF (
      "Due to type mismatch compilation stops here... %d\n"
    , "Because this is not an int"
    );

  int v = 0;
  int const * p = &v;
  TS_PRINTF ("%n", p);  // p should be non-const *
  */


  }
}

int main()
{
#ifdef _MSC_VER
  // to enable %n support in Visual C++
  _set_printf_count_output (1);
#endif

  tests::test__printf_variants  ();
  tests::test__scanner_literals ();
  tests::test__scanner_any_of   ();
  tests::test__scanner          ();

  if (tests::errors == 0)
  {
    TS_PRINTF ("All tests completed successfully!\n");
  }
  else
  {
    TS_PRINTF ("%u tests failed!\n", tests::errors);
  }

  return 0;
}

