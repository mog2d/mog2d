#import "MogEngineController.h"
#import "mog/core/Engine.h"
#import "app/App.h"
#import "mog/Constants.h"
#import "mog/core/NativeClass.h"
#import "mog/libs/rpmalloc.h"
#ifdef SCRIPT_BINDNG_ENGINE_HEADER
#import SCRIPT_BINDNG_ENGINE_HEADER
#endif

@implementation MogEngineController {
    MogViewController *_mogViewController;
    MogView *_mogView;
    UIView *_launchScreenView;
    CADisplayLink *_displayLink;
    shared_ptr<mog::Engine> _engine;
    float _fps;
    map<unsigned int, mog::TouchInput> _touches;
}

- (instancetype)initWithMog:(MogViewController *)viewController view:(MogView *)view {
    self = [super init];
    if (!self) return self;
    
    rpmalloc_initialize();
#ifdef ENABLE_SCRIPT_BINDNG
    _engine = SCRIPT_BINDNG_ENGINE_CLASS::create();
#else
    _engine = mog::Engine::create(make_shared<mog::App>());
#endif
    _engine->setDisplaySize(mog::Size(view.glWidth, view.glHeight), mog::Size(view.frame.size.width, view.frame.size.height));
    _engine->resetScreenSize();
    _mogViewController = viewController;
    _mogView = view;
    
    _engine->setNativeObject(MOG_VIEW_CONTROLLER, mog::NativeObject::create((__bridge void *)viewController));
    _engine->setNativeObject(MOG_VIEW, mog::NativeObject::create((__bridge void *)view));
    _engine->setNativeObject(MOG_ENGINE_CONTROLLER, mog::NativeObject::create((__bridge void *)self));
    
    _fps = DEFAULT_FPS;
    
    UIStoryboard *storyboard = [UIStoryboard storyboardWithName:@"LaunchScreen" bundle:nil];
    if ([[UIDevice currentDevice] userInterfaceIdiom] == UIUserInterfaceIdiomPad) {
        UIStoryboard *ipadStoryboard = [UIStoryboard storyboardWithName:@"LaunchScreen-iPad" bundle:nil];
        if (ipadStoryboard) storyboard = ipadStoryboard;
    }
    _launchScreenView = storyboard.instantiateInitialViewController.view;
    [view addSubview:_launchScreenView];
    
    return self;
}

- (void)startDraw {
    if (_displayLink) return;
    _displayLink = [CADisplayLink displayLinkWithTarget:self selector:@selector(drawFrame)];
    if ([CADisplayLink instancesRespondToSelector:@selector(setPreferredFramesPerSecond:)]) {
        [_displayLink setPreferredFramesPerSecond:(_fps)];
    } else {
        _displayLink.frameInterval = 60 / _fps;
    }
    [_displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
}

- (void)stopDraw {
    if (!_displayLink) return;
    [_displayLink invalidate];
    _displayLink = nil;
}

- (void)startEngine {
    [self startDraw];
    _engine->startEngine();
}

- (void)stopEngine {
    _engine->stopEngine();
    [self stopDraw];
}

- (void)terminateEngine {
    [self stopEngine];
    _engine = nullptr;
    rpmalloc_finalize();
}

- (void)didReceiveMemoryWarning {
    _engine->onLowMemory();
}

- (void)setFps:(float)fps {
    _fps = fps;
    [self stopDraw];
    [self startDraw];
}

- (void)drawFrame {
    @synchronized (self) {
        if (!_displayLink) return;
        
        [EAGLContext setCurrentContext:_mogView.glContext];
        
        _engine->onDrawFrame(_touches);
        _touches.clear();
        
        [_mogView.glContext presentRenderbuffer:GL_RENDERBUFFER];
        
        if (_launchScreenView) {
            [_launchScreenView removeFromSuperview];
            _launchScreenView = nil;
        }
    }
}

- (void)fireTouchEvent:(unsigned int)touchId location:(CGPoint)location action:(TouchAction)action {
    @synchronized (self) {
        switch (action) {
            case TouchActionDown:
                _touches[touchId] = mog::TouchInput(mog::TouchAction::TouchDown, touchId, location.x, location.y);
                break;
                
            case TouchActionMove:
                if (_touches.count(touchId) == 0 || _touches[touchId].action == mog::TouchAction::TouchMove) {
                    _touches[touchId] = mog::TouchInput(mog::TouchAction::TouchMove, touchId, location.x, location.y);
                }
                break;
                
            case TouchActionUp:
                if (_touches.count(touchId) > 0 &&
                    (_touches[touchId].action == mog::TouchAction::TouchDown || _touches[touchId].action == mog::TouchAction::TouchDownUp)) {
                    _touches[touchId] = mog::TouchInput(mog::TouchAction::TouchDownUp, touchId, location.x, location.y);
                } else {
                    _touches[touchId] = mog::TouchInput(mog::TouchAction::TouchUp, touchId, location.x, location.y);
                }
                break;
        }
    }
    
}

@end