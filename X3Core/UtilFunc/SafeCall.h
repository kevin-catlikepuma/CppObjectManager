//! \file SafeCall.h
//! \brief Define functions to call pointer safely.

#ifndef __APPCORE_SAFECALL_H
#define __APPCORE_SAFECALL_H

// Validate before call function of pointer, "v" is default return value
#ifndef SafeCall
#define SafeCall(p, f)                          if (p) p->f
#define SafeCallIf(p, f, v)                     ((p) ? (p->f) : (v))
#define SafeCallWithReturn(p, f, v)             if (p) return p->f; else return (v);
#define WndSafeCall(p, f)                       if (p && ::IsWindow(p->GetSafeHwnd())) p->f
#define WndSafeCallWithReturn(p, f, v)          if (p && ::IsWindow(p->GetSafeHwnd())) return p->f; else return (v);
#define InterfaceSafeCall(p, f)                 if (p.IsNotNull()) p->f
#define InterfaceSafeCallIf(p, f, v)            (p.IsNotNull() ? (p->f) : (v))
#define InterfaceSafeCallWithReturn(p, f, v)    if (p.IsNotNull()) return p->f; else return (v);
#endif // SafeCall

namespace x3 {

//! Delete pointer object.
/*!
    \ingroup _GROUP_UTILFUNC
    \param p pointer object created using 'new'.
*/
template<class T>
void SafeDelete(T*& p)
{
    if (p != NULL)
        delete p;
    p = NULL;
    *(&p) = NULL;
}

//! Delete pointer array object.
/*!
    \ingroup _GROUP_UTILFUNC
    \param p pointer array object created using 'new []'.
*/
template<class T>
void SafeDeleteArray(T*& p)
{
    if (p != NULL)
        delete []p;
    p = NULL;
}

//! Delete all elements in a container.
/*!
    \ingroup _GROUP_UTILFUNC
    \param container STL container variable (vector, list, map). eg: " vector<int*> arr; "
*/
template<class CONTAINER>
void DeletePtrInContainer(CONTAINER& container)
{
    typename CONTAINER::iterator it = container.begin();
    for(; it != container.end(); ++it)
        SafeDelete(*it);
    container.resize(0);
}

} // x3
#endif // __APPCORE_SAFECALL_H
