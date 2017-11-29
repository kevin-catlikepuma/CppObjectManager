// x3c - C++ PluginFramework
#include "stdafx.h"
#include <PluginInc.h>
#include <vecfunc.h>
#include "Cx_ObjectFactory.h"


Cx_ObjectFactory::Cx_ObjectFactory()
    : m_unloading(0), m_loading(0)
{
    ASSERT(x3InMainThread());   // see win32impl.h
}

Cx_ObjectFactory::~Cx_ObjectFactory()
{
    for (std::vector<MODULE*>::iterator it = m_modules.begin(); 
        it != m_modules.end(); ++it)
    {
        x3::SafeDelete(*it);
    }
    m_modules.clear();
}

bool Cx_ObjectFactory::IsCreatorRegister(const X3CLSID& clsid)
{
    return FindEntry(clsid) != NULL;
}

int Cx_ObjectFactory::CreateObject(const X3CLSID& clsid,
                                   X3IID iid,
                                   Ix_Object** ppv,
                                   HMODULE fromdll)
{
    ASSERT(clsid.valid() && ppv != NULL);
    *ppv = NULL;

    int moduleIndex = -1;
    X3CLASSENTRY* pEntry = FindEntry(clsid, &moduleIndex);

    if (pEntry && !pEntry->pfnObjectCreator && moduleIndex >= 0)
    {
        if (!LoadDelayedPlugin_(m_modules[moduleIndex]->filename)
            && 0 == m_unloading && x3InMainThread())
        {
            CLSMAP::iterator it = m_clsmap.find(clsid.str());
            if (it != m_clsmap.end())
            {
                it->second.second = -1; // next time: moduleIndex = -1
            }
        }
        pEntry = FindEntry(clsid, &moduleIndex);
    }

    if (pEntry && pEntry->pfnObjectCreator)
    {
        *ppv = pEntry->pfnObjectCreator(iid, fromdll);
    }

    return *ppv ? 0 : 3;
}

X3CLASSENTRY* Cx_ObjectFactory::FindEntry(const X3CLSID& clsid,
                                          int* moduleIndex)
{
    CLSMAP::iterator it = m_clsmap.find(clsid.str());
    if (moduleIndex)
    {
        *moduleIndex = (it == m_clsmap.end()) ? -1 : it->second.second;
    }
    return (it == m_clsmap.end()) ? NULL : &(it->second.first);
}

int Cx_ObjectFactory::FindModule(HMODULE hModule)
{
    if (!hModule)
    {
        return -1;
    }

    int i = x3::GetSize(m_modules);

    while (--i >= 0 && m_modules[i]->hdll != hModule)
    {
    }

    return i;
}

Ix_Module* Cx_ObjectFactory::GetModule(HMODULE hModule)
{
    int index = FindModule(hModule);
    if (index >= 0)
    {
        return m_modules[index]->module;
    }

    typedef Ix_Module* (*FUNC_MODULE)(Ix_ObjectFactory*, HMODULE);
    FUNC_MODULE pfn = (FUNC_MODULE)GetProcAddress(
        hModule, "x3GetModuleInterface");

    if (pfn != NULL)
    {
        Ix_Module* pModule = (*pfn)(this, hModule);
        return pModule;
    }
    else
    {
        return NULL;
    }
}

long Cx_ObjectFactory::RegisterClassEntryTable(int moduleIndex)
{
    ASSERT(moduleIndex >= 0);
    HMODULE hdll = m_modules[moduleIndex]->hdll;
    Ix_Module* pModule = m_modules[moduleIndex]->module;
    ASSERT(hdll && pModule);

    if (!m_modules[moduleIndex]->clsids.empty())
    {
        return x3::GetSize(m_modules[moduleIndex]->clsids);
    }

    typedef DWORD (*FUNC_GET)(DWORD*, DWORD*, X3CLASSENTRY*, DWORD);
    FUNC_GET pfn = (FUNC_GET)GetProcAddress(hdll, "x3GetClassEntryTable");

    if (!pfn)       // is not a plugin
    {
        return -1;
    }

    DWORD buildInfo = 0;
    int classCount = (*pfn)(&buildInfo, NULL, NULL, 0);

    if (classCount <= 0)
    {
        return 0;
    }

    std::vector<X3CLASSENTRY> table(classCount);
    DWORD size = sizeof(X3CLASSENTRY);

    classCount = (*pfn)(NULL, &size, &table[0], classCount);

    for (int i = 0; i < classCount; i++)
    {
        const X3CLASSENTRY& cls = table[i];

        if (cls.clsid.valid())
        {
            RegisterClass(moduleIndex, cls);
        }
    }

    return classCount;
}

bool Cx_ObjectFactory::RegisterClass(int moduleIndex,
                                     const X3CLASSENTRY& cls)
{
    ASSERT(moduleIndex >= 0 && cls.clsid.valid());
    X3CLASSENTRY* pOldCls = FindEntry(cls.clsid);

    if (pOldCls && pOldCls->pfnObjectCreator)
    {
#ifdef OutputDebugString
        char msg[256] = { 0 };
        sprintf_s(msg, 256,
            "The classid '%s' is already registered by '%s', "
            "then '%s' register fail.\n",
            cls.clsid.str(), pOldCls->className, cls.className);
        OutputDebugStringA(msg);
#endif
        return false;
    }

    m_clsmap[cls.clsid.str()] = MAPITEM(cls, moduleIndex);
    m_modules[moduleIndex]->clsids.push_back(cls.clsid);

    return true;
}

void Cx_ObjectFactory::ReleaseModule(HMODULE hModule)
{
    int index = FindModule(hModule);
    ASSERT(index >= 0);

    MODULE* item = m_modules[index];
    CLSIDS::const_iterator it = item->clsids.begin();

    for (; it != item->clsids.end(); ++it)
    {
        CLSMAP::iterator mit = m_clsmap.find(it->str());
        if (mit != m_clsmap.end())
        {
            m_clsmap.erase(mit);
        }
    }

    if (item->owned)
    {
        FreeLibrary(hModule);
    }

    // don't remove: m_modules.erase(m_modules.begin() + index);
    item->hdll = NULL;
    item->module = NULL;
    item->clsids.clear();
}
