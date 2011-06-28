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

#include "precompiled_headers.h" // On windows, everything above this line in PCH
#include "Document.h"

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

            using com::IDispatchPtr;
            using com::query_interface;

            typedef intrusive_ptr<IHTMLElement> IHTMLElementPtr;
            typedef intrusive_ptr<IHTMLWindow2> IHTMLWindow2Ptr;
            typedef intrusive_ptr<IHTMLDocument3> IHTMLDocument3Ptr;
            typedef intrusive_ptr<IHTMLElementCollection> 
                IHTMLElementCollectionPtr;

            Document::Document(const IDispatchAPIPtr& api,
                const IWebBrowserPtr& webBrowser) : \
                Element(api, webBrowser), Node(api, webBrowser),
                DOM::Document(api), DOM::Element(api), DOM::Node(api),
                document2_(query_interface(api->getIDispatch()))
            {
                if (!document2_) {
                    throw std::bad_cast("This is not a valid Document object");
                }
            }

            Document::~Document()
            {
                // nothing to do
            }

            DOM::WindowPtr Document::getWindow() const
            {
                IHTMLWindow2Ptr htmlWindow2;
                HRESULT hr = document2_->get_parentWindow(
                    com::addressof(htmlWindow2));
                if (FAILED(hr)) {
                    // TODO(jtojanen): should this throw exception?
                    return DOM::WindowPtr();
                }

                ActiveXBrowserHostPtr host(
                    static_pointer_cast<ActiveXBrowserHost>(
                    getJSObject()->getHost()));
                JSObjectPtr api(
                    IDispatchAPI::create(htmlWindow2.get(), host));
                return DOM::Window::create(api);
            }

            std::vector<DOM::ElementPtr> Document::getElementsByTagName(
                const std::string& tagName) const
            {
                IHTMLDocument3Ptr document3(query_interface(document2_));
                if (!document3) {
                    throw std::runtime_error(
                        "Could not get element by tag name");
                }

                IHTMLElementCollectionPtr list;
                std::vector<DOM::ElementPtr> tagList;
                const std::wstring name(utf8_to_wstring(tagName));
                HRESULT hr = document3->getElementsByTagName(
                    _bstr_t(name.c_str()), com::addressof(list));
                if (FAILED(hr)) {
                    // TODO(jtojanen): should this throw exception?
                    return tagList;
                }

                long length = 0;
                if (FAILED(hr = list->get_length(&length))) {
                    return tagList;
                }

                ActiveXBrowserHostPtr host(
                    static_pointer_cast<ActiveXBrowserHost>(
                    getJSObject()->getHost()));
                for (long i = 0; i < length; ++i) {
                    _variant_t index(i);
                    IDispatchPtr dispatch;
                    list->item(index, index, com::addressof(dispatch));
                    JSObjectPtr object(
                        IDispatchAPI::create(dispatch.get(), host));
                    tagList.push_back(DOM::Element::create(object));
                }

                return tagList;
            }

            DOM::ElementPtr Document::getElementById(
                const std::string& elem_id) const
            {
                IHTMLDocument3Ptr document3(query_interface(document2_));
                if (!document3) {
                    throw std::exception(
                        "Document does not support getElementById");
                }
             
                IHTMLElementPtr element;
                const std::wstring id(utf8_to_wstring(elem_id));
                HRESULT hr = document3->getElementById(
                    _bstr_t(id.c_str()), com::addressof(element));
                if (FAILED(hr)) {
                    return DOM::ElementPtr();
                }

                ActiveXBrowserHostPtr host(
                    static_pointer_cast<ActiveXBrowserHost>(
                    getJSObject()->getHost()));
                JSObjectPtr ptr(IDispatchAPI::create(element.get(), host));
                return DOM::Element::create(ptr);
            }
        }  // namespace AXDOM
    }  // namespace ActiveX
}  // namespace FB
