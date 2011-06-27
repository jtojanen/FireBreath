/**********************************************************\ 
Original Author: Richard Bateman (taxilian)

Created:    Dec 9, 2009
License:    Dual license model; choose one of two:
            New BSD License
            http://www.opensource.org/licenses/bsd-license.php
            - or -
            GNU Lesser General Public License, version 2.1
            http://www.gnu.org/licenses/lgpl-2.1.html

Copyright 2009 PacketPass, Inc and the Firebreath development team
\**********************************************************/

#include "variant_list.h"
#include "../precompiled_headers.h" // On windows, everything above this line in PCH
#include "Element.h"

#include <string>
#include <vector>

namespace FB
{
    namespace DOM
    {
        Element::Element(const FB::JSObjectPtr& element) : Node(element)
        {
        }

        Element::~Element()
        {
        }

        ElementPtr Element::element()
        {
            return boost::dynamic_pointer_cast<Element>(node());
        }

        // static
        ElementPtr Element::create(const FB::JSObjectPtr& api)
        {
            return api->getHost()->_createElement(api);
        }

        std::string Element::getInnerHTML() const
        {
            return getProperty<std::string>("innerHTML");
        }

        void Element::setInnerHTML(const std::string& html) const
        {
            setProperty("innerHTML", html);
        }

        int Element::getWidth() const
        {
            return getProperty<int>("width");
        }

        void Element::setWidth(int width) const
        {
            setProperty("width", width);
        }

        int Element::getScrollWidth() const
        {
            return getProperty<int>("scrollWidth");
        }

        int Element::getHeight() const
        {
            return getProperty<int>("height");
        }

        void Element::setHeight(int height) const
        {
            setProperty("height", height);
        }

        int Element::getScrollHeight() const
        {
            return getProperty<int>("scrollHeight");
        }

        int Element::getChildNodeCount() const
        {
            return getNode("childNodes")->getProperty<int>("length");
        }

        ElementPtr Element::getChildNode(int idx) const
        {
            return ElementPtr(getElement("childNodes")->getElement(idx));
        }

        ElementPtr Element::getParentNode() const
        {
            return ElementPtr(getElement("parentNode"));
        }

        ElementPtr Element::getElement(const std::string& name) const
        {
            JSObjectPtr element(getProperty<FB::JSObjectPtr>(name));
            return boost::make_shared<Element>(element);
        }

        ElementPtr Element::getElement(int idx) const
        {
            JSObjectPtr element(getProperty<FB::JSObjectPtr>(idx));
            return boost::make_shared<Element>(element);
        }

        ElementPtr Element::getElementById(const std::string& id) const
        {
            JSObjectPtr api(
                callMethod<JSObjectPtr>(
                "getElementById", variant_list_of(id)));
            return Element::create(api);
        }

        std::vector<ElementPtr> Element::getElementsByTagName(
            const std::wstring& tagName) const
        {
            return getElementsByTagName(FB::wstring_to_utf8(tagName));
        }

        std::vector<ElementPtr> Element::getElementsByTagName(
            const std::string& tagName) const
        {
            typedef std::vector<FB::JSObjectPtr> TagList;

            TagList tagList(callMethod<TagList >(
                "getElementsByTagName", FB::variant_list_of(tagName)));

            std::vector<ElementPtr> outList;
            for (TagList::iterator it = tagList.begin(),
                end = tagList.end(); it != end; ++it) {
                outList.push_back(Element::create(*it));
            }

            return outList;
        }

        std::string Element::getStringAttribute(const std::string& attr) const
        {
            return callMethod<std::string>("getAttribute",
                FB::variant_list_of(attr));
        }
    }
}
