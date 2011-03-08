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

#include "Node.h"

#include <string>

namespace FB
{
    namespace DOM
    {
        Node::Node(const JSObjectPtr& element) : m_element(element) {}
        Node::~Node() {}

        FB::JSObjectPtr Node::getJSObject() const
        {
            return m_element;
        }

        NodePtr Node::node()
        {
            return shared_from_this();
        }

        // static
        NodePtr Node::create(const FB::JSObjectPtr& api)
        {
            return api->host->_createNode(api);
        }

        NodePtr Node::getNode(const std::wstring& name) const
        {
            return getNode(FB::wstring_to_utf8(name));
        }

        NodePtr Node::getNode(const std::string& name) const
        {
            FB::JSObjectPtr api = getProperty<FB::JSObjectPtr>(name);
            return Node::create(api);
        }

        NodePtr Node::getNode(int idx) const
        {
            FB::JSObjectPtr api = getProperty<JSObjectPtr>(idx);
            return Node::create(api);
        }

        void Node::setProperty(const std::wstring& name,
            const FB::variant& val) const
        {
            setProperty(FB::wstring_to_utf8(name), val);
        }

        void Node::setProperty(const std::string& name,
            const FB::variant& val) const
        {
            m_element->SetProperty(name, val);
        }

        void Node::setProperty(int idx, const FB::variant& val) const
        {
            m_element->SetProperty(idx, val);
        }
    }
}

