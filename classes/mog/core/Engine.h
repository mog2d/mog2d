#ifndef Engine_h
#define Engine_h

#include <memory>
#include <unordered_map>
#include "mog/Constants.h"
#include "mog/core/TouchEventManager.h"
#include "mog/core/TouchInput.h"
#include "mog/core/KeyEvent.h"
#include "mog/core/NativeClass.h"
#include "mog/base/AppBase.h"

using namespace std;

namespace mog {
        
    class Engine {
    public:
        static Engine *getInstance();
        static Engine *initInstance();

        void initEngine(bool loadApp = true);
        void terminateEngine();
        void startEngine();
        void stopEngine();
        
        void onDrawFrame(map<unsigned int, TouchInput> touches);
        void onLowMemory();
        void onKeyEvent(const KeyEvent &keyEvent);

        shared_ptr<AppBase> getApp();
        void setApp(const shared_ptr<AppBase> &app);
        
        shared_ptr<NativeObject> getNativeObject(string name);
        void setNativeObject(string name, const shared_ptr<NativeObject> &nObj);
        void removeNativeObject(string name);
        
        bool isRunning();
        int getCurrentOrientation();
        unsigned long long getFrameCount();
        
        Size getDisplaySize();
        Size getScreenSize();
        float getScreenScale();
        float getDeviceDensity();
        void setDisplaySize(const Size &size, float deviceDensity);
        Density getDensity();
        void setScreenSizeBasedOnHeight(float height);
        void setScreenSizeBasedOnWidth(float width);
        
        Color getClearColor();
        void setClearColor(const Color &color);
        void clearColor();

        void startTimer();
        void stopTimer();
        long long getTimerElapsed();
        float getTimerElapsedSec();
        
        void setStatsViewEnable(bool enable);
        void setStatsViewAlignment(Alignment alignment);

        unsigned int registerOnUpdateFunc(function<void(unsigned int funcId)> onUpdateFunc);
        void removeOnUpdateFunc(unsigned int funcId);

    protected:
        static Engine *instance;
        static unsigned int onUpdateFuncIdCounter;
        
        shared_ptr<AppBase> app;
        unordered_map<string, shared_ptr<NativeObject>> nativeObjects;
        shared_ptr<Renderer> renderer;
        bool running = false;
        unsigned long long frameCount = 0;
        bool statsViewEnable = false;
        Size displaySize = Size::zero;
        Size screenSize = Size::zero;
        Color color = Color::black;
        Density density = Density::x1_0;
        float deviceDensity = 1.0f;

        Engine();

        int frameCountForFps = 0;
        bool timerRunning = false;
        long long timerStartTime = 0;
        long long timerPauseTime = 0;
        long long timerBackupTime = 0;
        float lastElapsedSec = 0;
        
        
        GLuint framebufferStack[8];
        int framebufferStackIndex = -1;
        unordered_map<unsigned int, function<void(unsigned int funcId)>> onUpdateFuncs;
        unordered_map<unsigned int, function<void(unsigned int funcId)>> onUpdateFuncsToAdd;
        vector<unsigned int> onUpdateFuncIdsToRemove;
        
        TouchEventManager *touchEventManager;
        void invokeOnUpdateFunc();
        
    private:
        bool displaySizeChanged = false;
        bool statsViewChanged = false;

        void initParameters();
        void setViewPortScale();
        void initScreen();
    };
}

#endif /* Engine_h */