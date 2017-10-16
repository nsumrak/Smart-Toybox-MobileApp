////////////////////////////////////////////////////////////////////////////
//                           **** WAVPACK ****                            //
//                  Hybrid Lossless Wavefile Compressor                   //
//              Copyright (c) 1998 - 2007 Conifer Software.               //
//                          All Rights Reserved.                          //
//      Distributed under the BSD Software License (see license.txt)      //
////////////////////////////////////////////////////////////////////////////

// wavpack.h

#ifndef WAVPACK_H
#define WAVPACK_H

// This header file contains all the definitions required to use the
// functions in "wputils.c" to read and write WavPack files and streams.

#include <sys/types.h>

#ifdef __BORLANDC__
typedef unsigned long uint32_t;
typedef long int32_t;
#elif defined(_WIN32) && !defined(__MINGW32__)
#include <stdlib.h>
typedef unsigned __int64 uint64_t;
typedef unsigned __int32 uint32_t;
typedef __int64 int64_t;
typedef __int32 int32_t;
#else
#include <inttypes.h>
#endif

typedef unsigned char   uchar;

#if !defined(__GNUC__) || defined(WIN32)
typedef unsigned short  ushort;
typedef unsigned int    uint;
#endif

///////////////////////// WavPack Configuration ///////////////////////////////

// This external structure is used during encode to provide configuration to
// the encoding engine.

typedef struct {
    int bitrate, shaping_weight;
    int bits_per_sample, bytes_per_sample, num_channels;
    int32_t block_samples, sample_rate;
    uint32_t flags;
} WavpackConfig;

#define CONFIG_HYBRID_FLAG      8       // hybrid mode
#define CONFIG_JOINT_STEREO     0x10    // joint stereo
#define CONFIG_HYBRID_SHAPE     0x40    // noise shape (hybrid mode only)
#define CONFIG_FAST_FLAG        0x200   // fast mode
#define CONFIG_HIGH_FLAG        0x800   // high quality mode
#define CONFIG_VERY_HIGH_FLAG   0x1000  // very high
#define CONFIG_SHAPE_OVERRIDE   0x8000  // shaping mode specified
#define CONFIG_JOINT_OVERRIDE   0x10000 // joint-stereo mode specified
#define CONFIG_CREATE_WVC       0x80000 // create correction file
#define CONFIG_OPTIMIZE_WVC     0x100000 // maximize bybrid compression

////////////// Callbacks used for writing WavPack streams //////////

typedef int (*WavpackBlockOutput)(void *id, void *data, int32_t bcount);

//////////////////////// function prototypes and macros //////////////////////

typedef void WavpackContext;

#ifdef __cplusplus
extern "C" {
#endif

char *WavpackGetErrorMessage (WavpackContext *wpc);
uint32_t WavpackGetNumSamples (WavpackContext *wpc);
uint32_t WavpackGetSampleIndex (WavpackContext *wpc);
int WavpackLossyBlocks (WavpackContext *wpc);
WavpackContext *WavpackCloseFile (WavpackContext *wpc);
uint32_t WavpackGetSampleRate (WavpackContext *wpc);
int WavpackGetBitsPerSample (WavpackContext *wpc);
int WavpackGetBytesPerSample (WavpackContext *wpc);
int WavpackGetNumChannels (WavpackContext *wpc);
uint32_t WavpackGetFileSize (WavpackContext *wpc);
WavpackContext *WavpackOpenFileOutput (WavpackBlockOutput blockout, void *wv_id, void *wvc_id);
int WavpackSetConfiguration (WavpackContext *wpc, WavpackConfig *config, uint32_t total_samples);
int WavpackPackInit (WavpackContext *wpc);
int WavpackPackSamples (WavpackContext *wpc, int32_t *sample_buffer, uint32_t sample_count);
int WavpackFlushSamples (WavpackContext *wpc);
void WavpackUpdateNumSamples (WavpackContext *wpc, void *first_block);
void WavpackNativeToLittleEndian (void *data, char *format);
void WavpackLittleEndianToNative (void *data, char *format);

int pack_buffer(char *input_buffer, int sample_count);

#ifdef __cplusplus
}
#endif

#endif
