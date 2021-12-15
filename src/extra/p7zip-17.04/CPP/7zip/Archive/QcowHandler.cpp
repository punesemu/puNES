// QcowHandler.cpp

#include "StdAfx.h"

// #include <stdio.h>

#include "../../../C/CpuArch.h"

#include "../../Common/ComTry.h"
#include "../../Common/IntToString.h"

#include "../../Windows/PropVariant.h"

#include "../Common/RegisterArc.h"
#include "../Common/StreamObjects.h"
#include "../Common/StreamUtils.h"

#include "../Compress/DeflateDecoder.h"

#include "HandlerCont.h"

#define Get32(p) GetBe32(p)
#define Get64(p) GetBe64(p)

using namespace NWindows;

namespace NArchive {
namespace NQcow {

#define SIGNATURE { 'Q', 'F', 'I', 0xFB, 0, 0, 0 }
  
static const Byte k_Signature[] = SIGNATURE;

class CHandler: public CHandlerImg
{
  unsigned _clusterBits;
  unsigned _numMidBits;
  UInt64 _compressedFlag;

  CObjectVector<CByteBuffer> _tables;
  UInt64 _cacheCluster;
  CByteBuffer _cache;
  CByteBuffer _cacheCompressed;

  UInt64 _comprPos;
  size_t _comprSize;

  UInt64 _phySize;

  CBufInStream *_bufInStreamSpec;
  CMyComPtr<ISequentialInStream> _bufInStream;

  CBufPtrSeqOutStream *_bufOutStreamSpec;
  CMyComPtr<ISequentialOutStream> _bufOutStream;

  NCompress::NDeflate::NDecoder::CCOMCoder *_deflateDecoderSpec;
  CMyComPtr<ICompressCoder> _deflateDecoder;

  bool _needDeflate;
  bool _isArc;
  bool _unsupported;

  UInt32 _version;
  UInt32 _cryptMethod;
  
  HRESULT Seek(UInt64 offset)
  {
    _posInArc = offset;
    return Stream->Seek(offset, STREAM_SEEK_SET, NULL);
  }

  HRESULT InitAndSeek()
  {
    _virtPos = 0;
    return Seek(0);
  }

  HRESULT Open2(IInStream *stream, IArchiveOpenCallback *openCallback);

public:
  INTERFACE_IInArchive_Img(;)

  STDMETHOD(GetStream)(UInt32 index, ISequentialInStream **stream);
  STDMETHOD(Read)(void *data, UInt32 size, UInt32 *processedSize);
};


STDMETHODIMP CHandler::Read(void *data, UInt32 size, UInt32 *processedSize)
{
  if (processedSize)
    *processedSize = 0;
  if (_virtPos >= _size)
    return S_OK;
  {
    UInt64 rem = _size - _virtPos;
    if (size > rem)
      size = (UInt32)rem;
    if (size == 0)
      return S_OK;
  }
 
  for (;;)
  {
    UInt64 cluster = _virtPos >> _clusterBits;
    size_t clusterSize = (size_t)1 << _clusterBits;
    size_t lowBits = (size_t)_virtPos & (clusterSize - 1);
    {
      size_t rem = clusterSize - lowBits;
      if (size > rem)
        size = (UInt32)rem;
    }

    if (cluster == _cacheCluster)
    {
      memcpy(data, _cache + lowBits, size);
      _virtPos += size;
      if (processedSize)
        *processedSize = size;
      return S_OK;
    }
    
    UInt64 high = cluster >> _numMidBits;
 
    if (high < _tables.Size())
    {
      const CByteBuffer &buffer = _tables[(unsigned)high];
    
      if (buffer.Size() != 0)
      {
        size_t midBits = (size_t)cluster & (((size_t)1 << _numMidBits) - 1);
        const Byte *p = (const Byte *)buffer + (midBits << 3);
        UInt64 v = Get64(p);
        
        if (v != 0)
        {
          if ((v & _compressedFlag) != 0)
          {
            if (_version <= 1)
              return E_FAIL;
            unsigned numOffsetBits = (62 - (_clusterBits - 8));
            UInt64 offset = v & (((UInt64)1 << 62) - 1);
            const size_t dataSize = ((size_t)(offset >> numOffsetBits) + 1) << 9;
            offset &= ((UInt64)1 << numOffsetBits) - 1;
            UInt64 sectorOffset = offset >> 9 << 9;
            UInt64 offset2inCache = sectorOffset - _comprPos;
            
            if (sectorOffset >= _comprPos && offset2inCache < _comprSize)
            {
              if (offset2inCache != 0)
              {
                _comprSize -= (size_t)offset2inCache;
                memmove(_cacheCompressed, _cacheCompressed + offset2inCache, _comprSize);
                _comprPos = sectorOffset;
              }
              sectorOffset += _comprSize;
            }
            else
            {
              _comprPos = sectorOffset;
              _comprSize = 0;
            }
            
            // printf("\nDeflate");
            if (sectorOffset != _posInArc)
            {
              // printf("\nDeflate %12I64x %12I64x\n", sectorOffset, sectorOffset - _posInArc);
              RINOK(Seek(sectorOffset));
            }
            
            if (_cacheCompressed.Size() < dataSize)
              return E_FAIL;
            size_t dataSize3 = dataSize - _comprSize;
            size_t dataSize2 = dataSize3;
            RINOK(ReadStream(Stream, _cacheCompressed + _comprSize, &dataSize2));
            _posInArc += dataSize2;
            if (dataSize2 != dataSize3)
              return E_FAIL;
            _comprSize += dataSize2;
            
            const size_t kSectorMask = (1 << 9) - 1;
            size_t offsetInSector = ((size_t)offset & kSectorMask);
            _bufInStreamSpec->Init(_cacheCompressed + offsetInSector, dataSize - offsetInSector);
            
            _cacheCluster = (UInt64)(Int64)-1;
            if (_cache.Size() < clusterSize)
              return E_FAIL;
            _bufOutStreamSpec->Init(_cache, clusterSize);
            
            // Do we need to use smaller block than clusterSize for last cluster?
            UInt64 blockSize64 = clusterSize;
            HRESULT res = _deflateDecoderSpec->Code(_bufInStream, _bufOutStream, NULL, &blockSize64, NULL);

            /*
            if (_bufOutStreamSpec->GetPos() != clusterSize)
              memset(_cache + _bufOutStreamSpec->GetPos(), 0, clusterSize - _bufOutStreamSpec->GetPos());
            */

            if (res == S_OK)
              if (!_deflateDecoderSpec->IsFinished()
                  || _bufOutStreamSpec->GetPos() != clusterSize)
                res = S_FALSE;

            RINOK(res);
            _cacheCluster = cluster;
            
            continue;
            /*
            memcpy(data, _cache + lowBits, size);
            _virtPos += size;
            if (processedSize)
              *processedSize = size;
            return S_OK;
            */
          }

          // version 3 support zero clusters
          if (((UInt32)v & 511) != 1)
          {
            v &= (_compressedFlag - 1);
            v += lowBits;
            if (v != _posInArc)
            {
              // printf("\n%12I64x\n", v - _posInArc);
              RINOK(Seek(v));
            }
            HRESULT res = Stream->Read(data, size, &size);
            _posInArc += size;
            _virtPos += size;
            if (processedSize)
              *processedSize = size;
            return res;
          }
        }
      }
    }
    
    memset(data, 0, size);
    _virtPos += size;
    if (processedSize)
      *processedSize = size;
    return S_OK;
  }
}


static const Byte kProps[] =
{
  kpidSize,
  kpidPackSize
};

static const Byte kArcProps[] =
{
  kpidClusterSize,
  kpidUnpackVer,
  kpidMethod
};

IMP_IInArchive_Props
IMP_IInArchive_ArcProps

STDMETHODIMP CHandler::GetArchiveProperty(PROPID propID, PROPVARIANT *value)
{
  COM_TRY_BEGIN
  NCOM::CPropVariant prop;

  switch (propID)
  {
    case kpidMainSubfile: prop = (UInt32)0; break;
    case kpidClusterSize: prop = (UInt32)1 << _clusterBits; break;
    case kpidPhySize: if (_phySize != 0) prop = _phySize; break;
    case kpidUnpackVer: prop = _version; break;

    case kpidMethod:
    {
      AString s;

      if (_needDeflate)
        s = "Deflate";

      if (_cryptMethod != 0)
      {
        s.Add_Space_if_NotEmpty();
        if (_cryptMethod == 1)
          s += "AES";
        else
          s.Add_UInt32(_cryptMethod);
      }
      
      if (!s.IsEmpty())
        prop = s;

      break;
    }

    case kpidErrorFlags:
    {
      UInt32 v = 0;
      if (!_isArc) v |= kpv_ErrorFlags_IsNotArc;;
      if (_unsupported) v |= kpv_ErrorFlags_UnsupportedMethod;
      // if (_headerError) v |= kpv_ErrorFlags_HeadersError;
      if (!Stream && v == 0 && _isArc)
        v = kpv_ErrorFlags_HeadersError;
      if (v != 0)
        prop = v;
      break;
    }
  }
  
  prop.Detach(value);
  return S_OK;
  COM_TRY_END
}


STDMETHODIMP CHandler::GetProperty(UInt32 /* index */, PROPID propID, PROPVARIANT *value)
{
  COM_TRY_BEGIN
  NCOM::CPropVariant prop;

  switch (propID)
  {
    case kpidSize: prop = _size; break;
    case kpidPackSize: prop = _phySize; break;
    case kpidExtension: prop = (_imgExt ? _imgExt : "img"); break;
  }
  
  prop.Detach(value);
  return S_OK;
  COM_TRY_END
}


HRESULT CHandler::Open2(IInStream *stream, IArchiveOpenCallback *openCallback)
{
  const unsigned kHeaderSize = 18 * 4;
  Byte buf[kHeaderSize];
  RINOK(ReadStream_FALSE(stream, buf, kHeaderSize));

  if (memcmp(buf, k_Signature, 4) != 0)
    return S_FALSE;

  _version = Get32(buf + 4);
  if (_version < 1 || _version > 3)
    return S_FALSE;
  
  const UInt64 backOffset = Get64(buf + 8);
  // UInt32 backSize = Get32(buf + 0x10);
  
  UInt64 l1Offset = 0;
  UInt32 l1Size = 0;

  if (_version == 1)
  {
    // _mTime = Get32(buf + 0x14); // is unused im most images
    _size = Get64(buf + 0x18);
    _clusterBits = buf[0x20];
    _numMidBits = buf[0x21];
    if (_clusterBits < 9 || _clusterBits > 30)
      return S_FALSE;
    if (_numMidBits < 1 || _numMidBits > 28)
      return S_FALSE;
    _cryptMethod = Get32(buf + 0x24);
    l1Offset = Get64(buf + 0x28);
    if (l1Offset < 0x30)
      return S_FALSE;
    unsigned numBits2 = (_clusterBits + _numMidBits);
    UInt64 l1Size64 = (_size + (((UInt64)1 << numBits2) - 1)) >> numBits2;
    if (l1Size64 > ((UInt32)1 << 31))
      return S_FALSE;
    l1Size = (UInt32)l1Size64;
  }
  else
  {
    _clusterBits = Get32(buf + 0x14);
    if (_clusterBits < 9 || _clusterBits > 30)
      return S_FALSE;
    _numMidBits = _clusterBits - 3;
    _size = Get64(buf + 0x18);
    _cryptMethod = Get32(buf + 0x20);
    l1Size = Get32(buf + 0x24);
    l1Offset = Get64(buf + 0x28); // must be aligned for cluster
    
    UInt64 refOffset = Get64(buf + 0x30); // must be aligned for cluster
    UInt32 refClusters = Get32(buf + 0x38);
    
    // UInt32 numSnapshots = Get32(buf + 0x3C);
    // UInt64 snapshotsOffset = Get64(buf + 0x40); // must be aligned for cluster
    /*
    if (numSnapshots != 0)
      return S_FALSE;
    */

    if (refClusters != 0)
    {
      size_t numBytes = refClusters << _clusterBits;
      /*
      CByteBuffer refs;
      refs.Alloc(numBytes);
      RINOK(stream->Seek(refOffset, STREAM_SEEK_SET, NULL));
      RINOK(ReadStream_FALSE(stream, refs, numBytes));
      */
      UInt64 end = refOffset + numBytes;
      if (_phySize < end)
        _phySize = end;
      /*
      for (size_t i = 0; i < numBytes; i += 2)
      {
        UInt32 v = GetBe16((const Byte *)refs + (size_t)i);
        if (v == 0)
          continue;
      }
      */
    }
  }

  _isArc = true;

  if (backOffset != 0)
  {
    _unsupported = true;
    return S_FALSE;
  }

  const size_t clusterSize = (size_t)1 << _clusterBits;

  CByteBuffer table;
  {
    size_t t1SizeBytes = (size_t)l1Size << 3;
    if ((t1SizeBytes >> 3) != l1Size)
      return S_FALSE;
    table.Alloc(t1SizeBytes);
    RINOK(stream->Seek(l1Offset, STREAM_SEEK_SET, NULL));
    RINOK(ReadStream_FALSE(stream, table, t1SizeBytes));
    
    {
      UInt64 end = l1Offset + t1SizeBytes;
      // we need to uses align end for empty qcow files
      end = (end + clusterSize - 1) >> _clusterBits << _clusterBits;
      if (_phySize < end)
        _phySize = end;
    }
  }

  if (openCallback)
  {
    UInt64 totalBytes = (UInt64)l1Size << (_numMidBits + 3);
    RINOK(openCallback->SetTotal(NULL, &totalBytes));
  }

  _compressedFlag = (_version <= 1) ? ((UInt64)1 << 63) : ((UInt64)1 << 62);
  const UInt64 offsetMask = _compressedFlag - 1;

  for (UInt32 i = 0; i < l1Size; i++)
  {
    if (openCallback)
    {
      UInt64 numBytes = (UInt64)i << (_numMidBits + 3);
      RINOK(openCallback->SetCompleted(NULL, &numBytes));
    }

    CByteBuffer &buf2 = _tables.AddNew();
    
    {
      UInt64 v = Get64((const Byte *)table + (size_t)i * 8);
      v &= offsetMask;
      if (v == 0)
        continue;
      
      buf2.Alloc((size_t)1 << (_numMidBits + 3));
      RINOK(stream->Seek(v, STREAM_SEEK_SET, NULL));
      RINOK(ReadStream_FALSE(stream, buf2, clusterSize));

      const UInt64 end = v + clusterSize;
      if (_phySize < end)
        _phySize = end;
    }

    for (size_t k = 0; k < clusterSize; k += 8)
    {
      const UInt64 v = Get64((const Byte *)buf2 + (size_t)k);
      if (v == 0)
        continue;
      UInt64 offset = v & offsetMask;
      size_t dataSize = clusterSize;
      
      if ((v & _compressedFlag) != 0)
      {
        if (_version <= 1)
        {
          unsigned numOffsetBits = (63 - _clusterBits);
          dataSize = ((size_t)(offset >> numOffsetBits) + 1) << 9;
          offset &= ((UInt64)1 << numOffsetBits) - 1;
          dataSize = 0;
          // offset >>= 9;
          // offset <<= 9;
        }
        else
        {
          unsigned numOffsetBits = (62 - (_clusterBits - 8));
          dataSize = ((size_t)(offset >> numOffsetBits) + 1) << 9;
          offset &= ((UInt64)1 << numOffsetBits) - 1;
          offset >>= 9;
          offset <<= 9;
        }
        _needDeflate = true;
      }
      else
      {
        UInt32 low = (UInt32)v & 511;
        if (low != 0)
        {
          // version 3 support zero clusters
          if (_version < 3 || low != 1)
          {
            _unsupported = true;
            return S_FALSE;
          }
        }
      }
      
      UInt64 end = offset + dataSize;
      if (_phySize < end)
        _phySize = end;
    }
  }

  if (_cryptMethod != 0)
    _unsupported = true;

  if (_needDeflate && _version <= 1) // that case was not implemented
    _unsupported = true;

  Stream = stream;
  return S_OK;
}


STDMETHODIMP CHandler::Close()
{
  _tables.Clear();
  _phySize = 0;
  _size = 0;

  _cacheCluster = (UInt64)(Int64)-1;
  _comprPos = 0;
  _comprSize = 0;
  _needDeflate = false;

  _isArc = false;
  _unsupported = false;

  _imgExt = NULL;
  Stream.Release();
  return S_OK;
}


STDMETHODIMP CHandler::GetStream(UInt32 /* index */, ISequentialInStream **stream)
{
  COM_TRY_BEGIN
  *stream = NULL;

  if (_unsupported)
    return S_FALSE;

  if (_needDeflate)
  {
    if (_version <= 1)
      return S_FALSE;

    if (!_bufInStream)
    {
      _bufInStreamSpec = new CBufInStream;
      _bufInStream = _bufInStreamSpec;
    }
    
    if (!_bufOutStream)
    {
      _bufOutStreamSpec = new CBufPtrSeqOutStream();
      _bufOutStream = _bufOutStreamSpec;
    }

    if (!_deflateDecoder)
    {
      _deflateDecoderSpec = new NCompress::NDeflate::NDecoder::CCOMCoder();
      _deflateDecoder = _deflateDecoderSpec;
      _deflateDecoderSpec->Set_NeedFinishInput(true);
    }
    
    size_t clusterSize = (size_t)1 << _clusterBits;
    _cache.AllocAtLeast(clusterSize);
    _cacheCompressed.AllocAtLeast(clusterSize * 2);
  }
    
  CMyComPtr<ISequentialInStream> streamTemp = this;
  RINOK(InitAndSeek());
  *stream = streamTemp.Detach();
  return S_OK;
  COM_TRY_END
}


REGISTER_ARC_I(
  "QCOW", "qcow qcow2 qcow2c", NULL, 0xCA,
  k_Signature,
  0,
  0,
  NULL)

}}
