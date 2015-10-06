tsprintf
========

tsprintf is library that brings type-safety to printf

printf (and its variants) has many strengths (even in the day of C++11)

1. Often outperforms C++ streams
2. No dynamic allocation of memory
3. In some case more readable than C++ streams (in the eye of the beholder)
4. C++ streams can cause code-bloat

One of the major short-comings of printf is that there's no type-safety meaning
this will compile and most likely crash when executing:
```c++
  printf (
      "This is so wrong: %s"
    , 314
    );
```

An easy and catastrophic mistake to make.

Now, many compilers (VC, g++, clang++) checks that the format string and the
argument types matches. In addition we have tools like LINT. What is the need of
type-safe printf?

1. You might have other printf like functions. The compilers tend to ignore
   those as it doesn't know the semantics of your functions.
2. It's interesting to see how much checks can be performed in compile-time in
   C++14

The following code compiles as expected:
```c++
  TS_PRINTF ("Hello\n");
  TS_PRINTF ("Hello %s,%lld\n", "World", 3LL);
```

However the following illegal code fails to compile as the format string
doesn't match the argument types.
```c++
  TS_PRINTF (
      "Due to type mismatch compilation stops here... %d\n"
    , "Because this is not an int"
    );
```

In VS2015 the error looks like this:
```
	1>c:\temp\github\tsprintf\src\test_suite\../tsprintf/tsprintf.hpp(316): error C2338: Type mismatch between format string and provided argument
	1>  c:\temp\github\tsprintf\src\test_suite\../tsprintf/tsprintf.hpp(357): note: see reference to class template instantiation 'typesafe_printf::details::error_reporter<0,const char *,int>' being compiled
```

error_reporter<0,const char *,int> is intepreted like this: For argument 0 the
argument type was const char * but was expected to be int (because of %d in the
format string).

g++ gives a similar error report.

TODO
----

1. Implement TS_SPRINTF and all other variants
2. Improve test suite
3. Make it compile in clang++
4. Performance tests (to make sure compilation times doesn't fall through the floor)
5. Find a way to replace TS_PRINTF macro with a template method
