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
            const ActiveXBrowserHostPtr&, const FB::variant&);

        typedef std::map<
            std::type_info const*,
            ComVariantBuilder,
            type_info_less> ComVariantBuilderMap;

        template<class T> const _variant_t makeArithmeticComVariant(
            const ActiveXBrowserHostPtr& host, const FB::variant& var)
        {
            return var.convert_cast<T>();
        }

        template<> inline const _variant_t makeArithmeticComVariant<char>(
            const ActiveXBrowserHostPtr& host, const FB::variant& var)
        {
            return var.cast<char>();
        }

        template<> inline const _variant_t
            makeArithmeticComVariant<unsigned char>(
            const ActiveXBrowserHostPtr& host, const FB::variant& var)
        {
            return var.cast<unsigned char>();
        }

        template<class T> const _variant_t makeComVariant(
            const ActiveXBrowserHostPtr& host, const FB::variant& var)
        {
            // TODO(jtojanen): could/should this be VT_EMPTY?
            _variant_t result;
            result.ChangeType(VT_NULL);
            return result;
        }

        template<> inline const _variant_t makeComVariant<FB::FBNull>(
            const ActiveXBrowserHostPtr& host, const FB::variant& var)
        {
            _variant_t result;
            result.ChangeType(VT_NULL);
            return result;
        }

        template<> inline const _variant_t makeComVariant<FB::FBVoid>(
            const ActiveXBrowserHostPtr& host, const FB::variant& var)
        {
            // just leave it at VT_EMPTY
            return _variant_t();
        }

        template<> inline const _variant_t makeComVariant<std::string>(
            const ActiveXBrowserHostPtr& host, const FB::variant& var)
        {
            std::wstring tmp(var.convert_cast<std::wstring>());
            return _variant_t(tmp.c_str());
        }

        template<> inline const _variant_t makeComVariant<std::wstring>(
            const ActiveXBrowserHostPtr& host, const FB::variant& var)
        {
            std::wstring tmp(var.convert_cast<std::wstring>());
            return _variant_t(tmp.c_str());
        }

        template<> inline const _variant_t makeComVariant<FB::VariantList>(
            const ActiveXBrowserHostPtr& host, const FB::variant& var)
        {
            FB::VariantList inArray(var.cast<FB::VariantList>());
            FB::JSObjectPtr outArray(host->getDOMWindow()->createArray());
            for (FB::VariantList::iterator it = inArray.begin(),
                end = inArray.end(); it != end; ++it) {
                    FB::VariantList variantList(boost::assign::list_of(*it));
                    outArray->Invoke("push", variantList);
            }

            _variant_t result;
            IDispatchAPIPtr api(ptr_cast<IDispatchAPI>(outArray));
            if (api) {
                result = api->getIDispatch();
            }

            return result;
        }

        template<> inline const _variant_t makeComVariant<FB::VariantMap>(
            const ActiveXBrowserHostPtr& host, const FB::variant& var)
        {
            FB::VariantMap inMap(var.cast<FB::VariantMap>());
            FB::JSObjectPtr outMap(host->getDOMWindow()->createMap());
            for (FB::VariantMap::iterator it = inMap.begin(),
                end = inMap.end(); it != end; ++it) {
                    outMap->SetProperty(it->first, it->second);
            }

            _variant_t result;
            IDispatchAPIPtr api(ptr_cast<IDispatchAPI>(outMap));
            if (api) {
                result = api->getIDispatch();
            }

            return result;
        }

        template<> inline const _variant_t makeComVariant<FB::JSAPIPtr>(
            const ActiveXBrowserHostPtr& host, const FB::variant& var)
        {
            using com::IDispatchExPtr;

            _variant_t result;

            FB::JSAPIPtr object(var.cast<FB::JSAPIPtr>());
            IDispatchAPIPtr api(ptr_cast<IDispatchAPI>(object));
            if (api) {
                result = api->getIDispatch();
            } else if (object) {
                // Add object to the list of shared_ptrs that browserhost keeps
                host->retainJSAPIPtr(object);
                const IDispatchExPtr dispatchEx(
                    host->getJSAPIWrapper(object, true));
                result = dispatchEx.get();
            } else {
                result.ChangeType(VT_NULL);
            }

            return result;
        }

        template<> inline const _variant_t makeComVariant<FB::JSAPIWeakPtr>(
            const ActiveXBrowserHostPtr& host, const FB::variant& var)
        {
            using com::IDispatchExPtr;

            _variant_t result;

            FB::JSAPIPtr object(var.convert_cast<FB::JSAPIPtr>());
            IDispatchAPIPtr api(ptr_cast<IDispatchAPI>(object));
            if (api) {
                result = api->getIDispatch();
            } else if (object) {
                const IDispatchExPtr dispatchEx(
                    host->getJSAPIWrapper(object));
                result = dispatchEx.get();
            } else {
                result.ChangeType(VT_NULL);
            }

            return result;
        }

        template<> inline const _variant_t makeComVariant<FB::JSObjectPtr>(
            const ActiveXBrowserHostPtr& host, const FB::variant& var)
        {
            _variant_t result;

            FB::JSObjectPtr object(var.cast<FB::JSObjectPtr>());
            IDispatchAPIPtr api(ptr_cast<IDispatchAPI>(object));
            if (api) {
                result = api->getIDispatch();
            } else if (object) {
                FB::JSAPIPtr ptr(var.convert_cast<FB::JSAPIPtr>());
                host->retainJSAPIPtr(ptr);
                result = getFactoryInstance()->createCOMJSObject(host, ptr);
                // TODO(jtojanen): we should avoid manual reference counting
                V_DISPATCH(&result)->Release();
            } else {
                result.ChangeType(VT_NULL);
            }

            return result;
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

