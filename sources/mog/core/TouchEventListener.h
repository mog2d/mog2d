#ifndef TouchEventListener_h
#define TouchEventListener_h

#include <memory>
#include <set>
#include <functional>
#include "mog/core/plain_objects.h"
#include "mog/core/Touch.h"
#include "mog/base/Entity.h"

using namespace std;

namespace mog {
    class TouchEventListener : public enable_shared_from_this<TouchEventListener> {
    public:
        ~TouchEventListener();
        
        static shared_ptr<TouchEventListener> create();
        
        void touchBegin(const Touch &touch, const shared_ptr<Entity> &entity);
        void touchMove(const Touch &touch, const shared_ptr<Entity> &entity);
        void touchEnd(const Touch &touch, const shared_ptr<Entity> &entity);

        void setOnTapEvent(function<void(const Touch &t, const shared_ptr<Entity> &e)> onTapEvent);
        void setOnTouchBeginEvent(function<bool(const Touch &t, const shared_ptr<Entity> &e)> onTouchBeginEvent);
        void setOnTouchMoveEvent(function<void(const Touch &t, const shared_ptr<Entity> &e)> onTouchMoveEvent);
        void setOnTouchEndEvent(function<void(const Touch &t, const shared_ptr<Entity> &e)> onTouchEndEvent);
        void setOnTouchEnterEvent(function<void(const Touch &t, const shared_ptr<Entity> &e)> onTouchEnterEvent);
        void setOnTouchOverEvent(function<void(const Touch &t, const shared_ptr<Entity> &e)> onTouchOverEvent);
        void setOnTouchExitEvent(function<void(const Touch &t, const shared_ptr<Entity> &e)> onTouchExitEvent);

        void setEnable(bool enabled);
        
    protected:
        TouchEventListener();
        bool enabled = true;
        set<unsigned long long> touchBeganIds;
        set<unsigned long long> touchEnteredIds;

        bool isTouchBegan(unsigned long long touchId);
        bool isTouchEntered(unsigned long long touchId);

        function<void(const Touch &t, const shared_ptr<Entity> &e)> onTapEvent;
        function<bool(const Touch &t, const shared_ptr<Entity> &e)> onTouchBeginEvent;
        function<void(const Touch &t, const shared_ptr<Entity> &e)> onTouchMoveEvent;
        function<void(const Touch &t, const shared_ptr<Entity> &e)> onTouchEndEvent;
        function<void(const Touch &t, const shared_ptr<Entity> &e)> onTouchEnterEvent;
        function<void(const Touch &t, const shared_ptr<Entity> &e)> onTouchOverEvent;
        function<void(const Touch &t, const shared_ptr<Entity> &e)> onTouchExitEvent;
    };
}

#endif /* TouchEventListener_h */
