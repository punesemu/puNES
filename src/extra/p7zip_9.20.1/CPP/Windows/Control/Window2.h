// Windows/Control/Window2.h

#ifndef __WINDOWS_CONTROL_WINDOW2_H
#define __WINDOWS_CONTROL_WINDOW2_H

#include "Windows/Window.h"
#include "Windows/Defs.h"

#ifndef _WIN32
typedef void * WNDPROC;
typedef void * CREATESTRUCT;
typedef struct
{
	HWND  hwndFrom;

	UINT  code;
#define NM_DBLCLK       1
#define LVN_ITEMCHANGED 2
#define LVN_COLUMNCLICK 3	
#define CBEN_BEGINEDIT  10
#define CBEN_ENDEDITW   11
	
	
} NMHDR;
typedef NMHDR * LPNMHDR;

typedef struct tagNMLISTVIEW
{
    NMHDR hdr;
    INT iItem;
    INT iSubItem;
    UINT uNewState;
    UINT uOldState;
    // UINT uChanged;
    // POINT ptAction;
    LPARAM  lParam;
} NMLISTVIEW, *LPNMLISTVIEW;

typedef void * LPNMITEMACTIVATE;

#define NM_RCLICK 1234 /* FIXME */

// FIXME
#define WM_CREATE 1
#define WM_COMMAND 2
#define WM_NOTIFY 3
#define WM_DESTROY 4
#define WM_CLOSE 5

#define HIWORD(l)              ((WORD)((DWORD_PTR)(l) >> 16))
#define LOWORD(l)              ((WORD)((DWORD_PTR)(l) & 0xFFFF))


#endif

namespace NWindows {
namespace NControl {

class CWindow2 // : public CWindow
{
  // LRESULT DefProc(UINT message, WPARAM wParam, LPARAM lParam);
public:
  // CWindow2(HWND newWindow = NULL): CWindow(newWindow){};
  CWindow2() {}
  virtual ~CWindow2() {}

#ifdef _WIN32
  bool CreateEx(DWORD exStyle, LPCTSTR className,
      LPCTSTR windowName, DWORD style,
      int x, int y, int width, int height,
      HWND parentWindow, HMENU idOrHMenu,
      HINSTANCE instance);

  #ifndef _UNICODE
  bool CreateEx(DWORD exStyle, LPCWSTR className,
      LPCWSTR windowName, DWORD style,
      int x, int y, int width, int height,
      HWND parentWindow, HMENU idOrHMenu,
      HINSTANCE instance);
  #endif
#endif

  virtual LRESULT OnMessage(UINT message, WPARAM wParam, LPARAM lParam);
  virtual bool OnCreate(CREATESTRUCT * /* createStruct */) { return true; }
  // virtual LRESULT OnCommand(WPARAM wParam, LPARAM lParam);
  virtual bool OnCommand(WPARAM wParam, LPARAM lParam, LRESULT &result);
  virtual bool OnCommand(int code, int itemID, LPARAM lParam, LRESULT &result);
  virtual bool OnSize(WPARAM /* wParam */, int /* xSize */, int /* ySize */) { return false; }
  virtual bool OnNotify(UINT /* controlID */, LPNMHDR /* lParam */, LRESULT & /* result */) { return false; }
  virtual void OnDestroy() { /* FIXME PostQuitMessage(0); */ }
  virtual void OnClose() { /* FIXME Destroy(); */ }
  /*
  virtual LRESULT  OnHelp(LPHELPINFO helpInfo) { OnHelp(); };
  virtual LRESULT  OnHelp() {};
  virtual bool OnButtonClicked(int buttonID, HWND buttonHWND);
  virtual void OnOK() {};
  virtual void OnCancel() {};
  */

#ifdef _WIN32
  LONG_PTR SetMsgResult(LONG_PTR newLongPtr )
    { return SetLongPtr(DWLP_MSGRESULT, newLongPtr); }
  LONG_PTR GetMsgResult() const
    { return GetLongPtr(DWLP_MSGRESULT); }
#endif
};

}}

#endif

