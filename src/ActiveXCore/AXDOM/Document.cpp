/**********************************************************\ 
Original Author: Richard Bateman (taxilian)

Created:    Sep 21, 2010
License:    Dual license model; choose one of two:
            New BSD License
            http://www.opensource.org/licenses/bsd-license.php
            - or -
            GNU Lesser General Public License, version 2.1
            http://www.gnu.org/licenses/lgpl-2.1.html

Copyright 2010 Facebook, Inc and the Firebreath development team
\**********************************************************/

#include "IDispatchAPI.h"
#include "DOM/Window.h"
#include "DOM/Element.h"
#include "Document.h"

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
            Document::Document(
                const FB::JSObjectPtr& obj, IWebBrowser* web) : \
                Element(obj, web),
                Node(obj, web),
                FB::DOM::Document(obj),
                FB::DOM::Element(obj),
                FB::DOM::Node(obj),
                htmlDocument2_(NULL)
            {
                IDispatch* dispatch =
                    FB::ptr_cast<IDispatchAPI>(obj)->getIDispatch();
                if (dispatch) {
                    dispatch->QueryInterface(__uuidof(IHTMLDocument2),
                        reinterpret_cast<void**>(&htmlDocument2_));
                }
                if (!htmlDocument2_) {
                    throw std::bad_cast("This is not a valid Document object");
                }
            }

            Document::~Document()
            {
                if (htmlDocument2_) {
                    htmlDocument2_->Release();
                    htmlDocument2_ = NULL;
                }
            }

            FB::DOM::WindowPtr Document::getWindow() const
            {
                CComPtr<IHTMLWindow2> htmlWin;
                HRESULT hr = htmlDocument2_->get_parentWindow(&htmlWin);
                if (FAILED(hr)) {
                    // TODO(jtojanen): should this throw exception?
                    return FB::DOM::WindowPtr();
                }

                ActiveXBrowserHostPtr host(
                    FB::ptr_cast<ActiveXBrowserHost>(getJSObject()->host));
                FB::JSObjectPtr api(IDispatchAPI::create(htmlWin, host));
                return FB::DOM::Window::create(api);
            }

            std::vector<FB::DOM::ElementPtr> Document::getElementsByTagName(
                const std::string& tagName) const
            {
                CComQIPtr<IHTMLDocument3> htmlDocument3(htmlDocument2_);
                if (!htmlDocument3) {
                    throw std::runtime_error(
                        "Could not get element by tag name");
                }

                CComPtr<IHTMLElementCollection> list;
                std::vector<FB::DOM::ElementPtr> tagList;
                const std::wstring name(FB::utf8_to_wstring(tagName));
                HRESULT hr = htmlDocument3->getElementsByTagName(
                    _bstr_t(name.c_str()), &list);
                if (FAILED(hr)) {
                    // TODO(jtojanen): should this throw exception?
                    return tagList;
                }

                long length = 0;
                if (SUCCEEDED(hr = list->get_length(&length))) {
                    ActiveXBrowserHostPtr host(
                        FB::ptr_cast<ActiveXBrowserHost>(getJSObject()->host));
                    for (long i = 0; i < length; ++i) {
                        _variant_t idx(i);
                        CComPtr<IDispatch> dispObj;
                        list->item(idx, idx, &dispObj);
                        FB::JSObjectPtr obj(
                            IDispatchAPI::create(dispObj, host));
                        tagList.push_back(FB::DOM::Element::create(obj));
                    }
                }

                return tagList;
            }


            FB::DOM::ElementPtr Document::getElementById(
                const std::string& elem_id) const
            {
                CComQIPtr<IHTMLDocument3> htmlDocument3(htmlDocument2_);
                if (!htmlDocument3) {
                    throw std::exception(
                        "Document does not support getElementById");
                }

                CComPtr<IHTMLElement> element;
                const std::wstring id(FB::utf8_to_wstring(elem_id));
                HRESULT hr = htmlDocument3->getElementById(
                    _bstr_t(id.c_str()), &element);
                if (FAILED(hr)) {
                    return FB::DOM::ElementPtr();
                }

                ActiveXBrowserHostPtr host(
                    FB::ptr_cast<ActiveXBrowserHost>(getJSObject()->host));
                FB::JSObjectPtr ptr(IDispatchAPI::create(element, host));
                return FB::DOM::Element::create(ptr);
            }
        }  // namespace AXDOM
    }  // namespace ActiveX
}  // namespace FB
