// Common/StringConvert.cpp

#include "StdAfx.h"
#include <stdlib.h>

#include "StringConvert.h"
extern "C"
{
int global_use_utf16_conversion = 0;
}


#ifdef LOCALE_IS_UTF8

#ifdef __APPLE_CC__
#define UInt32  macUIn32
#include <CoreFoundation/CoreFoundation.h>
#undef UInt32

UString MultiByteToUnicodeString(const AString &srcString, UINT codePage)
{
  if (!srcString.IsEmpty())
  {
    UString resultString;
    const char * path = &srcString[0];

// FIXME    size_t n = strlen(path);

    CFStringRef cfpath = CFStringCreateWithCString(NULL,path,kCFStringEncodingUTF8);

    if (cfpath)
    {

       CFMutableStringRef cfpath2 = CFStringCreateMutableCopy(NULL,0,cfpath);
       CFRelease(cfpath);
       CFStringNormalize(cfpath2,kCFStringNormalizationFormC);
    
       size_t n = CFStringGetLength(cfpath2);
       for(size_t i =   0 ; i< n ;i++) {
         resultString += CFStringGetCharacterAtIndex(cfpath2,i);
       }

       CFRelease(cfpath2);  

       return resultString;
    }
  }

  UString resultString;
  for (int i = 0; i < srcString.Length(); i++)
    resultString += wchar_t(srcString[i] & 255);

  return resultString;
}

AString UnicodeStringToMultiByte(const UString &srcString, UINT codePage)
{
  if (!srcString.IsEmpty())
  {
    const wchar_t * wcs = &srcString[0];
    char utf8[4096];
    UniChar unipath[4096];

    size_t n = wcslen(wcs);

    for(size_t i =   0 ; i<= n ;i++) {
      unipath[i] = wcs[i];
    }

    CFStringRef cfpath = CFStringCreateWithCharacters(NULL,unipath,n);

    CFMutableStringRef cfpath2 = CFStringCreateMutableCopy(NULL,0,cfpath);
    CFRelease(cfpath);
    CFStringNormalize(cfpath2,kCFStringNormalizationFormD);
    
    CFStringGetCString(cfpath2,(char *)utf8,4096,kCFStringEncodingUTF8);

    CFRelease(cfpath2);  

    return AString(utf8);
  }

  AString resultString;
  for (int i = 0; i < srcString.Length(); i++)
  {
    if (srcString[i] >= 256) resultString += '?';
    else                     resultString += char(srcString[i]);
  }
  return resultString;
}

#else /* __APPLE_CC__ */


#include "UTFConvert.h"

UString MultiByteToUnicodeString(const AString &srcString, UINT codePage)
{
  if ((global_use_utf16_conversion) && (!srcString.IsEmpty()))
  {
    UString resultString;
    bool bret = ConvertUTF8ToUnicode(srcString,resultString);
    if (bret) return resultString;
  }

  UString resultString;
  for (int i = 0; i < srcString.Length(); i++)
    resultString += wchar_t(srcString[i] & 255);

  return resultString;
}

AString UnicodeStringToMultiByte(const UString &srcString, UINT codePage)
{
  if ((global_use_utf16_conversion) && (!srcString.IsEmpty()))
  {
    AString resultString;
    bool bret = ConvertUnicodeToUTF8(srcString,resultString);
    if (bret) return resultString;
  }

  AString resultString;
  for (int i = 0; i < srcString.Length(); i++)
  {
    if (srcString[i] >= 256) resultString += '?';
    else                     resultString += char(srcString[i]);
  }
  return resultString;
}

#endif /* __APPLE_CC__ */

#else /* LOCALE_IS_UTF8 */

UString MultiByteToUnicodeString(const AString &srcString, UINT /* codePage */ )
{
#ifdef ENV_HAVE_MBSTOWCS
  if ((global_use_utf16_conversion) && (!srcString.IsEmpty()))
  {
    UString resultString;
    int numChars = mbstowcs(resultString.GetBuffer(srcString.Length()),srcString,srcString.Length()+1);
    if (numChars >= 0) {
      resultString.ReleaseBuffer(numChars);
      return resultString;
    }
  }
#endif

  UString resultString;
  for (int i = 0; i < srcString.Length(); i++)
    resultString += wchar_t(srcString[i] & 255);

  return resultString;
}

AString UnicodeStringToMultiByte(const UString &srcString, UINT /* codePage */ )
{
#ifdef ENV_HAVE_WCSTOMBS
  if ((global_use_utf16_conversion) && (!srcString.IsEmpty()))
  {
    AString resultString;
    int numRequiredBytes = srcString.Length() * 6+1;
    int numChars = wcstombs(resultString.GetBuffer(numRequiredBytes),srcString,numRequiredBytes);
    if (numChars >= 0) {
      resultString.ReleaseBuffer(numChars);
      return resultString;
    }
  }
#endif

  AString resultString;
  for (int i = 0; i < srcString.Length(); i++)
  {
    if (srcString[i] >= 256) resultString += '?';
    else                     resultString += char(srcString[i]);
  }
  return resultString;
}

#endif /* LOCALE_IS_UTF8 */

