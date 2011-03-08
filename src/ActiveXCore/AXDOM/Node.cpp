/**********************************************************\ 
Original Author: Jukka Ojanen (jtojanen)

Created:    Feb 10, 2011
License:    Dual license model; choose one of two:
            New BSD License
            http://www.opensource.org/licenses/bsd-license.php
            - or -
            GNU Lesser General Public License, version 2.1
            http://www.gnu.org/licenses/lgpl-2.1.html

Copyright 2011 Linkotec Oy
\**********************************************************/

#include "Node.h"
#include "IDispatchAPI.h"

namespace FB
{
    namespace ActiveX
    {
        namespace AXDOM
        {
            Node::Node(
                const FB::JSObjectPtr& element, IWebBrowser* webBrowser) :
            FB::DOM::Node(element), webBrowser_(webBrowser)
            {
                if (webBrowser_) {
                    webBrowser_->AddRef();
                }
            }

            Node::~Node()
            {
                if (webBrowser_) {
                    webBrowser_->Release();
                    webBrowser_ = NULL;
                }
            }
        }  // namespace AXDOM
    }  // namespace ActiveX
}  // namespace FB
