// Test7Zip.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "lib7zip.h"
#include <iostream>

class TestInStream : public C7ZipInStream
{
private:
	FILE * m_pFile;
	std::string m_strFileName;
	wstring m_strFileExt;
	int m_nFileSize;
public:
	TestInStream(std::string fileName, wstring ext) :
		m_strFileName(fileName),
		m_strFileExt(ext)
	{

		wprintf(L"fileName.c_str(): %s\n", fileName.c_str());
		wprintf(L"Ext:%ls\n", m_strFileExt.c_str());

		m_pFile = fopen(fileName.c_str(), "rb");

		if (m_pFile) {
			fseek(m_pFile, 0, SEEK_END);
			m_nFileSize = ftell(m_pFile);
			fseek(m_pFile, 0, SEEK_SET);
		}
		else {
			wprintf(L"fileName.c_str(): %s cant open\n", fileName.c_str());
		}
	}

	virtual ~TestInStream()
	{
		if (m_pFile)
			fclose(m_pFile);
	}

public:
	virtual wstring GetExt() const
	{
		wprintf(L"GetExt:%ls\n", m_strFileExt.c_str());
		return m_strFileExt;
	}

	virtual int Read(void *data, unsigned int size, unsigned int *processedSize)
	{
		if (!m_pFile)
			return 1;

		int count = fread(data, 1, size, m_pFile);
		wprintf(L"Read:%d %d\n", size, count);

		if (count >= 0) {
			if (processedSize != NULL)
				*processedSize = count;

			return 0;
		}

		return 1;
	}

	virtual int Seek(__int64 offset, unsigned int seekOrigin, unsigned __int64 *newPosition)
	{
		if (!m_pFile)
			return 1;

		int result = fseek(m_pFile, (long)offset, seekOrigin);

		if (!result) {
			if (newPosition)
				*newPosition = ftell(m_pFile);

			return 0;
		}

		return result;
	}

	virtual int GetSize(unsigned __int64 * size)
	{
		if (size)
			*size = m_nFileSize;
		return 0;
	}
};

const wchar_t * index_names[] = {
	L"kpidSize",
		L"kpidPackSize", //(Packed Size)
		L"kpidAttrib", //(Attributes)
		L"kpidCTime", //(Created)
		L"kpidATime", //(Accessed)
		L"kpidMTime", //(Modified)
		L"kpidSolid", //(Solid)
		L"kpidEncrypted", //(Encrypted)
		L"kpidUser", //(User)
		L"kpidGroup", //(Group)
		L"kpidComment", //(Comment)
		L"kpidPhySize", //(Physical Size)
		L"kpidHeadersSize", //(Headers Size)
		L"kpidChecksum", //(Checksum)
		L"kpidCharacts", //(Characteristics)
		L"kpidCreatorApp", //(Creator Application)
		L"kpidTotalSize", //(Total Size)
		L"kpidFreeSpace", //(Free Space)
		L"kpidClusterSize", //(Cluster Size)
		L"kpidVolumeName", //(Label)
		L"kpidPath", //(FullPath)
		L"kpidIsDir", //(IsDir)
};

#ifdef _WIN32
int _tmain(int argc, _TCHAR* argv[])
#else
int main(int argc, char * argv[])
#endif
{
	C7ZipLibrary lib;

	if (!lib.Initialize()) {
		wprintf(L"initialize fail!\n");
		return 1;
	}

	const char * files[] = {
		"Test7ZipProp.tar",
		"testzip.zip",
		"Test7Zip.7z",
		"lib7zip-1.4.0.tar.gz",
		"testbzip.bz2",
	};

	const wchar_t * files_ext[] = {
		L"tar",
		L"zip",
		L"7z",
		L"gz",
		L"bz2",
	};

	C7ZipArchive * pArchive = NULL;

	for (int i=0; i < sizeof(files) / sizeof(const char *); i++) {
		TestInStream stream(files[i], files_ext[i]);

		if (lib.OpenArchive(&stream, &pArchive)) {
			unsigned int numItems = 0;

			//print archive properties
			for(lib7zip::PropertyIndexEnum index = lib7zip::PROP_INDEX_BEGIN;
				index < lib7zip::PROP_INDEX_END;
				index = (lib7zip::PropertyIndexEnum)(index + 1)) {
				wstring strVal = L"";
				unsigned __int64 val = 0;
				bool bVal = false;

				bool result = pArchive->GetUInt64Property(index, val);

				wprintf(L"\n\nGetArciveProperty:%d %ls\n", (int)index,
						index_names[(int)index]);

				wprintf(L"Archive UInt64 result:%ls val=%ld\n",
						result ? L"true" : L"false",
						val);

				result = pArchive->GetBoolProperty(index, bVal);

				wprintf(L"Archive Bool result:%ls val=%ls\n",
						result ? L"true" : L"false",
						bVal ? L"true" : L"false");

				result = pArchive->GetStringProperty(index, strVal);

				wprintf(L"Archive String result:%ls val=%ls\n",
						result ? L"true" : L"false",
						strVal.c_str());

				result = pArchive->GetFileTimeProperty(index, val);

				wprintf(L"Archive FileTime result:%ls val=%ld\n",
						result ? L"true" : L"false",
						val);
			}

			pArchive->GetItemCount(&numItems);

			wprintf(L"Contains Items:%d\n", numItems);

			for(unsigned int i = 0;i < numItems;i++) {
				C7ZipArchiveItem * pArchiveItem = NULL;

				if (pArchive->GetItemInfo(i, &pArchiveItem)) {
					wprintf(L"%d,%ls,%d\n", pArchiveItem->GetArchiveIndex(),
							pArchiveItem->GetFullPath().c_str(),
							pArchiveItem->IsDir());

					wprintf(L"get all properties\n");
					for(lib7zip::PropertyIndexEnum index = lib7zip::PROP_INDEX_BEGIN;
						index <= lib7zip::PROP_INDEX_END;
						index = (lib7zip::PropertyIndexEnum)(index + 1)) {
						wstring strVal = L"";
						unsigned __int64 val = 0;
						bool bVal = false;

						bool result = pArchiveItem->GetUInt64Property(index, val);

						wprintf(L"\n\nGetProperty:%d %ls\n", (int)index,
								index_names[(int)index]);

						wprintf(L"UInt64 result:%ls val=%ld\n",
								result ? L"true" : L"false",
								val);

						result = pArchiveItem->GetBoolProperty(index, bVal);

						wprintf(L"Bool result:%ls val=%ls\n",
								result ? L"true" : L"false",
								bVal ? L"true" : L"false");

						result = pArchiveItem->GetStringProperty(index, strVal);

						wprintf(L"String result:%ls val=%ls\n",
								result ? L"true" : L"false",
								strVal.c_str());

						result = pArchiveItem->GetFileTimeProperty(index, val);

						wprintf(L"FileTime result:%ls val=%ld\n",
								result ? L"true" : L"false",
								val);
					}
				}
			}
		}
		else {
			wprintf(L"open archive %s fail\n", files[i]);
		}
	}

	if (pArchive != NULL)
		delete pArchive;

	return 0;
}
