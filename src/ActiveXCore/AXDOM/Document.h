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
#include "../com_utils.h"

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
            typedef boost::intrusive_ptr<IHTMLDocument2> IHTMLDocument2Ptr;
            typedef boost::intrusive_ptr<IWebBrowser> IWebBrowserPtr;


            ///////////////////////////////////////////////////////////////////
            /// @class  Document
            ///
            /// @brief  ActiveX specific implementation of DOM::Document
            ///////////////////////////////////////////////////////////////////
            class Document :
                public virtual Element,
                public virtual DOM::Document
            {
            public:
                Document(const IDispatchAPIPtr& api,
                    const IWebBrowserPtr& webBrowser);
                virtual ~Document();

                DOM::WindowPtr getWindow() const;
                DOM::ElementPtr getElementById(
                    const std::string& elementId) const;
                std::vector<DOM::ElementPtr> getElementsByTagName(
                    const std::string& tagName) const;

            private:
                 IHTMLDocument2Ptr document2_;
            };
        }
    }
}

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

#endif  // H_AXDOM_DOCUMENT

