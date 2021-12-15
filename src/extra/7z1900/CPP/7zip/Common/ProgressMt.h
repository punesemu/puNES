// ProgressMt.h

#ifndef __PROGRESSMT_H
#define __PROGRESSMT_H

#include "../../Common/MyCom.h"
#include "../../Common/MyVector.h"
#include "../../Windows/Synchronization.h"

#include "../ICoder.h"
#include "../IProgress.h"

class CMtCompressProgressMixer
{
  CMyComPtr<ICompressProgressInfo> _progress;
  CRecordVector<UInt64> InSizes;
  CRecordVector<UInt64> OutSizes;
  UInt64 TotalInSize;
  UInt64 TotalOutSize;
public:
  NWindows::NSynchronization::CCriticalSection CriticalSection;
  void Init(int numItems, ICompressProgressInfo *progress);
  void Reinit(int index);
  HRESULT SetRatioInfo(int index, const UInt64 *inSize, const UInt64 *outSize);
};

class CMtCompressProgress:
  public ICompressProgressInfo,
  public CMyUnknownImp
{
  CMtCompressProgressMixer *_progress;
  int _index;
public:
  void Init(CMtCompressProgressMixer *progress, int index)
  {
    _progress = progress;
    _index = index;
  }
  void Reinit() { _progress->Reinit(_index); }

  MY_UNKNOWN_IMP

  STDMETHOD(SetRatioInfo)(const UInt64 *inSize, const UInt64 *outSize);
};

#endif
