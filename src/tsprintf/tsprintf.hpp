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

#ifndef TYPESAFE_PRINTF__TSPRINTF_HPP
#define TYPESAFE_PRINTF__TSPRINTF_HPP

#include <cstdio>
#include <cstdint>
#include <cwchar>
#include <type_traits>

#define TYPESAFE_PRINTF__TYPE_MAP(key, value) \
  template<>                                  \
  struct type_id_map<key>                     \
  {                                           \
    using type = value;                       \
  }

#define TS_PRINTF(format, ...)                                                                                    \
  (void) typesafe_printf::details::check_types<typesafe_printf::details::scanner::encode (format)> (__VA_ARGS__); \
  printf (format, ##__VA_ARGS__)

namespace typesafe_printf
{
  namespace details
  {
    using index_type                        = std::uint32_t;
    using size_type                         = std::uint32_t;
    using encoded_types_t                   = std::uint64_t;

    struct error_type
    {
    };

    enum type_id : size_type
    {
      tid__error_type             = 0x01  ,
      tid__char_p                 = 0x02  ,
      tid__double                 = 0x03  ,
      tid__int                    = 0x04  ,
      tid__int_p                  = 0x05  ,
      tid__intmax_t               = 0x06  ,
      tid__intmax_t_p             = 0x07  ,
      tid__long                   = 0x08  ,
      tid__long_double            = 0x09  ,
      tid__long_long              = 0x0A  ,
      tid__long_long_p            = 0x0B  ,
      tid__long_p                 = 0x0C  ,
      tid__ptrdiff_t              = 0x0D  ,
      tid__ptrdiff_t_p            = 0x0E  ,
      tid__short                  = 0x0F  ,
      tid__short_p                = 0x10  ,
      tid__signed_char            = 0x11  ,
      tid__signed_char_p          = 0x12  ,
      tid__signed_size_t          = 0x13  ,
      tid__signed_size_t_p        = 0x14  ,
      tid__size_t                 = 0x15  ,
      tid__uintmax_t              = 0x16  ,
      tid__unsigned_char          = 0x17  ,
      tid__unsigned_int           = 0x18  ,
      tid__unsigned_long          = 0x19  ,
      tid__unsigned_long_long     = 0x1A  ,
      tid__unsigned_ptrdiff_t     = 0x1B  ,
      tid__unsigned_short         = 0x1C  ,
      tid__void_p                 = 0x1D  ,
      tid__wchar_t_p              = 0x1E  ,
      tid__wint_t                 = 0x1F  ,
    };

    constexpr encoded_types_t type_id__mask = 0x1F;
    constexpr size_type       type_id__bits = 5   ;

    constexpr size_type max_encoded_types   = (sizeof(encoded_types_t) * 8) / type_id__bits;

    template<encoded_types_t encoded_types>
    struct type_id_map
    {
      using type = error_type;
    };

    TYPESAFE_PRINTF__TYPE_MAP (tid__char_p              , char const *                );
    TYPESAFE_PRINTF__TYPE_MAP (tid__double              , double                      );
    TYPESAFE_PRINTF__TYPE_MAP (tid__int                 , int                         );
    TYPESAFE_PRINTF__TYPE_MAP (tid__int_p               , int const *                 );
    TYPESAFE_PRINTF__TYPE_MAP (tid__intmax_t            , std::intmax_t               );
    TYPESAFE_PRINTF__TYPE_MAP (tid__intmax_t_p          , std::intmax_t const *       );
    TYPESAFE_PRINTF__TYPE_MAP (tid__long                , long                        );
    TYPESAFE_PRINTF__TYPE_MAP (tid__long_double         , long double                 );
    TYPESAFE_PRINTF__TYPE_MAP (tid__long_long           , long long                   );
    TYPESAFE_PRINTF__TYPE_MAP (tid__long_long_p         , long long const *           );
    TYPESAFE_PRINTF__TYPE_MAP (tid__long_p              , long const *                );
    TYPESAFE_PRINTF__TYPE_MAP (tid__ptrdiff_t           , std::ptrdiff_t              );
    TYPESAFE_PRINTF__TYPE_MAP (tid__ptrdiff_t_p         , std::ptrdiff_t const *      );
    TYPESAFE_PRINTF__TYPE_MAP (tid__short               , short                       );
    TYPESAFE_PRINTF__TYPE_MAP (tid__short_p             , short const *               );
    TYPESAFE_PRINTF__TYPE_MAP (tid__signed_char         , signed char                 );
    TYPESAFE_PRINTF__TYPE_MAP (tid__signed_char_p       , signed char const *         );
    TYPESAFE_PRINTF__TYPE_MAP (tid__signed_size_t       , std::size_t                 );  // TODO: Should be signed std::size_t
    TYPESAFE_PRINTF__TYPE_MAP (tid__signed_size_t_p     , std::size_t const *         );  // TODO: Should be signed std::size_t
    TYPESAFE_PRINTF__TYPE_MAP (tid__size_t              , std::size_t                 );
    TYPESAFE_PRINTF__TYPE_MAP (tid__uintmax_t           , std::uintmax_t              );
    TYPESAFE_PRINTF__TYPE_MAP (tid__unsigned_char       , unsigned char               );
    TYPESAFE_PRINTF__TYPE_MAP (tid__unsigned_int        , unsigned int                );
    TYPESAFE_PRINTF__TYPE_MAP (tid__unsigned_long       , unsigned long               );
    TYPESAFE_PRINTF__TYPE_MAP (tid__unsigned_long_long  , unsigned long long          );
    TYPESAFE_PRINTF__TYPE_MAP (tid__unsigned_ptrdiff_t  , std::ptrdiff_t              );  // TODO: Should be unsigned std::ptrdiff_t
    TYPESAFE_PRINTF__TYPE_MAP (tid__unsigned_short      , unsigned short              );
    TYPESAFE_PRINTF__TYPE_MAP (tid__void_p              , void const *                );
    TYPESAFE_PRINTF__TYPE_MAP (tid__wchar_t_p           , wchar_t const *             );
    TYPESAFE_PRINTF__TYPE_MAP (tid__wint_t              , std::wint_t                 );

    template<encoded_types_t encoded_types>
    using type_id_map_t = typename type_id_map<encoded_types>::type;

    namespace scanner
    {
      // union of conversion specifier and argument type chars
      constexpr char const union_of_cs_at[]         = "AEFGLXacdefghijlnopstuxz";

      constexpr char const union_of_signed_ints[]   = "di"                      ;
      constexpr char const union_of_unsigned_ints[] = "Xoux"                    ;
      constexpr char const union_of_floats[]        = "AEFGaefg"                ;

      enum conversion_specifier : size_type
      {
                                      // Maps to conversion specifier
        cs__char              = 0x0 , /*c               */
        cs__string            = 0x1 , /*s               */
        cs__signed_integer    = 0x2 , /*d/i             */
        cs__unsigned_integer  = 0x3 , /*o/x/X/u         */
        cs__floating_point    = 0x4 , /*f/F/e/E/a/A/g/G */
        cs__chars_written     = 0x5 , /*n               */
        cs__pointer           = 0x6 , /*p               */
      };

      constexpr size_type conversion_specifier__count = 7;

      enum argument_type : size_type
      {
        at__hh    = 0x0 ,
        at__h     = 0x1 ,
        at__none  = 0x2 ,
        at__l     = 0x3 ,
        at__ll    = 0x4 ,
        at__j     = 0x5 ,
        at__z     = 0x6 ,
        at__t     = 0x7 ,
        at__L     = 0x8 ,
      };

      constexpr size_type argument_type__count = 9;

      // Table from: http://en.cppreference.com/w/cpp/io/c/fprintf
      constexpr type_id const type_ids[conversion_specifier__count][argument_type__count] =
      {
//                    hh                    h                     (none)              l                     ll                        j                 z                       t                         L
/*c               */{ tid__error_type     , tid__error_type     , tid__int          , tid__wint_t         , tid__error_type         , tid__error_type , tid__error_type       , tid__error_type         , tid__error_type   },
/*s               */{ tid__error_type     , tid__error_type     , tid__char_p       , tid__wchar_t_p      , tid__error_type         , tid__error_type , tid__error_type       , tid__error_type         , tid__error_type   },
/*d/i             */{ tid__signed_char    , tid__short          , tid__int          , tid__long           , tid__long_long          , tid__intmax_t   , tid__signed_size_t    , tid__ptrdiff_t          , tid__error_type   },
/*o/x/X/u         */{ tid__unsigned_char  , tid__unsigned_short , tid__unsigned_int , tid__unsigned_long  , tid__unsigned_long_long , tid__uintmax_t  , tid__size_t           , tid__unsigned_ptrdiff_t , tid__error_type   },
/*f/F/e/E/a/A/g/G */{ tid__error_type     , tid__error_type     , tid__double       , tid__double         , tid__error_type         , tid__error_type , tid__error_type       , tid__error_type         , tid__long_double  },
/*n               */{ tid__signed_char_p  , tid__short_p        , tid__int_p        , tid__long_p         , tid__long_long_p        , tid__intmax_t_p , tid__signed_size_t_p  , tid__ptrdiff_t_p        , tid__error_type   },
/*p               */{ tid__error_type     , tid__error_type     , tid__void_p       , tid__error_type     , tid__error_type         , tid__error_type , tid__error_type       , tid__error_type         , tid__error_type   },
      };

      constexpr type_id get_type_id (argument_type at, conversion_specifier cs) noexcept
      {
        return cs < conversion_specifier__count && at < argument_type__count
          ? type_ids[cs][at]
          : tid__error_type
          ;
      }

      constexpr encoded_types_t merge_type (encoded_types_t ec, size_type count, type_id ti) noexcept
      {
        return count < max_encoded_types
          ? ((ti & type_id__mask) << (count * type_id__bits)) | (~(type_id__mask << (count * type_id__bits)) & ec)
          : ec
          ;
      }

      template<size_type N>
      constexpr bool any_of (char ch, char const (&arr) [N], index_type i = 0) noexcept
      {
        return i < N && arr[i] != 0
          ? (arr[i] == ch ? true : any_of (ch, arr, i + 1))
          : false
          ;
      }

      template<size_type N>
      constexpr encoded_types_t scan (encoded_types_t ec, size_type count, char const (&arr) [N], index_type i) noexcept;

      template<size_type N>
      constexpr encoded_types_t error_detected (encoded_types_t ec, size_type count, char const (&arr) [N], index_type i) noexcept
      {
        return scan (merge_type (ec, count, tid__error_type), count + 1, arr, i);
      }

      template<size_type N>
      constexpr encoded_types_t compute_type_id (encoded_types_t ec, size_type count, argument_type at, conversion_specifier cs, char const (&arr) [N], index_type i) noexcept
      {
        return scan (merge_type (ec, count, get_type_id (at, cs)), count + 1, arr, i);
      }

      template<size_type N>
      constexpr encoded_types_t parse_conversion_specifier (encoded_types_t ec, size_type count, argument_type at, char const (&arr) [N], index_type i) noexcept
      {
        return i < N && arr[i] != 0
          ? (arr[i] == 'c'
            ? compute_type_id (ec, count, at, cs__char, arr, i + 1)
            : (arr[i] == 's'
              ? compute_type_id (ec, count, at, cs__string, arr, i + 1)
              : (any_of (arr[i], union_of_signed_ints)
                ? compute_type_id (ec, count, at, cs__signed_integer, arr, i + 1)
                : (any_of (arr[i], union_of_unsigned_ints)
                  ? compute_type_id (ec, count, at, cs__unsigned_integer, arr, i + 1)
                  : (any_of (arr[i], union_of_floats)
                    ? compute_type_id (ec, count, at, cs__floating_point, arr, i + 1)
                    : (arr[i] == 'n'
                      ? compute_type_id (ec, count, at, cs__chars_written, arr, i + 1)
                      : (arr[i] == 'p'
                        ? compute_type_id (ec, count, at, cs__pointer, arr, i + 1)
                        : error_detected (ec, count, arr, i))))))))
          : ec
          ;
      }


      template<size_type N>
      constexpr encoded_types_t parse_argument_type_2 (encoded_types_t ec, size_type count, char ch, char const (&arr) [N], index_type i) noexcept
      {
        return i < N && arr[i] != 0
          ? (arr[i] == 'h' && ch == 'h'
            ? parse_conversion_specifier (ec, count, at__hh, arr, i + 1)
            : (arr[i] == 'l' && ch == 'l'
              ? parse_conversion_specifier (ec, count, at__ll, arr, i + 1)
              : error_detected (ec, count, arr, i)))
          : ec
          ;
      }

      template<size_type N>
      constexpr encoded_types_t parse_argument_type (encoded_types_t ec, size_type count, char const (&arr) [N], index_type i) noexcept
      {
        return i < N && arr[i] != 0
          ? (arr[i] == 'h'
            ? parse_argument_type_2 (ec, count, 'h', arr, i + 1)
            : (arr[i] == 'l'
              ? parse_argument_type_2 (ec, count, 'l', arr, i + 1)
              : (arr[i] == 'j'
                ? parse_conversion_specifier (ec, count, at__j, arr, i + 1)
                : (arr[i] == 'z'
                  ? parse_conversion_specifier (ec, count, at__z, arr, i + 1)
                  : (arr[i] == 't'
                    ? parse_conversion_specifier (ec, count, at__t, arr, i + 1)
                    : (arr[i] == 'L'
                      ? parse_conversion_specifier (ec, count, at__L, arr, i + 1)
                      : parse_conversion_specifier (ec, count, at__none, arr, i)))))))
          : ec
          ;
      }

      template<size_type N>
      constexpr encoded_types_t consume_options (encoded_types_t ec, size_type count, char const (&arr) [N], index_type i) noexcept
      {
        return i < N && arr[i] != 0
          ? (!any_of (arr[i], union_of_cs_at) ? consume_options (ec, count, arr, i + 1) : parse_argument_type (ec, count, arr, i))
          : ec
          ;
      }

      template<size_type N>
      constexpr encoded_types_t start_token_found (encoded_types_t ec, size_type count, char const (&arr) [N], index_type i) noexcept
      {
        return i < N && arr[i] != 0
          ? (arr[i] != '%' ? consume_options (ec, count, arr, i) : scan (ec, count, arr, i + 1))
          : ec
          ;
      }

      template<size_type N>
      constexpr encoded_types_t scan (encoded_types_t ec, size_type count, char const (&arr) [N], index_type i) noexcept
      {
        return i < N && arr[i] != 0
          ? (arr[i] != '%' ? scan (ec, count, arr, i + 1) : start_token_found (ec, count, arr, i + 1))
          : ec
          ;
      }

      template<size_type N>
      constexpr encoded_types_t encode (char const (&arr) [N]) noexcept
      {
        return scan (0, 0, arr, 0);
      }
    }

    template<size_type Pos, typename TArg, typename TExpected>
    struct error_reporter
    {
      static_assert (!std::is_same<error_type, TExpected>::value  , "Malformed format string"                                   );
      static_assert (std::is_pod<TArg>::value                     , "Argument must be a POD type (see argument list)"           );
      static_assert (std::is_same<TArg, TExpected>::value         , "Type mismatch between format string and provided argument" );

      using type = TExpected;
    };

    template<bool HasMoreEncodedTypes, size_type Pos, encoded_types_t EncodedTypes, typename ...TArgs>
    struct type_checker;

    template<size_type Pos, typename ...TArgs>
    struct type_checker<false, Pos, 0U, TArgs...>
    {
      static_assert (0U == sizeof... (TArgs), "Too many arguments passed to ts_printf (see format string)");

      enum
      {
        zero = 0,
      };
    };

    template<size_type Pos, encoded_types_t EncodedTypes>
    struct type_checker<true, Pos, EncodedTypes>
    {
      static_assert (0U == EncodedTypes, "Too few arguments passed to ts_printf (see format string)");

      enum
      {
        zero = 0,
      };
    };

    template<size_type Pos, encoded_types_t EncodedTypes, typename THead, typename ...TArgs>
    struct type_checker<true, Pos, EncodedTypes, THead, TArgs...> : type_checker<(EncodedTypes >> type_id__bits) != 0, Pos + 1, (EncodedTypes >> type_id__bits), TArgs...>
    {
      enum : encoded_types_t
      {
        encoded_type  = EncodedTypes & type_id__mask ,
      };

      // using arg_type = std::decay_t<THead>;
      using arg_type = typename std::decay<THead>::type;  // std::decay_t doesn't exist in GCC 4.8.1 
      using exp_type = type_id_map_t<encoded_type>;

      using type    = typename error_reporter<Pos, arg_type, exp_type>::type;
    };


    template<encoded_types_t EncodedTypes, typename ...TArgs>
    constexpr int check_types (TArgs && ...args) noexcept
    {
      static_assert (sizeof... (TArgs) <= details::max_encoded_types, "Too many arguments passed to ts_printf (max_encoded_types is the upper limit)");
      return type_checker<EncodedTypes != 0, 0U, EncodedTypes, TArgs...>::zero;
    }
  }

  /*
  This would be preferable over the macro but can't get it to compile
  template<details::size_type N, typename ...TArgs>
  constexpr auto ts_printf (char const (&format) [N], TArgs && ...args) noexcept
  {
    using t = details::type_checker<0U, details::scanner::encode (format), TArgs...>;
    printf (format, std::forward<TArgs> (args)...);
  }
  */
  // ts_printf ("Hello %s,%x\n", "World", 3u);
}

#endif // TYPESAFE_PRINTF__TSPRINTF_HPP
