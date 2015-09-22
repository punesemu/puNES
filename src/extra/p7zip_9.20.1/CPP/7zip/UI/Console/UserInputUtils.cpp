// UserInputUtils.cpp

#include "StdAfx.h"

#include "Common/StdInStream.h"
#include "Common/StringConvert.h"

#include "UserInputUtils.h"

#ifdef USE_FLTK
// the programs like file-roller or xarchiver do not support archives with password
// these programs freeze because p7zip is waiting for a password
// defining USE_FLTK allows p7zip to use a popup in order to ask the password.
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/fl_ask.H>
#else
#ifdef ENV_HAVE_GETPASS
#include <pwd.h>
#include <unistd.h>
#include "Common/MyException.h"
#endif
#endif

static const char kYes = 'Y';
static const char kNo = 'N';
static const char kYesAll = 'A';
static const char kNoAll = 'S';
static const char kAutoRenameAll = 'U';
static const char kQuit = 'Q';

static const char *kFirstQuestionMessage = "?\n";
static const char *kHelpQuestionMessage =
  "(Y)es / (N)o / (A)lways / (S)kip all / A(u)to rename all / (Q)uit? ";

// return true if pressed Quite;

NUserAnswerMode::EEnum ScanUserYesNoAllQuit(CStdOutStream *outStream)
{
  (*outStream) << kFirstQuestionMessage;
  for (;;)
  {
    (*outStream) << kHelpQuestionMessage;
    outStream->Flush();
    AString scannedString = g_StdIn.ScanStringUntilNewLine();
    scannedString.Trim();
    if (!scannedString.IsEmpty())
      switch(
        ::MyCharUpper(
        #ifdef UNDER_CE
        (wchar_t)
        #endif
        scannedString[0]))
      {
        case kYes:
          return NUserAnswerMode::kYes;
        case kNo:
          return NUserAnswerMode::kNo;
        case kYesAll:
          return NUserAnswerMode::kYesAll;
        case kNoAll:
          return NUserAnswerMode::kNoAll;
        case kAutoRenameAll:
          return NUserAnswerMode::kAutoRenameAll;
        case kQuit:
          return NUserAnswerMode::kQuit;
      }
  }
}

UString GetPassword(CStdOutStream *outStream,bool verify)
{
#ifdef USE_FLTK 
  const char *r = fl_password("Enter password", 0);
  AString oemPassword = "";
  if (r) oemPassword = r;
#else /* USE_FLTK */
#ifdef ENV_HAVE_GETPASS
  (*outStream) << "\nEnter password (will not be echoed) :";
  outStream->Flush();
  AString oemPassword = getpass("");
  if (verify)
  {
    (*outStream) << "Verify password (will not be echoed) :";
  outStream->Flush();
    AString oemPassword2 = getpass("");
    if (oemPassword != oemPassword2) throw "password verification failed";
  }
#else
  (*outStream) << "\nEnter password:";
  outStream->Flush();
  AString oemPassword = g_StdIn.ScanStringUntilNewLine();
#endif
#endif /* USE_FLTK */
  return MultiByteToUnicodeString(oemPassword, CP_OEMCP);
}
