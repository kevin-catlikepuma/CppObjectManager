// Included by portimpl.h to implement functions on Windows.

#ifndef X3WIN32_PORTABILITY_IMPL_H
#define X3WIN32_PORTABILITY_IMPL_H

#include "x3win.h"

bool x3OpenFileForRead(HANDLE& hfile, const wchar_t* filename)
{
    hfile = ::CreateFileW(filename,
        GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    return hfile != INVALID_HANDLE_VALUE;
}

bool x3OpenFileForWrite(HANDLE& hfile, const wchar_t* filename)
{
    hfile = ::CreateFileW(filename,
        GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
    return hfile != INVALID_HANDLE_VALUE;
}

bool x3CloseFile(HANDLE file)
{
    return !!CloseHandle(file);
}

HMODULE x3LoadLibrary(const wchar_t* filename)
{
    return ::LoadLibraryExW(filename, NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
}

bool x3CheckFileAttributes(const wchar_t* filename, bool* readonly, bool* folder)
{
    DWORD attr = GetFileAttributesW(filename);

    if (readonly)
        *readonly = !!(attr & FILE_ATTRIBUTE_READONLY);
    if (folder)
        *folder = !!(attr & FILE_ATTRIBUTE_DIRECTORY);

    return attr != (DWORD)-1;
}

bool x3SetFileAttributesNormal(const wchar_t* filename)
{
    return !!SetFileAttributesW(filename, FILE_ATTRIBUTE_NORMAL);
}

bool x3InMainThread()
{
    static DWORD s_mainid = 0;
    DWORD id = GetCurrentThreadId();

    if (s_mainid == 0)
    {
        s_mainid = GetCurrentThreadId();
    }

    return id == s_mainid;
}

#endif // X3WIN32_PORTABILITY_IMPL_H
