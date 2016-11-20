/*
 *  Copyright (C) 2010-2016 Fabio Cavallo (aka FHorse)
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
#include "lib7zip.h"
#include "info.h"
#include "gui.h"

static struct _l7z {
	BYTE present;
	C7ZipLibrary lib;
} l7z;

class in_stream: public C7ZipInStream {
	private:
		FILE * m_pFile;
#if defined (__WIN32__)
		wstring m_strFileName;
#else
		std::string m_strFileName;
#endif
		wstring m_strFileExt;
		int m_nFileSize;
	public:
#if defined (__WIN32__)
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

			result = fseek(m_pFile, (long) offset, seekOrigin);

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
#if defined (__WIN32__)
		wstring m_strFileName;
#else
		std::string m_strFileName;
#endif
		wstring m_strFileExt;
		int m_nFileSize;
	public:
#if defined (__WIN32__)
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
	memset(&l7z, 0x00, sizeof(l7z));

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

#if defined (__WIN32__)
		if (ustrlen(ext + 1) != ustrlen(uPTCHAR(l7zext.c_str()))) {
			continue;
		}

		if (ustrcasecmp(uPTCHAR(ext + 1), uPTCHAR(l7zext.c_str())) == 0) {
			return (EXIT_OK);
		}
#else
		wcstombs((char *) uncomp.buffer, l7zext.c_str(), sizeof(uncomp.buffer) - 1);

		if (strlen(ext + 1) != strlen(uncomp.buffer)) {
			continue;
		}

		if (strcasecmp(ext + 1, uncomp.buffer) == 0) {
			return (EXIT_OK);
		}
#endif
	}

	return (EXIT_ERROR);
}
BYTE l7z_control_in_archive(void) {
	C7ZipArchive *archive = NULL;
	unsigned int a, mode, num_items = 0;
	in_stream stream(info.rom_file);

	if (!l7z.lib.OpenArchive(&stream, &archive)) {
		ufprintf(stderr, uL("open archive " uPERCENTs " fail\n"), info.rom_file);
		return (EXIT_ERROR);
	}

	archive->GetItemCount(&num_items);

	for (mode = UNCOMP_CTRL_FILE_COUNT_ROMS; mode <= UNCOMP_CTRL_FILE_SAVE_DATA; mode++) {
		uncomp.files_founded = 0;

		for (a = 0; a < num_items; a++) {
			C7ZipArchiveItem *archive_item = NULL;
			unsigned int b;

			if (!(archive->GetItemInfo(a, &archive_item))) {
				continue;
			}

			// se e' una directory continuo
			if (archive_item->IsDir()) {
				continue;
			}

			//printf("%d,%ls,%d\n", archive_item->GetArchiveIndex(),
			//       archive_item->GetFullPath().c_str(), archive_item->IsDir());

			for (b = 0; b < LENGTH(format_supported); b++) {
				uTCHAR *ext;

				umemset(uncomp.buffer, 0x00, usizeof(uncomp.buffer));
#if defined (__WIN32__)
				ustrncpy(uncomp.buffer, archive_item->GetFullPath().c_str(),
						usizeof(uncomp.buffer) - 1);
#else
				wcstombs((char *) uncomp.buffer, archive_item->GetFullPath().c_str(),
						sizeof(uncomp.buffer) - 1);
#endif

				ext = ustrrchr(uncomp.buffer, uL('.'));

				if ((ext != NULL) && (ustrcasecmp(ext, (uTCHAR *) format_supported[b].ext) == 0)) {
					if (mode == UNCOMP_CTRL_FILE_SAVE_DATA) {
						uncomp.file[uncomp.files_founded].num = archive_item->GetArchiveIndex();
						uncomp.file[uncomp.files_founded].format = format_supported[b].id;
					}
					uncomp.files_founded++;
					break;
				}
			}
		}

		if ((mode == UNCOMP_CTRL_FILE_COUNT_ROMS) && (uncomp.files_founded > 0)) {
			uncomp.file = (_uncomp_file_data *) malloc(
			        uncomp.files_founded * sizeof(_uncomp_file_data));
		}
	}

	delete(archive);

	return (EXIT_OK);
}
BYTE l7z_file_from_archive(_uncomp_file_data *file) {
	C7ZipArchive *archive = NULL;
	C7ZipArchiveItem *archive_item = NULL;
	uTCHAR basename[255];
	in_stream stream(info.rom_file);

	if (!l7z.lib.OpenArchive(&stream, &archive)) {
		return (EXIT_ERROR);
	}

	if (!archive->GetItemInfo(file->num, &archive_item)) {
		delete(archive);
		return (EXIT_ERROR);
	}

#if defined (__WIN32__)
	ustrncpy(uncomp.buffer, archive_item->GetFullPath().c_str(), usizeof(uncomp.buffer) - 1);
#else
	wcstombs((char *) uncomp.buffer, archive_item->GetFullPath().c_str(), sizeof(uncomp.buffer) - 1);
#endif
	gui_utf_basename(uncomp.buffer, basename, usizeof(basename));
	usnprintf(uncomp.uncompress_file, usizeof(uncomp.uncompress_file),
			uL("" uPERCENTs TMP_FOLDER "/" uPERCENTs),info.base_folder, basename);
	out_stream o_stream(uncomp.uncompress_file);

	if (!archive->Extract(archive_item, &o_stream)) {
		fprintf(stderr, "uncompress file failed!\n");
		delete(archive);
		return (EXIT_ERROR);
	}

	ustrncpy(uncomp.compress_archive, info.rom_file, usizeof(uncomp.compress_archive));
	ustrncpy(info.rom_file, uncomp.uncompress_file, usizeof(info.rom_file));
	info.uncompress_rom = TRUE;

	delete(archive);

	return (EXIT_OK);
}
BYTE l7z_name_file_compress(_uncomp_file_data *file) {
	C7ZipArchive *archive = NULL;
	C7ZipArchiveItem *archive_item = NULL;
	in_stream stream(info.rom_file);

	if (!l7z.lib.OpenArchive(&stream, &archive)) {
		return (EXIT_ERROR);
	}

	if (!archive->GetItemInfo(file->num, &archive_item)) {
		delete(archive);
		return (EXIT_ERROR);
	}

	umemset(uncomp.buffer, 0x00, usizeof(uncomp.buffer));
#if defined (__WIN32__)
	ustrncpy(uncomp.buffer, archive_item->GetFullPath().c_str(), usizeof(uncomp.buffer) - 1);
# else
	wcstombs((char *) uncomp.buffer, archive_item->GetFullPath().c_str(), usizeof(uncomp.buffer) - 1);
#endif

	delete(archive);

	return (EXIT_OK);
}
