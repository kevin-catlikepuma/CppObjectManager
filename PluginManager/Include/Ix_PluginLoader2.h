/*! \file Ix_PluginLoader2.h
 *  \brief Define plugin loading interface: Ix_PluginLoader2
 *  \author Zhang Yungui, X3 C++ PluginFramework
 *  \date   2011.06.30
 */
#ifndef X3_MANAGER_IPLUGINLOADER2_H_
#define X3_MANAGER_IPLUGINLOADER2_H_

#include "Ix_ObjectFactory.h"

/*! \ingroup _GROUP_PLUGIN_CORE2_
 *  \interface Ix_PluginLoader2
 *  \brief Plugin loading interface
 *  \see   Ix_PluginLoader, Ix_ObjectFactory, x3GetObjectFactory()
 */
class Ix_PluginLoader2 : public Ix_Object
{
public:
    X3DEFINE_IID(Ix_PluginLoader2)

    virtual long GetPluginCount() = 0;
    virtual bool GetPluginFileName(long index, HMODULE& hdll, std::wstring& filename) = 0;
};

#endif // X3_MANAGER_IPLUGINLOADER2_H_
