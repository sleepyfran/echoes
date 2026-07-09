#pragma once

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

/**
 * Objective-C mirror of `entities::ProviderId`. Swift's Objective-C importer can't see C++
 * types (even through a bridging header), so this plain NS_ENUM is what gets exposed to Swift;
 * it's translated to/from `entities::ProviderId` inside ProviderBridge.mm.
 */
typedef NS_ENUM(NSInteger, ProviderIdBridge) {
    ProviderIdBridgeOneDrive,
    ProviderIdBridgeSpotify,
};

/**
 * Called once the authentication flow finishes, either successfully or with an error.
 * `errorMessage` is only populated when `success` is NO.
 */
typedef void (^ProviderAuthCompletion)(BOOL success, NSString * _Nullable errorMessage);

// Objective-C facade over the C++ provider methods.
@interface ProviderBridge : NSObject

+ (NSString *)start_authentication:(ProviderIdBridge)provider_id
                         completion:(ProviderAuthCompletion)completion;

@end

NS_ASSUME_NONNULL_END
