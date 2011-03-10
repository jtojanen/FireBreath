#ifndef __COM_UTILS_H__
#define __COM_UTILS_H__

#if defined(_MSC_VER) && (_MSC_VER > 1020)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER > 1020)

#include <dispex.h>

#include <boost/intrusive_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <boost/utility/addressof.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits/is_base_of.hpp>

namespace boost
{	
	template<typename T>
	inline typename enable_if<is_base_of<IUnknown, T> >::type 
		intrusive_ptr_add_ref(T* p)
	{
		p->AddRef();  
	}

	template<typename T>
	inline typename enable_if<is_base_of<IUnknown, T> >::type 
		intrusive_ptr_release(T* p)
	{
		p->Release();
	}
}

namespace com
{
    typedef boost::intrusive_ptr<IDispatch> IDispatchPtr;
    typedef boost::intrusive_ptr<IDispatchEx> IDispatchExPtr;

    template<typename T> 
	inline typename boost::enable_if<boost::is_base_of<IUnknown, T>, T>::type**
		addressof(boost::intrusive_ptr<T>& ptr) 
	{
		BOOST_ASSERT(!*reinterpret_cast<T**>(boost::addressof(ptr))); 
		return reinterpret_cast<T**>(boost::addressof(ptr)); 
	} 

	template<class U, class T> 
	inline const boost::intrusive_ptr<U>
		query_interface(T* object, const IID& iid = __uuidof(U))
	{
        if (!object) {
            return boost::intrusive_ptr<U>();
        }
		void* p = NULL;
		object->QueryInterface(iid, &p);
		return boost::intrusive_ptr<U>(static_cast<U*>(p), false);
	}

    template<class U, class T>
    inline const boost::intrusive_ptr<U>
        query_interface(const boost::intrusive_ptr<T>& object)
    {
        return query_interface(object.get());
    }

	namespace detail
	{
		template <class T>
		class query_interface : private boost::noncopyable
		{
		public:
			explicit query_interface(T* object) : object_(object) {}

			template <class U>
			operator const boost::intrusive_ptr<U>() const
			{
				return com::query_interface<U>(object_);
			}

		private:
			T* object_;
		};
	} // namespace detail

	template <class T> 
	inline const detail::query_interface<T> query_interface(T* object)
	{
        return detail::query_interface<T>(object);
	}

    template <class T>
    inline const detail::query_interface<T> query_interface(
        const boost::intrusive_ptr<T>& object)
    {
        return detail::query_interface<T>(object.get());
    }

	template <class T>
	static HRESULT CoCreateInstance(REFCLSID rclsid, LPUNKNOWN pUnkOuter, 
		DWORD dwClsContext, boost::intrusive_ptr<T>& object)
	{
		void* p = NULL;
		HRESULT hr = ::CoCreateInstance(
			rclsid, pUnkOuter, dwClsContext, __uuidof(T), &p);
		object.reset(static_cast<T*>(p));
		return hr;
	}

	static HRESULT GetProperty(IDispatch* dispatch, DISPID dispId, 
		EXCEPINFO* exceptionInfo, VARIANT *variant) throw()
	{
		BOOST_ASSERT(dispatch);
		BOOST_ASSERT(variant);
		if (!variant) { 
			return E_POINTER;
		}

		if (!dispatch) {
			return E_INVALIDARG;
		}

		DISPPARAMS dispparamsNoArgs = { NULL, NULL, 0, 0 };
		return dispatch->Invoke(dispId, IID_NULL, LOCALE_USER_DEFAULT, 
			DISPATCH_PROPERTYGET, &dispparamsNoArgs, 
			variant, exceptionInfo, NULL);
	}

	static HRESULT PutProperty(IDispatch* dispatch, DISPID dispId, 
		EXCEPINFO* exceptionInfo, VARIANT* variant) throw()
	{
		BOOST_ASSERT(dispatch);
		BOOST_ASSERT(variant);
		if (!variant) {
			return E_POINTER;
		}

		if (!dispatch) {
			return E_INVALIDARG;
		}

		DISPPARAMS dispparams = { NULL, NULL, 1, 1 };
		dispparams.rgvarg = variant;
		DISPID dispidPut = DISPID_PROPERTYPUT;
		dispparams.rgdispidNamedArgs = &dispidPut;

		if (V_VT(variant) == VT_UNKNOWN || V_VT(variant) == VT_DISPATCH || 
			(V_VT(variant) & VT_ARRAY) || (V_VT(variant) & VT_BYREF))
		{
			HRESULT hr = dispatch->Invoke(dispId, IID_NULL, 
				LOCALE_USER_DEFAULT, DISPATCH_PROPERTYPUTREF, &dispparams,
				NULL, exceptionInfo, NULL);
			if (SUCCEEDED(hr)) {
				return hr;
			}
		}

		return dispatch->Invoke(dispId, IID_NULL, LOCALE_USER_DEFAULT, 
			DISPATCH_PROPERTYPUT, &dispparams, NULL, exceptionInfo, NULL);
	}

	struct ExceptionInfo : EXCEPINFO
	{
		ExceptionInfo()
		{
			memset(this, 0, sizeof(*this));
		}

		~ExceptionInfo()
		{
			clear();
		}

		void clear()
		{
			::SysFreeString(bstrSource);
			::SysFreeString(bstrDescription);
			::SysFreeString(bstrHelpFile);
			memset(this, 0, sizeof(*this));
		}
	};
}

#endif // __COM_UTILS_H__
