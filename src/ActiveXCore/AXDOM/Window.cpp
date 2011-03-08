/**********************************************************\ 
Original Author: Richard Bateman (taxilian)

Created:    Sep 21, 2010
License:    Dual license model; choose one of two:
New BSD License
http://www.opensource.org/licenses/bsd-license.php
- or -
GNU Lesser General Public License, version 2.1
http://www.gnu.org/licenses/lgpl-2.1.html

Copyright 2009 PacketPass, Inc and the Firebreath development team
\**********************************************************/

#include "Document.h"
#include "IDispatchAPI.h"
#include "Window.h"

#include <string>

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
            Window::Window(const FB::JSObjectPtr& obj, IWebBrowser* web) : \
                Node(obj, web), FB::DOM::Window(obj), FB::DOM::Node(obj),
                htmlWindow2_(NULL)
            {
                IDispatch* dispatch =
                    FB::ptr_cast<IDispatchAPI>(obj)->getIDispatch();
                if (dispatch) {
                    dispatch->QueryInterface(__uuidof(IHTMLWindow2),
                        reinterpret_cast<void**>(htmlWindow2_));
                }
                if (!htmlWindow2_) {
                    throw std::bad_cast("This is not a valid Window object");
                }
            }

            Window::~Window()
            {
                if (htmlWindow2_) {
                    htmlWindow2_->Release();
                    htmlWindow2_ = NULL;
                }
            }

            FB::DOM::DocumentPtr Window::getDocument() const
            {
                CComPtr<IHTMLDocument2> document2;
                HRESULT hr = htmlWindow2_->get_document(&document2);
                if (FAILED(hr)) {
                    // TODO(jtojanen): could/should we throw exception
                    return FB::DOM::DocumentPtr();
                }

                ActiveXBrowserHostPtr host(
                    FB::ptr_cast<ActiveXBrowserHost>(
                    this->getJSObject()->host));
                FB::JSObjectPtr api(IDispatchAPI::create(document2, host));
                return FB::DOM::Document::create(api);
            }

            void Window::alert(const std::string& str) const
            {
                std::wstring message(FB::utf8_to_wstring(str));
                htmlWindow2_->alert(_bstr_t(message.c_str()));
            }

            std::string Window::getLocation() const
            {
                CComPtr<IHTMLLocation> location;
                HRESULT hr = htmlWindow2_->get_location(&location);
                if (FAILED(hr)) {
                    // TODO(jtojanen): could/should we throw exception
                    return std::string();
                }

                _bstr_t href;
                if (FAILED(hr = location->get_href(href.GetAddress()))) {
                    // TODO(jtojanen): could/should we throw exception
                    return std::string();
                }

                std::wstring tmp(href);
                return FB::wstring_to_utf8(tmp);
            }
        }  // namespace AXDOM
    }  // namespace ActiveX
}  // namespace FB
