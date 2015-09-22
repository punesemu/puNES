// CopyDialog.cpp

#include "StdAfx.h"

#include "Windows/FileName.h"

#include "Windows/Control/Static.h"

#include "BrowseDialog.h"
#include "CopyDialog.h"

#ifdef LANG
#include "LangUtils.h"
#endif

using namespace NWindows;

#ifdef LANG
static CIDLangPair kIDLangPairs[] =
{
  { IDOK, 0x02000702 },
  { IDCANCEL, 0x02000710 }
};
#endif

#ifndef _WIN32
extern const TCHAR * nameWindowToUnix(const TCHAR * lpFileName);
#endif

bool CCopyDialog::OnInit()
{
  #ifdef LANG
  LangSetDlgItemsText(HWND(*this), kIDLangPairs, sizeof(kIDLangPairs) / sizeof(kIDLangPairs[0]));
  #endif
  _path.Attach(GetItem(IDC_COPY_COMBO));
  SetText(Title);

  NControl::CStatic staticContol;
  staticContol.Attach(GetItem(IDC_COPY_STATIC));
  staticContol.SetText(Static);
  #ifdef UNDER_CE
  // we do it, since WinCE selects Value\something instead of Value !!!!
  _path.AddString(Value);
  #endif
  for (int i = 0; i < Strings.Size(); i++)
    _path.AddString(Strings[i]);
#ifndef _WIN32
  UString tmp = nameWindowToUnix(Value);
  Value = tmp;
#endif
  _path.SetText(Value);
  SetItemText(IDC_COPY_INFO, Info);
  NormalizeSize(true);
  return CModalDialog::OnInit();
}

bool CCopyDialog::OnButtonClicked(int buttonID, HWND buttonHWND)
{
  switch(buttonID)
  {
    case IDC_COPY_SET_PATH:
      OnButtonSetPath();
      return true;
  }
  return CModalDialog::OnButtonClicked(buttonID, buttonHWND);
}

void CCopyDialog::OnButtonSetPath()
{
  UString currentPath;
  _path.GetText(currentPath);

  UString title = LangStringSpec(IDS_SET_FOLDER, 0x03020209);

  UString resultPath;
  if (!MyBrowseForFolder(HWND(*this), title, currentPath, resultPath))
    return;
  NFile::NName::NormalizeDirPathPrefix(resultPath);
  _path.SetCurSel(-1);
  _path.SetText(resultPath);
}

void CCopyDialog::OnOK()
{
  _path.GetText(Value);
  CModalDialog::OnOK();
}
