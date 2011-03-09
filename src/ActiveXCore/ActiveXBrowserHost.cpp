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

#include <atlcomcli.h>
#include <utility>

using boost::assign::list_of;
using boost::int64_t;
using boost::make_shared;
using boost::shared_lock;
using boost::shared_mutex;
using boost::static_pointer_cast;
using boost::uint64_t;
using boost::upgrade_lock;

namespace FB
{
    namespace ActiveX
    {
        template<class T>
        ComVariantBuilderMap::value_type makeBuilderEntry()
        {
            return ComVariantBuilderMap::value_type(&typeid(T),
                select_ccomvariant_builder::select<T>());
        }

        const ComVariantBuilderMap makeComVariantBuilderMap()
        {
            ComVariantBuilderMap tdm;
            tdm.insert(makeBuilderEntry<bool>());
            tdm.insert(makeBuilderEntry<char>());
            tdm.insert(makeBuilderEntry<unsigned char>());
            tdm.insert(makeBuilderEntry<short>());
            tdm.insert(makeBuilderEntry<unsigned short>());
            tdm.insert(makeBuilderEntry<int>());
            tdm.insert(makeBuilderEntry<unsigned int>());
            tdm.insert(makeBuilderEntry<long>());
            tdm.insert(makeBuilderEntry<unsigned long>());

#ifndef BOOST_NO_LONG_LONG
            tdm.insert(makeBuilderEntry<long long>());
            tdm.insert(makeBuilderEntry<unsigned long long>());
#endif

            tdm.insert(makeBuilderEntry<float>());
            tdm.insert(makeBuilderEntry<double>());

            tdm.insert(makeBuilderEntry<std::string>());
            tdm.insert(makeBuilderEntry<std::wstring>());

            tdm.insert(makeBuilderEntry<VariantList>());
            tdm.insert(makeBuilderEntry<VariantMap>());
            tdm.insert(makeBuilderEntry<JSAPIPtr>());
            tdm.insert(makeBuilderEntry<JSAPIWeakPtr>());
            tdm.insert(makeBuilderEntry<JSObjectPtr>());

            tdm.insert(makeBuilderEntry<FBVoid>());
            tdm.insert(makeBuilderEntry<FBNull>());

            return tdm;
        }

        const ComVariantBuilderMap& getComVariantBuilderMap()
        {
            static const ComVariantBuilderMap tdm(makeComVariantBuilderMap());
            return tdm;
        }

        ActiveXBrowserHost::ActiveXBrowserHost(IWebBrowser* webBrowser,
            IOleClientSite* site) :
        m_spClientSite(site), m_webBrowser(webBrowser),
            m_messageWin(new WinMessageWindow())
        {
            if (m_webBrowser) {
                m_webBrowser->get_Document(&m_htmlDocument);
                CComQIPtr<IHTMLDocument2> htmlDocument2(m_htmlDocument);
                htmlDocument2->get_parentWindow(&m_htmlWindow);
            }
        }

        ActiveXBrowserHost::~ActiveXBrowserHost(void)
        {
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
            return DOM::WindowPtr(
                new AXDOM::Window(ptr_cast<IDispatchAPI>(obj), m_webBrowser));
        }

        DOM::DocumentPtr ActiveXBrowserHost::_createDocument(
            const JSObjectPtr& obj) const
        {
            return DOM::DocumentPtr(
                new AXDOM::Document(ptr_cast<IDispatchAPI>(obj), m_webBrowser));
        }

        DOM::ElementPtr ActiveXBrowserHost::_createElement(
            const JSObjectPtr& obj) const
        {
            return DOM::ElementPtr(
                new AXDOM::Element(ptr_cast<IDispatchAPI>(obj), m_webBrowser));
        }

        DOM::NodePtr ActiveXBrowserHost::_createNode(
            const JSObjectPtr& obj) const
        {
            return DOM::NodePtr(
                new AXDOM::Node(ptr_cast<IDispatchAPI>(obj), m_webBrowser));
        }

        void ActiveXBrowserHost::initDOMObjects()
        {
            if (!m_window) {
                ActiveXBrowserHostPtr shared_this(
                    static_pointer_cast<ActiveXBrowserHost>(
                    shared_from_this()));
                m_window = DOM::Window::create(
                    IDispatchAPI::create(m_htmlWindow, shared_this));
                m_document = DOM::Document::create(
                    IDispatchAPI::create(m_htmlDocument, shared_this));
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
            CComQIPtr<IOleControlSite> site(m_spClientSite);

            CComPtr<IDispatch> dispatch;
            HRESULT hr = site->GetExtendedControl(&dispatch);
            if (FAILED(hr)) {
                return DOM::ElementPtr();
            }

            CComQIPtr<IHTMLElement2> htmlElement(dispatch);
            ActiveXBrowserHostPtr shared_this(
                static_pointer_cast<ActiveXBrowserHost>(shared_from_this()));
            return DOM::Document::create(
                IDispatchAPI::create(htmlElement, shared_this));
        }

        void ActiveXBrowserHost::evaluateJavaScript(const std::string &script)
        {
            if (!m_htmlWindow) {
                throw script_error("Can't execute JavaScript: Window is NULL");
            }

            _variant_t result;
            _bstr_t code(script.c_str());
            static const _bstr_t language(L"javascript");
            HRESULT hr = m_htmlWindow->execScript(code, language, &result);
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
            m_spClientSite.Release();
            m_htmlDocument.Release();
            m_htmlWindow.Release();
            m_webBrowser.Release();
            m_window.reset();
            m_document.reset();
            DoDeferredRelease();
            assert(m_deferredObjects.empty());
        }

        const variant ActiveXBrowserHost::getVariant(const VARIANT* variant)
        {
            FB::variant target;

            _variant_t source(variant);
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
            _variant_t variant((it->second)(shared_this, var));
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

        bool hasExpired(const std::pair<void*, WeakIDispatchRef>& pair)
        {
            return pair.second.expired();
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

            IDispatch* deferred;
            while (m_deferredObjects.try_pop(deferred)) {
                deferred->Release();
            }

            // Also remove any expired IDispatch WeakReferences
            IDispatchRefMap::iterator iter = m_cachedIDispatch.begin();
            IDispatchRefMap::iterator endIter = m_cachedIDispatch.end();
            while (iter != endIter) {
                if (hasExpired(*iter)) {
                    iter = m_cachedIDispatch.erase(iter);
                } else {
                    ++iter;
                }
            }
        }

        void ActiveXBrowserHost::deferred_release(IDispatch* m_obj) const
        {
            m_deferredObjects.push(m_obj);
        }

        IDispatchEx* ActiveXBrowserHost::getJSAPIWrapper(
            const JSAPIWeakPtr& api, bool autoRelease /* = false*/)
        {
            typedef boost::shared_ptr<ShareableReference<IDispatchEx> >
                SharedIDispatchRef;

            // This should only be called on the main thread
            assertMainThread();

            JSAPIPtr ptr(api.lock());
            if (!ptr) {
                return getFactoryInstance()->createCOMJSObject(
                    shared_from_this(), api, false);
            }

            IDispatchEx* ret = NULL;
            IDispatchRefMap::iterator fnd = m_cachedIDispatch.find(ptr.get());
            if (fnd != m_cachedIDispatch.end()) {
                SharedIDispatchRef ref(fnd->second.lock());
                if (ref) {
                    // Fortunately this doesn't have to be threadsafe since
                    // this method only gets called from the main thread and
                    // the browser access happens on that thread as well!
                    ret = ref->getPtr();
                    ret->AddRef();
                } else {
                    m_cachedIDispatch.erase(fnd);
                }
            }
            if (!ret) {
                ret = getFactoryInstance()->createCOMJSObject(
                    shared_from_this(), api, autoRelease);
                m_cachedIDispatch[ptr.get()] = _getWeakRefFromCOMJSWrapper(ret);
            }

            return ret;
        }
    }  // namespace ActiveX
}  // namespace FB
