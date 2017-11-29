/*! \file RelToAbs.h
 *  \brief Define functions converting relative path to absolute path: RelToAbsWithPlugin, FileNameRelToAbs
 *  \author Zhang Yungui, X3 C++ PluginFramework
 *  \date   2010.10.22
 */
#ifndef UTILFUNC_RELTOABS_H_
#define UTILFUNC_RELTOABS_H_

#include "XComPtr.h"
#include <string>

namespace x3 {

//! Return the module handle of main application which load plugins initiative.
inline HMODULE GetMainModuleHandle()
{
    HMODULE hModule = GetModuleHandleW(L"PluginManager.plugin");
    typedef HMODULE (*FUNC_GET)();
    FUNC_GET fn = (FUNC_GET)GetProcAddress(hModule, "x3GetMainModuleHandle");
    return fn ? (*fn)() : NULL;
}

//! Converte relative path to absolute path basing on the current plugin file.
/*!
    \ingroup _GROUP_UTILFUNC
    \param relname Relative path or file name to be converted.
    \param isfile 'relname' is a file name or path.
    \return Absolute path converted. If isfile is false then the result will end with slash char.
    \see x3::FileNameRelToAbs, x3::GetAppWorkPath()
*/
inline std::wstring RelToAbsWithPlugin(const wchar_t* relname, bool isfile = true)
{
    wchar_t path[MAX_PATH] = { 0 };

    GetModuleFileNameW(x3GetModuleHandle(), path, MAX_PATH);
    PathRemoveFileSpecW(path);
    PathAppendW(path, relname);

    PathRemoveBackslashW(path);
    if (!isfile)
    {
        PathAddBackslashW(path);
    }

    return path;
}

//! Converte relative path to absolute path basing on the main application.
/*!
    \ingroup _GROUP_UTILFUNC
    \param relname Relative path or file name to be converted.
    \param isfile 'relname' is a file name or path.
    \return Absolute path converted. If isfile is false then the result will end with slash char.
    \see x3::RelToAbsWithPlugin, x3::GetAppWorkPath()
*/
inline std::wstring FileNameRelToAbs(const wchar_t* relname, bool isfile = true)
{
    wchar_t path[MAX_PATH] = { 0 };

    GetModuleFileNameW(GetMainModuleHandle(), path, MAX_PATH);
    PathRemoveFileSpecW(path);
    PathAppendW(path, relname);

    PathRemoveBackslashW(path);
    if (!isfile)
    {
        PathAddBackslashW(path);
    }

    return path;
}

//! Ensure a path ending with slash char.
/*!
    \ingroup _GROUP_UTILFUNC
    \param[in,out] path absolute path name.
    \return same as 'path'.
*/
inline std::wstring EnsurePathHasSlash(std::wstring& path)
{
    if (!path.empty())
    {
        wchar_t c = path[path.size() - 1];
        if (c != L'\\' && c != L'/')
        {
            path += L'\\';
        }
    }
    return path;
}

} // x3
#endif // UTILFUNC_RELTOABS_H_
