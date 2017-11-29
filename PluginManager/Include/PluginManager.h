/*! \file PluginManager.h
 *  \brief Define plugin manager wrapper class
 *  \author Zhang Yungui, X3 C++ PluginFramework
 *  \date   2011.06.30
 */
#ifndef X3_MANAGER_PLUGINMANAGER_H_
#define X3_MANAGER_PLUGINMANAGER_H_

#include "Ix_ObjectFactory.h"
#include "Ix_PluginLoader.h"
#include "XComPtr.h"
/*! \ingroup _GROUP_PLUGIN_CORE_
 *  \brief Plugin manager wrapper class
 *  \see   Ix_ObjectFactory, Ix_PluginLoader
 */
class CPluginManager
{
public:
    CPluginManager()
    {
		wcscpy_s(m_filename, MAX_PATH, L"PluginManager.plugin");
    }

    ~CPluginManager()
    {
		UnLoadFunPlugin();
        Unload();
    }

    //! Unload plugin manager and plugins
    void Unload(bool all = false)
    {
        if (all || Handle())
        {
            Ix_PluginLoader* pLoader = GetPluginLoader();
            if (pLoader != NULL)
            {
                pLoader->UnloadPlugins();
            }
        }
        if (Handle() != NULL)
        {
            FreeLibrary(Handle());
            Handle() = NULL;
            Factory() = NULL;
        }
    }

	bool UnLoadFunPlugin()
	{
		//MSXML2::IXMLDOMDocumentPtr ptrDoc = NULL;
		//ptrDoc.CreateInstance(__uuidof(MSXML2::DOMDocument));
		//if(ptrDoc == NULL)
		//{
		//	return false; //fail to create document
		//}
		//
		//wchar_t bufTemp[MAX_PATH + 1];
		//::GetModuleFileNameW(NULL, bufTemp, MAX_PATH);
		//wstring wstrTemp = bufTemp;
		//std::wstring::size_type pos = wstrTemp.rfind(L'\\');
		//wstrTemp = wstrTemp.substr(0,pos) + L"\\UnLoadConfig.xml";
		//VARIANT_BOOL varbool = ptrDoc->load(wstrTemp.c_str());
		//
		//if(0 == varbool || NULL == ptrDoc->documentElement)
		//{
		//	ptrDoc = NULL; // fail to load print template file
		//	return false;
		//}
		//
		//
		//MSXML2::IXMLDOMElementPtr ptrElement = ptrDoc->documentElement;

		//MSXML2::IXMLDOMNodeListPtr nodeList = ptrElement->getElementsByTagName(L"FunPlugins");

		//int nodeCounts = static_cast<int>(nodeList->Getlength());
		//ASSERT(nodeCounts == 1);
		//ptrElement = nodeList->Getitem(0);

		//nodeList = ptrElement->getElementsByTagName(L"PluginName");
		//nodeCounts = static_cast<int>(nodeList->Getlength());
		//MSXML2::IXMLDOMNodePtr ptrChildNode = NULL;

		//Ix_PluginLoader* pLoader = GetPluginLoader();

		//for(int i = 0; i < nodeCounts; i++)
		//{
		//	ptrChildNode = nodeList->Getitem(i);
		//	const wchar_t* wstrChildName = ptrChildNode->Gettext();
		//	pLoader->UnloadPlugin(wstrChildName);
		//}

		return true;
	}

    //! Load plugin manager.
    /*!
        \param subdir PluginManager's path, absolute path or relative to instance.
        \param instance used if subdir is a relative path.
        \return true if plugin manager is loaded.
    */
    bool LoadPluginManager(const wchar_t* subdir, HMODULE instance = NULL)
    {
        GetModuleFileNameW(instance, m_filename, MAX_PATH);
        PathRemoveFileSpecW(m_filename);
        PathAppendW(m_filename, subdir);

#ifdef WIN64
		 PathAppendW(m_filename, L"PluginManager64" PLNEXT);
#else
		 PathAppendW(m_filename, L"PluginManager" PLNEXT);
#endif
       
        if (GetModuleHandleW(m_filename) || Handle())
        {
            return !!GetObjectFactory();
        }

        Handle() = LoadLibraryW(m_filename);

        return Handle() && GetObjectFactory();
    }

    //! Load plugin manager and core plugins.
    /*!
        \param subdir plugin path, absolute path or relative to instance.
        \param instance used if subdir is a relative path.
        \param enableDelayLoading enable delay-loading feature or not.
        \return true if any plugin is loaded.
    */
    bool LoadPlugins(std::vector<wstring> vecModuleName)
    {
	
        Ix_PluginLoader* pLoader = GetPluginLoader();
        if (pLoader)
        {
			int nSize = vecModuleName.size();
			wstring wstrTemp = L"";
			for (int i = 0; i < nSize; i++)
			{
				wstrTemp = vecModuleName[i];

				if (string::npos == wstrTemp.find(L".plugin"))
				{
					wstrTemp = wstrTemp + L".plugin";
				}

				pLoader->LoadPlugin(wstrTemp.c_str());
			}

			//此接口包含了界面性插件初始化工作，即管理器加载的所有插件
            pLoader->InitializePlugins();

            return true;
        }

        return false;
    }

    //! Return the object creator.
    Ix_ObjectFactory* GetObjectFactory()
    {
        typedef Ix_ObjectFactory* (*FUNC)();
        Ix_ObjectFactory* p = Factory();

        if (!p)
        {
            HMODULE hdll = Handle() ? Handle() : GetModuleHandleW(m_filename);
            FUNC pfn = (FUNC)GetProcAddress(hdll, "x3GetRegisterBank");

            p = pfn ? (*pfn)() : NULL;
            if (p)
            {
                Factory() = p;
            }
        }

        return p;
    }

    //! Return the plugin loading object.
    Ix_PluginLoader* GetPluginLoader()
    {
        Cx_Interface<Ix_PluginLoader> factory(GetObjectFactory());
        return factory.P();
    }

    static HMODULE& Handle()
    {
        static HMODULE s_dll = NULL;
        return s_dll;
    }

    static Ix_ObjectFactory*& Factory()
    {
        static Ix_ObjectFactory* s_factory = NULL;
        return s_factory;
    }

private:
    CPluginManager(const CPluginManager&);
    void operator=(const CPluginManager&);

    wchar_t     m_filename[MAX_PATH];
};

#endif // X3_MANAGER_PLUGINMANAGER_H_
