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

namespace FB
{
    namespace ActiveX
    {
        namespace AXDOM
        {
            Node::Node(const IDispatchAPIPtr& api,
                const IWebBrowserPtr& webBrowser) : \
                DOM::Node(api), webBrowser_(webBrowser)
            {
                // nothing to do
            }

            Node::~Node()
            {
                // nothing to do
            }
        }  // namespace AXDOM
    }  // namespace ActiveX
}  // namespace FB
