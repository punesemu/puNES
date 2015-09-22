// CompressCall.cpp

#include "StdAfx.h"

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#undef _WIN32

#include "CompressCall.h"

// FIXME #include "Common/Random.h"
#include "Common/IntToString.h"
#include "Common/MyCom.h"
#include "Common/StringConvert.h"

#include "Windows/Synchronization.h"
// FIXME #include "Windows/FileMapping.h"
#include "Windows/FileDir.h"

#include "../FileManager/ProgramLocation.h"
#include "../FileManager/RegistryUtils.h"

#define NEED_NAME_WINDOWS_TO_UNIX
#include "myPrivate.h"

#ifndef _UNICODE
extern bool g_IsNT;
#endif // _UNICODE

using namespace NWindows;

static LPCWSTR kShowDialogSwitch = L" -ad";
static LPCWSTR kEmailSwitch = L" -seml.";
static LPCWSTR kMapSwitch = L" -i#";
static LPCWSTR kArchiveNoNameSwitch = L" -an";
static LPCWSTR kArchiveTypeSwitch = L" -t";
static LPCWSTR kArchiveMapSwitch = L" -ai#";
static LPCWSTR kStopSwitchParsing = L" --";
static LPCWSTR kLargePagesDisable = L" -slp-";

static void AddLagePagesSwitch(UString &params)
{
#ifdef _WIN32
  if (!ReadLockMemoryEnable())
    params += kLargePagesDisable;
#endif
}

HRESULT MyCreateProcess(const UString &params,
    LPCWSTR curDir, bool waitFinish,
    NWindows::NSynchronization::CBaseEvent *event)
{
	printf("MyCreateProcess: waitFinish=%d event=%p\n",(unsigned)waitFinish,event);
	printf("\tparams : %ls\n",(const wchar_t*)params);
	printf("\tcurDir : %ls\n",(const wchar_t*)curDir);

	wxString cmd(params);
	wxString memoCurDir = wxGetCwd();

	if (curDir) {  // FIXME
		wxSetWorkingDirectory(wxString(curDir));
		
		
		// under MacOSX, a bundle does not keep the current directory
		// between 7zFM and 7zG ...
		// So, try to use the environment variable P7ZIP_CURRENT_DIR	
	
		char p7zip_current_dir[MAX_PATH];
			
		AString aCurPath = GetAnsiString(curDir);
			
		const char *dir2 = nameWindowToUnix((const char *)aCurPath);
			
		snprintf(p7zip_current_dir,sizeof(p7zip_current_dir),"P7ZIP_CURRENT_DIR=%s/",dir2);
			
		p7zip_current_dir[sizeof(p7zip_current_dir)-1] = 0;
			
		putenv(p7zip_current_dir);
			
		printf("putenv(%s)\n",p7zip_current_dir);
		
	}	
	

	printf("MyCreateProcess: cmd='%ls'\n",(const wchar_t *)cmd);
	long pid = 0;
	if (waitFinish) pid = wxExecute(cmd, wxEXEC_SYNC); // FIXME process never ends and stays zombie ...
	else            pid = wxExecute(cmd, wxEXEC_ASYNC);

	if (curDir) wxSetWorkingDirectory(memoCurDir);


	// FIXME if (pid == 0) return E_FAIL;

	return S_OK;
#ifdef _WIN32 // FIXME
  const UString params2 = params;
  BOOL result;
  {
    STARTUPINFOW startupInfo;
    startupInfo.cb = sizeof(startupInfo);
    startupInfo.lpReserved = 0;
    startupInfo.lpDesktop = 0;
    startupInfo.lpTitle = 0;
    startupInfo.dwFlags = 0;
    startupInfo.cbReserved2 = 0;
    startupInfo.lpReserved2 = 0;
    
    result = ::CreateProcessW(NULL, (LPWSTR)(LPCWSTR)params,
      NULL, NULL, FALSE, 0, NULL,
      curDir,
      &startupInfo, &processInformation);
  }
  if (result == 0)
    return ::GetLastError();
  else
  {
    ::CloseHandle(processInformation.hThread);
    if (waitFinish)
      WaitForSingleObject(processInformation.hProcess, INFINITE);
    else if (event != NULL)
    {
      HANDLE handles[] = {processInformation.hProcess, *event };
      ::WaitForMultipleObjects(sizeof(handles) / sizeof(handles[0]),
        handles, FALSE, INFINITE);
    }
    ::CloseHandle(processInformation.hProcess);
  }
  return S_OK;
#endif
}

UString GetQuotedString(const UString &s)
{
  return UString(L"\"") + s + UString(L"\"");
}

static UString Get7zGuiPath()
{
  UString path;
  UString folder;
  if (GetProgramFolderPath(folder))
    path += folder;
#ifdef _WIN32
  path += L"7zG.exe";
#else
  path += L"7zG";
#endif
  return GetQuotedString(path);
}

#ifdef _WIN32
static HRESULT CreateTempEvent(const wchar_t *name,
    NSynchronization::CManualResetEvent &event, UString &eventName)
{
  CRandom random;
  random.Init(GetTickCount());
  for (;;)
  {
    int number = random.Generate();
    wchar_t temp[32];
    ConvertUInt64ToString((UInt32)number, temp);
    eventName = name;
    eventName += temp;
    RINOK(event.CreateWithName(false, GetSystemString(eventName)));
    if (::GetLastError() != ERROR_ALREADY_EXISTS)
      return S_OK;
    event.Close();
  }
}

static HRESULT CreateMap(const UStringVector &names,
    const UString &id,
    CFileMapping &fileMapping, NSynchronization::CManualResetEvent &event,
    UString &params)
{
  UInt32 extraSize = 2;
  UInt32 dataSize = 0;
  for (int i = 0; i < names.Size(); i++)
    dataSize += (names[i].Length() + 1) * sizeof(wchar_t);
  UInt32 totalSize = extraSize + dataSize;
  
  UString mappingName;
  
  CRandom random;
  random.Init(GetTickCount());
  for (;;)
  {
    int number = random.Generate();
    wchar_t temp[32];
    ConvertUInt64ToString(UInt32(number), temp);
    mappingName = id;
    mappingName += L"Mapping";
    mappingName += temp;
    if (!fileMapping.Create(INVALID_HANDLE_VALUE, NULL,
        PAGE_READWRITE, totalSize, GetSystemString(mappingName)))
      return E_FAIL;
    if (::GetLastError() != ERROR_ALREADY_EXISTS)
      break;
    fileMapping.Close();
  }
  
  UString eventName;
  RINOK(CreateTempEvent(id + L"MappingEndEvent", event, eventName));

  params += mappingName;
  params += L":";
  wchar_t string[10];
  ConvertUInt64ToString(totalSize, string);
  params += string;
  
  params += L":";
  params += eventName;

  LPVOID data = fileMapping.MapViewOfFile(FILE_MAP_WRITE, 0, totalSize);
  if (data == NULL)
    return E_FAIL;
  {
    wchar_t *curData = (wchar_t *)data;
    *curData = 0;
    curData++;
    for (int i = 0; i < names.Size(); i++)
    {
      const UString &s = names[i];
      memcpy(curData, (const wchar_t *)s, s.Length() * sizeof(wchar_t));
      curData += s.Length();
      *curData++ = L'\0';
    }
  }
  return S_OK;
}
#endif

HRESULT CompressFiles(
    const UString &curDir,
    const UString &archiveName,
    const UString &archiveType,
    const UStringVector &names,
    // const UString &outFolder,
    bool email,
    bool showDialog,
    bool waitFinish)
{
  /*
  UString curDir;
  if (names.Size() > 0)
  {
    NFile::NDirectory::GetOnlyDirPrefix(names[0], curDir);
  }
  */
  UString params;
  params = Get7zGuiPath();
  params += L" a";
#ifdef _WIN32
  params += kMapSwitch;
  // params += _fileNames[0];
  
  UInt32 extraSize = 2;
  UInt32 dataSize = 0;
  for (int i = 0; i < names.Size(); i++)
    dataSize += (names[i].Length() + 1) * sizeof(wchar_t);
  UInt32 totalSize = extraSize + dataSize;
  
  UString mappingName;
  
  CFileMapping fileMapping;
  CRandom random;
  random.Init(GetTickCount());
  for (;;)
  {
    int number = random.Generate();
    wchar_t temp[32];
    ConvertUInt64ToString(UInt32(number), temp);
    mappingName = L"7zCompressMapping";
    mappingName += temp;
    if (!fileMapping.Create(INVALID_HANDLE_VALUE, NULL,
      PAGE_READWRITE, totalSize, GetSystemString(mappingName)))
    {
      // MyMessageBox(IDS_ERROR, 0x02000605);
      return E_FAIL;
    }
    if (::GetLastError() != ERROR_ALREADY_EXISTS)
      break;
    fileMapping.Close();
  }
  
  NSynchronization::CManualResetEvent event;
  UString eventName;
  RINOK(CreateTempEvent(L"7zCompressMappingEndEvent", event, eventName));

  params += mappingName;
  params += L":";
  wchar_t string[10];
  ConvertUInt64ToString(totalSize, string);
  params += string;
  
  params += L":";
  params += eventName;
#else
  char tempFile[256];
  static int count = 1000;
  
  sprintf(tempFile,"/tmp/7zCompress_%d_%d.tmp",(int)getpid(),count++);

  FILE * file = fopen(tempFile,"w");
  if (file)
  {
    for (int i = 0; i < names.Size(); i++) {
	  fprintf(file,"%ls\n",(const wchar_t *)names[i]);
	  printf(" TMP_%d : '%ls'\n",i,(const wchar_t *)names[i]);
   }

    fclose(file);
  }
  params += L" -i@";
  params += GetUnicodeString(tempFile);
#endif

  if (!archiveType.IsEmpty())
  {
    params += kArchiveTypeSwitch;
    params += archiveType;
  }

  if (email)
    params += kEmailSwitch;

  if (showDialog)
    params += kShowDialogSwitch;

  AddLagePagesSwitch(params);

  params += kStopSwitchParsing;
  params += L" ";
  
  params += GetQuotedString(archiveName);
  
#ifdef _WIN32
  LPVOID data = fileMapping.MapViewOfFile(FILE_MAP_WRITE, 0, totalSize);
  if (data == NULL)
  {
    // MyMessageBox(IDS_ERROR, 0x02000605);
    return E_FAIL;
  }
  try
  {
    wchar_t *curData = (wchar_t *)data;
    *curData = 0;
    curData++;
    for (int i = 0; i < names.Size(); i++)
    {
      const UString &unicodeString = names[i];
      memcpy(curData, (const wchar_t *)unicodeString ,
        unicodeString .Length() * sizeof(wchar_t));
      curData += unicodeString.Length();
      *curData++ = L'\0';
    }
    // MessageBox(0, params, 0, 0);
    RINOK(MyCreateProcess(params,
      (curDir.IsEmpty()? 0: (LPCWSTR)curDir),
      waitFinish, &event));
  }
  catch(...)
  {
    UnmapViewOfFile(data);
    throw;
  }
  UnmapViewOfFile(data);
  

  /*
  CThreadCompressMain *compressor = new CThreadCompressMain();;
  compressor->FileNames = _fileNames;
  CThread thread;
  if (!thread.Create(CThreadCompressMain::MyThreadFunction, compressor))
  throw 271824;
  */
#else
  printf("CompressFiles : -%ls-\n",(const wchar_t *)params);
  HRESULT res = MyCreateProcess(params,
      (curDir.IsEmpty()? 0: (LPCWSTR)curDir),
	  true, /* &event FIXME */ 0);
  printf("CompressFiles : END\n");

  remove(tempFile);
#endif
  return S_OK;
}

static HRESULT ExtractGroupCommand(const UStringVector &archivePaths,
    const UString &params)
{
  UString params2 = params;
  AddLagePagesSwitch(params2);
  params2 += kArchiveNoNameSwitch;
#ifdef _WIN32
  params2 += kArchiveMapSwitch;
  CFileMapping fileMapping;
  NSynchronization::CManualResetEvent event;
  RINOK(CreateMap(archivePaths, L"7zExtract", fileMapping, event, params2));
  return MyCreateProcess(params2, 0, false, &event);
#else
  char tempFile[256];
  static int count = 1000;
  
  sprintf(tempFile,"/tmp/7zExtract_%d_%d.tmp",(int)getpid(),count++);

  FILE * file = fopen(tempFile,"w");
  if (file)
  {
    for (int i = 0; i < archivePaths.Size(); i++) {
	  fprintf(file,"%ls\n",(const wchar_t *)archivePaths[i]);
	  printf(" TMP_%d : '%ls'\n",i,(const wchar_t *)archivePaths[i]);
    }

    fclose(file);
  }
  params2 += L" -ai@";
  params2 += GetUnicodeString(tempFile);
  printf("ExtractGroupCommand : -%ls-\n",(const wchar_t *)params2);
  HRESULT res = MyCreateProcess(params2, 0, true, /* &event FIXME */ 0);
  printf("ExtractGroupCommand : END\n");

  remove(tempFile);

  return res;
#endif
}

HRESULT ExtractArchives(const UStringVector &archivePaths,
    const UString &outFolder, bool showDialog)
{
  UString params;
  params = Get7zGuiPath();
  params += L" x";
  if (!outFolder.IsEmpty())
  {
    params += L" \"-o";
    params += outFolder;
    params += L"\"";
  }
  if (showDialog)
    params += kShowDialogSwitch;
  return ExtractGroupCommand(archivePaths, params);
}

HRESULT TestArchives(const UStringVector &archivePaths)
{
  UString params;
  params = Get7zGuiPath();
  params += L" t";
  return ExtractGroupCommand(archivePaths, params);
}

HRESULT Benchmark()
{
  UString params;
  params = Get7zGuiPath();
  params += L" b";
  return MyCreateProcess(params, 0, false, NULL);
}
