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

#include <string>
#include <boost/assign.hpp>
#include "./ActiveXBrowserHost.h"
#include "./axstream.h"
#include "./ComVariantUtil.h"
#include "DOM/Document.h"
#include "DOM/Window.h"
#include "../ScriptingCore/AsyncFunctionCall.h"
#include "Win/WinMessageWindow.h"
#include "AXDOM/Window.h"
#include "AXDOM/Element.h"
#include "AXDOM/Node.h"

#include "ActiveXFactoryDefinitions.h"

#include <utility>

namespace FB
{
    namespace ActiveX
    {
        using boost::assign::list_of;
        using boost::intrusive_ptr;
        using boost::int64_t;
        using boost::make_shared;
        using boost::shared_lock;
        using boost::shared_mutex;
        using boost::static_pointer_cast;
        using boost::uint64_t;
        using boost::upgrade_lock;

        using com::IDispatchPtr;
        using com::IDispatchExPtr;
        using com::query_interface;

        typedef intrusive_ptr<IHTMLDocument2> IHTMLDocument2Ptr;
        typedef intrusive_ptr<IOleControlSite> IOleControlSitePtr;

        ActiveXBrowserHost::ActiveXBrowserHost(IWebBrowser* webBrowser,
            IOleClientSite* clientSite) : \
            m_messageWin(new WinMessageWindow())
        {
            resume(webBrowser, clientSite);
        }

        ActiveXBrowserHost::~ActiveXBrowserHost(void)
        {
            // nothing to do
        }

        bool ActiveXBrowserHost::_scheduleAsyncCall(
            void (*func)(void *), void* userData) const
        {
            shared_lock<shared_mutex> lock(m_xtmutex);

            if (isShutDown() || !m_messageWin) {
                return false;
            }

            FBLOG_TRACE("ActiveXHost", "Scheduling async call for main thread");
            return !!::PostMessage(m_messageWin->getHWND(),
                WM_ASYNCTHREADINVOKE, NULL,
                reinterpret_cast<LPARAM>(
                new AsyncFunctionCall(func, userData)));
        }

        void* ActiveXBrowserHost::getContextID() const
        {
            return const_cast<ActiveXBrowserHost*>(this);
        }

        DOM::WindowPtr ActiveXBrowserHost::_createWindow(
            const JSObjectPtr& obj) const
        {
            IDispatchAPIPtr api(ptr_cast<IDispatchAPI>(obj));
            if (!api) {
                return DOM::WindowPtr();
            }
            return make_shared<AXDOM::Window>(api, webBrowser_);
        }

        DOM::DocumentPtr ActiveXBrowserHost::_createDocument(
            const JSObjectPtr& obj) const
        {
            IDispatchAPIPtr api(ptr_cast<IDispatchAPI>(obj));
            if (!api) {
                return DOM::DocumentPtr();
            }
            return make_shared<AXDOM::Document>(api, webBrowser_);
        }

        DOM::ElementPtr ActiveXBrowserHost::_createElement(
            const JSObjectPtr& obj) const
        {
            IDispatchAPIPtr api(ptr_cast<IDispatchAPI>(obj));
            if (!api) {
                return DOM::ElementPtr();
            }
            return make_shared<AXDOM::Element>(api, webBrowser_);
        }

        DOM::NodePtr ActiveXBrowserHost::_createNode(
            const JSObjectPtr& obj) const
        {
            IDispatchAPIPtr api(ptr_cast<IDispatchAPI>(obj));
            if (!api) {
                return DOM::NodePtr();
            }
            return make_shared<AXDOM::Node>(api, webBrowser_);
        }

        void ActiveXBrowserHost::initDOMObjects()
        {
            if (!m_window) {
                ActiveXBrowserHostPtr shared_this(
                    static_pointer_cast<ActiveXBrowserHost>(
                    shared_from_this()));
                m_window = DOM::Window::create(
                    IDispatchAPI::create(htmlWindow2_.get(), shared_this));
                m_document = DOM::Document::create(
                    IDispatchAPI::create(htmlDocument_.get(), shared_this));
            }
        }

        DOM::DocumentPtr ActiveXBrowserHost::getDOMDocument()
        {
            initDOMObjects();
            return m_document;
        }

        DOM::WindowPtr ActiveXBrowserHost::getDOMWindow()
        {
            initDOMObjects();
            return m_window;
        }

        DOM::ElementPtr ActiveXBrowserHost::getDOMElement()
        {
            IOleControlSitePtr site(query_interface(clientSite_));

            IDispatchPtr control;
            HRESULT hr = site->GetExtendedControl(com::addressof(control));
            if (FAILED(hr)) {
                return DOM::ElementPtr();
            }

            ActiveXBrowserHostPtr shared_this(
                static_pointer_cast<ActiveXBrowserHost>(shared_from_this()));
            return DOM::Document::create(
                IDispatchAPI::create(control.get(), shared_this));
        }

        void ActiveXBrowserHost::evaluateJavaScript(const std::string& script)
        {
            if (!htmlWindow2_) {
                throw script_error("Can't execute JavaScript: Window is NULL");
            }

            _variant_t result;
            _bstr_t code(script.c_str());
            static const _bstr_t language(L"javascript");
            HRESULT hr = htmlWindow2_->execScript(code, language, &result);
            if (FAILED(hr)) {
                throw script_error("Error executing JavaScript code");
            }
        }

        void ActiveXBrowserHost::shutdown()
        {
            // scope for lock
            {
                // First, make sure that no async calls are
                // made while we're shutting down
                upgrade_lock<shared_mutex> lock(m_xtmutex);

                // Next, kill the message window so that
                // none that have been made go through
                m_messageWin.reset();
            }

            // Finally, run the main browser shutdown,
            // which will fire off any cross-thread
            // calls that somehow haven't made it through yet
            BrowserHost::shutdown();

            // Once that's done let's release any ActiveX resources that
            // the browserhost is holding
            suspend();
            BOOST_ASSERT(m_deferredObjects.empty());
        }

        void ActiveXBrowserHost::resume(IWebBrowser* webBrowser, IOleClientSite* clientSite)
        {
            webBrowser_ = webBrowser;
            clientSite_ = clientSite;

            if (webBrowser_ && !htmlDocument_) {
                webBrowser_->get_Document(com::addressof(htmlDocument_));                
                IHTMLDocument2Ptr htmlDocument2(
                    query_interface(htmlDocument_));
                BOOST_ASSERT(htmlDocument2);
                htmlDocument2->get_parentWindow(com::addressof(htmlWindow2_));
                BOOST_ASSERT(htmlWindow2_);
            }
        }

        void ActiveXBrowserHost::suspend()
        {
            // release any ActiveX resources that the browser host is holding
            clientSite_.reset();
            htmlDocument_.reset();
            htmlWindow2_.reset();
            webBrowser_.reset();

            // these are created on demand, don't need to be restored
            m_window.reset();
            m_document.reset();

            DoDeferredRelease();
        }

        const variant ActiveXBrowserHost::getVariant(const VARIANT* var)
        {
            variant target;

            _variant_t source(var);
            switch (V_VT(&source))
            {
            case VT_R4:
            case VT_R8:
            case VT_DECIMAL:
                target = static_cast<double>(source);
                break;
            case VT_I1:
            case VT_I2:
            case VT_I4:
            case VT_UI1:
            case VT_UI2:
            case VT_INT:
                target = static_cast<long>(source);
                break;
            case VT_UI4:
            case VT_UINT:
                target = static_cast<unsigned long>(source);
                break;
            case VT_I8:
                target = static_cast<int64_t>(static_cast<__int64>(source));
                break;
            case VT_UI8:
                target = static_cast<uint64_t>(
                    static_cast<unsigned __int64>(source));
                break;
            case VT_LPSTR:
            case VT_LPWSTR:
            case VT_BSTR:
            case VT_CLSID:
                {
                    _bstr_t string(source);
                    const wchar_t* p = static_cast<const wchar_t*>(string);
                    if (p) {
                        // return it as a UTF8 std::string
                        std::wstring tmp(p);
                        target = wstring_to_utf8(tmp);
                    }
                }
                break;
            case VT_UNKNOWN:
            case VT_DISPATCH:
                {
                    ActiveXBrowserHostPtr shared_this(
                        static_pointer_cast<ActiveXBrowserHost>(
                        shared_from_this()));
                    target = JSObjectPtr(IDispatchAPI::create(
                        static_cast<IDispatch*>(source), shared_this));
                    V_DISPATCH(&source)->Release();
                }
                break;
            case VT_ERROR:
            case VT_BOOL:
                target = static_cast<bool>(source);
                break;
            case VT_NULL:
                target = FBNull();
                break;
            case VT_EMPTY:
            default:
                // target is already empty, leave it such
                break;
            }

            return target;
        }

        void ActiveXBrowserHost::getComVariant(
            VARIANT *dest, const variant& var)
        {
            // cleanup
            ::VariantInit(dest);

            const ComVariantBuilderMap& builderMap = getComVariantBuilderMap();
            const std::type_info& type = var.get_type();
            ComVariantBuilderMap::const_iterator it = builderMap.find(&type);
            if (it == builderMap.end()) {
                // unhandled type :(
                return;
            }

            ActiveXBrowserHostPtr shared_this(
                static_pointer_cast<ActiveXBrowserHost>(shared_from_this()));
            _variant_t variant(it->second(shared_this, var));
            *dest = variant.Detach();
        }

        BrowserStreamPtr ActiveXBrowserHost::_createStream(
            const std::string& url, const PluginEventSinkPtr& callback,
            bool cache, bool seekable, size_t internalBufferSize) const
        {
            assertMainThread();

            ActiveXStreamPtr stream(make_shared<ActiveXStream>(
                url, cache, seekable, internalBufferSize));

            stream->AttachObserver(callback);

            if (!stream->init()) {
                return ActiveXStreamPtr();
            }

            StreamCreatedEvent ev(stream.get());
            stream->SendEvent(&ev);
            if (seekable) {
                stream->signalOpened();
            }

            return stream;
        }

        BrowserStreamPtr ActiveXBrowserHost::_createPostStream(
            const std::string& url, const PluginEventSinkPtr& callback,
            const std::string& postdata, bool cache, bool seekable,
            size_t internalBufferSize) const
        {
            assertMainThread();

            ActiveXStreamPtr stream(make_shared<ActiveXStream>(
                url, cache, seekable, internalBufferSize, postdata));

            stream->AttachObserver(callback);

            if (!stream->init()) {
                return ActiveXStreamPtr();
            }

            StreamCreatedEvent ev(stream.get());
            stream->SendEvent(&ev);
            if (seekable) {
                stream->signalOpened();
            }

            return stream;
        }

        void ActiveXBrowserHost::DoDeferredRelease() const
        {
            assertMainThread();

            IDispatchWRef deferred;
            while (m_deferredObjects.try_pop(deferred)) {
                if (deferred.expired()) {
                    continue;
                }
                IDispatchSRef ptr(deferred.lock());
                IDispatchRefList::iterator iter = std::find(
                    m_heldIDispatch.begin(), m_heldIDispatch.end(), ptr);
                if (iter != m_heldIDispatch.end()) {
                    m_heldIDispatch.erase(iter);
                }
                ptr->getPtr()->Release();
            }

            // remove any expired IDispatch WeakReferences
            IDispatchExRefMap::iterator iter = m_cachedIDispatch.begin();
            IDispatchExRefMap::iterator end = m_cachedIDispatch.end();
            while (iter != end) {
                if (iter->second.expired()) {
                    iter = m_cachedIDispatch.erase(iter);
                } else {
                    ++iter;
                }
            }
        }

        void ActiveXBrowserHost::deferred_release(const IDispatchWRef& obj) const
        {
            m_deferredObjects.push(obj);
            if (isMainThread()) {
                DoDeferredRelease();
            }
        }

        const IDispatchExPtr ActiveXBrowserHost::getJSAPIWrapper(
            const JSAPIWeakPtr& api, bool autoRelease /* = false*/)
        {
            typedef boost::shared_ptr<ShareableReference<IDispatchEx> >
                SharedIDispatchRef;

            // This should only be called on the main thread
            assertMainThread();

            JSAPIPtr ptr(api.lock());
            if (!ptr) {
                // createCOMJSObject returns already addrefs pointer
                return IDispatchExPtr(
                    getFactoryInstance()->createCOMJSObject(
                    shared_from_this(), api, false), false);
            }

            IDispatchExRefMap::iterator fnd = m_cachedIDispatch.find(ptr.get());
            if (fnd != m_cachedIDispatch.end()) {
                SharedIDispatchRef ref(fnd->second.lock());
                if (ref) {
                    // Fortunately this doesn't have to be threadsafe since
                    // this method only gets called from the main thread and
                    // the browser access happens on that thread as well!
                    IDispatchExPtr dispatchEx(ref->getPtr());
                    BOOST_ASSERT(dispatchEx);
                    return dispatchEx;
                } else {
                    m_cachedIDispatch.erase(fnd);
                }
            }

            // createCOMJSObject returns already addrefs pointer
            IDispatchExPtr dispatchEx(getFactoryInstance()->createCOMJSObject(
                shared_from_this(), api, autoRelease), false);
            m_cachedIDispatch[ptr.get()] = 
                _getWeakRefFromCOMJSWrapper(dispatchEx.get());
            return dispatchEx;
        }

        IDispatchWRef ActiveXBrowserHost::getIDispatchRef(IDispatch* obj)
        {
            IDispatchSRef ref(
                make_shared<ShareableReference<IDispatch> >(obj));
            obj->AddRef();
            m_heldIDispatch.push_back(ref);
            return ref;
        }

        void ActiveXBrowserHost::ReleaseAllHeldObjects()
        {
            for (IDispatchRefList::iterator iter = m_heldIDispatch.begin(),
                end = m_heldIDispatch.end(); iter != end;) {
                    (*iter)->getPtr()->Release();
                    iter = m_heldIDispatch.erase(iter);
            }
        }
    }  // namespace ActiveX
}  // namespace FB
