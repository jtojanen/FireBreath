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

#ifndef H_AXDOM_ELEMENT
#define H_AXDOM_ELEMENT

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <string>
#include <vector>
#include "./Node.h"
#include "DOM/Element.h"
#include "JSObject.h"
#include "../com_utils.h"

// forward declarations
struct IDispatch;

namespace FB
{
    namespace ActiveX
    {
        namespace AXDOM
        {
            typedef boost::intrusive_ptr<IWebBrowser> IWebBrowserPtr;

            ///////////////////////////////////////////////////////////////////
            /// @class  Element
            ///
            /// @brief  ActiveX specific implementation of DOM::Element
            ///////////////////////////////////////////////////////////////////
            class Element :
                public virtual Node,
                public virtual DOM::Element
            {
            public:
                Element(const IDispatchAPIPtr& api,
                    const IWebBrowserPtr& webBrowser);
                virtual ~Element();

                std::vector<DOM::ElementPtr> getElementsByTagName(
                    const std::string& tagName) const;
                std::string getStringAttribute(const std::string& attr) const;

            private:
                com::IDispatchPtr dispatch_;
            };
        }
    }
}

#endif  // H_AXDOM_ELEMENT

