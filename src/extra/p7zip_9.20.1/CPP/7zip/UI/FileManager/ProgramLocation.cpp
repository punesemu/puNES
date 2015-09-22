// ProgramLocation.h

#include "StdAfx.h"

#include "ProgramLocation.h"

// #include "Windows/FileName.h"
#include "Common/StringConvert.h"


bool GetProgramFolderPath(UString &folder)
{
  const char *p7zip_home_dir = getenv("P7ZIP_HOME_DIR");
  if (p7zip_home_dir == 0) p7zip_home_dir="./";

  folder = MultiByteToUnicodeString(p7zip_home_dir);

  return true;
}

