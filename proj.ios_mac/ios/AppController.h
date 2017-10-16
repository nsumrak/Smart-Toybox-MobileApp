#import <UIKit/UIKit.h>
#import <Foundation/Foundation.h>
#import <AudioToolbox/AudioQueue.h>
#import <AudioToolbox/AudioFile.h>

@class RootViewController;

#define NUM_BUFFERS 3
#define AUDIOBUF_SIZE 160000 // 10 sec buffer * 16KHz * 16bit
#define CHUNK_SIZE 735 //1000

typedef struct {
	AudioQueueRef queue;
	AudioQueueBufferRef buffers[NUM_BUFFERS];
	short audioBuff[AUDIOBUF_SIZE];
	size_t recorded, played; // number of shorts
	bool recording, playing;
} AudioContext;

@interface AppController : NSObject <UIApplicationDelegate> {
    UIWindow *window;
	
	@public AudioContext audioCtx;
}

@property(nonatomic, readonly) RootViewController* viewController;

@end

