#include "lib7zip.h"

// Minimal stub implementation of C7ZipObjectPtrArray
// This provides the required symbols without the complex template dependencies

C7ZipObjectPtrArray::C7ZipObjectPtrArray(bool auto_release) 
    : m_bAutoRelease(auto_release) {
}

C7ZipObjectPtrArray::~C7ZipObjectPtrArray() {
    clear();
}

void C7ZipObjectPtrArray::clear() {
    if (m_bAutoRelease) {
        for (size_t i = 0; i < size(); i++) {
            delete (*this)[i];
        }
    }
    std::vector<C7ZipObject*>::clear();
}