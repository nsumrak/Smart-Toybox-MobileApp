////////////////////////////////////////////////////////////////////////////
//                           **** WAVPACK ****                            //
//                  Hybrid Lossless Wavefile Compressor                   //
//              Copyright (c) 1998 - 2007 Conifer Software.               //
//                          All Rights Reserved.                          //
//      Distributed under the BSD Software License (see license.txt)      //
////////////////////////////////////////////////////////////////////////////

// tinypack.c

#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <math.h>
#include <stdio.h>

#include "wavpack.h"

#define CLEAR(destin) memset (&destin, 0, sizeof (destin));

#ifndef FALSE
#define FALSE 0
#define TRUE 1
#endif


///////////////////////////// local variable storage //////////////////////////

// This structure and function are used to write completed WavPack blocks in
// a device independent way.

typedef struct {
	char *outbuff;
	uint32_t bytes_written;
} write_id;

static int write_block (void *id, void *data, int32_t length)
{
	if (!id) return FALSE;
	write_id *wid = (write_id *)id;

	if (data && length && wid->outbuff) {
		memcpy(wid->outbuff + wid->bytes_written, data, length);
		wid->bytes_written += length;
	}

	return TRUE;
}

// Pack 16000hz,16bit,mono,pcm buffer for wv!
// input_buffer should be at least sample_count * 4 in size!!!
// writes packed data into input buffer
// returns written packed data size in bytes
int pack_buffer(char *input_buffer, int sample_count)
{
	WavpackConfig config;
    write_id wv_file;
    WavpackContext *wpc;

	CLEAR(config);
    CLEAR (wv_file);

	wv_file.outbuff = input_buffer;
	wpc = WavpackOpenFileOutput (write_block, &wv_file, 0);

	config.flags = CONFIG_HYBRID_FLAG | CONFIG_FAST_FLAG; // CONFIG_HIGH_FLAG;
	config.bitrate = 640; // 512-4096 (bitspersample*256)
	//config.shaping_weight = (int)(strtod(++*argv, argv) * 1024.0); // -1024 - 1024
	//if (!config.shaping_weight) {
	//	config.flags |= CONFIG_SHAPE_OVERRIDE;
	//	config.flags &= ~CONFIG_HYBRID_SHAPE;
	//}
	//else config.flags |= (CONFIG_HYBRID_SHAPE | CONFIG_SHAPE_OVERRIDE);

	config.bits_per_sample = 16;
    config.bytes_per_sample = 2;
    config.num_channels = 1;
    config.sample_rate = 16000;

    WavpackSetConfiguration (wpc, &config, sample_count);
	WavpackPackInit(wpc);

	int32_t *sample_buffer = (int32_t *)input_buffer;
	uchar *sptr = (uchar*)input_buffer + (sample_count-1)*2;
	int32_t *dptr = sample_buffer + sample_count-1;
	int i;
	for (i = 0; i < sample_count; i++) {
		*dptr-- = sptr[0] | ((int32_t)(signed char)sptr[1] << 8);
		sptr -= 2;
	}

	int res = WavpackPackSamples(wpc, sample_buffer, sample_count);
	res = WavpackFlushSamples(wpc) || res;

	WavpackCloseFile (wpc);

	return res ? wv_file.bytes_written : 0;
}
