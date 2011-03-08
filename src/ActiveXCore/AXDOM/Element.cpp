/**********************************************************\ 
Original Author: Richard Bateman (taxilian)

Created:    Sep 21, 2009
License:    Dual license model; choose one of two:
            New BSD License
            http://www.opensource.org/licenses/bsd-license.php
            - or -
            GNU Lesser General Public License, version 2.1
            http://www.gnu.org/licenses/lgpl-2.1.html

Copyright 2010 Facebook, Inc and the Firebreath development team
\**********************************************************/

#include "Element.h"
#include "IDispatchAPI.h"

#include <string>
#include <vector>

// TODO(jtojanen): CComPtr and CComQIPtr
#include <atlcomcli.h>

// TODO(jtojanen): temporary addition, win_common.h is the right place for this
#include <comdef.h>

namespace FB
{
    namespace ActiveX
    {
        namespace AXDOM
        {
            Element::Element(
                const FB::JSObjectPtr& element, IWebBrowser* web) : \
                Node(element, web),
                FB::DOM::Node(element),
                FB::DOM::Element(element),
                dispatch_(FB::ptr_cast<IDispatchAPI>(element)->getIDispatch())
            {
                if (!dispatch_) {
                    throw std::bad_cast("This is not a valid object");
                }
            }

            Element::~Element()
            {
                if (dispatch_) {
                    dispatch_->Release();
                    dispatch_ = NULL;
                }
            }

            std::vector<FB::DOM::ElementPtr> Element::getElementsByTagName(
                const std::string& tagName) const
            {
                HRESULT hr = E_FAIL;
                CComPtr<IHTMLElementCollection> list;
                _bstr_t name(FB::utf8_to_wstring(tagName).c_str());

                CComQIPtr<IHTMLElement2> element2(dispatch_);
                if (element2) {
                    hr = element2->getElementsByTagName(name, &list);
                }
                if (FAILED(hr)) {
                    CComQIPtr<IHTMLDocument3> document3(dispatch_);
                    if (document3) {
                        hr = document3->getElementsByTagName(name, &list);
                    }
                }
                if (FAILED(hr)) {
                     throw std::runtime_error(
                         "Could not get element by tag name");
                }

                long length = 0;
                std::vector<FB::DOM::ElementPtr> tagList;
                if (SUCCEEDED(list->get_length(&length))) {
                    ActiveXBrowserHostPtr host(
                        FB::ptr_cast<ActiveXBrowserHost>(getJSObject()->host));
                    for (long i = 0; i < length; i++) {
                        CComPtr<IDispatch> dispObj;
                        _variant_t idx(i);
                        list->item(idx, idx, &dispObj);
                        FB::JSObjectPtr obj(
                            IDispatchAPI::create(dispObj, host));
                        tagList.push_back(FB::DOM::Element::create(obj));
                    }
                }

                return tagList;
            }

            std::string Element::getStringAttribute(
                const std::string& attr) const
            {
                CComQIPtr<IHTMLElement> element(dispatch_);
                if (!element) {
                    return getProperty<std::string>(attr);
                }

                _variant_t value;
                const std::wstring name(FB::utf8_to_wstring(attr));
                HRESULT hr = element->getAttribute(
                    _bstr_t(name.c_str()), 0, &value);
                if (FAILED(hr)) {
                    // TODO(jtojanen): could/should we throw exception?
                    return std::string();
                }

                ActiveXBrowserHostPtr host(
                    FB::ptr_cast<ActiveXBrowserHost>(getJSObject()->host));
                FB::variant variant(host->getVariant(&value));
                return variant.convert_cast<std::string>();
            }
        }  // namespace AXDOM
    }  // namespace ActiveX
}  // namespace FB
