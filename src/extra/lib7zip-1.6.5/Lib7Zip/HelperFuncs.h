#ifndef __HELPER_FUNCS_H__
#define __HELPER_FUNCS_H__

#ifndef RBOOLOK
#define RBOOLOK(x) { int __result__ = (x); if (__result__ != 0) return false; }
#endif

#ifndef FAIL_RET
#define FAIL_RET(x, pResult) { HRESULT __result__ = (x); if (pResult) *pResult = __result__; if (__result__ != S_OK) return __result__; }
#endif

#ifndef _WIN32
#define WINAPI
#endif

typedef UInt32 (WINAPI *GetMethodPropertyFunc)(UInt32 index, PROPID propID, PROPVARIANT *value);
typedef UInt32 (WINAPI *GetNumberOfMethodsFunc)(UInt32 *numMethods);
typedef UInt32 (WINAPI *GetNumberOfFormatsFunc)(UInt32 *numFormats);
typedef UInt32 (WINAPI *GetHandlerPropertyFunc)(PROPID propID, PROPVARIANT *value);
typedef UInt32 (WINAPI *GetHandlerPropertyFunc2)(UInt32 index, PROPID propID, PROPVARIANT *value);
typedef UInt32 (WINAPI *CreateObjectFunc)(const GUID *clsID, const GUID *iid, void **outObject);
typedef UInt32 (WINAPI *SetLargePageModeFunc)();

const wchar_t kDirDelimiter = CHAR_PATH_SEPARATOR;

HRESULT ReadProp(GetHandlerPropertyFunc getProp,
				 GetHandlerPropertyFunc2 getProp2,
				 UInt32 index, PROPID propID, 
				 NWindows::NCOM::CPropVariant &prop);

HRESULT ReadBoolProp(GetHandlerPropertyFunc getProp,
					 GetHandlerPropertyFunc2 getProp2,
					 UInt32 index, PROPID propID, bool &res);

HRESULT ReadStringProp(GetHandlerPropertyFunc getProp,
					   GetHandlerPropertyFunc2 getProp2,
					   UInt32 index, PROPID propID, wstring &res);

void SplitString(const wstring &srcString, WStringArray &destStrings);

HRESULT GetCoderClass(GetMethodPropertyFunc getMethodProperty, UInt32 index,
					  PROPID propId, GUID & clsId, bool &isAssigned);

HRESULT IsArchiveItemFolder(IInArchive *archive, UInt32 index, bool &result);

int MyStringCompareNoCase(const wchar_t *s1, const wchar_t *s2);

HRESULT GetMethodPropertyString(GetMethodPropertyFunc getMethodProperty, UInt32 index,
                             PROPID propId, wstring & val);
HRESULT GetMethodPropertyGUID(GetMethodPropertyFunc getMethodProperty, UInt32 index,
                             PROPID propId, GUID & val);

UInt64 ConvertPropVariantToUInt64(const PROPVARIANT &);

HRESULT GetFilePathExt(const wstring & path, wstring & ext);
HRESULT GetArchiveItemPath(IInArchive *archive, UInt32 index, wstring &result);
HRESULT GetArchiveItemPath(IInArchive *archive, UInt32 index, const wstring &defaultName, wstring &result);
wstring WidenString( const string& str );
string NarrowString( const wstring& str );

#endif

