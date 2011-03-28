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

// TODO(jtojanen): temporary addition, win_common.h is the right place for this
#include <comdef.h>

namespace FB
{
    namespace ActiveX
    {
        namespace AXDOM
        {
            using boost::intrusive_ptr;
            using boost::static_pointer_cast;

            using com::query_interface;

            typedef intrusive_ptr<IHTMLLocation> IHTMLLocationPtr;
            typedef intrusive_ptr<IHTMLDocument2> IHTMLDocument2Ptr;

            Window::Window(const IDispatchAPIPtr& api,
                const IWebBrowserPtr& webBrowser) : \
                Node(api, webBrowser), DOM::Window(api), DOM::Node(api),
                window2_(query_interface(api->getIDispatch()))
            {
                if (!window2_) {
                    throw std::bad_cast("This is not a valid Window object");
                }
            }

            Window::~Window()
            {
                // nothing to do
            }

            DOM::DocumentPtr Window::getDocument() const
            {
                IHTMLDocument2Ptr document2;
                HRESULT hr = window2_->get_document(com::addressof(document2));
                if (FAILED(hr)) {
                    // TODO(jtojanen): could/should we throw exception
                    return DOM::DocumentPtr();
                }

                ActiveXBrowserHostPtr host(
                    static_pointer_cast<ActiveXBrowserHost>(
                    getJSObject()->getHost()));
                JSObjectPtr api(IDispatchAPI::create(document2.get(), host));
                return DOM::Document::create(api);
            }

            void Window::alert(const std::string& str) const
            {
                std::wstring message(utf8_to_wstring(str));
                window2_->alert(_bstr_t(message.c_str()));
            }

            std::string Window::getLocation() const
            {
                IHTMLLocationPtr location;
                HRESULT hr = window2_->get_location(com::addressof(location));
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
                return wstring_to_utf8(tmp);
            }
        }  // namespace AXDOM
    }  // namespace ActiveX
}  // namespace FB
