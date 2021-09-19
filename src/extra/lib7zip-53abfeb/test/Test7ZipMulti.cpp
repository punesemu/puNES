// Test7Zip.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "lib7zip.h"
#include <iostream>

#include <locale>
#include <iostream>
#include <string>
#include <sstream>

wstring widen( const string& str )
{
	std::wostringstream wstm ;
      wstm.imbue(std::locale("en_US.utf8"));
      const std::ctype<wchar_t>& ctfacet =
		  std::use_facet< std::ctype<wchar_t> >( wstm.getloc() ) ;
      for( size_t i=0 ; i<str.size() ; ++i )
      wstm << ctfacet.widen( str[i] ) ;
      return wstm.str() ;
}

string narrow( const wstring& str )
{
	std::ostringstream stm ;
      stm.imbue(std::locale("C"));
      const std::ctype<char>& ctfacet =
		  std::use_facet< std::ctype<char> >( stm.getloc() ) ;
      for( size_t i=0 ; i<str.size() ; ++i )
      stm << ctfacet.narrow( str[i], 0 ) ;
      return stm.str() ;
}

class TestInStream : public C7ZipInStream
{
private:
	FILE * m_pFile;
	wstring m_strFileName;
	wstring m_strFileExt;
	int m_nFileSize;
public:
	TestInStream(wstring fileName) :
	  m_pFile(NULL)
      , m_strFileName(fileName)
      , m_strFileExt(L"001")
	{
		string f = narrow(fileName);

		m_pFile = fopen(f.c_str(), "rb");

		if (m_pFile)
		{
			fseek(m_pFile, 0, SEEK_END);
			m_nFileSize = ftell(m_pFile);
			fseek(m_pFile, 0, SEEK_SET);
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
		return m_strFileExt;
	}

	virtual int Read(void *data, unsigned int size, unsigned int *processedSize)
	{
		wprintf(L"Read\n");
		int count = fread(data, 1, size, m_pFile);
		wprintf(L"Read:%d %d\n", size, count);

		if (count >= 0)
		{
			if (processedSize != NULL)
				*processedSize = count;

			return 0;
		}

		return 1;
	}

	virtual int Seek(__int64 offset, unsigned int seekOrigin, unsigned __int64 *newPosition)
	{
		wprintf(L"Seek\n");
		int result = fseek(m_pFile, (long)offset, seekOrigin);
		wprintf(L"Seek:%ld %ld\n", offset, result);
		if (!result)
		{
			if (newPosition)
				*newPosition = ftell(m_pFile);

			return 0;
		}

		return result;
	}

	virtual int GetSize(unsigned __int64 * size)
	{
		wprintf(L"Size\n");
		if (size)
			*size = m_nFileSize;
		return 0;
	}
};

class TestMultiVolumes : public C7ZipMultiVolumes
{
private:
	FILE * m_pFile;
	wstring m_strFileName;
	int m_nFileSize;
	wstring m_strCurVolume;
	bool m_done;

public:
	TestMultiVolumes(wstring fileName) :
	  m_pFile(NULL),
	  m_strFileName(fileName),
	  m_done(false)
	{
	}

	virtual ~TestMultiVolumes()
	{
		if (m_pFile)
			fclose(m_pFile);
	}

public:
	virtual wstring GetFirstVolumeName() {
		m_strCurVolume = m_strFileName;
		MoveToVolume(m_strCurVolume);
		return m_strCurVolume;
	}

	virtual bool MoveToVolume(const wstring& volumeName) {
		m_strCurVolume = volumeName;
		wprintf(L"move to volume:%ls\n", volumeName.c_str());

		if (m_pFile)
			fclose(m_pFile);
		m_pFile = NULL;
		string f = narrow(volumeName);
		wprintf(L"narrow volume:%s\n", f.c_str());

		m_pFile = fopen(f.c_str(), "rb");

		if (!m_pFile)
			m_done = true;
		else {
		fseek(m_pFile, 0, SEEK_END);
		m_nFileSize = ftell(m_pFile);
		fseek(m_pFile, 0, SEEK_SET);
		}

		return !m_done;
	}

	virtual C7ZipInStream * OpenCurrentVolumeStream() {
		return new TestInStream(m_strCurVolume);
	}

	virtual unsigned __int64 GetCurrentVolumeSize() {
		wprintf(L"get current volume size:%ls\n", m_strCurVolume.c_str());
		return m_nFileSize;
	}
};

class TestOutStream : public C7ZipOutStream
{
private:
	FILE * m_pFile;
	std::string m_strFileName;
	wstring m_strFileExt;
	int m_nFileSize;
public:
	TestOutStream(std::string fileName) :
	  m_strFileName(fileName),
	  m_strFileExt(L"7z")
	{
		m_pFile = fopen(fileName.c_str(), "wb");
		m_nFileSize = 0;

		auto pos = m_strFileName.find_last_of(".");

		if (pos != m_strFileName.npos)
		{
#ifdef _WIN32
			std::string tmp = m_strFileName.substr(pos + 1);
			int nLen = MultiByteToWideChar(CP_ACP, 0, tmp.c_str(), -1, NULL, NULL);
			LPWSTR lpszW = new WCHAR[nLen];
			MultiByteToWideChar(CP_ACP, 0,
			   tmp.c_str(), -1, lpszW, nLen);
			m_strFileExt = lpszW;
			// free the string
			delete[] lpszW;
#else
			m_strFileExt = L"7z";
#endif
		}
		wprintf(L"Ext:%ls\n", m_strFileExt.c_str());
	}

	virtual ~TestOutStream()
	{
		fclose(m_pFile);
	}

public:
	int GetFileSize() const
	{
		return m_nFileSize;
	}

	virtual int Write(const void *data, unsigned int size, unsigned int *processedSize)
	{
		int count = fwrite(data, 1, size, m_pFile);
		wprintf(L"Write:%d %d\n", size, count);

		if (count >= 0)
		{
			if (processedSize != NULL)
				*processedSize = count;

			m_nFileSize += count;
			return 0;
		}

		return 1;
	}

	virtual int Seek(__int64 offset, unsigned int seekOrigin, unsigned __int64 *newPosition)
	{
		int result = fseek(m_pFile, (long)offset, seekOrigin);

		if (!result)
		{
			if (newPosition)
				*newPosition = ftell(m_pFile);

			return 0;
		}

		return result;
	}

	virtual int SetSize(unsigned __int64 size)
	{
		wprintf(L"SetFileSize:%ld\n", size);
		return 0;
	}
};

#ifdef _WIN32
int _tmain(int argc, _TCHAR* argv[])
#else
int main(int argc, char * argv[])
#endif
{
	C7ZipLibrary lib;

	if (!lib.Initialize())
	{
		wprintf(L"initialize fail!\n");
		return 1;
	}

	WStringArray exts;

	if (!lib.GetSupportedExts(exts))
	{
		wprintf(L"get supported exts fail\n");
		return 1;
	}

	size_t size = exts.size();

	for(size_t i = 0; i < size; i++)
	{
		wstring ext = exts[i];

		for(size_t j = 0; j < ext.size(); j++)
		{
			wprintf(L"%c", (char)(ext[j] &0xFF));
		}

		wprintf(L"\n");
	}

	C7ZipArchive * pArchive = NULL;

	TestMultiVolumes volumes(L"test.7z.001");
	TestOutStream oStream("TestMultiResult.txt");
	if (lib.OpenMultiVolumeArchive(&volumes, &pArchive))
	{
		unsigned int numItems = 0;

		pArchive->GetItemCount(&numItems);

		wprintf(L"%d\n", numItems);

		for(unsigned int i = 0;i < numItems;i++)
		{
			C7ZipArchiveItem * pArchiveItem = NULL;

			if (pArchive->GetItemInfo(i, &pArchiveItem))
			{
				wprintf(L"%d,%ls,%d\n", pArchiveItem->GetArchiveIndex(),
					pArchiveItem->GetFullPath().c_str(),
					pArchiveItem->IsDir());
			}
				//set archive password or item password
				pArchive->SetArchivePassword(L"test");
				if (i==0) {
					//Or set password for each archive item
					//pArchiveItem->SetArchiveItemPassword(L"test");
					pArchive->Extract(pArchiveItem, &oStream);
				}
		}
	}
	else
	{
		wprintf(L"open archive test.7z.001 fail\n");
	}

	if (pArchive != NULL)
		delete pArchive;

	return 0;
}
