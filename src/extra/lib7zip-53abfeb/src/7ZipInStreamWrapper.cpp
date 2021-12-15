#if !defined(_WIN32) && !defined(_OS2)
#include "CPP/myWindows/StdAfx.h"
#include "CPP/include_windows/windows.h"
#endif

#include "C/7zVersion.h"
#include "CPP/7zip/Archive/IArchive.h"
#include "CPP/Windows/PropVariant.h"
#include "CPP/Common/MyCom.h"
#include "CPP/7zip/ICoder.h"
#include "CPP/7zip/IPassword.h"
#include "Common/ComTry.h"
#include "Windows/PropVariant.h"
using namespace NWindows;

#include "lib7zip.h"
#include "7ZipInStreamWrapper.h"

/*----------------- C7ZipInStreamWrapper ---------------------*/
C7ZipInStreamWrapper::C7ZipInStreamWrapper(C7ZipInStream * pInStream) :
m_pInStream(pInStream)
{
}

STDMETHODIMP C7ZipInStreamWrapper::Read(void *data, UInt32 size, UInt32 *processedSize)
{
    return m_pInStream->Read(data,size,processedSize);
}

STDMETHODIMP C7ZipInStreamWrapper::Seek(Int64 offset, UInt32 seekOrigin, UInt64 *newPosition)
{
    return m_pInStream->Seek(offset,seekOrigin,newPosition);
}

STDMETHODIMP C7ZipInStreamWrapper::GetSize(UInt64 *size)
{
    return m_pInStream->GetSize(size);
}
