#include "StdAfx.h"

#include "../Common/StringConvert.h"

#include "myPrivate.h"

#ifdef ENV_HAVE_LOCALE
#include <locale.h>
#endif

extern void my_windows_split_path(const AString &p_path, AString &dir , AString &base);

void mySplitCommandLine(int numArguments,const char *arguments[],UStringVector &parts) {

  { // define P7ZIP_HOME_DIR
    static char p7zip_home_dir[MAX_PATH];
    AString dir,name;
    my_windows_split_path(arguments[0],dir,name);
    snprintf(p7zip_home_dir,sizeof(p7zip_home_dir),"P7ZIP_HOME_DIR=%s/",(const char *)dir);
    p7zip_home_dir[sizeof(p7zip_home_dir)-1] = 0;
    putenv(p7zip_home_dir);
  }

#ifdef ENV_HAVE_LOCALE
  // set the program's current locale from the user's environment variables
  setlocale(LC_ALL,"");


  // auto-detect which conversion p7zip should use
  char *locale = setlocale(LC_CTYPE,0);
  if (locale) {
    size_t len = strlen(locale);
    char *locale_upper = (char *)malloc(len+1);
    if (locale_upper) {
      strcpy(locale_upper,locale);

      for(size_t i=0;i<len;i++)
        locale_upper[i] = toupper(locale_upper[i] & 255);

      if (    (strcmp(locale_upper,"") != 0)
              && (strcmp(locale_upper,"C") != 0)
              && (strcmp(locale_upper,"POSIX") != 0) ) {
        global_use_utf16_conversion = 1;
      }
      free(locale_upper);
    }
  }
#elif defined(LOCALE_IS_UTF8)
  global_use_utf16_conversion = 1; // assume LC_CTYPE="utf8"
#else
  global_use_utf16_conversion = 0; // assume LC_CTYPE="C"
#endif

  parts.Clear();
  for(int ind=0;ind < numArguments; ind++) {
    if ((ind <= 2) && (strcmp(arguments[ind],"-no-utf16") == 0)) {
      global_use_utf16_conversion = 0;
    } else if ((ind <= 2) && (strcmp(arguments[ind],"-utf16") == 0)) {
      global_use_utf16_conversion = 1;
    } else {
      UString tmp = MultiByteToUnicodeString(arguments[ind]);
      // tmp.Trim(); " " is a valid filename ...
      if (!tmp.IsEmpty()) {
        parts.Add(tmp);
      }
    }
  }
}

const char *my_getlocale(void) {
#ifdef ENV_HAVE_LOCALE
  const char* ret = setlocale(LC_CTYPE,0);
  if (ret == 0)
    ret ="C";
  return ret;
#elif defined(LOCALE_IS_UTF8)
  return "utf8";
#else
  return "C";
#endif
}

