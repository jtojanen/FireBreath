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
            typedef intrusive_ptr<IHTMLElement2> IHTMLElement2Ptr;
            typedef intrusive_ptr<IHTMLDocument3> IHTMLDocument3Ptr;
            typedef intrusive_ptr<IHTMLElementCollection> 
                IHTMLElementCollectionPtr;

            Element::Element(const IDispatchAPIPtr& api,
                const IWebBrowserPtr& webBrowser) : \
                Node(api, webBrowser), DOM::Node(api), DOM::Element(api),
                dispatch_(api->getIDispatch())
            {
                if (!dispatch_) {
                    throw std::bad_cast("This is not a valid object");
                }
            }

            Element::~Element()
            {
                // nothing to do
            }

            std::vector<DOM::ElementPtr> Element::getElementsByTagName(
                const std::string& tagName) const
            {
                HRESULT hr = E_FAIL;
                IHTMLElementCollectionPtr list;
                _bstr_t name(utf8_to_wstring(tagName).c_str());
                IHTMLElement2Ptr element2(query_interface(dispatch_));
                if (element2) {
                    hr = element2->getElementsByTagName(
                        name, com::addressof(list));
                }
                if (FAILED(hr)) {
                    IHTMLDocument3Ptr document3(query_interface(dispatch_));
                    if (document3) {
                        hr = document3->getElementsByTagName(
                            name, com::addressof(list));
                    }
                }
                if (FAILED(hr)) {
                    throw std::runtime_error(
                        "Could not get element by tag name");
                }

                long length = 0;
                std::vector<DOM::ElementPtr> tagList;
                if (FAILED(list->get_length(&length))) {
                    return tagList;
                }

                ActiveXBrowserHostPtr host(
                    static_pointer_cast<ActiveXBrowserHost>(
                    getJSObject()->host));
                for (long i = 0; i < length; i++) {
                    _variant_t index(i);
                    IDispatchPtr dispatch;
                    list->item(index, index, com::addressof(dispatch));
                    JSObjectPtr object(
                        IDispatchAPI::create(dispatch.get(), host));
                    tagList.push_back(DOM::Element::create(object));
                }

                return tagList;
            }

            std::string Element::getStringAttribute(
                const std::string& attr) const
            {
                IHTMLElementPtr element(query_interface(dispatch_));
                if (!element) {
                    return getProperty<std::string>(attr);
                }

                _variant_t value;
                const std::wstring name(utf8_to_wstring(attr));
                HRESULT hr = element->getAttribute(
                    _bstr_t(name.c_str()), 0, &value);
                if (FAILED(hr)) {
                    // TODO(jtojanen): could/should we throw exception?
                    return std::string();
                }

                ActiveXBrowserHostPtr host(
                    static_pointer_cast<ActiveXBrowserHost>(
                    getJSObject()->host));
                variant variant(host->getVariant(&value));
                return variant.convert_cast<std::string>();
            }
        }  // namespace AXDOM
    }  // namespace ActiveX
}  // namespace FB
