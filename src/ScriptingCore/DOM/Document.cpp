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

#include "variant.h"
#include "variant_list.h"
#include "Window.h"
#include "../precompiled_headers.h" // On windows, everything above this line in PCH

#include "Document.h"

namespace FB
{
    namespace DOM
    {
        Document::Document(const FB::JSObjectPtr& element) : \
            Element(element), Node(element)
        {
        }

        Document::~Document()
        {
        }

        DocumentPtr Document::document()
        {
            return boost::dynamic_pointer_cast<Document>(node());
        }

        // static
        DocumentPtr Document::create(const FB::JSObjectPtr& api)
        {
            return api->getHost()->_createDocument(api);
        }

        WindowPtr Document::getWindow() const
        {
            JSObjectPtr window(getProperty<FB::JSObjectPtr>("window"));
            return Window::create(window);
        }

        ElementPtr Document::getBody() const
        {
            JSObjectPtr body(getProperty<FB::JSObjectPtr>("body"));
            return Element::create(body);
        }
    }
}
