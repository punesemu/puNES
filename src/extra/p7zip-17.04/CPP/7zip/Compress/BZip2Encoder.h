// BZip2Encoder.h

#ifndef __COMPRESS_BZIP2_ENCODER_H
#define __COMPRESS_BZIP2_ENCODER_H

#include "../../Common/Defs.h"
#include "../../Common/MyCom.h"

#ifndef _7ZIP_ST
#include "../../Windows/Synchronization.h"
#include "../../Windows/Thread.h"
#endif

#include "../ICoder.h"

#include "../Common/InBuffer.h"
#include "../Common/OutBuffer.h"

#include "BitmEncoder.h"
#include "BZip2Const.h"
#include "BZip2Crc.h"

namespace NCompress {
namespace NBZip2 {

class CMsbfEncoderTemp
{
  UInt32 _pos;
  unsigned _bitPos;
  Byte _curByte;
  Byte *_buf;
public:
  void SetStream(Byte *buf) { _buf = buf;  }
  Byte *GetStream() const { return _buf; }

  void Init()
  {
    _pos = 0;
    _bitPos = 8;
    _curByte = 0;
  }

  void Flush()
  {
    if (_bitPos < 8)
      WriteBits(0, _bitPos);
  }

  void WriteBits(UInt32 value, unsigned numBits)
  {
    while (numBits > 0)
    {
      unsigned numNewBits = MyMin(numBits, _bitPos);
      numBits -= numNewBits;
      
      _curByte <<= numNewBits;
      UInt32 newBits = value >> numBits;
      _curByte |= Byte(newBits);
      value -= (newBits << numBits);
      
      _bitPos -= numNewBits;
      
      if (_bitPos == 0)
      {
       _buf[_pos++] = _curByte;
        _bitPos = 8;
      }
    }
  }
  
  UInt32 GetBytePos() const { return _pos ; }
  UInt32 GetPos() const { return _pos * 8 + (8 - _bitPos); }
  Byte GetCurByte() const { return _curByte; }
  void SetPos(UInt32 bitPos)
  {
    _pos = bitPos >> 3;
    _bitPos = 8 - ((unsigned)bitPos & 7);
  }
  void SetCurState(unsigned bitPos, Byte curByte)
  {
    _bitPos = 8 - bitPos;
    _curByte = curByte;
  }
};

class CEncoder;

const unsigned kNumPassesMax = 10;

class CThreadInfo
{
public:
  Byte *m_Block;
private:
  Byte *m_MtfArray;
  Byte *m_TempArray;
  UInt32 *m_BlockSorterIndex;

  CMsbfEncoderTemp *m_OutStreamCurrent;

  Byte Lens[kNumTablesMax][kMaxAlphaSize];
  UInt32 Freqs[kNumTablesMax][kMaxAlphaSize];
  UInt32 Codes[kNumTablesMax][kMaxAlphaSize];

  Byte m_Selectors[kNumSelectorsMax];

  UInt32 m_CRCs[1 << kNumPassesMax];
  UInt32 m_NumCrcs;

  UInt32 m_BlockIndex;

  void WriteBits2(UInt32 value, unsigned numBits);
  void WriteByte2(Byte b);
  void WriteBit2(Byte v);
  void WriteCrc2(UInt32 v);

  void EncodeBlock(const Byte *block, UInt32 blockSize);
  UInt32 EncodeBlockWithHeaders(const Byte *block, UInt32 blockSize);
  void EncodeBlock2(const Byte *block, UInt32 blockSize, UInt32 numPasses);
public:
  bool m_OptimizeNumTables;
  CEncoder *Encoder;
  #ifndef _7ZIP_ST
  NWindows::CThread Thread;

  NWindows::NSynchronization::CAutoResetEvent StreamWasFinishedEvent;
  NWindows::NSynchronization::CAutoResetEvent WaitingWasStartedEvent;

  // it's not member of this thread. We just need one event per thread
  NWindows::NSynchronization::CAutoResetEvent CanWriteEvent;

  UInt64 m_PackSize;

  Byte MtPad[1 << 8]; // It's pad for Multi-Threading. Must be >= Cache_Line_Size.
  HRESULT Create();
  void FinishStream(bool needLeave);
  DWORD ThreadFunc();
  #endif

  CThreadInfo(): m_BlockSorterIndex(0), m_Block(0) {}
  ~CThreadInfo() { Free(); }
  bool Alloc();
  void Free();

  HRESULT EncodeBlock3(UInt32 blockSize);
};

struct CEncProps
{
  UInt32 BlockSizeMult;
  UInt32 NumPasses;
  
  CEncProps()
  {
    BlockSizeMult = (UInt32)(Int32)-1;
    NumPasses = (UInt32)(Int32)-1;
  }
  void Normalize(int level);
  bool DoOptimizeNumTables() const { return NumPasses > 1; }
};

class CEncoder :
  public ICompressCoder,
  public ICompressSetCoderProperties,
  #ifndef _7ZIP_ST
  public ICompressSetCoderMt,
  #endif
  public CMyUnknownImp
{
  UInt32 m_NumThreadsPrev;
public:
  CInBuffer m_InStream;
  Byte MtPad[1 << 8]; // It's pad for Multi-Threading. Must be >= Cache_Line_Size.
  CBitmEncoder<COutBuffer> m_OutStream;
  CEncProps _props;
  CBZip2CombinedCrc CombinedCrc;

  #ifndef _7ZIP_ST
  CThreadInfo *ThreadsInfo;
  NWindows::NSynchronization::CManualResetEvent CanProcessEvent;
  NWindows::NSynchronization::CCriticalSection CS;
  UInt32 NumThreads;
  bool MtMode;
  UInt32 NextBlockIndex;

  bool CloseThreads;
  bool StreamWasFinished;
  NWindows::NSynchronization::CManualResetEvent CanStartWaitingEvent;

  HRESULT Result;
  ICompressProgressInfo *Progress;
  #else
  CThreadInfo ThreadsInfo;
  #endif

  UInt32 ReadRleBlock(Byte *buf);
  void WriteBytes(const Byte *data, UInt32 sizeInBits, Byte lastByte);

  void WriteBits(UInt32 value, unsigned numBits);
  void WriteByte(Byte b);
  // void WriteBit(Byte v);
  void WriteCrc(UInt32 v);

  #ifndef _7ZIP_ST
  HRESULT Create();
  void Free();
  #endif

public:
  CEncoder();
  #ifndef _7ZIP_ST
  ~CEncoder();
  #endif

  HRESULT Flush() { return m_OutStream.Flush(); }
  
  MY_QUERYINTERFACE_BEGIN2(ICompressCoder)
  #ifndef _7ZIP_ST
  MY_QUERYINTERFACE_ENTRY(ICompressSetCoderMt)
  #endif
  MY_QUERYINTERFACE_ENTRY(ICompressSetCoderProperties)
  MY_QUERYINTERFACE_END
  MY_ADDREF_RELEASE

  HRESULT CodeReal(ISequentialInStream *inStream, ISequentialOutStream *outStream,
      const UInt64 *inSize, const UInt64 *outSize, ICompressProgressInfo *progress);

  STDMETHOD(Code)(ISequentialInStream *inStream, ISequentialOutStream *outStream,
      const UInt64 *inSize, const UInt64 *outSize, ICompressProgressInfo *progress);
  STDMETHOD(SetCoderProperties)(const PROPID *propIDs, const PROPVARIANT *props, UInt32 numProps);

  #ifndef _7ZIP_ST
  STDMETHOD(SetNumberOfThreads)(UInt32 numThreads);
  #endif
};

}}

#endif
