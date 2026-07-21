#ifndef LIB7ZIP_COMPAT_H
#define LIB7ZIP_COMPAT_H

// Compatibility layer for 7zip 25.0

// Remove conflicting defines
#ifdef CLASS_E_CLASSNOTAVAILABLE
#undef CLASS_E_CLASSNOTAVAILABLE
#endif

// Include 7zip headers first
#include "CPP/Common/MyCom.h"

// Use proper COM implementation macros
#define MY_UNKNOWN_IMP1_MT(i) \
  STDMETHOD(QueryInterface)(REFIID iid, void **outObject) throw(); \
  STDMETHOD_(ULONG, AddRef)() throw(); \
  STDMETHOD_(ULONG, Release)() throw(); \
  private: LONG _refCount; \
  public: CMyComPtr<i> _impl;

#define MY_UNKNOWN_IMP2_MT(i1, i2) \
  STDMETHOD(QueryInterface)(REFIID iid, void **outObject) throw(); \
  STDMETHOD_(ULONG, AddRef)() throw(); \
  STDMETHOD_(ULONG, Release)() throw(); \
  private: LONG _refCount; \
  public: CMyComPtr<i1> _impl1; CMyComPtr<i2> _impl2;

#define MY_UNKNOWN_IMP3_MT(i1, i2, i3) \
  STDMETHOD(QueryInterface)(REFIID iid, void **outObject) throw(); \
  STDMETHOD_(ULONG, AddRef)() throw(); \
  STDMETHOD_(ULONG, Release)() throw(); \
  private: LONG _refCount; \
  public: CMyComPtr<i1> _impl1; CMyComPtr<i2> _impl2; CMyComPtr<i3> _impl3;

#define Z7_COM_UNKNOWN_IMP_1(i) MY_UNKNOWN_IMP1_MT(i)
#define Z7_COM_UNKNOWN_IMP_2(i1, i2) MY_UNKNOWN_IMP2_MT(i1, i2)
#define Z7_COM_UNKNOWN_IMP_3(i1, i2, i3) MY_UNKNOWN_IMP3_MT(i1, i2, i3)

// For newer versions, methods should use throw() specification
#if MY_VER_MAJOR >= 15
#define COM_METHOD_THROW throw()
#else  
#define COM_METHOD_THROW
#endif

#endif // LIB7ZIP_COMPAT_H