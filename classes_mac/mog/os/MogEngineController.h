#import <Foundation/Foundation.h>
#import "MogViewController.h"
#import "MogView.h"
#import "MogTouchEventDelegate.h"

@interface MogEngineController : NSObject <MogTouchEventDelegate>

- (instancetype)initWithMog:(MogViewController *)viewController view:(MogView *)view;
- (void)startEngine;
- (void)stopEngine;
- (void)didReceiveMemoryWarning;
- (void)terminateEngine;
- (void)setFps:(float)fps;
- (void)fireTouchEvent:(unsigned int)touchId location:(CGPoint)location action:(TouchAction)action;

@end