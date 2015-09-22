#include "lib7zip.h"

#ifdef S_OK
#undef S_OK
#endif

#include "CPP/7zip/Archive/IArchive.h"
#include "CPP/Windows/PropVariant.h"
#include "CPP/Common/MyCom.h"
#include "CPP/7zip/ICoder.h"
#include "CPP/7zip/IPassword.h"
#include "Common/ComTry.h"
#include "Windows/PropVariant.h"
#include "CPP/Common/Buffer.h"

using namespace NWindows;

#include "HelperFuncs.h"
#include "7ZipFunctions.h"
#include "7ZipDllHandler.h"
#include "7ZipCodecInfo.h"
#include "7ZipFormatInfo.h"
#include "7ZipArchiveOpenCallback.h"
#include "7ZipCompressCodecsInfo.h"
#include "7ZipInStreamWrapper.h"

const UInt64 kMaxCheckStartPosition = 1 << 22;

extern bool Create7ZipArchive(C7ZipLibrary * pLibrary, IInArchive * pInArchive, C7ZipArchive ** pArchive);

static bool ReadStream(CMyComPtr<IInStream> & inStream, Int64 offset, UINT32 seekOrigin, CByteBuffer & signature) 
{
  UInt64 savedPosition = 0;
  UInt64 newPosition = 0;
  UInt32 readCount = signature.GetCapacity();
  unsigned char * buf = signature;

  if (S_OK != inStream->Seek(0, FILE_CURRENT, &savedPosition))
    return false;

  if (S_OK != inStream->Seek(offset, seekOrigin, &newPosition)) {
    inStream->Seek(savedPosition, FILE_BEGIN, &newPosition); //restore pos
    return false;
  }

  while (readCount > 0) {
    UInt32 processedCount = 0;

    if (S_OK != inStream->Read(buf, readCount, &processedCount)) {
      inStream->Seek(savedPosition, FILE_BEGIN, &newPosition); //restore pos
      return false;
    }

    if (processedCount == 0)
      break;

    readCount -= processedCount;
    buf += processedCount;
  }

  inStream->Seek(savedPosition, FILE_BEGIN, &newPosition); //restore pos

  return readCount == 0;
}

static int CreateInArchive(pU7ZipFunctions pFunctions,
						   const C7ZipObjectPtrArray & formatInfos,
                           CMyComPtr<IInStream> & inStream,
						   wstring ext,
						   CMyComPtr<IInArchive> & archive,
                           bool fCheckFileTypeBySignature)
{
  for (C7ZipObjectPtrArray::const_iterator it = formatInfos.begin();
       it != formatInfos.end();it++) {
    const C7ZipFormatInfo * pInfo = dynamic_cast<const C7ZipFormatInfo *>(*it);

    if (!fCheckFileTypeBySignature) {
      for(WStringArray::const_iterator extIt = pInfo->Exts.begin(); extIt != pInfo->Exts.end(); extIt++) {
        if (MyStringCompareNoCase((*extIt).c_str(), ext.c_str()) == 0) {
          return pFunctions->v.CreateObject(&pInfo->m_ClassID, 
                                            &IID_IInArchive, (void **)&archive);
        }
      }
    } else {
      if (pInfo->m_StartSignature.GetCapacity() == 0 /*&& pInfo->m_FinishSignature.length() == 0*/)
        continue; //no signature

      CByteBuffer signature;
      signature.SetCapacity(pInfo->m_StartSignature.GetCapacity());

      if (!ReadStream(inStream, 0, FILE_BEGIN, signature))
        continue; //unable to read signature

      if (signature == pInfo->m_StartSignature) {
        return pFunctions->v.CreateObject(&pInfo->m_ClassID, 
                                          &IID_IInArchive, (void **)&archive);
      }
    } //check file type by signature
  }

  return CLASS_E_CLASSNOTAVAILABLE;
}

static HRESULT InternalOpenArchive(C7ZipLibrary * pLibrary,
								   C7ZipDllHandler * pHandler,
								   C7ZipInStream * pInStream,
								   C7ZipArchiveOpenCallback * pOpenCallBack,
								   C7ZipArchive ** ppArchive, 
								   HRESULT * pResult,
                                   bool fCheckFileTypeBySignature);

HRESULT Lib7ZipOpenArchive(C7ZipLibrary * pLibrary,
						   C7ZipDllHandler * pHandler,
						   C7ZipInStream * pInStream,
						   C7ZipArchive ** ppArchive,
						   const wstring & passwd,
						   HRESULT * pResult,
                           bool fCheckFileTypeBySignature)
{
	C7ZipArchiveOpenCallback * pOpenCallBack = new C7ZipArchiveOpenCallback(NULL);
	
	if (passwd.length() > 0) {
		pOpenCallBack->PasswordIsDefined = true;
		pOpenCallBack->Password = passwd;
	}
	
	return InternalOpenArchive(pLibrary, pHandler, pInStream, 
                               pOpenCallBack, ppArchive, pResult, fCheckFileTypeBySignature);
}

HRESULT Lib7ZipOpenMultiVolumeArchive(C7ZipLibrary * pLibrary,
                                      C7ZipDllHandler * pHandler,
                                      C7ZipMultiVolumes * pMultiVolumes,
                                      C7ZipArchive ** ppArchive,
                                      const wstring & passwd,
                                      HRESULT * pResult,
                                      bool fCheckFileTypeBySignature)
{
	wstring firstVolumeName = pMultiVolumes->GetFirstVolumeName();

	if (!pMultiVolumes->MoveToVolume(firstVolumeName))
		return false;

	C7ZipInStream * pInStream = pMultiVolumes->OpenCurrentVolumeStream();

	if (pInStream == NULL)
		return false;
	
	C7ZipArchiveOpenCallback * pOpenCallBack = new C7ZipArchiveOpenCallback(pMultiVolumes);

	if (passwd.length() > 0) {
		pOpenCallBack->PasswordIsDefined = true;
		pOpenCallBack->Password = passwd;
	}

	return InternalOpenArchive(pLibrary, pHandler, pInStream, 
                               pOpenCallBack, ppArchive, pResult, fCheckFileTypeBySignature);
}

static HRESULT InternalOpenArchive(C7ZipLibrary * pLibrary,
								   C7ZipDllHandler * pHandler,
								   C7ZipInStream * pInStream,
								   C7ZipArchiveOpenCallback * pOpenCallBack,
								   C7ZipArchive ** ppArchive, 
								   HRESULT * pResult,
                                   bool fCheckFileTypeBySignature)
{
	CMyComPtr<IInArchive> archive = NULL;
	CMyComPtr<ISetCompressCodecsInfo> setCompressCodecsInfo = NULL;
	CMyComPtr<IInArchiveGetStream> getStream = NULL;
	wstring extension = pInStream->GetExt();

	C7ZipInStreamWrapper * pArchiveStream = new C7ZipInStreamWrapper(pInStream);

	CMyComPtr<IInStream> inStream(pArchiveStream); 

	CMyComPtr<IArchiveOpenCallback> openCallBack(pOpenCallBack);

	do {
		FAIL_RET(CreateInArchive(pHandler->GetFunctions(),
								 pHandler->GetFormatInfoArray(),
                                 inStream,
								 extension,
								 archive,
                                 fCheckFileTypeBySignature), pResult);

		if (archive == NULL)
			return false;

		archive.QueryInterface(IID_ISetCompressCodecsInfo, (void **)&setCompressCodecsInfo);

		if (setCompressCodecsInfo) {
			C7ZipCompressCodecsInfo * pCompressCodecsInfo =
				new C7ZipCompressCodecsInfo(pLibrary);
			RBOOLOK(setCompressCodecsInfo->SetCompressCodecsInfo(pCompressCodecsInfo));
		}

		FAIL_RET(archive->Open(inStream, &kMaxCheckStartPosition, openCallBack), pResult);

		UInt32 mainSubfile;
		{
			NCOM::CPropVariant prop;
			FAIL_RET(archive->GetArchiveProperty(kpidMainSubfile, &prop), pResult);
			if (prop.vt == VT_UI4)
				mainSubfile = prop.ulVal;
			else {
				break;
			}

			UInt32 numItems;
			FAIL_RET(archive->GetNumberOfItems(&numItems), pResult);
			if (mainSubfile >= numItems)
				break;
		}

		if (archive->QueryInterface(IID_IInArchiveGetStream, (void **)&getStream) != S_OK || !getStream)
			break;
    
		CMyComPtr<ISequentialInStream> subSeqStream;
		if (getStream->GetStream(mainSubfile, &subSeqStream) != S_OK || !subSeqStream)
			break;
    	
		inStream = NULL;
		if (subSeqStream.QueryInterface(IID_IInStream, &inStream) != S_OK || !inStream)
			break;
    
		wstring path;

		FAIL_RET(GetArchiveItemPath(archive, mainSubfile, path), pResult);

		CMyComPtr<IArchiveOpenSetSubArchiveName> setSubArchiveName;

		openCallBack->QueryInterface(IID_IArchiveOpenSetSubArchiveName, (void **)&setSubArchiveName);
		if (setSubArchiveName) {
			setSubArchiveName->SetSubArchiveName(path.c_str());
		}

		FAIL_RET(GetFilePathExt(path, extension), pResult);
	} while(true);

	if (archive == NULL)
		return S_FALSE;

	return Create7ZipArchive(pLibrary, archive, ppArchive) ? S_OK : S_FALSE;
}
