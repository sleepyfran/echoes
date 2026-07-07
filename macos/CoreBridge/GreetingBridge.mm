#import "GreetingBridge.h"

#include <string>

#include "echoes/greeting.h"

@implementation EchoesGreeting

+ (NSString *)greeting
{
    const std::string value = echoes::greeting();
    return [NSString stringWithUTF8String:value.c_str()];
}

@end
