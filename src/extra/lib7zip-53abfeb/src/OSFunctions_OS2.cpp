#ifdef OS2

#include <iostream>
#include "HelperFuncs.h"

static HMODULE *hmod2;

void *GetProcAddress (HMODULE hmod, const char *symbol) {
    void *addr = NULL;
    PFN      ModuleAddr     = 0;
    APIRET rc = 0;
    HMODULE hmod3 = *hmod2;
    //HMODULE hmod3 = hmod;
    std::cerr << "DosQueryProcAddr " << symbol << " from module hmod3: " << hmod2 << std::endl;
    char *symbol3 = strdup(symbol);
    //std::cerr << "DosQueryProcAddr (2) " << symbol2 << " from module hmod3: " << hmod2 << std::endl;

    /* Load DLL and get hmod with DosLoadModule*/
    /* Get address for process in DLL */
    rc = DosQueryProcAddr(hmod3, 0, (PSZ )symbol3, &ModuleAddr);
    /*
	  return codes:
	  0 NO_ERROR
	  6 ERROR_INVALID_HANDLE
	  123 ERROR_INVALID_NAME
	  182 ERROR_INVALID_ORDINAL
	  65079 ERROR_ENTRY_IS_CALLGATE (this error code is not valid in OS/2 Warp PowerPC Edition)
    */
    if(rc) {
        // error;
#if defined(_OS2_LIBDEBUG)
        std::cerr << "DosQueryProcAddr of " << symbol << " from module at " << hmod2 << " failed. " <<  " (ret code: " << rc << ")" << std::endl;
#endif
    } else {
        addr = (void *)ModuleAddr;
#if defined(_OS2_LIBDEBUG)
        std::cerr << "DosQueryProcAddr " << symbol << " from module at " << hmod2 << " succeeded (addr: " << &ModuleAddr << " ). " << std::endl;
#endif

    }
    return addr;
}

HMODULE Load7ZLibrary(const wstring & name) {
    unsigned char  szErrorName[CCHMAXPATH];
    unsigned char libname[20];
    szErrorName[0]= '\0';
    void *pHandler = NULL;
    HMODULE *hmod = (HMODULE *) calloc(1, sizeof(HMODULE));
    APIRET rc;
	string tmpName = NarrowString(wstring(L".\\") + name + L".dll");
    strncpy((char *)libname, tmpName.c_str(), 19);
    rc = DosLoadModule(szErrorName, CCHMAXPATH-1, libname, hmod);
    /*
	  return codes:
	  0	NO_ERROR
	  2	ERROR_FILE_NOT_FOUND
	  3	ERROR_PATH_NOT_FOUND
	  4	ERROR_TOO_MANY_OPEN_FILES
	  5	ERROR_ACCESS_DENIED
	  8	ERROR_NOT_ENOUGH_MEMORY
	  11	ERROR_BAD_FORMAT
	  26	ERROR_NOT_DOS_DISK
	  32	ERROR_SHARING_VIOLATION
	  33	ERROR_LOCK_VIOLATION
	  36	ERROR_SHARING_BUFFER_EXCEEDED
	  95	ERROR_INTERRUPT
	  108	ERROR_DRIVE_LOCKED
	  123	ERROR_INVALID_NAME
	  127	ERROR_PROC_NOT_FOUND
	  180	ERROR_INVALID_SEGMENT_NUMBER
	  182	ERROR_INVALID_ORDINAL
	  190	ERROR_INVALID_MODULETYPE
	  191	ERROR_INVALID_EXE_SIGNATURE
	  192	ERROR_EXE_MARKED_INVALID
	  194	ERROR_ITERATED_DATA_EXCEEDS_64K
	  195	ERROR_INVALID_MINALLOCSIZE
	  196	ERROR_DYNLINK_FROM_INVALID_RING
	  198	ERROR_INVALID_SEGDPL
	  199	ERROR_AUTODATASEG_EXCEEDS_64K
	  201	ERROR_RELOCSRC_CHAIN_EXCEEDS_SEGLIMIT
	  206	ERROR_FILENAME_EXCED_RANGE
	  295	ERROR_INIT_ROUTINE_FAILED
    */
    hmod2 = hmod;
    pHandler = hmod2;
#if defined(_OS2_LIBDEBUG)
    std::cerr << "DosLoadModule: rc:" << rc << ", pHandler: " << pHandler << std::endl;
#endif
    if (rc) {
#if defined(_OS2_LIBDEBUG)
        std::cerr << "DosLoadModule: loading " << libname << " failed: " <<  szErrorName << ",  (ret code: " << rc << ")" << std::endl;
#endif
		pHandler = NULL;
    } else {
#if defined(_OS2_LIBDEBUG)
        std::cerr << "DosLoadModule: loading " << libname << " succeeded. " << std::endl;
#endif
    }

	return pHandler;
}

void Free7ZLibrary(HMODULE pModule)
{
	APIRET rc=0;
	rc = DosFreeModule(pModule);
	if (rc) {
#if defined(_OS2_LIBDEBUG)
        std::cerr << "DosFreeModule: unloading  lib at " << pModule << " failed,  (ret code: " << rc << ")" << std::endl;
#endif
    } else {
#if defined(_OS2_LIBDEBUG)
        std::cerr << "DosFreeModule: unloading lib at " << pModule << " succeeded. " << std::endl;
#endif
    }
	free(hmod2);
	hmod2 = NULL;
}

wstring GetHandlerPath(void * pHandler)
{
    HMODULE *handler = (HMODULE *)pHandler
		HMODULE hmod = *handler;

    char module_name[CCHMAXPATH];
    char *result;
    int rc;
    module_name[0] = '\0';

	/*
	  return values
	  0 NO_ERROR
	  6 ERROR_INVALID_HANDLE
	  24 ERROR_BAD_LENGTH
	*/
    rc = DosQueryModuleName(*hmod2, sizeof module_name, module_name);

    if (rc ) {
		std::cerr << "DosQueryModuleName of " << handler << "  failed: (ret:  "  << rc <<   ")"<< std::endl;
	} else {
#if defined(_OS2_LIBDEBUG)
		std::cerr << "DosQueryModuleName of " << handler << "  succeeded. " << std::endl;
#endif
    }

    return WidenString(module_name);
}

bool LoadDllFromFolder(C7ZipDllHandler * pMainHandler, const wstring & wfolder_name, C7ZipObjectPtrArray & handlers)
{
#if defined(_OS2_LIBDEBUG)
		std::cerr << "LoadDllFromFolder do nothing on OS2 " << std::endl;
#endif
	return true;
}
#endif //_OS2
