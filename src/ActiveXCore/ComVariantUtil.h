/**********************************************************\
Original Author: Georg Fritzsche
 
Created:    Nov 7, 2010
License:    Dual license model; choose one of two:
            New BSD License
            http://www.opensource.org/licenses/bsd-license.php
            - or -
            GNU Lesser General Public License, version 2.1
            http://www.gnu.org/licenses/lgpl-2.1.html
 
Copyright 20100 Georg Fritzsche, Firebreath development team
\**********************************************************/

#ifndef FB_H_CCOMVARIANTUTIL
#define FB_H_CCOMVARIANTUTIL

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <map>
#include <string>
#include <typeinfo>

#include <boost/assign/list_of.hpp>
#include <boost/type_traits/is_arithmetic.hpp>
#include <boost/type_traits/is_integral.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/type_traits/is_floating_point.hpp>
#include <boost/integer_traits.hpp>

#include "FactoryBase.h"
#include "ActiveXBrowserHost.h"
#include "DOM/Document.h"
#include "DOM/Window.h"
#include "AXDOM/Window.h"
#include "AXDOM/Document.h"
#include "AXDOM/Element.h"
#include "AXDOM/Node.h"
#include "IDispatchAPI.h"

// TODO(jtojanen): temporary addition, win_common.h is the right place for this
#include <comdef.h>

#include "./com_utils.h"

namespace FB {
    namespace ActiveX {
        struct type_info_less
        {
            bool operator()(
                const std::type_info* const lhs,
                const std::type_info* const rhs) const
            {
                return lhs->before(*rhs) ? true : false;
            }
        };

        typedef const _variant_t (*ComVariantBuilder)(
            const ActiveXBrowserHostPtr&, const variant&);

        typedef std::map<
            std::type_info const*,
            ComVariantBuilder,
            type_info_less> ComVariantBuilderMap;

        template<class T> const _variant_t makeArithmeticComVariant(
            const ActiveXBrowserHostPtr& host, const variant& var)
        {
            return var.convert_cast<T>();
        }

        template<> inline const _variant_t makeArithmeticComVariant<char>(
            const ActiveXBrowserHostPtr& host, const variant& var)
        {
            return var.cast<char>();
        }

        template<> inline const _variant_t
            makeArithmeticComVariant<unsigned char>(
            const ActiveXBrowserHostPtr& host, const variant& var)
        {
            return var.cast<unsigned char>();
        }

        template<class T> const _variant_t makeComVariant(
            const ActiveXBrowserHostPtr& host, const variant& var)
        {
            // TODO(jtojanen): could/should this be VT_EMPTY?
            _variant_t result;
            V_VT(&result) = VT_NULL;
            return result;
        }

        template<> inline const _variant_t makeComVariant<FBNull>(
            const ActiveXBrowserHostPtr& host, const variant& var)
        {
            _variant_t result;
            V_VT(&result) = VT_NULL;
            return result;
        }

        template<> inline const _variant_t makeComVariant<FBVoid>(
            const ActiveXBrowserHostPtr& host, const variant& var)
        {
            // just leave it at VT_EMPTY
            return _variant_t();
        }

        template<> inline const _variant_t makeComVariant<std::string>(
            const ActiveXBrowserHostPtr& host, const variant& var)
        {
            std::wstring tmp(var.convert_cast<std::wstring>());
            return _variant_t(tmp.c_str());
        }

        template<> inline const _variant_t makeComVariant<std::wstring>(
            const ActiveXBrowserHostPtr& host, const variant& var)
        {
            std::wstring tmp(var.convert_cast<std::wstring>());
            return _variant_t(tmp.c_str());
        }

        template<> inline const _variant_t makeComVariant<VariantList>(
            const ActiveXBrowserHostPtr& host, const variant& var)
        {
            using boost::static_pointer_cast;

            const VariantList inArray(var.cast<VariantList>());
            JSObjectPtr outArray(host->getDOMWindow()->createArray());
            for (VariantList::const_iterator it = inArray.begin(),
                end = inArray.end(); it != end; ++it) {
                    VariantList variantList(boost::assign::list_of(*it));
                    outArray->Invoke("push", variantList);
            }

            // This works as JSObject is implemented by IDispatchAPI or
            //  NPObjectAPI, and currently implementations don't mix
            // We could "visit" implementation to obtain actual type
            //  (visition should be implemented in JSAPI interface)
            const IDispatchAPIPtr dispatchAPI(
                static_pointer_cast<IDispatchAPI>(outArray));
            return _variant_t(dispatchAPI->getIDispatch());
        }

        template<> inline const _variant_t makeComVariant<VariantMap>(
            const ActiveXBrowserHostPtr& host, const variant& var)
        {
            using boost::static_pointer_cast;

            const VariantMap inMap(var.cast<VariantMap>());
            JSObjectPtr outMap(host->getDOMWindow()->createMap());
            for (VariantMap::const_iterator it = inMap.begin(),
                end = inMap.end(); it != end; ++it) {
                    outMap->SetProperty(it->first, it->second);
            }

            // This works as JSObject is implemented by IDispatchAPI or
            //  NPObjectAPI, and currently implementations don't mix
            // We could "visit" implementation to obtain actual type
            //  (visition should be implemented in JSAPI interface)
            const IDispatchAPIPtr dispatchAPI(
                static_pointer_cast<IDispatchAPI>(outMap));
            return _variant_t(dispatchAPI->getIDispatch());
        }

        template<> inline const _variant_t makeComVariant<JSAPIPtr>(
            const ActiveXBrowserHostPtr& host, const variant& var)
        {
            using boost::get_pointer;
            using boost::static_pointer_cast;
            using com::IDispatchExPtr;

            const JSAPIPtr jsapi(var.cast<JSAPIPtr>());
            if (!jsapi) {
                _variant_t null;
                V_VT(&null) = VT_NULL;
                return null;
            }

            const JSObjectPtr jsobject(jsapi->getJSObject());
            if (jsobject) {
                // This works as JSObject is implemented by IDispatchAPI or
                //  NPObjectAPI, and currently implementations don't mix
                // We could "visit" implementation to obtain actual type
                //  (visition should be implemented in JSAPI interface)
                const IDispatchAPIPtr dispatchAPI(
                    static_pointer_cast<IDispatchAPI>(jsobject));
                return _variant_t(dispatchAPI->getIDispatch());
            }

            const IDispatchExPtr dispatchEx(
                host->getJSAPIWrapper(jsapi, true));
            if (!dispatchEx) {
                _variant_t null;
                V_VT(&null) = VT_NULL;
                return null;
            }

            // Add object to the list of shared_ptrs that browserhost keeps
            host->retainJSAPIPtr(jsapi);
            return _variant_t(get_pointer(dispatchEx));
        }

        template<> inline const _variant_t makeComVariant<JSAPIWeakPtr>(
            const ActiveXBrowserHostPtr& host, const variant& var)
        {
            using boost::get_pointer;
            using boost::static_pointer_cast;
            using com::IDispatchExPtr;

            const FB::JSAPIPtr jsapi(var.convert_cast<FB::JSAPIPtr>());
            if (!jsapi) {
                _variant_t null;
                V_VT(&null) = VT_NULL;
                return null;
            }

            const JSObjectPtr jsobject(jsapi->getJSObject());
            if (jsobject) {
                // This works as JSObject is implemented by IDispatchAPI or
                //  NPObjectAPI, and currently implementations don't mix
                // We could "visit" implementation to obtain actual type
                //  (visition should be implemented in JSAPI interface)
                const IDispatchAPIPtr dispatchAPI(
                    static_pointer_cast<IDispatchAPI>(jsobject));
                return _variant_t(dispatchAPI->getIDispatch());
            }

            const IDispatchExPtr dispatchEx(host->getJSAPIWrapper(jsapi));
            if (!dispatchEx) {
                _variant_t null;
                V_VT(&null) = VT_NULL;
                return null;
            }

            return _variant_t(get_pointer(dispatchEx));
        }

        template<> inline const _variant_t makeComVariant<JSObjectPtr>(
            const ActiveXBrowserHostPtr& host, const variant& var)
        {
            using boost::static_pointer_cast;

            const JSObjectPtr jsobject(var.cast<JSObjectPtr>());
            if (!jsobject) {
                _variant_t null;
                V_VT(&null) = VT_NULL;
                return null;
            }

            // This works as JSObject is implemented by IDispatchAPI or
            //  NPObjectAPI, and currently implementations don't mix
            // We could "visit" implementation to obtain actual type
            //  (visition should be implemented in JSAPI interface)
            const IDispatchAPIPtr dispatchAPI(
                static_pointer_cast<IDispatchAPI>(jsobject));
            return _variant_t(dispatchAPI->getIDispatch());
        }

        namespace select_ccomvariant_builder
        {
            template<class T>
            ComVariantBuilder isArithmetic(
                const boost::true_type& /* is_arithmetic */)
            {
                return &makeArithmeticComVariant<T>;
            }

            template<class T>
            ComVariantBuilder isArithmetic(
                const boost::false_type& /* is_arithmetic */)
            {
                return &makeComVariant<T>;
            }

            template<class T>
            ComVariantBuilder select()
            {
                return isArithmetic<T>(boost::is_arithmetic<T>());
            }
        }
    }
}

#endif  // FB_H_CCOMVARIANTUTIL

