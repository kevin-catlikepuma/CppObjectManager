// PluginManager.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include <afxdllx.h>

#include <x3win.h>
#include <RelToAbs.h>
#include "Cx_PluginLoader.h"
#include <Cx_Object.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


class Cx_PluginLoaderOut : public Cx_Object<Cx_PluginLoader>
{
public:
    Cx_PluginLoaderOut() : m_hModule(NULL)
    {
    }

    HMODULE         m_hModule;
    std::wstring    m_path;

    int CreateObject(const X3CLSID& clsid, X3IID iid, Ix_Object** ppv, HMODULE fromdll)
    {
        if (x3::CLSID_AppWorkPath == clsid
            || x3::CLSID_PluginDelayLoad == clsid)
        {
            return QueryObject(iid, ppv, fromdll) ? 0 : 1;
        }
        return Cx_PluginLoader::CreateObject(clsid, iid, ppv, fromdll);
    }

private:
    Cx_PluginLoaderOut(const Cx_PluginLoaderOut&);
    void operator=(const Cx_PluginLoaderOut&);

    std::wstring GetWorkPath();
    void SetWorkPath(const std::wstring& path);
    std::wstring GetLocalAppDataPath(const wchar_t* company);
    std::wstring GetTranslationsPath(const wchar_t* subfolder);

    bool IsOnVistaDisk();
    bool GetLocalAppDataPath_(wchar_t* path);
};

static Cx_PluginLoaderOut s_loader;

OUTAPI Ix_ObjectFactory* x3GetRegisterBank()
{
    return &s_loader;
}

Ix_ObjectFactory* x3GetObjectFactory()
{
    return &s_loader;
}

OUTAPI HMODULE x3GetMainModuleHandle()
{
    return s_loader.GetMainModuleHandle();
}

HMODULE x3GetModuleHandle()
{
    return s_loader.m_hModule;
}

int x3CreateObject(const X3CLSID& clsid, X3IID iid, Ix_Object** ppv)
{
    return s_loader.CreateObject(clsid, iid, ppv, x3GetModuleHandle());
}

static inline void GetBasePath(wchar_t* path)
{
    GetModuleFileNameW(x3GetModuleHandle(), path, MAX_PATH);
    PathRemoveFileSpecW(path);
    if (_wcsicmp(L"plugins", PathFindFileNameW(path)) == 0)
        PathRemoveFileSpecW(path);
}

std::wstring Cx_PluginLoaderOut::GetWorkPath()
{
    if (m_path.empty())
    {
        if (IsOnVistaDisk())
        {
            m_path = GetLocalAppDataPath(L"x3c");
        }
        else
        {
            wchar_t path[MAX_PATH] = { 0 };
            GetBasePath(path);
            PathAddBackslashW(path);
            m_path = path;
        }
    }
    return m_path;
}

void Cx_PluginLoaderOut::SetWorkPath(const std::wstring& path)
{
    m_path = path;
    x3::EnsurePathHasSlash(m_path);
}

std::wstring Cx_PluginLoaderOut::GetLocalAppDataPath(const wchar_t* company)
{
    wchar_t path[MAX_PATH] = { 0 };

    if (GetLocalAppDataPath_(path))
    {
        wchar_t appname[MAX_PATH];
        GetModuleFileNameW(GetMainModuleHandle(), appname, MAX_PATH);

        if (company)
            PathAppendW(path, company);
        PathAppendW(path, PathFindFileNameW(appname));
        PathRenameExtensionW(path, L"");
        PathAddBackslashW(path);
    }
    else
    {
        GetBasePath(path);
        PathAddBackslashW(path);
    }

    return path;
}

std::wstring Cx_PluginLoaderOut::GetTranslationsPath(const wchar_t* subfolder)
{
    wchar_t code[4] = L"chs";
    wchar_t path[MAX_PATH] = { 0 };

#ifdef _WIN32
    typedef LANGID (WINAPI*PFNGETUSERDEFAULTUILANGUAGE)();
    PFNGETUSERDEFAULTUILANGUAGE pfnGetLang;

    pfnGetLang = (PFNGETUSERDEFAULTUILANGUAGE)::GetProcAddress(
        ::GetModuleHandleW(L"kernel32.dll"), "GetUserDefaultUILanguage");
    if (pfnGetLang != NULL)
    {
        LANGID langid = pfnGetLang();
        ::GetLocaleInfoW(langid, LOCALE_SABBREVLANGNAME, code, 4);
    }
#endif

    GetBasePath(path);
    PathAppendW(path, L"translations");
    PathAppendW(path, code);
    if (subfolder && *subfolder)
        PathAppendW(path, subfolder);

    return path;
}

bool Cx_PluginLoaderOut::IsOnVistaDisk()
{
    bool ret = false;
#ifdef WINOLEAPI_
    OSVERSIONINFO osvi = { sizeof(OSVERSIONINFO) };
    WCHAR winpath[MAX_PATH], exepath[MAX_PATH];

    if (GetVersionEx(&osvi) && osvi.dwMajorVersion >= 6)
    {
        GetSystemDirectoryW(winpath, MAX_PATH);
        GetModuleFileNameW(GetMainModuleHandle(), exepath, MAX_PATH);
        ret = (StrCmpNIW(winpath, exepath, 2) == 0);
    }
#endif
    return ret;
}

bool Cx_PluginLoaderOut::GetLocalAppDataPath_(wchar_t* path)
{
    bool ret = false;
#if defined(WINOLEAPI_) && !defined(__GNUC__)
    // FOLDERID_LocalAppDataGUID {F1B32785-6FBA-4FCF-9D55-7B8E7F157091}
    const GUID uuidLocalAppData = {0xF1B32785,0x6FBA,0x4FCF,
        {0x9D,0x55,0x7B,0x8E,0x7F,0x15,0x70,0x91}};

    typedef HRESULT (STDAPICALLTYPE *PFNGET)(REFGUID, DWORD, HANDLE, PWSTR*);
    HMODULE hdll = LoadLibraryW(L"SHELL32.DLL");

    if (hdll != NULL)
    {
        PWSTR shpath = NULL;
        PFNGET pfn = (PFNGET)GetProcAddress(hdll, "SHGetKnownFolderPath");

        if (pfn != NULL)
        {
            pfn(uuidLocalAppData, 0, NULL, &shpath);
        }
        if (shpath != NULL)
        {
            wcscpy(path, shpath);
            ret = (*path != 0);
            CoTaskMemFree(shpath);
        }
        FreeLibrary(hdll);
    }
#endif
    return ret;
}

static AFX_EXTENSION_MODULE PluginManagerDLL = { NULL, NULL };

extern "C" int APIENTRY
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	// Remove this if you use lpReserved
	UNREFERENCED_PARAMETER(lpReserved);

	if (dwReason == DLL_PROCESS_ATTACH)
	{
		TRACE0("PLUGINMANAGER.DLL Initializing!\n");
		
		// Extension DLL one-time initialization
		if (!AfxInitExtensionModule(PluginManagerDLL, hInstance))
			return 0;

		// Insert this DLL into the resource chain
		// NOTE: If this Extension DLL is being implicitly linked to by
		//  an MFC Regular DLL (such as an ActiveX Control)
		//  instead of an MFC application, then you will want to
		//  remove this line from DllMain and put it in a separate
		//  function exported from this Extension DLL.  The Regular DLL
		//  that uses this Extension DLL should then explicitly call that
		//  function to initialize this Extension DLL.  Otherwise,
		//  the CDynLinkLibrary object will not be attached to the
		//  Regular DLL's resource chain, and serious problems will
		//  result.

		new CDynLinkLibrary(PluginManagerDLL);

		s_loader.m_hModule = (HMODULE)hInstance;
        DisableThreadLibraryCalls(s_loader.m_hModule);
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		TRACE0("PLUGINMANAGER.DLL Terminating!\n");
		// Terminate the library before destructors are called
		AfxTermExtensionModule(PluginManagerDLL);
	}
	return 1;   // ok
}
