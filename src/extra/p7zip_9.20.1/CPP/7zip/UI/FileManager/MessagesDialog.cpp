// MessagesDialog.cpp
 
#include "StdAfx.h"

#include "Common/IntToString.h"

#include "Windows/ResourceString.h"

#include "MessagesDialog.h"

#ifdef LANG
#include "LangUtils.h"
#endif

using namespace NWindows;

#ifdef LANG
static CIDLangPair kIDLangPairs[] =
{
  { IDOK, 0x02000713 }
};
#endif

void CMessagesDialog::AddMessageDirect(LPCWSTR message)
{
  int itemIndex = _messageList.GetItemCount();
  wchar_t sz[32];
  ConvertInt64ToString(itemIndex, sz);
  _messageList.InsertItem(itemIndex, sz);
  _messageList.SetSubItem(itemIndex, 1, message);
}

void CMessagesDialog::AddMessage(LPCWSTR message)
{
  UString s = message;
  while (!s.IsEmpty())
  {
    int pos = s.Find(L'\n');
    if (pos < 0)
      break;
    AddMessageDirect(s.Left(pos));
    s.Delete(0, pos + 1);
  }
  AddMessageDirect(s);
}

bool CMessagesDialog::OnInit()
{
  #ifdef LANG
  LangSetWindowText(HWND(*this), 0x02000A00);
  LangSetDlgItemsText(HWND(*this), kIDLangPairs, sizeof(kIDLangPairs) / sizeof(kIDLangPairs[0]));
  #endif
  _messageList.Attach(GetItem(IDC_MESSAGE_LIST));

  #ifndef UNDER_CE
  _messageList.SetUnicodeFormat(true);
  #endif

  _messageList.InsertColumn(0, L"", 30);

  const UString s =
    #ifdef LANG
    LangString(IDS_MESSAGES_DIALOG_MESSAGE_COLUMN, 0x02000A80);
    #else
    MyLoadStringW(IDS_MESSAGES_DIALOG_MESSAGE_COLUMN);
    #endif

  _messageList.InsertColumn(1, s, 600);

  for (int i = 0; i < Messages->Size(); i++)
    AddMessage((*Messages)[i]);

  _messageList.SetColumnWidthAuto(0);
  _messageList.SetColumnWidthAuto(1);
  NormalizeSize();
  return CModalDialog::OnInit();
}
