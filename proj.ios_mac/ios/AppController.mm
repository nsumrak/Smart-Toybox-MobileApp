/****************************************************************************
 Copyright (c) 2010 cocos2d-x.org

 http://www.cocos2d-x.org

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/

#import "AppController.h"
#import "platform/ios/CCEAGLView-ios.h"
#import "cocos2d.h"
#import "AppDelegate.h"
#import "RootViewController.h"

@implementation AppController

#pragma mark -
#pragma mark Application lifecycle

// cocos2d application instance
static AppDelegate s_sharedApplication;

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {    

    cocos2d::Application *app = cocos2d::Application::getInstance();
    app->initGLContextAttrs();
    cocos2d::GLViewImpl::convertAttrs();

    // Override point for customization after application launch.

    // Add the view controller's view to the window and display.
    window = [[UIWindow alloc] initWithFrame: [[UIScreen mainScreen] bounds]];

    // Init the CCEAGLView
    CCEAGLView *eaglView = [CCEAGLView viewWithFrame: [window bounds]
                                         pixelFormat: (NSString*)cocos2d::GLViewImpl::_pixelFormat
                                         depthFormat: cocos2d::GLViewImpl::_depthFormat
                                  preserveBackbuffer: NO
                                          sharegroup: nil
                                       multiSampling: NO
                                     numberOfSamples: 0 ];
    
    // Enable or disable multiple touches
    [eaglView setMultipleTouchEnabled:NO];

    // Use RootViewController manage CCEAGLView 
    _viewController = [[RootViewController alloc] initWithNibName:nil bundle:nil];
//    _viewController.wantsFullScreenLayout = YES;
    _viewController.view = eaglView;

    // Set RootViewController to window
    if ( [[UIDevice currentDevice].systemVersion floatValue] < 6.0)
    {
        // warning: addSubView doesn't work on iOS6
        [window addSubview: _viewController.view];
    }
    else
    {
        // use this method on ios6
        [window setRootViewController:_viewController];
    }

    [window makeKeyAndVisible];

//    [[UIApplication sharedApplication] setStatusBarHidden:true];

    // IMPORTANT: Setting the GLView should be done after creating the RootViewController
    cocos2d::GLView *glview = cocos2d::GLViewImpl::createWithEAGLView(eaglView);
    cocos2d::Director::getInstance()->setOpenGLView(glview);

    app->run();

    return YES;
}


- (void)applicationWillResignActive:(UIApplication *)application {
    /*
     Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
     Use this method to pause ongoing tasks, disable timers, and throttle down OpenGL ES frame rates. Games should use this method to pause the game.
     */
     //We don't need to call this method any more. It will interupt user defined game pause&resume logic
    /* cocos2d::Director::getInstance()->pause(); */
}

- (void)applicationDidBecomeActive:(UIApplication *)application {
    /*
     Restart any tasks that were paused (or not yet started) while the application was inactive. If the application was previously in the background, optionally refresh the user interface.
     */
     //We don't need to call this method any more. It will interupt user defined game pause&resume logic
    /* cocos2d::Director::getInstance()->resume(); */
}

- (void)applicationDidEnterBackground:(UIApplication *)application {
    /*
     Use this method to release shared resources, save user data, invalidate timers, and store enough application state information to restore your application to its current state in case it is terminated later. 
     If your application supports background execution, called instead of applicationWillTerminate: when the user quits.
     */
    cocos2d::Application::getInstance()->applicationDidEnterBackground();
}

- (void)applicationWillEnterForeground:(UIApplication *)application {
    /*
     Called as part of  transition from the background to the inactive state: here you can undo many of the changes made on entering the background.
     */
    cocos2d::Application::getInstance()->applicationWillEnterForeground();
}

- (void)applicationWillTerminate:(UIApplication *)application {
    /*
     Called when the application is about to terminate.
     See also applicationDidEnterBackground:.
     */
}


#pragma mark -
#pragma mark Memory management

- (void)applicationDidReceiveMemoryWarning:(UIApplication *)application {
    /*
     Free up as much memory as possible by purging cached data objects that can be recreated (or reloaded from disk) later.
     */
}


- (void)dealloc {
    [window release];
    [super dealloc];
}

#pragma mark -
#pragma mark Audio Playback and Recording

static AudioStreamBasicDescription dataFormat = {
	.mSampleRate = 16000.0,
	.mFormatID = kAudioFormatLinearPCM,
	.mFormatFlags = kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked,
	.mFramesPerPacket  = 1,
	.mChannelsPerFrame = 1,
	.mBytesPerFrame    = 2,
	.mBytesPerPacket   = 2,
	.mBitsPerChannel   = 16,
};

void AudioOutputCallback(void *userData, AudioQueueRef queue, AudioQueueBufferRef outBuff)
{
	//cocos2d::log("AudioOutputCallback");
	AudioContext *ctx = (AudioContext *)userData;
	if (!ctx->playing) return;

	if (ctx->played < ctx->recorded) {
		//cocos2d::log("AudioOutputCallback played = %ld recorded = %ld", ctx->played, ctx->recorded);
		outBuff->mAudioDataByteSize = (int)(ctx->recorded - ctx->played);
		if (outBuff->mAudioDataByteSize > CHUNK_SIZE) outBuff->mAudioDataByteSize = CHUNK_SIZE;
		memcpy(outBuff->mAudioData, ctx->audioBuff + ctx->played, outBuff->mAudioDataByteSize);
		ctx->played += outBuff->mAudioDataByteSize / 2;
		AudioQueueEnqueueBuffer(queue, outBuff, 0, NULL);
	} else {
		cocos2d::log("AudioOutputCallback done!");
		return;
	}
}

void AudioInputCallback(void * inUserData,  // Custom audio metadata
                        AudioQueueRef inAQ,
                        AudioQueueBufferRef inBuffer,
                        const AudioTimeStamp * inStartTime,
                        UInt32 inNumberPacketDescriptions,
                        const AudioStreamPacketDescription * inPacketDescs)
{
	AudioContext * audioCtx = (AudioContext *)inUserData;
	
	if (!audioCtx->recording) return;

//	cocos2d::log("AudioIntputCallback recorded = %ld,  currently recorded = %ld", audioCtx->recorded, inBuffer->mAudioDataByteSize/2);

	memcpy(audioCtx->audioBuff + audioCtx->recorded, inBuffer->mAudioData, inBuffer->mAudioDataByteSize);
	audioCtx->recorded += inBuffer->mAudioDataByteSize / 2;
	if (audioCtx->recording)
		AudioQueueEnqueueBuffer(audioCtx->queue, inBuffer, 0, NULL);
}

- (BOOL) startPlayback {
	cocos2d::log("Start playback, recorded = %ld", audioCtx.recorded);

	[self stopRecording];
	[self stopPlaying];

	audioCtx.played = 0;

	OSStatus status = AudioQueueNewOutput(&dataFormat,
								 AudioOutputCallback,
								 &audioCtx,
								 CFRunLoopGetCurrent(),
								 kCFRunLoopCommonModes,
								 0,
								 &audioCtx.queue);

	if (status) {
		cocos2d::log("error creating audio output queue");
		return FALSE;
	}

	audioCtx.playing = true;

	for (int i = 0; i < NUM_BUFFERS; i++) {
		AudioQueueAllocateBuffer(audioCtx.queue, CHUNK_SIZE, &audioCtx.buffers[i]);
		//AudioQueueEnqueueBuffer(audioCtx.queue, audioCtx.buffers[i], 0, nil);
		AudioOutputCallback(&audioCtx, audioCtx.queue, audioCtx.buffers[i]);
	}

	status = AudioQueueStart(audioCtx.queue, NULL);
	return !status;
}

- (BOOL) startPlaybackWithBuffer:(short *)buffer len:(size_t)len {
	cocos2d::log("start playback with buffer, buffer len = %ld", len);
	audioCtx.recorded = len > AUDIOBUF_SIZE ? AUDIOBUF_SIZE : len;
	memcpy(audioCtx.audioBuff, buffer, audioCtx.recorded * 2);
	return [self startPlayback];
}

- (BOOL) startRecording {
	cocos2d::log("Start recording");

	[self stopRecording];
	[self stopPlaying];
	
	memset(audioCtx.audioBuff, 0, AUDIOBUF_SIZE * 2);
	audioCtx.recorded = 0;

	OSStatus status = AudioQueueNewInput(&dataFormat,
							  AudioInputCallback,
							  &audioCtx,
							  NULL,
							  kCFRunLoopCommonModes,
							  0,
							  &audioCtx.queue);

	if (status) {
		cocos2d::log("error creating audio input queue");
		return FALSE;
	}

	for (int i = 0; i < NUM_BUFFERS; i++) {
		AudioQueueAllocateBuffer(audioCtx.queue, CHUNK_SIZE, &audioCtx.buffers[i]);
		AudioQueueEnqueueBuffer(audioCtx.queue, audioCtx.buffers[i], 0, nil);
	}

	audioCtx.recording = true;

	status = AudioQueueStart(audioCtx.queue, NULL);
	return !status;
}

- (void)stopPlaying {
	if (!audioCtx.playing) return;

	cocos2d::log("stopPlaying");
	
	audioCtx.playing = false;

	AudioQueueStop(audioCtx.queue, true);

	for (int i = 0; i < NUM_BUFFERS; i++) {
		AudioQueueFreeBuffer(audioCtx.queue, audioCtx.buffers[i]);
	}

	AudioQueueDispose(audioCtx.queue, true);
}

- (size_t)stopRecording
{
	if (!audioCtx.recording) return 0;

	cocos2d::log("stopRecording");
	audioCtx.recording = false;

	AudioQueueStop(audioCtx.queue, true);

	for (int i = 0; i < NUM_BUFFERS; i++) {
		AudioQueueFreeBuffer(audioCtx.queue, audioCtx.buffers[i]);
	}

	AudioQueueDispose(audioCtx.queue, true);

	cocos2d::log("stopRecording returned %ld shorts recorded", audioCtx.recorded);
	
	return audioCtx.recorded;
}


@end

#pragma mark -
#pragma mark Native Defs

// RECORDING

int native_stopAudioRec()
{
	cocos2d::log("native_stopAudioRec()");
	AppController *appController =  (AppController *)[[UIApplication sharedApplication] delegate];
	return (int)[appController stopRecording];
//	if (!wavein) return 0;
//	stopping = true;
//	waveInStop(wavein);
//	waveInUnprepareHeader(wavein, &wihdr, sizeof(wihdr));
//	waveInClose(wavein);
//	wavein = 0;
//	stopping = false;
//	return (recorded = wihdr.dwBytesRecorded / 2);
//	//change_pitch((short*)rec_data_buf, wihdr.dwBytesRecorded / 2, 0, -7.0);
//	//pack_buffer(rec_data_buf, wihdr.dwBytesRecorded / 2, "c:\\w\\output.wv");
}

bool native_startAudioRec()
{
//	if (wavein) native_stopAudioRec();
//	if (waveout) native_stopPlay();
	cocos2d::log("native_startAudioRec()");
	AppController *appController =  (AppController *)[[UIApplication sharedApplication] delegate];
	return [appController startRecording];
}

bool native_getRecBuffer(short *buf, int size)
{
	cocos2d::log("native_getRecBuffer()");
	AppController *appController =  (AppController *)[[UIApplication sharedApplication] delegate];
	if (!appController->audioCtx.recorded) return false;
	if (size > appController->audioCtx.recorded) size = (int)appController->audioCtx.recorded;
	memcpy(buf, appController->audioCtx.audioBuff, size * 2);
	return true;
}

void native_stopPlay()
{
	cocos2d::log("native_stopPlay()");
	AppController *appController =  (AppController *)[[UIApplication sharedApplication] delegate];
	[appController stopPlaying];
//	if (!waveout) return;
//	if (stopping) return;
//	stopping = true;
//	waveOutReset(waveout);
//	waveOutUnprepareHeader(waveout, &wihdr, sizeof(wihdr));
//	waveOutClose(waveout);
//	stopping = false;
//	waveout = 0;
}

bool native_playBuffer(short *buf, int size)
{
	cocos2d::log("native_playBuffer");
	AppController *appController =  (AppController *)[[UIApplication sharedApplication] delegate];
	return [appController startPlaybackWithBuffer:buf len:size];
}


static int numtries = 0;

void native_startDnsSD()
{
	numtries = 0;
}

int native_getDnsSDdiscoveryNum()
{
	//return ++numtries / 8;
	return 0;   //Forces Demo mode
}

void native_getDnsSDdiscoveryItem(int i, char *buf, int bsize)
{
	buf[0] = 0;
	strcpy(buf, "192.168.0.103");
}


void native_stopDnsSD()
{
	// TODO
}

void native_startSmartConfig(const char *ssid, const char *pwd)
{
	// TODO
}

void native_stopSmartConfig()
{
	// TODO
}

void native_getSSID(char *buf, unsigned bsize)
{
	strncpy(buf, "IoTarium", bsize);
	buf[bsize - 1] = 0;
}

