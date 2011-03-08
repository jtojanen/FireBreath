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

#ifndef H_AXDOM_DOCUMENT
#define H_AXDOM_DOCUMENT

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <string>
#include <vector>
#include "./Element.h"
#include "DOM/Document.h"
#include "DOM/Element.h"
#include "JSObject.h"
#include "win_common.h"

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4250)
#endif

// forward declarations
struct IHTMLDocument2;

namespace FB
{
    namespace ActiveX
    {
        namespace AXDOM
        {
            ///////////////////////////////////////////////////////////////////
            /// @class  Document
            ///
            /// @brief  ActiveX specific implementation of DOM::Document
            ///////////////////////////////////////////////////////////////////
            class Document :
                public virtual Element,
                public virtual FB::DOM::Document
            {
            public:
                Document(
                    const FB::JSObjectPtr& element, IWebBrowser* webBrowser);
                virtual ~Document();

                FB::DOM::WindowPtr getWindow() const;
                FB::DOM::ElementPtr getElementById(
                    const std::string& elementId) const;
                std::vector<FB::DOM::ElementPtr> getElementsByTagName(
                    const std::string& tagName) const;

            private:
                IHTMLDocument2* htmlDocument2_;
            };
        }
    }
}

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

#endif  // H_AXDOM_DOCUMENT

