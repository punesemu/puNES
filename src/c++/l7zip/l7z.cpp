/*
 *  Copyright (C) 2010-2022 Fabio Cavallo (aka FHorse)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <cstdio>
#include <cstring>
#include <libgen.h>
#include <stdlib.h>
#include <strings.h>
#include "l7z.h"
#include "extra/lib7zip-53abfeb/src/lib7zip.h"
#include "info.h"
#include "gui.h"

static struct _l7z {
	BYTE present;
	C7ZipLibrary lib;
} l7z;

class in_stream: public C7ZipInStream {
	private:
		FILE * m_pFile;
#if defined (_WIN32)
		wstring m_strFileName;
#else
		std::string m_strFileName;
#endif
		wstring m_strFileExt;
		int m_nFileSize;
	public:
#if defined (_WIN32)
		in_stream(wstring file) : m_strFileName(file), m_strFileExt(L"7z") {
#else
		in_stream(std::string file) : m_strFileName(file), m_strFileExt(L"7z") {
#endif
			m_pFile = ufopen(file.c_str(), uL("rb"));

			if (m_pFile) {
				fseek(m_pFile, 0, SEEK_END);
				m_nFileSize = ftell(m_pFile);
				fseek(m_pFile, 0, SEEK_SET);

				size_t pos = m_strFileName.find_last_of('.');

				if (pos) {
					pos++;

					m_strFileExt = wstring(m_strFileName.c_str() + pos,
							(m_strFileName.c_str() + pos) + ustrlen(m_strFileName.c_str() + pos));
				}
			}
		}

		virtual ~in_stream() {
			fclose(m_pFile);
		}
	public:
		virtual wstring GetExt() const {
			return (m_strFileExt);
		}

		virtual int Read(void *data, unsigned int size, unsigned int *processedSize) {
			int count;

			if (!m_pFile) {
				return (EXIT_ERROR);
			}

			count = fread(data, 1, size, m_pFile);

			if (count >= 0) {
				if (processedSize != NULL) {
					*processedSize = count;
				}

				return (EXIT_OK);
			}

			return (EXIT_ERROR);
		}

		virtual int Seek(__int64 offset, unsigned int seekOrigin, unsigned __int64 *newPosition) {
			int result;

			if (!m_pFile) {
				return (EXIT_ERROR);
			}

			result = fseek(m_pFile, (long)offset, seekOrigin);

			if (!result) {
				if (newPosition) {
					*newPosition = ftell(m_pFile);
				}

				return (EXIT_OK);
			}

			return (result);
		}

		virtual int GetSize(unsigned __int64 *size) {
			if (size) {
				*size = m_nFileSize;
			}

			return (EXIT_OK);
		}
};
class out_stream: public C7ZipOutStream {
	private:
		FILE * m_pFile;
#if defined (_WIN32)
		wstring m_strFileName;
#else
		std::string m_strFileName;
#endif
		wstring m_strFileExt;
		int m_nFileSize;
	public:
#if defined (_WIN32)
		out_stream(wstring file) : m_strFileName(file), m_strFileExt(L"7z") {
#else
		out_stream(std::string file) : m_strFileName(file), m_strFileExt(L"7z") {
#endif
			m_pFile = ufopen(file.c_str(), uL("wb"));
			m_nFileSize = 0;
		}

		virtual ~out_stream() {
			fclose(m_pFile);
		}
	public:
		int GetFileSize() const {
			return m_nFileSize;
		}

		virtual int Write(const void *data, unsigned int size, unsigned int *processedSize) {
			int count = fwrite(data, 1, size, m_pFile);

			if (count >= 0) {
				if (processedSize != NULL) {
					*processedSize = count;
				}

				m_nFileSize += count;

				return (EXIT_OK);
			}

			return (EXIT_ERROR);
		}

		virtual int Seek(__int64 offset, unsigned int seekOrigin, unsigned __int64 *newPosition) {
			int result = fseek(m_pFile, (long) offset, seekOrigin);

			if (!result) {
				if (newPosition) {
					*newPosition = ftell(m_pFile);
				}

				return (EXIT_OK);
			}

			return (result);
		}

		virtual int SetSize(unsigned __int64 size) {
			if (size) {
				return (EXIT_OK);
			}

			return (EXIT_OK);
		}
};

BYTE l7z_init(void) {
	l7z = {};

	if (!l7z.lib.Initialize()) {
		return (EXIT_ERROR);
	}

	l7z.present = TRUE;

	return (EXIT_OK);
}
void l7z_quit(void) {
	if (l7z.present == TRUE) {
		l7z.lib.Deinitialize();
		l7z.present = FALSE;
	}
}
BYTE l7z_present(void) {
	return (l7z.present);
}
BYTE l7z_control_ext(const uTCHAR *ext) {
	WStringArray exts;

	if (!l7z.lib.GetSupportedExts(exts)) {
		return (EXIT_ERROR);
	}

	for (size_t i = 0; i < exts.size(); i++) {
		wstring l7zext = exts[i];

#if defined (_WIN32)
		if (ustrlen(ext + 1) != ustrlen(uPTCHAR(l7zext.c_str()))) {
			continue;
		}

		if (ustrcasecmp(uPTCHAR(ext + 1), uPTCHAR(l7zext.c_str())) == 0) {
			return (EXIT_OK);
		}
#else
		uTCHAR buffer[10];

		wcstombs((char *)buffer, l7zext.c_str(), sizeof(buffer) - 1);

		if (strlen(ext + 1) != strlen(buffer)) {
			continue;
		}

		if (strcasecmp(ext + 1, buffer) == 0) {
			return (EXIT_OK);
		}
#endif
	}

	return (EXIT_ERROR);
}
BYTE l7z_examine_archive(_uncompress_archive *archive) {
	C7ZipArchive *c7zarchive = NULL;
	unsigned int a, num_items = 0;
	in_stream stream(archive->file);

	if (!l7z.lib.OpenArchive(&stream, &c7zarchive)) {
		ufprintf(stderr, uL("open archive " uPs("") " fail\n"), archive->file);
		return (UNCOMPRESS_EXIT_ERROR_ON_UNCOMP);
	}

	c7zarchive->GetItemCount(&num_items);

	for (a = 0; a < num_items; a++) {
		uTCHAR file[LENGTH_FILE_NAME_LONG], *ext;
		C7ZipArchiveItem *item = NULL;
		unsigned int b;

		if (!(c7zarchive->GetItemInfo(a, &item))) {
			continue;
		}

		// se e' una directory continuo
		if (item->IsDir()) {
			continue;
		}

		//printf("%d,%ls,%d\n", item->GetArchiveIndex(), item->GetFullPath().c_str(), item->IsDir());

		umemset(file, 0x00, usizeof(file));
#if defined (_WIN32)
		ustrncpy(file, item->GetFullPath().c_str(), usizeof(file) - 1);
#else
		wcstombs((char *)file, item->GetFullPath().c_str(), sizeof(file) - 1);
#endif
		if ((ext = ustrrchr(file, uL('.'))) == NULL) {
			continue;
		}

		for (b = 0; b < LENGTH(uncompress_exts); b++) {
			if (ustrcasecmp(ext, (uTCHAR *)uncompress_exts[b].e) == 0) {
				_uncompress_archive_items_list *list = &archive->list;

				if (uncompress_exts[b].type == UNCOMPRESS_TYPE_ROM) {
					archive->rom.count++;
				} else if (uncompress_exts[b].type == UNCOMPRESS_TYPE_PATCH) {
					archive->patch.count++;
				} else {
					continue;
				}

				list->item = (_uncompress_archive_item *)realloc(list->item,
					(list->count + 1) * sizeof(_uncompress_archive_item));

				{
					_uncompress_archive_item *aitem = &list->item[list->count];

					aitem->type = uncompress_exts[b].type;
					aitem->index = item->GetArchiveIndex();
				}

				list->count++;
				break;
			}
		}
	}

	delete (c7zarchive);

	return (UNCOMPRESS_EXIT_OK);
}
BYTE l7z_extract_from_archive(_uncompress_archive *archive, uint32_t selected, BYTE type) {
	C7ZipArchive *c7zarchive = NULL;
	C7ZipArchiveItem *item = NULL;
	uTCHAR file[LENGTH_FILE_NAME_LONG], basename[255];
	in_stream stream(archive->file);
	_uncompress_archive_item *aitem;

	if ((aitem = uncompress_archive_find_item(archive, selected, type)) == NULL) {
		return (UNCOMPRESS_EXIT_ERROR_ON_UNCOMP);
	}

	if (!l7z.lib.OpenArchive(&stream, &c7zarchive)) {
		return (UNCOMPRESS_EXIT_ERROR_ON_UNCOMP);
	}

	if (!c7zarchive->GetItemInfo(aitem->index, &item)) {
		delete (c7zarchive);
		return (UNCOMPRESS_EXIT_ERROR_ON_UNCOMP);
	}

#if defined (_WIN32)
	ustrncpy(file, item->GetFullPath().c_str(), usizeof(file) - 1);
#else
	wcstombs((char *)file, item->GetFullPath().c_str(), sizeof(file) - 1);
#endif
	gui_utf_basename(file, basename, usizeof(basename));
	usnprintf(file, usizeof(file), uL("" uPs("") TMP_FOLDER "/" uPs("")), info.base_folder, basename);

	{
		out_stream o_stream(file);
		uint32_t storage_index;

		if (!c7zarchive->Extract(item, &o_stream)) {
			fprintf(stderr, "uncompress file failed!\n");
			delete (c7zarchive);
			return (UNCOMPRESS_EXIT_ERROR_ON_UNCOMP);
		}

		storage_index = uncompress_storage_add_to_list(archive, aitem, file);

		switch (type) {
			default:
			case UNCOMPRESS_TYPE_ALL:
			case UNCOMPRESS_TYPE_ROM:
				archive->rom.storage_index = storage_index;
				break;
			case UNCOMPRESS_TYPE_PATCH:
				archive->patch.storage_index = storage_index;
				break;
		}
	}

	delete (c7zarchive);

	return (UNCOMPRESS_EXIT_OK);
}
uTCHAR *l7z_item_file_name(_uncompress_archive *archive, uint32_t selected, BYTE type) {
	static uTCHAR file[LENGTH_FILE_NAME_LONG];
	C7ZipArchive *c7zarchive = NULL;
	C7ZipArchiveItem *item = NULL;
	in_stream stream(archive->file);
	_uncompress_archive_item *aitem;

	if ((aitem = uncompress_archive_find_item(archive, selected, type)) == NULL) {
		return (NULL);
	}

	if (!l7z.lib.OpenArchive(&stream, &c7zarchive)) {
		return (NULL);
	}

	if (!c7zarchive->GetItemInfo(aitem->index, &item)) {
		delete (c7zarchive);
		return (NULL);
	}

	umemset(file, 0x00, usizeof(file));
#if defined (_WIN32)
	ustrncpy(file, item->GetFullPath().c_str(), usizeof(file) - 1);
# else
	wcstombs((char *)file, item->GetFullPath().c_str(), usizeof(file) - 1);
#endif

	delete (c7zarchive);

	return (file);
}
