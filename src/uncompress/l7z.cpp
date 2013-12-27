/*
 *  l7z.cpp
 *
 *   Created on: 22/dic/2013
 *       Author: fhorse
 */

#include <cstdio>
#include <cstring>
#include <libgen.h>
#include <stdlib.h>
#include <strings.h>
#include "l7z.h"
#include "lib7zip.h"

struct _l7z {
	BYTE present;
	C7ZipLibrary lib;
} l7z;

class in_stream: public C7ZipInStream {
	private:
		FILE * m_pFile;
		std::string m_strFileName;
		wstring m_strFileExt;
		int m_nFileSize;
	public:
		in_stream(std::string file) : m_strFileName(file), m_strFileExt(L"7z") {
			//printf("file.c_str(): %s\n", file.c_str());
			m_pFile = fopen(file.c_str(), "rb");

			if (m_pFile) {
				fseek(m_pFile, 0, SEEK_END);
				m_nFileSize = ftell(m_pFile);
				fseek(m_pFile, 0, SEEK_SET);

				size_t pos = m_strFileName.find_last_of('.');

				if (pos) {
					pos++;

					m_strFileExt = wstring(m_strFileName.c_str() + pos,
					        (m_strFileName.c_str() + pos) + strlen(m_strFileName.c_str() + pos));
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
		std::string m_strFileName;
		wstring m_strFileExt;
		int m_nFileSize;
	public:
		out_stream(std::string file) : m_strFileName(file), m_strFileExt(L"7z") {
			m_pFile = fopen(file.c_str(), "wb");
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
BYTE l7z_control_ext(char *ext) {
	WStringArray exts;

	if (!l7z.lib.GetSupportedExts(exts)) {
		return (EXIT_ERROR);
	}

	for (size_t i = 0; i < exts.size(); i++) {
		wstring l7zext = exts[i];

		wcstombs((char *) &uncomp.buffer, l7zext.c_str(), sizeof(uncomp.buffer));

		if (strlen(ext + 1) != strlen(uncomp.buffer)) {
			continue;
		}

		if (strcasecmp(ext + 1, uncomp.buffer) == 0) {
			return (EXIT_OK);
		}
	}

	return (EXIT_ERROR);
}
BYTE l7z_control_in_archive(void) {
	C7ZipArchive *archive = NULL;
	unsigned int a, num_items = 0;

	in_stream stream(info.rom_file);

	if (!l7z.lib.OpenArchive(&stream, &archive)) {
		fprintf(stderr, "open archive %s fail\n", info.rom_file);
		return (EXIT_ERROR);
	}

	archive->GetItemCount(&num_items);

	for (a = 0; a < num_items; a++) {
		C7ZipArchiveItem *archive_item = NULL;
		unsigned int b;

		if (!(archive->GetItemInfo(a, &archive_item))) {
			continue;
		}

		/* se e' una directory continuo */
		if (archive_item->IsDir()) {
			continue;
		}

		//printf("%d,%ls,%d\n", archive_item->GetArchiveIndex(),
		//       archive_item->GetFullPath().c_str(), archive_item->IsDir());

		for (b = 0; b < LENGTH(format_supported); b++) {
			char buf[50];
			char *ext;

			memset(&buf, 0x00, sizeof(buf));
			wcstombs((char *) &buf, archive_item->GetFullPath().c_str(), sizeof(buf));

			ext = strrchr(buf, '.');

			if ((ext != NULL) && (strcasecmp(ext, format_supported[b].ext) == 0)) {
				uncomp.file[uncomp.files_founded].num = archive_item->GetArchiveIndex();
				uncomp.file[uncomp.files_founded].format = format_supported[b].id;
				uncomp.files_founded++;
				break;
			}
		}
	}

	delete (archive);
	return (EXIT_OK);
}
BYTE l7z_file_from_archive(_uncomp_file_data *file) {
	C7ZipArchive *archive = NULL;
	C7ZipArchiveItem *archive_item = NULL;
	in_stream stream(info.rom_file);

	if (!l7z.lib.OpenArchive(&stream, &archive)) {
		return (EXIT_ERROR);
	}

	if (!archive->GetItemInfo(file->num, &archive_item)) {
		delete (archive);
		return (EXIT_ERROR);
	}

	wcstombs((char *) &uncomp.buffer, archive_item->GetFullPath().c_str(), sizeof(uncomp.buffer));

	snprintf(uncomp.uncompress_file, sizeof(uncomp.uncompress_file), "%s" TMP_FOLDER "/%s",
	        info.base_folder, basename(uncomp.buffer));

	out_stream o_stream(uncomp.uncompress_file);

	if (!archive->Extract(archive_item, &o_stream)) {
		fprintf(stderr, "uncompress file failed!\n");
		delete (archive);
		return (EXIT_ERROR);
	}

	strncpy(uncomp.compress_archive, info.rom_file, sizeof(uncomp.compress_archive));
	strncpy(info.rom_file, uncomp.uncompress_file, sizeof(info.rom_file));
	info.uncompress_rom = TRUE;

	delete (archive);
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
		delete (archive);
		return (EXIT_ERROR);
	}

	wcstombs((char *) &uncomp.buffer, archive_item->GetFullPath().c_str(), sizeof(uncomp.buffer));

	delete (archive);
	return (EXIT_OK);
}
