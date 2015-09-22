#include "lib7zip.h"

#ifdef S_OK
#undef S_OK
#endif

#include "CPP/7zip/Archive/IArchive.h"
#include "CPP/Windows/PropVariant.h"
#include "CPP/Common/MyCom.h"
#include "CPP/7zip/ICoder.h"
#include "CPP/7zip/IPassword.h"
#include "CPP/7zip/Common/FileStreams.h"

#include "HelperFuncs.h"

const wchar_t *kEmptyFileAlias = L"[Content]";

class C7ZipArchiveItemImpl : public virtual C7ZipArchiveItem
{
public:
	C7ZipArchiveItemImpl(IInArchive * pInArchive,
						 unsigned int nIndex);
	virtual ~C7ZipArchiveItemImpl();

public:
	virtual wstring GetFullPath() const;
	virtual UInt64 GetSize() const;
	virtual bool IsDir() const;
	virtual bool IsEncrypted() const;
	virtual unsigned int GetArchiveIndex() const;
	virtual wstring GetArchiveItemPassword() const;
	virtual void SetArchiveItemPassword(const wstring & password);
	bool IsPasswordSet() const;

	virtual bool GetUInt64Property(lib7zip::PropertyIndexEnum propertyIndex,
								   unsigned __int64 & val) const;
	virtual bool GetBoolProperty(lib7zip::PropertyIndexEnum propertyIndex,
								 bool & val) const;
	virtual bool GetStringProperty(lib7zip::PropertyIndexEnum propertyIndex,
								   wstring & val) const;
	virtual bool GetFileTimeProperty(lib7zip::PropertyIndexEnum propertyIndex,
								   unsigned __int64 & val) const;
private:
	CMyComPtr<IInArchive> m_pInArchive;
	unsigned int m_nIndex;
	wstring m_Password;
};

C7ZipArchiveItemImpl::C7ZipArchiveItemImpl(IInArchive * pInArchive,
										   unsigned int nIndex) :
	m_pInArchive(pInArchive),
	m_nIndex(nIndex)
{
}

C7ZipArchiveItemImpl::~C7ZipArchiveItemImpl()
{
}

wstring C7ZipArchiveItemImpl::GetFullPath() const
{
	// Get Name
	NWindows::NCOM::CPropVariant prop;
	wstring fullPath = kEmptyFileAlias;

	if (!m_pInArchive->GetProperty(m_nIndex, kpidPath, &prop)) {
		if (prop.vt == VT_BSTR)
			fullPath = prop.bstrVal;
	}

	return fullPath;
}

UInt64 C7ZipArchiveItemImpl::GetSize() const
{
	// Get uncompressed size
	NWindows::NCOM::CPropVariant prop;
	if (m_pInArchive->GetProperty(m_nIndex, kpidSize, &prop) != 0)
		return 0;

	UInt64 size = 0;

	if (prop.vt == VT_UI8 || prop.vt == VT_UI4)
		size = ConvertPropVariantToUInt64(prop);

	return size;
}

bool C7ZipArchiveItemImpl::IsEncrypted() const
{
	// Check if encrypted (password protected)
	NWindows::NCOM::CPropVariant prop;
	bool isEncrypted = false;
	if (m_pInArchive->GetProperty(m_nIndex, kpidEncrypted, &prop) == 0 && prop.vt == VT_BOOL)
		isEncrypted = prop.bVal;
	return isEncrypted;
}

bool C7ZipArchiveItemImpl::IsDir() const
{
	// Check IsDir
	NWindows::NCOM::CPropVariant prop;
	bool isDir = false;
	IsArchiveItemFolder(m_pInArchive, m_nIndex, isDir);

	return isDir;
}

unsigned int C7ZipArchiveItemImpl::GetArchiveIndex() const
{
	return m_nIndex;
}

wstring C7ZipArchiveItemImpl::GetArchiveItemPassword() const
{
	return m_Password;
}

void C7ZipArchiveItemImpl::SetArchiveItemPassword(const wstring & password)
{
	m_Password = password;
}

bool C7ZipArchiveItemImpl::IsPasswordSet() const
{
	return !(m_Password == L"");
}


bool C7ZipArchiveItemImpl::GetUInt64Property(lib7zip::PropertyIndexEnum propertyIndex,
											 unsigned __int64 & val) const
{
	int p7zip_index = 0;

	switch(propertyIndex) {
	case lib7zip::kpidSize:
		p7zip_index = kpidSize;
		break;
	case lib7zip::kpidPackSize: //(Packed Size)
		p7zip_index = kpidPackSize;
		break;
	case lib7zip::kpidAttrib: //(Attributes)
		p7zip_index = kpidAttrib;
		break;
	case lib7zip::kpidPhySize: //(Physical Size)
		p7zip_index = kpidPhySize;
		break;
	case lib7zip::kpidHeadersSize: //(Headers Size)
		p7zip_index = kpidHeadersSize;
		break;
	case lib7zip::kpidChecksum: //(Checksum)
		p7zip_index = kpidChecksum;
		break;
	case lib7zip::kpidTotalSize: //(Total Size)
		p7zip_index = kpidTotalSize;
		break;
	case lib7zip::kpidFreeSpace: //(Free Space)
		p7zip_index = kpidFreeSpace;
		break;
	case lib7zip::kpidClusterSize: //(Cluster Size)
		p7zip_index = kpidClusterSize;
		break;
	default:
		return false;
	}

	NWindows::NCOM::CPropVariant prop;

	if (m_pInArchive->GetProperty(m_nIndex, p7zip_index, &prop) != 0)
		return false;

	if (prop.vt == VT_UI8 || prop.vt == VT_UI4) {
		val = ConvertPropVariantToUInt64(prop);
		return true;
	}

	return false;
}

bool C7ZipArchiveItemImpl::GetBoolProperty(lib7zip::PropertyIndexEnum propertyIndex,
										   bool & val) const
{
	int p7zip_index = 0;

	switch(propertyIndex) {
	case lib7zip::kpidSolid: //(Solid)
		p7zip_index = kpidSolid;
		break;
	case lib7zip::kpidEncrypted: //(Encrypted)
		p7zip_index = kpidEncrypted;
		break;
	case lib7zip::kpidIsDir: //(IsDir)
		return IsArchiveItemFolder(m_pInArchive, m_nIndex, val) == S_OK;
	default:
		return false;
	}

	NWindows::NCOM::CPropVariant prop;

	if (m_pInArchive->GetProperty(m_nIndex, p7zip_index, &prop) == 0 && 
		prop.vt == VT_BOOL) {
		val = prop.bVal;
		return true;
	}

	return false;
}

bool C7ZipArchiveItemImpl::GetStringProperty(lib7zip::PropertyIndexEnum propertyIndex,
					   wstring & val) const
{
	int p7zip_index = 0;

	switch(propertyIndex) {
	case lib7zip::kpidComment: //(Comment)
		p7zip_index = kpidComment;
		break;
	case lib7zip::kpidCharacts: //(Characteristics)
		p7zip_index = kpidCharacts;
		break;
	case lib7zip::kpidCreatorApp: //(Creator Application)
		p7zip_index = kpidCreatorApp;
		break;
	case lib7zip::kpidVolumeName: //(Label)
		p7zip_index = kpidVolumeName;
		break;
	case lib7zip::kpidPath: //(FullPath)
		p7zip_index = kpidPath;
		break;
	case lib7zip::kpidUser: //(User)
		p7zip_index = kpidUser;
		break;
	case lib7zip::kpidGroup: //(Group)
		p7zip_index = kpidGroup;
		break;
	default:
		return false;
	}

	NWindows::NCOM::CPropVariant prop;

	if (!m_pInArchive->GetProperty(m_nIndex, p7zip_index, &prop) &&
		prop.vt == VT_BSTR) {
		val = prop.bstrVal;
		return true;
	}

	return false;
}

bool C7ZipArchiveItemImpl::GetFileTimeProperty(lib7zip::PropertyIndexEnum propertyIndex,
											 unsigned __int64 & val) const
{
	int p7zip_index = 0;

	switch(propertyIndex) {
	case lib7zip::kpidCTime: //(Created)
		p7zip_index = kpidCTime;
		break;
	case lib7zip::kpidATime: //(Accessed)
		p7zip_index = kpidATime;
		break;
	case lib7zip::kpidMTime: //(Modified)
		p7zip_index = kpidMTime;
		break;
	default:
		return false;
	}

	NWindows::NCOM::CPropVariant prop;

	if (m_pInArchive->GetProperty(m_nIndex, p7zip_index, &prop) != 0)
		return false;

	if (prop.vt == VT_FILETIME) {
		unsigned __int64 tmp_val = 0;
		memmove(&tmp_val, &prop.filetime, sizeof(unsigned __int64));
		val = tmp_val;
		return true;
	}

	return false;
}

bool Create7ZipArchiveItem(C7ZipArchive * pArchive, 
						   IInArchive * pInArchive,
						   unsigned int nIndex,
						   C7ZipArchiveItem ** ppItem)
{
	*ppItem = new C7ZipArchiveItemImpl(pInArchive, nIndex);

	return true;
}

/*-------------------- C7ZipArchiveItem ----------------------*/
C7ZipArchiveItem::C7ZipArchiveItem()
{
}

C7ZipArchiveItem::~C7ZipArchiveItem()
{
}

