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
#include <atlsafe.h>
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
            type_info_less
        > ComVariantBuilderMap;

        inline const _variant_t createSafeArray(std::vector<_variant_t>& variants)
        {
            // create array
            _variant_t result;
            V_VT(&result) = VT_VARIANT | VT_ARRAY;
            SAFEARRAY* p = V_ARRAY(&result) =
                ::SafeArrayCreateVector(VT_VARIANT, 0, variants.size());

            // lock array in-memory to access it       
            HRESULT hr = ::SafeArrayLock(p);
            if (FAILED(hr)) {
                return _variant_t();
            }

            // scope for lock
            {
                boost::shared_ptr<void> lock(p, ::SafeArrayUnlock);

                // transfer ownership of variant to array
                VARIANT* data = static_cast<VARIANT*>(p->pvData);
                for (std::vector<_variant_t>::iterator iter = variants.begin(),
                    end = variants.end(); iter != end; ++data, ++iter) {
                        *data = iter->Detach();
                }
            }

            return result;
        }

        //  GJS  ---
        //  I would probably put the ComVariantBuilderMap code into ComVariantUtil.cpp?
        template<class T>
        inline ComVariantBuilderMap::value_type makeBuilderEntry()
        {
            return ComVariantBuilderMap::value_type(&typeid(T), select_ccomvariant_builder::select<T>());
        }

        inline const ComVariantBuilderMap makeComVariantBuilderMap()
        {
            ComVariantBuilderMap tdm;
            tdm.insert(makeBuilderEntry<bool>());
            tdm.insert(makeBuilderEntry<char>());
            tdm.insert(makeBuilderEntry<unsigned char>());
            tdm.insert(makeBuilderEntry<short>());
            tdm.insert(makeBuilderEntry<unsigned short>());
            tdm.insert(makeBuilderEntry<int>());
            tdm.insert(makeBuilderEntry<unsigned int>());
            tdm.insert(makeBuilderEntry<long>());
            tdm.insert(makeBuilderEntry<unsigned long>());

#ifndef BOOST_NO_LONG_LONG
            tdm.insert(makeBuilderEntry<long long>());
            tdm.insert(makeBuilderEntry<unsigned long long>());
#endif

            tdm.insert(makeBuilderEntry<float>());
            tdm.insert(makeBuilderEntry<double>());

            tdm.insert(makeBuilderEntry<std::string>());
            tdm.insert(makeBuilderEntry<std::wstring>());

            tdm.insert(makeBuilderEntry<FB::VariantList>());
            tdm.insert(makeBuilderEntry<FB::VariantMap>());
            tdm.insert(makeBuilderEntry<FB::JSAPIPtr>());
            tdm.insert(makeBuilderEntry<FB::JSAPIWeakPtr>());
            tdm.insert(makeBuilderEntry<FB::JSObjectPtr>());

            tdm.insert(makeBuilderEntry<FB::FBVoid>());
            tdm.insert(makeBuilderEntry<FB::FBNull>());

            return tdm;
        }

        inline const ComVariantBuilderMap& getComVariantBuilderMap()
        {
            static const ComVariantBuilderMap tdm = makeComVariantBuilderMap();
            return tdm;
        }
        //  GJS  ---

        template<class T> inline const _variant_t makeArithmeticComVariant(
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

        template<class T> inline const _variant_t makeComVariant(
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

            _variant_t result;
            const VariantList inArray(var.cast<VariantList>());
            if (host->hasHTMLWindow()) {
                JSObjectPtr outArray(host->getDOMWindow()->createArray());
                for (VariantList::const_iterator iter = inArray.begin(),
                    end = inArray.end(); iter != end; ++iter) {
                        VariantList args(boost::assign::list_of(*iter));
                        outArray->Invoke("push", args);
                }

                //IDispatchAPIPtr api = ptr_cast<IDispatchAPI>(outArr);
                //if (api) {
                //    result = api->getIDispatch();
                //}

                // This works as JSObject is implemented by IDispatchAPI or
                //  NPObjectAPI, and currently implementations don't mix
                // We could "visit" implementation to obtain actual type
                //  (visition should be implemented in JSAPI interface)
                const IDispatchAPIPtr dispatchAPI(
                    static_pointer_cast<IDispatchAPI>(outArray));
                result = dispatchAPI->getIDispatch();
            } else {
                std::vector<_variant_t> outArray;
                outArray.reserve(inArray.size());
                const ComVariantBuilderMap& builderMap =
                    getComVariantBuilderMap();
                for (VariantList::const_iterator iter = inArray.begin(),
                    end = inArray.end(); iter != end; ++iter) {
                        const std::type_info& type = iter->get_type();
                        ComVariantBuilderMap::const_iterator found =
                            builderMap.find(&type);
                        if (found == builderMap.end()) {
                            continue;
                        }                        
                        _variant_t tmp(found->second(host, *iter));
                        outArray.push_back(tmp);
                }
                if (!outArray.empty()) {
                    result = createSafeArray(outArray);
                }
            }

            return result;
        }

        template<> inline const _variant_t makeComVariant<VariantMap>(
            const ActiveXBrowserHostPtr& host, const variant& var)
        {          
            using boost::static_pointer_cast;

            _variant_t result;
            const VariantMap inMap(var.cast<VariantMap>());
            if (host->hasHTMLWindow()) {
                JSObjectPtr outMap(host->getDOMWindow()->createMap());
                for (VariantMap::const_iterator iter = inMap.begin(),
                    end = inMap.end(); iter != end; ++iter) {
                        outMap->SetProperty(iter->first, iter->second);
                }

                //IDispatchAPIPtr api = ptr_cast<IDispatchAPI>(outMap);
                //if (api) {
                //    result = api->getIDispatch();
                //}
                
                // This works as JSObject is implemented by IDispatchAPI or
                //  NPObjectAPI, and currently implementations don't mix
                // We could "visit" implementation to obtain actual type
                //  (visition should be implemented in JSAPI interface)
                const IDispatchAPIPtr dispatchAPI(
                    static_pointer_cast<IDispatchAPI>(outMap));
                result = dispatchAPI->getIDispatch();
            } else {
                std::vector<_variant_t> outMap;
                outMap.reserve(inMap.size());
                const ComVariantBuilderMap& builderMap =
                    getComVariantBuilderMap();
                for (FB::VariantMap::const_iterator iter = inMap.begin(),
                    end = inMap.end(); iter != end; ++iter) {
                    const std::type_info& valType = iter->second.get_type();
                    ComVariantBuilderMap::const_iterator found = builderMap.find(&valType);
                    if (found == builderMap.end()) {
                        continue;
                    }
                    std::vector<_variant_t> pair(2);
                    pair[0] = makeComVariant<std::string>(host, iter->first);
                    pair[1] = found->second(host, iter->second);
                    _variant_t tmp(createSafeArray(pair));
                    outMap.push_back(tmp);
                }
                if (!outMap.empty()) {
                    result = createSafeArray(outMap);
                }
            }
            
            return result;
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

