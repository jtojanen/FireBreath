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
#include "win_common.h"

// forward declarations
struct IWebBrowser;

namespace FB
{
    namespace ActiveX
    {
        namespace AXDOM
        {
            ///////////////////////////////////////////////////////////////////
            /// @class  Node
            ///
            /// @brief  ActiveX specific implementation of DOM::Node
            ///////////////////////////////////////////////////////////////////
            class Node : public virtual FB::DOM::Node
            {
            public:
                Node(const FB::JSObjectPtr& element, IWebBrowser* webBrowser);
                virtual ~Node();

            private:
                IWebBrowser* webBrowser_;
            };

            typedef boost::shared_ptr<Node> NodePtr;
        }
    }
}

#endif  // H_AXDOM_NODE

