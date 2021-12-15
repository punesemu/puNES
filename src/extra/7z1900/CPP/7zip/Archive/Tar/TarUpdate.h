// TarUpdate.h

#ifndef __TAR_UPDATE_H
#define __TAR_UPDATE_H

#include "../IArchive.h"

#include "TarItem.h"

namespace NArchive {
namespace NTar {

struct CUpdateItem
{
  int IndexInArc;
  int IndexInClient;
  UInt64 Size;
  Int64 MTime;
  UInt32 Mode;
  bool NewData;
  bool NewProps;
  bool IsDir;
  AString Name;
  AString User;
  AString Group;

  CUpdateItem(): Size(0), IsDir(false) {}
};

HRESULT UpdateArchive(IInStream *inStream, ISequentialOutStream *outStream,
    const CObjectVector<CItemEx> &inputItems,
    const CObjectVector<CUpdateItem> &updateItems,
    UINT codePage,
    IArchiveUpdateCallback *updateCallback);

}}

#endif
