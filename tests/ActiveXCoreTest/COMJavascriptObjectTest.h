/**********************************************************\ 
Original Author: Richard Bateman (taxilian)

Created:    Nov 7, 2009
License:    Dual license model; choose one of two:
            New BSD License
            http://www.opensource.org/licenses/bsd-license.php
            - or -
            GNU Lesser General Public License, version 2.1
            http://www.gnu.org/licenses/lgpl-2.1.html

Copyright 2009 Richard Bateman, Firebreath development team
\**********************************************************/

#include <vector>
#include <string>

#include "APITypes.h"
#include "variant_list.h"
#include "COMJavascriptObject.h"
#include "ActiveXBrowserHost.h"
#include "FactoryBase.h"
#include "TestJSAPI.h"

// TODO(jtojanen): temporary addition, win_common.h is the right place for this
#include <comdef.h>

using namespace FB::ActiveX;

DISPID getIDForName(IDispatchEx* object, const std::string& name)
{
    std::wstring tmp(FB::utf8_to_wstring(name));
    OLECHAR* p = const_cast<OLECHAR *>(tmp.c_str());
    DISPID dispId;
    HRESULT hr = object->GetIDsOfNames(IID_NULL, &p, 1,
        LOCALE_SYSTEM_DEFAULT, &dispId);
    if (FAILED(hr)) {
        return DISPID_UNKNOWN;
    }

    BOOST_ASSERT(dispId != DISPID_UNKNOWN);
    return dispId;
}

const _variant_t InvokeMethod(IDispatchEx* object,
                              const std::string& methodName,
                              VARIANT* args, size_t count)
{
    DISPPARAMS params;
    params.cArgs = count;
    params.cNamedArgs = 0;
    params.rgvarg = args;

    _variant_t result;
    HRESULT hr = object->Invoke(getIDForName(object, methodName), IID_NULL,
        LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, &result, NULL, NULL);
    if (FAILED(hr)) {
        throw FB::script_error("Could not get property");
    }

    return result;
}

TEST (COMJavascriptObject_CreateTest)
{
    PRINT_TESTNAME;

    ActiveXBrowserHostPtr host(new ActiveXBrowserHost(NULL, NULL));

    boost::shared_ptr<TestObjectJSAPI> testIf(new TestObjectJSAPI());
    CComPtr<IDispatchEx> obj(
        getFactoryInstance()->createCOMJSObject(host, testIf));
    
    // createCOMJSObject returns already addrefs pointer
    obj.p->Release();

    CHECK (true);
}

TEST(COMJavascriptObject_Methods)
{
    PRINT_TESTNAME;

    ActiveXBrowserHostPtr host(new ActiveXBrowserHost(NULL, NULL));

    boost::shared_ptr<TestObjectJSAPI> testIf(new TestObjectJSAPI());
    CComPtr<IDispatchEx> obj(
        getFactoryInstance()->createCOMJSObject(host, testIf));

    // createCOMJSObject returns already addrefs pointer
    obj.p->Release();

    HRESULT hr = S_OK;

    DISPID func;
    CHECK(func = getIDForName(obj, "setValue") != -1);

    {
        try {
            _variant_t args[2] = {"This is a test", 0};
            InvokeMethod(obj, "setValue", args, 2);
            CHECK(true);
        } catch (...) {
            CHECK(false);
        }
        try {
            _variant_t arg(0);
            _variant_t ret = InvokeMethod(obj, "getValue", &arg, 1);

            CW2A cStr(ret.bstrVal);
            std::string strRes(cStr);
            CHECK(strRes == "This is a test");
        } catch (...) {
            CHECK(false);
        }
    }

    try {
        // Test setting and then getting integer values
        for (int i = 0; i < TESTOBJECTJSAPI_ACCESSLISTLENGTH; i++) {
            _variant_t args[2] = {i, i};
            InvokeMethod(obj, "setValue", args, 2);

            _variant_t arg(i);
            _variant_t ret = InvokeMethod(obj, "getValue", &arg, 1);
            ret.ChangeType(VT_I4);
            CHECK(ret.lVal == i);
        }
    } catch (...) {
        CHECK(FALSE);
    }

    try {
        // Test setting and then getting integer values
        for (int i = 0; i < TESTOBJECTJSAPI_ACCESSLISTLENGTH; i++) {
            _variant_t args[2] = {static_cast<double>(i * 1.5), i};
            InvokeMethod(obj, "setValue", args, 2);

            _variant_t arg(i);
            _variant_t ret = InvokeMethod(obj, "getValue", &arg, 1);
            ret.ChangeType(VT_R8);
            CHECK(ret.dblVal == static_cast<double>(i * 1.5));
        }
    } catch (...) {
        CHECK(FALSE);
    }

    //// Test invalid parameters; this should *not* fire an exception *ever*
    // try {
    //    CHECK(!host->Invoke(obj, STRID(host, "setValue"), NULL, 0, &res));
    //    CHECK(true);
    // } catch (...) {
    //    CHECK(false);
    // }

    //// Test invalid method name
    // try {
    //    CHECK(!host->Invoke(obj,
    //     STRID(host, "someMethodThatDoesntExist"), NULL, 0, &res));
    //    CHECK(true);
    // } catch (...) {
    //    CHECK(false);
    // }
}

TEST(COMJavascriptObject_Properties)
{
    // PRINT_TESTNAME;

    // NpapiPluginModule module;
    // NpapiHost testHost(NULL, NULL, NULL);
    // module.setNetscapeFuncs(testHost.getBrowserFuncs());

    // NpapiBrowserHostPtr host =
    //  new NpapiBrowserHost(&module, testHost.getPluginInstance());
    // host->setBrowserFuncs(testHost.getBrowserFuncs());

    // NPVariant res;
    // NPVariant oneParam;
    // boost::shared_ptr<TestObjectJSAPI> testIf = new TestObjectJSAPI();
    // NPJavascriptObject *obj = NPJavascriptObject::NewObject(host, testIf);

    //// Test setting and getting many different datatypes with properties
    // try {
    //    for (int i = 0; i < 10000; i++) {
    //        INT32_TO_NPVARIANT(i, oneParam);
    //        CHECK(host->SetProperty(obj, STRID(host, "value"), &oneParam));
    //        CHECK(host->GetProperty(obj, STRID(host, "value"), &res));
    //        CHECK(NPVARIANT_TO_INT32(res) == i);

    //        DOUBLE_TO_NPVARIANT((double)(i * 3.74), oneParam);
    //        CHECK(host->SetProperty(obj, STRID(host, "value"), &oneParam));
    //        CHECK(host->GetProperty(obj, STRID(host, "value"), &res));
    //        CHECK(NPVARIANT_TO_DOUBLE(res) == (double)(i * 3.74));

    //        // Roundabout, but easy, way to convert this to a string =]
    //        std::string testVal =
    //         FB::variant(
    //          (double)(i * 653.251235)).convert_cast<std::string>();
    //        host->getNPVariant(&oneParam, FB::variant(testVal));
    //        CHECK(host->SetProperty(obj, STRID(host, "value"), &oneParam));
    //        CHECK(host->GetProperty(obj, STRID(host, "value"), &res));
    //        CHECK(testVal == res.value.stringValue.utf8characters);
    //    }
    // } catch (...) {
    //    CHECK(false);
    // }

    //// Test exception handling
    // try {
    //    CHECK(!host->GetProperty(obj,
    //     STRID(host, "SomePropertyThatDoesntExist"), &res));
    //    CHECK(true);
    // } catch (...) {
    //    CHECK(false);
    // }
}

