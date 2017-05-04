/**
 *	@file    SslDelegateProtocol.h
 *	@version 66615a4 (HEAD, tag: MATRIXSSL-3-6-2-OPEN, tag: MATRIXSSL-3-6-2-COMM, origin/master, origin/HEAD, master)
 *
 *	Summary.
 */
#import <UIKit/UIKit.h>


@protocol SslDelegateProtocol

// The delegate can receive text notifications about status and error messages.
- (void) logDebugMessage:(NSString*)message;

// The delegate can receive data from the SSL connection.
- (void) handleData:(NSString*)data;

@end
