#include "lib7zip.h"

#ifdef S_OK
#undef S_OK
#endif

/*------------------------ C7ZipObjectPtrArray ---------------------*/
C7ZipObjectPtrArray::C7ZipObjectPtrArray(bool auto_release) : m_bAutoRelease(auto_release)
{
}

C7ZipObjectPtrArray::~C7ZipObjectPtrArray()
{
    clear();
}

void C7ZipObjectPtrArray::clear()
{
    if (m_bAutoRelease)
    {
        for(C7ZipObjectPtrArray::iterator it = begin(); it != end(); it ++)
        {
            delete *it;
        }
    }

    std::vector<C7ZipObject *>::clear();
}

