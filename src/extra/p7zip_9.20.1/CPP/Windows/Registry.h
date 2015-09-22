// Windows/Registry.h

#ifndef __WINDOWS_REGISTRY_H
#define __WINDOWS_REGISTRY_H

#include "Common/Buffer.h"
#include "Common/MyString.h"
#include "Common/Types.h"

#ifndef _WIN32
class HKEY_Impl;

typedef HKEY_Impl * HKEY;

/*
#define HKEY_CLASSES_ROOT       ((HKEY) 0x80000000)
#define HKEY_LOCAL_MACHINE      ((HKEY) 0x80000002)
#define HKEY_USERS              ((HKEY) 0x80000003)
#define HKEY_PERFORMANCE_DATA   ((HKEY) 0x80000004)
#define HKEY_CURRENT_CONFIG     ((HKEY) 0x80000005)
#define HKEY_DYN_DATA           ((HKEY) 0x80000006)
*/
#define HKEY_CURRENT_USER       ((HKEY) 0x80000001)


typedef DWORD REGSAM;
#define ERROR_SUCCESS (0)
#define KEY_READ	(0x1234) // FIXME
#define KEY_ALL_ACCESS  (~0)     // FIXME

/* ------------------------------ end registry ------------------------------ */

#endif

namespace NWindows {
namespace NRegistry {

const TCHAR kKeyNameDelimiter = TEXT(CHAR_PATH_SEPARATOR);

// LONG SetValue(HKEY parentKey, LPCTSTR keyName, LPCTSTR valueName, LPCTSTR value);

class CKey
{
  HKEY _object;
public:
  CKey(): _object(NULL) {}
  ~CKey() { Close(); }

  operator HKEY() const { return _object; }

 #if 0
  HKEY Detach();
  void Attach(HKEY key);
  LONG Create(HKEY parentKey, LPCTSTR keyName,
      LPTSTR keyClass = REG_NONE, DWORD options = REG_OPTION_NON_VOLATILE,
      REGSAM accessMask = KEY_ALL_ACCESS,
      LPSECURITY_ATTRIBUTES securityAttributes = NULL,
      LPDWORD disposition = NULL);
  LONG Open(HKEY parentKey, LPCTSTR keyName,
      REGSAM accessMask = KEY_ALL_ACCESS);
#endif // #if 0
  LONG Create(HKEY parentKey, LPCTSTR keyName);
  LONG Open(HKEY parentKey, LPCTSTR keyName,
      REGSAM accessMask = KEY_ALL_ACCESS);

  LONG Close();

  LONG DeleteSubKey(LPCTSTR subKeyName);
  LONG RecurseDeleteKey(LPCTSTR subKeyName);

  LONG DeleteValue(LPCTSTR name);
  #ifndef _UNICODE
  LONG DeleteValue(LPCWSTR name);
  #endif

  LONG SetValue(LPCTSTR valueName, UInt32 value);
  LONG SetValue(LPCTSTR valueName, bool value);
  LONG SetValue(LPCTSTR valueName, LPCTSTR value);
  // LONG SetValue(LPCTSTR valueName, const CSysString &value);
  #ifndef _UNICODE
  LONG SetValue(LPCWSTR name, LPCWSTR value);
  // LONG SetValue(LPCWSTR name, const UString &value);
  #endif

  LONG SetValue(LPCTSTR name, const void *value, UInt32 size);

  LONG SetValue_Strings(LPCTSTR valueName, const UStringVector &strings);
  LONG GetValue_Strings(LPCTSTR valueName, UStringVector &strings);

  LONG SetKeyValue(LPCTSTR keyName, LPCTSTR valueName, LPCTSTR value);

  LONG QueryValue(LPCTSTR name, UInt32 &value);
  LONG QueryValue(LPCTSTR name, bool &value);
  LONG QueryValue(LPCTSTR name, LPTSTR value, UInt32 &dataSize);
  LONG QueryValue(LPCTSTR name, CSysString &value);

  LONG GetValue_IfOk(LPCTSTR name, UInt32 &value);
  LONG GetValue_IfOk(LPCTSTR name, bool &value);

  #ifndef _UNICODE
  LONG QueryValue(LPCWSTR name, LPWSTR value, UInt32 &dataSize);
  LONG QueryValue(LPCWSTR name, UString &value);
  #endif

  LONG QueryValue(LPCTSTR name, void *value, UInt32 &dataSize);
  LONG QueryValue(LPCTSTR name, CByteBuffer &value, UInt32 &dataSize);

  LONG EnumKeys(CSysStringVector &keyNames);
};

}}

#endif
