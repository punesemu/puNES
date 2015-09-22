// FileFolderPluginOpen.cpp

#include "StdAfx.h"

#include "resource.h"

#include "Windows/Thread.h"

#include "../Agent/Agent.h"

#include "LangUtils.h"
#include "OpenCallback.h"
#include "PluginLoader.h"
#include "RegistryAssociations.h"
#include "RegistryPlugins.h"

using namespace NWindows;
using namespace NRegistryAssociations;

struct CThreadArchiveOpen
{
  UString Path;
  UString ArcFormat;
  CMyComPtr<IInStream> InStream;
  CMyComPtr<IFolderManager> FolderManager;
  CMyComPtr<IProgress> OpenCallback;
  COpenArchiveCallback *OpenCallbackSpec;

  CMyComPtr<IFolderFolder> Folder;
  HRESULT Result;

  void Process()
  {
    try
    {
      CProgressCloser closer(OpenCallbackSpec->ProgressDialog);
      Result = FolderManager->OpenFolderFile(InStream, Path, ArcFormat, &Folder, OpenCallback);
    }
    catch(...) { Result = E_FAIL; }
  }
  
  static THREAD_FUNC_DECL MyThreadFunction(void *param)
  {
    ((CThreadArchiveOpen *)param)->Process();
    return 0;
  }
};

/*
static int FindPlugin(const CObjectVector<CPluginInfo> &plugins, const UString &pluginName)
{
  for (int i = 0; i < plugins.Size(); i++)
    if (plugins[i].Name.CompareNoCase(pluginName) == 0)
      return i;
  return -1;
}
*/

HRESULT OpenFileFolderPlugin(
    IInStream *inStream,
    const UString &path,
    const UString &arcFormat,
    HMODULE *module,
    IFolderFolder **resultFolder,
    HWND parentWindow,
    bool &encrypted, UString &password)
{
#ifdef _WIN32
  CObjectVector<CPluginInfo> plugins;
  ReadFileFolderPluginInfoList(plugins);
#endif

  UString extension, name, pureName, dot;

  int slashPos = path.ReverseFind(WCHAR_PATH_SEPARATOR);
  UString dirPrefix;
  UString fileName;
  if (slashPos >= 0)
  {
    dirPrefix = path.Left(slashPos + 1);
    fileName = path.Mid(slashPos + 1);
  }
  else
    fileName = path;

  NFile::NName::SplitNameToPureNameAndExtension(fileName, pureName, dot, extension);

  /*
  if (!extension.IsEmpty())
  {
    CExtInfo extInfo;
    if (ReadInternalAssociation(extension, extInfo))
    {
      for (int i = extInfo.Plugins.Size() - 1; i >= 0; i--)
      {
        int pluginIndex = FindPlugin(plugins, extInfo.Plugins[i]);
        if (pluginIndex >= 0)
        {
          const CPluginInfo plugin = plugins[pluginIndex];
          plugins.Delete(pluginIndex);
          plugins.Insert(0, plugin);
        }
      }
    }
  }
  */

#ifdef _WIN32
  for (int i = 0; i < plugins.Size(); i++)
  {
    const CPluginInfo &plugin = plugins[i];
    if (!plugin.ClassIDDefined)
      continue;
#endif
    CPluginLibrary library;

    CThreadArchiveOpen t;

#ifdef _WIN32
    if (plugin.FilePath.IsEmpty())
      t.FolderManager = new CArchiveFolderManager;
    else if (library.LoadAndCreateManager(plugin.FilePath, plugin.ClassID, &t.FolderManager) != S_OK)
      continue;
#else
      t.FolderManager = new CArchiveFolderManager;
#endif

    t.OpenCallbackSpec = new COpenArchiveCallback;
    t.OpenCallback = t.OpenCallbackSpec;
    t.OpenCallbackSpec->PasswordIsDefined = encrypted;
    t.OpenCallbackSpec->Password = password;
    t.OpenCallbackSpec->ParentWindow = parentWindow;

    if (inStream)
      t.OpenCallbackSpec->SetSubArchiveName(fileName);
    else
      t.OpenCallbackSpec->LoadFileInfo(dirPrefix, fileName);

    t.InStream = inStream;
    t.Path = path;
    t.ArcFormat = arcFormat;

    UString progressTitle = LangString(IDS_OPENNING, 0x03020283);
    t.OpenCallbackSpec->ProgressDialog.MainWindow = parentWindow;
    t.OpenCallbackSpec->ProgressDialog.MainTitle = LangString(IDS_APP_TITLE, 0x03000000);
    t.OpenCallbackSpec->ProgressDialog.MainAddTitle = progressTitle + UString(L" ");
    // FIXME t.OpenCallbackSpec->ProgressDialog.WaitMode = true;

    {
      NWindows::CThread thread;
      RINOK(thread.Create(CThreadArchiveOpen::MyThreadFunction, &t));
      t.OpenCallbackSpec->StartProgressDialog(progressTitle, thread);
    }

    if (t.Result == E_ABORT)
      return t.Result;

    if (t.Result == S_OK)
    {
      // if (openCallbackSpec->PasswordWasAsked)
      {
        encrypted = t.OpenCallbackSpec->PasswordIsDefined;
        password = t.OpenCallbackSpec->Password;
      }
      *module = library.Detach();
      *resultFolder = t.Folder.Detach();
      return S_OK;
    }
    
    if (t.Result != S_FALSE)
      return t.Result;
#ifdef _WIN32
  }
#endif
  return S_FALSE;
}