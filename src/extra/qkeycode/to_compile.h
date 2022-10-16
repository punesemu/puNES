#ifndef TO_COMPILE_H
#define TO_COMPILE_H

#if defined(_WIN32)
#if !defined(Q_OS_WINDOWS)
#define Q_OS_WINDOWS
#endif
#else
#if !defined(Q_OS_LINUX)
#define Q_OS_LINUX
#endif
#endif

#endif // TO_COMPILE_H
