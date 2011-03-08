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

#ifndef H_AXDOM_WINDOW
#define H_AXDOM_WINDOW

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <string>
#include "Node.h"
#include "DOM/Window.h"
#include "win_common.h"

// forward declarations
struct IHTMLWindow2;

namespace FB
{
    namespace ActiveX
    {
        namespace AXDOM
        {
            ///////////////////////////////////////////////////////////////////
            /// @class  Window
            ///
            /// @brief  ActiveX specific implementation of DOM::Window
            ///////////////////////////////////////////////////////////////////
            class Window :
                public virtual Node,
                public virtual FB::DOM::Window
            {
            public:
                Window(const FB::JSObjectPtr& obj, IWebBrowser* web);
                virtual ~Window();

                FB::DOM::DocumentPtr getDocument() const;
                void alert(const std::string& str) const;
                std::string getLocation() const;

            private:
                IHTMLWindow2* htmlWindow2_;
            };
        }  // namespace AXDOM
    }  // namespace ActiveX
}  // namespace FB

#endif  // H_AXDOM_WINDOW

