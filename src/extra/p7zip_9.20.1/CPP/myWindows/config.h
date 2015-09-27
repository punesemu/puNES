
#if !defined(__DJGPP__)

#ifndef __CYGWIN__
  #define FILESYSTEM_IS_CASE_SENSITIVE 1
#endif

  #if !defined(ENV_MACOSX) && !defined(ENV_BEOS)

    /* <wchar.h> */
    /* ENV_HAVE_WCHAR__H and not ENV_HAVE_WCHAR_H to avoid warning with wxWidgets */
    #define ENV_HAVE_WCHAR__H

    /* <wctype.h> */
    #define ENV_HAVE_WCTYPE_H

    /* mbrtowc */
/* #ifndef __hpux */
/*    #define ENV_HAVE_MBRTOWC */
/* #endif */

    /* towupper */
    #define ENV_HAVE_TOWUPPER

  #endif /* !ENV_MACOSX && !ENV_BEOS */

  #if !defined(ENV_BEOS)
    #define ENV_HAVE_GETPASS

    #if !defined(sun)
      #define ENV_HAVE_TIMEGM
    #endif


  #endif

  /* lstat, readlink and S_ISLNK */
  #define ENV_HAVE_LSTAT

  /* <locale.h> */
  #define ENV_HAVE_LOCALE

  /* mbstowcs */
  #define ENV_HAVE_MBSTOWCS

  /* wcstombs */
  #define ENV_HAVE_WCSTOMBS

#endif /* !__DJGPP__ */

#ifndef ENV_BEOS
#define ENV_HAVE_PTHREAD
#endif

#if defined(ENV_MACOSX)
#define LOCALE_IS_UTF8
#endif

#ifdef LOCALE_IS_UTF8
#undef ENV_HAVE_LOCALE
#undef ENV_HAVE_MBSTOWCS
#undef ENV_HAVE_WCSTOMBS
/* #undef ENV_HAVE_MBRTOWC */
#endif

#define MAX_PATHNAME_LEN   1024

