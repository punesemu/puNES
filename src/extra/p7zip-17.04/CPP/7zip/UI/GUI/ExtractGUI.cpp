// ExtractGUI.cpp

#include "StdAfx.h"

#include "../../../Common/IntToString.h"
#include "../../../Common/StringConvert.h"

#include "../../../Windows/FileDir.h"
#include "../../../Windows/FileFind.h"
#include "../../../Windows/FileName.h"
#include "../../../Windows/Thread.h"

#include "../FileManager/ExtractCallback.h"
#include "../FileManager/FormatUtils.h"
#include "../FileManager/LangUtils.h"
#include "../FileManager/resourceGui.h"
#include "../FileManager/OverwriteDialogRes.h"

#include "../Common/ArchiveExtractCallback.h"
#include "../Common/PropIDUtils.h"

#include "../Explorer/MyMessages.h"

#include "resource2.h"
#include "ExtractRes.h"

#include "ExtractDialog.h"
#include "ExtractGUI.h"
#include "HashGUI.h"

#include "../FileManager/PropertyNameRes.h"

using namespace NWindows;
using namespace NFile;
using namespace NDir;

static const wchar_t *kIncorrectOutDir = L"Incorrect output directory path";

#ifndef _SFX

static void AddValuePair(UString &s, UINT resourceID, UInt64 value, bool addColon = true)
{
  AddLangString(s, resourceID);
  if (addColon)
    s += L':';
  s.Add_Space();
  char sz[32];
  ConvertUInt64ToString(value, sz);
  s.AddAscii(sz);
  s.Add_LF();
}

static void AddSizePair(UString &s, UINT resourceID, UInt64 value)
{
  wchar_t sz[32];
  AddLangString(s, resourceID);
  s += L": ";
  ConvertUInt64ToString(value, sz);
  s += MyFormatNew(IDS_FILE_SIZE, sz);
  // s += sz;
  if (value >= (1 << 20))
  {
    ConvertUInt64ToString(value >> 20, sz);
    s += L" (";
    s += sz;
    s += L" MB)";
  }
  s.Add_LF();
}

#endif

class CThreadExtracting: public CProgressThreadVirt
{
  HRESULT ProcessVirt();
public:
  CCodecs *codecs;
  CExtractCallbackImp *ExtractCallbackSpec;
  const CObjectVector<COpenType> *FormatIndices;
  const CIntVector *ExcludedFormatIndices;

  UStringVector *ArchivePaths;
  UStringVector *ArchivePathsFull;
  const NWildcard::CCensorNode *WildcardCensor;
  const CExtractOptions *Options;
  #ifndef _SFX
  CHashBundle *HashBundle;
  #endif
  CMyComPtr<IExtractCallbackUI> ExtractCallback;
  UString Title;
};

HRESULT CThreadExtracting::ProcessVirt()
{
  CDecompressStat Stat;
  #ifndef _SFX
  if (HashBundle)
    HashBundle->Init();
  #endif

  HRESULT res = Extract(codecs,
      *FormatIndices, *ExcludedFormatIndices,
      *ArchivePaths, *ArchivePathsFull,
      *WildcardCensor, *Options, ExtractCallbackSpec, ExtractCallback,
      #ifndef _SFX
        HashBundle,
      #endif
      FinalMessage.ErrorMessage.Message, Stat);
  #ifndef _SFX
  if (res == S_OK && Options->TestMode && ExtractCallbackSpec->IsOK())
  {
    UString s;
    
    AddValuePair(s, IDS_ARCHIVES_COLON, Stat.NumArchives, false);
    AddSizePair(s, IDS_PROP_PACKED_SIZE, Stat.PackSize);

    if (!HashBundle)
    {
      if (Stat.NumFolders != 0)
        AddValuePair(s, IDS_PROP_FOLDERS, Stat.NumFolders);
      AddValuePair(s, IDS_PROP_FILES, Stat.NumFiles);
      AddSizePair(s, IDS_PROP_SIZE, Stat.UnpackSize);
      if (Stat.NumAltStreams != 0)
      {
        s.Add_LF();
        AddValuePair(s, IDS_PROP_NUM_ALT_STREAMS, Stat.NumAltStreams);
        AddSizePair(s, IDS_PROP_ALT_STREAMS_SIZE, Stat.AltStreams_UnpackSize);
      }
    }
    
    if (HashBundle)
    {
      s.Add_LF();
      AddHashBundleRes(s, *HashBundle, UString());
    }
    
    s.Add_LF();
    AddLangString(s, IDS_MESSAGE_NO_ERRORS);
    
    FinalMessage.OkMessage.Title = Title;
    FinalMessage.OkMessage.Message = s;
  }
  #endif
  return res;
}

HRESULT ExtractGUI(
    CCodecs *codecs,
    const CObjectVector<COpenType> &formatIndices,
    const CIntVector &excludedFormatIndices,
    UStringVector &archivePaths,
    UStringVector &archivePathsFull,
    const NWildcard::CCensorNode &wildcardCensor,
    CExtractOptions &options,
    #ifndef _SFX
    CHashBundle *hb,
    #endif
    bool showDialog,
    bool &messageWasDisplayed,
    CExtractCallbackImp *extractCallback,
    HWND hwndParent)
{
  messageWasDisplayed = false;

  CThreadExtracting extracter;
  extracter.codecs = codecs;
  extracter.FormatIndices = &formatIndices;
  extracter.ExcludedFormatIndices = &excludedFormatIndices;

  if (!options.TestMode)
  {
    FString outputDir = options.OutputDir;
    #ifndef UNDER_CE
    if (outputDir.IsEmpty())
      GetCurrentDir(outputDir);
    #endif
    if (showDialog)
    {
      CExtractDialog dialog;
      FString outputDirFull;
      if (!MyGetFullPathName(outputDir, outputDirFull))
      {
        ShowErrorMessage(kIncorrectOutDir);
        messageWasDisplayed = true;
        return E_FAIL;
      }
      NName::NormalizeDirPathPrefix(outputDirFull);

      dialog.DirPath = fs2us(outputDirFull);
#ifndef _WIN32
      {
	extern const TCHAR * nameWindowToUnix(const TCHAR * lpFileName);
        UString tmpDirectoryPath = dialog.DirPath;
	dialog.DirPath = nameWindowToUnix(tmpDirectoryPath);
      }
#endif

      dialog.OverwriteMode = options.OverwriteMode;
      dialog.OverwriteMode_Force = options.OverwriteMode_Force;
      dialog.PathMode = options.PathMode;
      dialog.PathMode_Force = options.PathMode_Force;
      dialog.ElimDup = options.ElimDup;

      if (archivePathsFull.Size() == 1)
        dialog.ArcPath = archivePathsFull[0];

      #ifndef _SFX
      // dialog.AltStreams = options.NtOptions.AltStreams;
      dialog.NtSecurity = options.NtOptions.NtSecurity;
      if (extractCallback->PasswordIsDefined)
        dialog.Password = extractCallback->Password;
      #endif

      if (dialog.Create(hwndParent) != IDOK)
        return E_ABORT;

      outputDir = us2fs(dialog.DirPath);

      options.OverwriteMode = dialog.OverwriteMode;
      options.PathMode = dialog.PathMode;
      options.ElimDup = dialog.ElimDup;
      
      #ifndef _SFX
      // options.NtOptions.AltStreams = dialog.AltStreams;
      options.NtOptions.NtSecurity = dialog.NtSecurity;
      extractCallback->Password = dialog.Password;
      extractCallback->PasswordIsDefined = !dialog.Password.IsEmpty();
      #endif
    }
    if (!MyGetFullPathName(outputDir, options.OutputDir))
    {
      ShowErrorMessage(kIncorrectOutDir);
      messageWasDisplayed = true;
      return E_FAIL;
    }
    NName::NormalizeDirPathPrefix(options.OutputDir);
    
    /*
    if (!CreateComplexDirectory(options.OutputDir))
    {
      UString s = GetUnicodeString(NError::MyFormatMessage(GetLastError()));
      UString s2 = MyFormatNew(IDS_CANNOT_CREATE_FOLDER,
      #ifdef LANG
      0x02000603,
      #endif
      options.OutputDir);
      s2.Add_LF();
      s2 += s;
      MyMessageBox(s2);
      return E_FAIL;
    }
    */
  }
  
  UString title = LangString(options.TestMode ? IDS_PROGRESS_TESTING : IDS_PROGRESS_EXTRACTING);

  extracter.Title = title;
  extracter.ExtractCallbackSpec = extractCallback;
  extracter.ExtractCallbackSpec->ProgressDialog = &extracter.ProgressDialog;
  extracter.ExtractCallback = extractCallback;
  extracter.ExtractCallbackSpec->Init();

  extracter.ProgressDialog.CompressingMode = false;

  extracter.ArchivePaths = &archivePaths;
  extracter.ArchivePathsFull = &archivePathsFull;
  extracter.WildcardCensor = &wildcardCensor;
  extracter.Options = &options;
  #ifndef _SFX
  extracter.HashBundle = hb;
  #endif

  extracter.ProgressDialog.IconID = IDI_ICON;

  RINOK(extracter.Create(title, hwndParent));
  messageWasDisplayed = extracter.ThreadFinishedOK &
      extracter.ProgressDialog.MessagesDisplayed;
  return extracter.Result;
}
