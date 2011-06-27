/**********************************************************\ 
Original Author: Richard Bateman (taxilian)

Created:    Oct 30, 2009
License:    Dual license model; choose one of two:
New BSD License
http://www.opensource.org/licenses/bsd-license.php
- or -
GNU Lesser General Public License, version 2.1
http://www.gnu.org/licenses/lgpl-2.1.html

Copyright 2009 Richard Bateman, Firebreath development team
\**********************************************************/

#ifndef H_ACTIVEXBROWSERHOST
#define H_ACTIVEXBROWSERHOST

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "win_common.h"

#include <map>
#include <string>

#include "BrowserHost.h"
#include "APITypes.h"
#include "FBPointers.h"
#include "SafeQueue.h"
#include "ShareableReference.h"
#include "ActiveXFactoryDefinitions.h"

#include "./com_utils.h"

// forward declarations
struct IHTMLWindow2;
struct IWebBrowser;

namespace FB
{
    class WinMessageWindow;

    namespace ActiveX
    {
        FB_FORWARD_PTR(ActiveXBrowserHost);
        FB_FORWARD_PTR(IDispatchAPI);

        typedef boost::weak_ptr<FB::ShareableReference<IDispatch> > IDispatchWRef;
        typedef boost::shared_ptr<FB::ShareableReference<IDispatch> > IDispatchSRef;
        typedef boost::intrusive_ptr<IOleClientSite> IOleClientSitePtr;
        typedef boost::intrusive_ptr<IHTMLWindow2> IHTMLWindow2Ptr;
        typedef boost::intrusive_ptr<IWebBrowser> IWebBrowserPtr;

        ///////////////////////////////////////////////////////////////////////
        /// @class  ActiveXBrowserHost
        ///
        /// @brief  Provides a BrowserHost implementation for ActiveX
        ///////////////////////////////////////////////////////////////////////
        class ActiveXBrowserHost :
            public FB::BrowserHost
        {
        public:
            ActiveXBrowserHost(IWebBrowser* webBrowser,
                IOleClientSite* clientSite);
            virtual ~ActiveXBrowserHost(void);

            bool _scheduleAsyncCall(
                void (*func)(void *), void *userData) const;

            void* getContextID() const;

            BrowserStreamPtr _createStream(const std::string& url,
                const PluginEventSinkPtr& callback,
                bool cache = true, bool seekable = false,
                size_t internalBufferSize = 128 * 1024) const;

            IDispatchWRef getIDispatchRef(IDispatch* obj);

            const com::IDispatchExPtr getJSAPIWrapper(
                const JSAPIWeakPtr& api, bool autoRelease = false);

            BrowserStreamPtr _createPostStream(const std::string& url,
                const PluginEventSinkPtr& callback,
                const std::string& postdata, bool cache = true,
                bool seekable = false,
                size_t internalBufferSize = 128 * 1024) const;

        public:
            void evaluateJavaScript(const std::string& script);
            DOM::DocumentPtr getDOMDocument();
            DOM::WindowPtr getDOMWindow();
            DOM::ElementPtr getDOMElement();
            bool hasHTMLWindow() { return !!htmlWindow2_; }
            void shutdown();

            void resume(IWebBrowser* webBrowser, IOleClientSite* clientSite);
            void suspend();

        public:
            DOM::WindowPtr _createWindow(const JSObjectPtr& obj) const;
            DOM::DocumentPtr _createDocument(const JSObjectPtr& obj) const;
            DOM::ElementPtr _createElement(const JSObjectPtr& obj) const;
            DOM::NodePtr _createNode(const JSObjectPtr& obj) const;

        private:
            void initDOMObjects();
            IOleClientSitePtr clientSite_;
            com::IDispatchPtr htmlDocument_;
            IHTMLWindow2Ptr htmlWindow2_;
            IWebBrowserPtr webBrowser_;
            mutable DOM::WindowPtr m_window;
            mutable DOM::DocumentPtr m_document;
            boost::scoped_ptr<WinMessageWindow> m_messageWin;

        private:
            mutable boost::shared_mutex m_xtmutex;
            mutable FB::SafeQueue<IDispatchWRef> m_deferredObjects;
            typedef std::map<void*, FB::WeakIDispatchExRef> IDispatchExRefMap;
            typedef std::list<IDispatchSRef> IDispatchRefList;
            mutable IDispatchExRefMap m_cachedIDispatch;
            mutable IDispatchRefList m_heldIDispatch;

        public:
            const variant getVariant(const VARIANT* cVar);
            void getComVariant(VARIANT* dest, const variant& var);
            void deferred_release(const IDispatchWRef& obj) const;
            void DoDeferredRelease() const;
            void ReleaseAllHeldObjects();
        };
    }  // namespace ActiveX
}  // namespace FB

#endif  // H_ACTIVEXBROWSERHOST

