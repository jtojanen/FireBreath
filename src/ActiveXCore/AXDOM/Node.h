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

#ifndef H_AXDOM_NODE
#define H_AXDOM_NODE

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "DOM/Node.h"
#include "IDispatchAPI.h"
#include "../com_utils.h"

// forward declarations
struct IWebBrowser;

namespace FB
{
    namespace ActiveX
    {
        namespace AXDOM
        {
            typedef boost::intrusive_ptr<IWebBrowser> IWebBrowserPtr;

            ///////////////////////////////////////////////////////////////////
            /// @class  Node
            ///
            /// @brief  ActiveX specific implementation of DOM::Node
            ///////////////////////////////////////////////////////////////////
            class Node : public virtual DOM::Node
            {
            public:
                Node(const IDispatchAPIPtr& api,
                    const IWebBrowserPtr& webBrowser);
                virtual ~Node();

            private:
                IWebBrowserPtr webBrowser_;
            };

            typedef boost::shared_ptr<Node> NodePtr;
        }
    }
}

#endif  // H_AXDOM_NODE

