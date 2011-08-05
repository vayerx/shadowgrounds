
#ifndef ATLBASEHAX_H
#define ATLBASEHAX_H

//#include <atldef.h>

#include <windows.h>
//#include <winnls.h>
//#include <ole2.h>

//#include <comcat.h>
//#include <stddef.h>

#include <assert.h>

/////////////////////////////////////////////////////////////////////////////
// Smart Pointer helpers

inline IUnknown* AtlComPtrAssign(IUnknown** pp, IUnknown* lp)
{
        if(pp == NULL)
                return NULL;
                
        if (lp != NULL)
                lp->AddRef();
        
        if (*pp)
                (*pp)->Release();
        *pp = lp;
        return lp;
}



template <class T>
class _NoAddRefReleaseOnCComPtr : public T
{
        private:
                STDMETHOD_(ULONG, AddRef)()=0;
                STDMETHOD_(ULONG, Release)()=0;
};

template <class T>
class CComPtr
{
public:
        typedef T _PtrClass;
        CComPtr()
        {
                p=NULL;
        }
        CComPtr(T* lp)
        {
                if ((p = lp) != NULL)
                        p->AddRef();
        }
        CComPtr(const CComPtr<T>& lp)
        {
                if ((p = lp.p) != NULL)
                        p->AddRef();
        }
        ~CComPtr()
        {
                if (p)
                        p->Release();
        }
        void Release()
        {
                IUnknown* pTemp = p;
                if (pTemp)
                {
                        p = NULL;
                        pTemp->Release();
                }
        }
        operator T*() const
        {
                return (T*)p;
        }
        T& operator*() const
        {
                assert(p!=NULL);
                return *p;
        }
        //The assert on operator& usually indicates a bug.  If this is really
        //what is needed, however, take the address of the p member explicitly.
        T** operator&()
        {
                assert(p==NULL);
                return &p;
        }
        _NoAddRefReleaseOnCComPtr<T>* operator->() const
        {
                assert(p!=NULL);
                return (_NoAddRefReleaseOnCComPtr<T>*)p;
        }
        T* operator=(T* lp)
        {
                return (T*)AtlComPtrAssign((IUnknown**)&p, lp);
        }
        T* operator=(const CComPtr<T>& lp)
        {
                return (T*)AtlComPtrAssign((IUnknown**)&p, lp.p);
        }
        bool operator!() const
        {
                return (p == NULL);
        }
        bool operator<(T* pT) const
        {
                return p < pT;
        }
        bool operator==(T* pT) const
        {
                return p == pT;
        }
        // Compare two objects for equivalence
        bool IsEqualObject(IUnknown* pOther)
        {
                if (p == NULL && pOther == NULL)
                        return true; // They are both NULL objects

                if (p == NULL || pOther == NULL)
                        return false; // One is NULL the other is not

                CComPtr<IUnknown> punk1;
                CComPtr<IUnknown> punk2;
                p->QueryInterface(IID_IUnknown, (void**)&punk1);
                pOther->QueryInterface(IID_IUnknown, (void**)&punk2);
                return punk1 == punk2;
        }
        void Attach(T* p2)
        {
                if (p)
                        p->Release();
                p = p2;
        }
        T* Detach()
        {
                T* pt = p;
                p = NULL;
                return pt;
        }
        HRESULT CopyTo(T** ppT)
        {
                assert(ppT != NULL);
                if (ppT == NULL)
                        return E_POINTER;
                *ppT = p;
                if (p)
                        p->AddRef();
                return S_OK;
        }
        HRESULT SetSite(IUnknown* punkParent)
        {
                return AtlSetChildSite(p, punkParent);
        }
        HRESULT Advise(IUnknown* pUnk, const IID& iid, LPDWORD pdw)
        {
                return AtlAdvise(p, pUnk, iid, pdw);
        }
        HRESULT CoCreateInstance(REFCLSID rclsid, LPUNKNOWN pUnkOuter = NULL, DWORD dwClsContext = CLSCTX_ALL)
        {
			assert(p == NULL);
#ifdef __GNUC__
			assert(!"CoCreateInstance: not supported");
            return 0;
#else
			return ::CoCreateInstance(rclsid, pUnkOuter, dwClsContext, __uuidof(T), (void**)&p);
#endif
		}
		HRESULT CoCreateInstance(LPCOLESTR szProgID, LPUNKNOWN pUnkOuter = NULL, DWORD dwClsContext = CLSCTX_ALL)
		{
#ifdef __GNUC__
			assert(!"CoCreateInstance: not supported");
            return 0;
#else
			CLSID clsid;
				HRESULT hr = CLSIDFromProgID(szProgID, &clsid);
				assert(p == NULL);
				if (SUCCEEDED(hr))
					hr = ::CoCreateInstance(clsid, pUnkOuter, dwClsContext, __uuidof(T), (void**)&p);
				return hr;
#endif
		}
		template <class Q>
			HRESULT QueryInterface(Q** pp) const
		{
			assert(pp != NULL && *pp == NULL);
#ifdef __GNUC__
			assert(!"QueryInterface: not supported");
            return 0;
#else
			return p->QueryInterface(__uuidof(Q), (void**)pp);
#endif
		}
		T* p;
};

#endif
