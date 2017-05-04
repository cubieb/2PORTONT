/**
 *	@file    iphoneClientAppDelegate.h
 *	@version 66615a4 (HEAD, tag: MATRIXSSL-3-6-2-OPEN, tag: MATRIXSSL-3-6-2-COMM, origin/master, origin/HEAD, master)
 *
 *	Summary.
 */
#import <UIKit/UIKit.h>

@class IphoneClientViewController;

@interface iphoneClientAppDelegate : NSObject <UIApplicationDelegate>
{
    UIWindow* window;
    IphoneClientViewController* viewController;
}

@property (nonatomic, retain) IBOutlet UIWindow* window;
@property (nonatomic, retain) IBOutlet IphoneClientViewController* viewController;

@end

