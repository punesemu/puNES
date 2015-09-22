// Windows/Control/Edit.h

#ifndef __WINDOWS_CONTROL_EDIT_H
#define __WINDOWS_CONTROL_EDIT_H

#include "Windows/Window.h"
#include "Windows/Defs.h"

namespace NWindows {
namespace NControl {

class CEdit: public CWindow
{
public:
	void SetPasswordChar(WPARAM c);
	void Show(int cmdShow);
	virtual void SetText(LPCWSTR s);
	virtual bool GetText(CSysString &s);
};

}}

#endif

