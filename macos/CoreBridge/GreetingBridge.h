#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

// Objective-C facade over the C++ `echoes::core` library. Only Objective-C
// types are exposed here so that this header is safe to import from Swift via
// the bridging header.
@interface EchoesGreeting : NSObject

+ (NSString *)greeting;

@end

NS_ASSUME_NONNULL_END
