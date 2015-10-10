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

    if (expected == actual)
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

  void test__printf_variants ()
  {
    TEST_CASE ();

    char buffer[100] {};

    TS_PRINTF   (             "None\n"                        );
    TS_PRINTF   (             "Pair %s,%lld\n"  , "World", 3LL);
    TS_SPRINTF  (buffer,      "Int: %d\n"       , 3           );
    TS_SNPRINTF (buffer, 20,  "Char: %c\n"      , 65          );
    TS_FPRINTF  (stdout,      "Float: %f\n"     , 3.14        );

  /*
  TODO: Figure out a good way to test negative test-cases
  TS_PRINTF (
      "Due to type mismatch compilation stops here... %d\n"
    , "Because this is not an int"
    );
  */


  }
}

int main()
{
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

