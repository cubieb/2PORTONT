/*
 *  aplay.c - plays and records
 *
 *      CREATIVE LABS CHANNEL-files
 *      Microsoft WAVE-files
 *      SPARC AUDIO .AU-files
 *      Raw Data
 *
 *  Copyright (c) by Jaroslav Kysela <perex@suse.cz>
 *  Based on vplay program by Michael Beck
 *
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <malloc.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
//#include <getopt.h>
#include <fcntl.h>
//#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <locale.h>
#include <alsa/asoundlib.h>
#include <assert.h>
#include <sys/poll.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <sys/signal.h>
//#include <asm/byteorder.h>
#include <pthread.h>
#include "sc1445x_mcu_debug.h"
//#include "aconfig.h"
 
#include "sc1445x_ringplay.h"
//#include "version.h"

   #if ENABLE_NLS
# include <libintl.h>
#else
# define gettext(msgid) (msgid)
# define textdomain(domain)
# define bindtextdomain(domain, dir)
#endif

#define _(msgid) gettext (msgid)
#define gettext_noop(msgid) msgid
#define N_(msgid) gettext_noop (msgid)

#ifndef LLONG_MAX
//#define LLONG_MAX    9223372036854775807LL
#define LLONG_MAX	LONG_MAX
#endif

#define DEFAULT_FORMAT		SND_PCM_FORMAT_U8
#define DEFAULT_SPEED 		8000

#define FORMAT_DEFAULT		-1
#define FORMAT_RAW		0
 
#define FORMAT_WAVE		2
#define FORMAT_AU		3
/* global data */

static snd_pcm_t *handle;
static snd_hwdep_t *hwdephandle;

static struct {
	snd_pcm_format_t format;
	unsigned int channels;
	unsigned int rate;
} hwparams, rhwparams;

static int timelimit = 0; 
static int quiet_mode = 0;
 
static unsigned int sleep_min = 0;
 
//static snd_pcm_stream_t stream = SND_PCM_STREAM_PLAYBACK;
static unsigned char *audiobuf = NULL;
static snd_pcm_uframes_t chunk_size = 0;
static unsigned period_time = 0;
static unsigned buffer_time = 0;
static snd_pcm_uframes_t period_frames = 0;
static snd_pcm_uframes_t buffer_frames = 0;
static int avail_min = -1;
static int start_delay = 0;
static int stop_delay = 0;
//static int verbose = 0;
 
static size_t bits_per_sample, bits_per_frame;
static size_t chunk_bytes;
 
static int fd = -1;
static unsigned long pbrec_count = LLONG_MAX, fdcount;
 

/* needed prototypes */

static int playback(char *filename);
 
static void signal_handler(int sig);
 

ssize_t safe_read(int fd, void *buf, size_t count);
size_t test_wavefile_read(int fd, unsigned char *buffer, size_t *size, size_t reqsize, int line);
static ssize_t test_wavefile(int fd, unsigned char *_buffer, size_t size);
static int set_params(void);
static int xrun(void);
static int suspend(void);
static ssize_t pcm_write(unsigned char *data, size_t count);
static unsigned long calc_count(void);
static int playback_go(int fd, size_t loaded, unsigned long count, int rtype, char *name);
static int playback(char *name);
static int set_audio_mode( void );
static int restore_audio_mode(void );
static int threadintrunning =0;
static void signal_handler(int sig)
{

 	ua_stopringplay();
  
	if (fd > 1) {
		close(fd);
		fd = -1;
	}

	if (handle) {
		snd_pcm_close(handle);
		handle = NULL;
	}
	exit (0);
 
}

int ua_stopringplay(void)
{
  	if (!quiet_mode )
	{ 
 		quiet_mode  =1;
		while (threadintrunning) usleep(10000);
		return 0;
	}
	return -1;
}
int playringthread(void *pcm_name1);
int ua_playring(char *ringtone)
{
	pthread_t thread;
	if (threadintrunning ) 	return -1;
	threadintrunning =1; 
	pthread_create (&thread, NULL, playringthread, (void *) ringtone);
	pthread_detach(thread) ;

	return 0;
}
extern int AlsaIsOpened ;

int playringthread(void *arg)
{
	int times= 0 ;
	int err;
	int playstatus;
	char *filename;
	snd_pcm_info_t *info;

	// fix 30 June 2009
	filename = (unsigned char *)malloc(strlen((char*)arg));
	if (filename ==NULL) goto _end;

	strcpy(filename,(char*)arg);
	quiet_mode  =0;
_playagain:
	set_audio_mode();
	snd_pcm_info_alloca(&info);
 	 
  	chunk_size = -1;
	rhwparams.format = DEFAULT_FORMAT;
	rhwparams.rate = DEFAULT_SPEED;
	rhwparams.channels = 1;
again:
	err = snd_pcm_open(&handle, "hw", SND_PCM_STREAM_PLAYBACK, 0);
	if (err < 0) {
		if (++times<5) {usleep(20000);goto again;}
 		DPRINT( "[%s] Couldn't open a playback stream \n", __FUNCTION__);	 
 		goto _end;
	}

	if ((err = snd_pcm_info(handle, info)) < 0) {
 
		DPRINT("[%s] info error: %s \n", __FUNCTION__, snd_strerror(err));
		goto _end;
	}

	chunk_size = 1024;
	hwparams = rhwparams;

	audiobuf = (unsigned char *)malloc(1024);
	if (audiobuf == NULL) {
		DPRINT( "[%s] Not enough memory \n", __FUNCTION__);	 
 		goto _end;
	}
  	signal(SIGINT, signal_handler);
	signal(SIGTERM, signal_handler);
	signal(SIGABRT, signal_handler);
 
 	playstatus = playback(filename);
  
	snd_pcm_close(handle);
	handle=NULL;
 	free(audiobuf);
   	snd_config_update_free_global();
  
	restore_audio_mode();
 
  	if (!playstatus && !quiet_mode) { goto _playagain;}
  
_end:
	if (filename) //Fix 30 June 2009
		free(filename);
  	threadintrunning = 0;
  	return EXIT_SUCCESS;
}

/*
 * Safe read (for pipes)
 */
 
ssize_t safe_read(int fd, void *buf, size_t count)
{
	ssize_t result = 0, res;

	while (count > 0) {
		if ((res = read(fd, buf, count)) == 0)
			break;
		if (res < 0)
			return result > 0 ? result : res;
		count -= res;
		result += res;
		buf = (char *)buf + res;
	}
	return result;
}
 
/*
 * helper for test_wavefile
 */

size_t test_wavefile_read(int fd, unsigned char *buffer, size_t *size, size_t reqsize, int line)
{
	if (*size >= reqsize)
		return *size;
 	if ((size_t)safe_read(fd, buffer + *size, reqsize - *size) != reqsize - *size) {
 		DPRINT( "[%s] read error called from line %i \n", __FUNCTION__, line);	 
 
 		return 0;
	}
 
	return *size = reqsize;
}

#define check_wavefile_space(buffer, len, blimit) \
	if (len > blimit) { \
		blimit = len; \
		if ((buffer = realloc(buffer, blimit)) == NULL) { \
			DPRINT( "[%s] not enough memory \n", __FUNCTION__);	  \
			return -1; \
		} \
	}

/*
 * test, if it's a .WAV file, > 0 if ok (and set the speed, stereo etc.)
 *                            == 0 if not
 * Value returned is bytes to be discarded.
 */
static ssize_t test_wavefile(int fd, unsigned char *_buffer, size_t size)
{
	WaveHeader *h = (WaveHeader *)_buffer;
	unsigned char *buffer = NULL;
	size_t blimit = 0;
	WaveFmtBody *f;
	WaveChunkHeader *c;
	unsigned int type, len;
 
	if (size < sizeof(WaveHeader))
		return -1;
	if (h->magic != WAV_RIFF || h->type != WAV_WAVE)
		return -1;
	if (size > sizeof(WaveHeader)) {
		check_wavefile_space(buffer, size - sizeof(WaveHeader), blimit);
		memcpy(buffer, _buffer + sizeof(WaveHeader), size - sizeof(WaveHeader));
	}
	size -= sizeof(WaveHeader);
	while (1) {
		check_wavefile_space(buffer, sizeof(WaveChunkHeader), blimit);
		if (!test_wavefile_read(fd, buffer, &size, sizeof(WaveChunkHeader), __LINE__)) return 0;

		c = (WaveChunkHeader*)buffer;
		type = c->type;
		len = LE_INT(c->length);
		len += len % 2;
		if (size > sizeof(WaveChunkHeader))
			memmove(buffer, buffer + sizeof(WaveChunkHeader), size - sizeof(WaveChunkHeader));
		size -= sizeof(WaveChunkHeader);
		if (type == WAV_FMT)
			break;
		check_wavefile_space(buffer, len, blimit);
		test_wavefile_read(fd, buffer, &size, len, __LINE__);
		if (size > len)
			memmove(buffer, buffer + len, size - len);
		size -= len;
	}

	if (len < sizeof(WaveFmtBody)) {
		DPRINT("[%s]unknown length of 'fmt ' chunk (read %u, should be %u at least)  \n" ,__FUNCTION__,
		      len, (unsigned int)sizeof(WaveFmtBody));
 		return 0;
	}
  	check_wavefile_space(buffer, len, blimit);
	test_wavefile_read(fd, buffer, &size, len, __LINE__);
	f = (WaveFmtBody*) buffer;
	if (LE_SHORT(f->format) != WAV_PCM_CODE) {
   		DPRINT( "[%s] can't play not PCM-coded WAVE-files  \n", __FUNCTION__);	
 		return -1;
	}
	if (LE_SHORT(f->modus) < 1) {
    		DPRINT( "[%s] can't play WAVE-files with %d tracks  \n", __FUNCTION__);	
 		return -1;
	}
	hwparams.channels = LE_SHORT(f->modus);
	 
	switch (LE_SHORT(f->bit_p_spl)) {
	case 8:
 		if (hwparams.format != DEFAULT_FORMAT &&
		    hwparams.format != SND_PCM_FORMAT_U8)
			fprintf(stderr, _("Warning: format is changed to U8\n"));
		hwparams.format = SND_PCM_FORMAT_U8;
 
		break;
	case 16:
 		if (hwparams.format != DEFAULT_FORMAT &&
		    hwparams.format != SND_PCM_FORMAT_S16_LE)
			fprintf(stderr, _("Warning: format is changed to S16_LE\n"));
		hwparams.format = SND_PCM_FORMAT_S16_LE;
 
		break;
	case 24:
 
		switch (LE_SHORT(f->byte_p_spl) / hwparams.channels) {
		case 3:
			if (hwparams.format != DEFAULT_FORMAT &&
			    hwparams.format != SND_PCM_FORMAT_S24_3LE)
				fprintf(stderr, _("Warning: format is changed to S24_3LE\n"));
			hwparams.format = SND_PCM_FORMAT_S24_3LE;
			break;
		case 4:
			if (hwparams.format != DEFAULT_FORMAT &&
			    hwparams.format != SND_PCM_FORMAT_S24_LE)
				fprintf(stderr, _("Warning: format is changed to S24_LE\n"));
			hwparams.format = SND_PCM_FORMAT_S24_LE;
			break;
		default:
		 
			DPRINT("[%s] can't play WAVE-files with sample %d bits in %d bytes wide (%d channels) \n" ,__FUNCTION__,
			      LE_SHORT(f->bit_p_spl), LE_SHORT(f->byte_p_spl), hwparams.channels);
			return -1;
		}
		break;
 
	case 32:
		hwparams.format = SND_PCM_FORMAT_S32_LE;
		break;
	default:
		PRINT("[%s] can't play WAVE-files with sample %d bits wide \n" ,__FUNCTION__,
		      LE_SHORT(f->bit_p_spl));
		return  -1;
	}
	hwparams.rate = LE_INT(f->sample_fq);
	
	if (size > len)
		memmove(buffer, buffer + len, size - len);
	size -= len;
	
	while (1) {
		unsigned int type, len;

		check_wavefile_space(buffer, sizeof(WaveChunkHeader), blimit);
		test_wavefile_read(fd, buffer, &size, sizeof(WaveChunkHeader), __LINE__);
		c = (WaveChunkHeader*)buffer;
		type = c->type;
		len = LE_INT(c->length);
		if (size > sizeof(WaveChunkHeader))
			memmove(buffer, buffer + sizeof(WaveChunkHeader), size - sizeof(WaveChunkHeader));
		size -= sizeof(WaveChunkHeader);
		if (type == WAV_DATA) {
			if (len < pbrec_count && len < 0x7ffffffe)
				pbrec_count = len;
			if (size > 0)
				memcpy(_buffer, buffer, size);
			free(buffer);
			return size;
		}
		len += len % 2;
		check_wavefile_space(buffer, len, blimit);
		test_wavefile_read(fd, buffer, &size, len, __LINE__);
		if (size > len)
			memmove(buffer, buffer + len, size - len);
		size -= len;
	}

	/* shouldn't be reached */
	return -1;
}

  
static int set_params(void)
{
	snd_pcm_hw_params_t *params;
	snd_pcm_sw_params_t *swparams;
	snd_pcm_uframes_t buffer_size;
	int err;
	size_t n;
	snd_pcm_uframes_t xfer_align;
	unsigned int rate;
	snd_pcm_uframes_t start_threshold, stop_threshold;

	snd_pcm_hw_params_alloca(&params);
	snd_pcm_sw_params_alloca(&swparams);
	err = snd_pcm_hw_params_any(handle, params);
	if (err < 0) {
	 	DPRINT( "[%s] Broken configuration for this PCM: no configurations available\n", __FUNCTION__);	  
 		return -1;
	}

	err = snd_pcm_hw_params_set_access(handle, params,
						   SND_PCM_ACCESS_RW_INTERLEAVED);

	if (err < 0) {
 	 	DPRINT( "[%s] Access type not available \n", __FUNCTION__);	  
		return -1;
	}
	err = snd_pcm_hw_params_set_format(handle, params, hwparams.format);
	if (err < 0) {
 	 	DPRINT( "[%s] Sample format non available \n", __FUNCTION__);	  
 		return -1;
	}
	err = snd_pcm_hw_params_set_channels(handle, params, hwparams.channels);
	if (err < 0) {
	 	DPRINT( "[%s] Channels count non available\n", __FUNCTION__);	  
 		return -1;
	}

#if 0
	err = snd_pcm_hw_params_set_periods_min(handle, params, 2);
	assert(err >= 0);
#endif

	rate = hwparams.rate;
	err = snd_pcm_hw_params_set_rate_near(handle, params, &hwparams.rate, 0);
	assert(err >= 0);
	if ((float)rate * 1.05 < hwparams.rate || (float)rate * 0.95 > hwparams.rate) {
		if (!quiet_mode) {
			char plugex[64];
			const char *pcmname = snd_pcm_name(handle);
			fprintf(stderr, _("Warning: rate is not accurate (requested = %iHz, got = %iHz)\n"), rate, hwparams.rate);
			if (! pcmname || strchr(snd_pcm_name(handle), ':'))
				*plugex = 0;
			else
				snprintf(plugex, sizeof(plugex), "(-Dplug:%s)",
					 snd_pcm_name(handle));
			fprintf(stderr, _("         please, try the plug plugin %s\n"),
				plugex);
		}
	}
	rate = hwparams.rate;
	if (buffer_time == 0 && buffer_frames == 0) {
		err = snd_pcm_hw_params_get_buffer_time_max(params,
							    &buffer_time, 0);
		assert(err >= 0);
		if (buffer_time > 500000)
			buffer_time = 500000;
	}
	if (period_time == 0 && period_frames == 0) {
		if (buffer_time > 0)
			period_time = buffer_time / 4;
		else
			period_frames = buffer_frames / 4;
	}
	if (period_time > 0)
		err = snd_pcm_hw_params_set_period_time_near(handle, params,
							     &period_time, 0);
	else
		err = snd_pcm_hw_params_set_period_size_near(handle, params,
							     &period_frames, 0);
	assert(err >= 0);
	if (buffer_time > 0) {
		err = snd_pcm_hw_params_set_buffer_time_near(handle, params,
							     &buffer_time, 0);
	} else {
		err = snd_pcm_hw_params_set_buffer_size_near(handle, params,
							     &buffer_frames);
	}
	assert(err >= 0);
	err = snd_pcm_hw_params(handle, params);
	if (err < 0) {
 	 	DPRINT( "[%s] Unable to install hw params \n", __FUNCTION__);	  
  		return -1;
	}
	snd_pcm_hw_params_get_period_size(params, &chunk_size, 0);
	snd_pcm_hw_params_get_buffer_size(params, &buffer_size);

	if (chunk_size == buffer_size) {
		DPRINT("[%s] Can't use period equal to buffer size (%lu == %lu) \n" ,__FUNCTION__, 
		      chunk_size, buffer_size);
		return -1;
	}
	snd_pcm_sw_params_current(handle, swparams);
	err = snd_pcm_sw_params_get_xfer_align(swparams, &xfer_align);
	if (err < 0) {
	 	DPRINT( "[%s] Unable to obtain xfer align \n", __FUNCTION__);	  
 		return -1;
	}
	if (sleep_min)
		xfer_align = 1;
	err = snd_pcm_sw_params_set_sleep_min(handle, swparams,
					      sleep_min);
	assert(err >= 0);
	if (avail_min < 0)
		n = chunk_size;
	else
		n = (double) rate * avail_min / 1000000;

	err = snd_pcm_sw_params_set_avail_min(handle, swparams, n);

	/* round up to closest transfer boundary */
	n = (buffer_size / xfer_align) * xfer_align;
	if (start_delay <= 0) {
		start_threshold = n + (double) rate * start_delay / 1000000;
	} else
		start_threshold = (double) rate * start_delay / 1000000;
	if (start_threshold < 1)
		start_threshold = 1;
	if (start_threshold > n)
		start_threshold = n;
	err = snd_pcm_sw_params_set_start_threshold(handle, swparams, start_threshold);
	assert(err >= 0);
	if (stop_delay <= 0) 
		stop_threshold = buffer_size + (double) rate * stop_delay / 1000000;
	else
		stop_threshold = (double) rate * stop_delay / 1000000;
	err = snd_pcm_sw_params_set_stop_threshold(handle, swparams, stop_threshold);
	assert(err >= 0);

	err = snd_pcm_sw_params_set_xfer_align(handle, swparams, xfer_align);
	assert(err >= 0);

	if (snd_pcm_sw_params(handle, swparams) < 0) {
 	 	DPRINT( "[%s] unable to install sw params \n", __FUNCTION__);	  
  		return -1;
	}
/*
	if (verbose) {
		snd_pcm_hw_params_dump(params, log);
		snd_pcm_sw_params_dump(swparams, log);
		//snd_pcm_dump(handle, log);
	}
*/
	bits_per_sample = snd_pcm_format_physical_width(hwparams.format);
	bits_per_frame = bits_per_sample * hwparams.channels;
	chunk_bytes = chunk_size * bits_per_frame / 8;
	audiobuf = realloc(audiobuf, chunk_bytes);
	if (audiobuf == NULL) {
 	 	DPRINT( "[%s] not enough memory \n", __FUNCTION__);	  
 		return -1;
	}
	// fprintf(stderr, "real chunk_size = %i, frags = %i, total = %i\n", chunk_size, setup.buf.block.frags, setup.buf.block.frags * chunk_size);
	return 0;
}

#ifndef timersub
#define	timersub(a, b, result) \
do { \
	(result)->tv_sec = (a)->tv_sec - (b)->tv_sec; \
	(result)->tv_usec = (a)->tv_usec - (b)->tv_usec; \
	if ((result)->tv_usec < 0) { \
		--(result)->tv_sec; \
		(result)->tv_usec += 1000000; \
	} \
} while (0)
#endif
 
/* I/O error handler */
static int xrun(void)
{
	snd_pcm_status_t *status;
	int res;
	
	snd_pcm_status_alloca(&status);
	if ((res = snd_pcm_status(handle, status))<0) {
		DPRINT("[%s] status error: %s \n", __FUNCTION__, snd_strerror(res)) ;

		return -1;
	}
	if (snd_pcm_status_get_state(status) == SND_PCM_STATE_XRUN) {
		struct timeval now, diff, tstamp;
		gettimeofday(&now, 0);
		snd_pcm_status_get_trigger_tstamp(status, &tstamp);
		timersub(&now, &tstamp, &diff);
		//fprintf(stderr, _("%s!!! (at least %.3f ms long)\n"),
		//	stream == SND_PCM_STREAM_PLAYBACK ? _("underrun") : _("overrun"),
 		//	diff.tv_sec * 1000 + diff.tv_usec / 1000.0);
//		if (verbose) {
//			fprintf(stderr, _("Status:\n"));
//			snd_pcm_status_dump(status, log);
//		}
		if ((res = snd_pcm_prepare(handle))<0) {
			DPRINT("[%s] xrun: prepare error: %s \n" , __FUNCTION__, snd_strerror(res));
			return -1;
		}
		return 0;		/* ok, data should be accepted again */
	} if (snd_pcm_status_get_state(status) == SND_PCM_STATE_DRAINING) {
			;
//		if (verbose) {
//			fprintf(stderr, _("Status(DRAINING):\n"));
//			snd_pcm_status_dump(status, log);
//		}
	}
//	if (verbose) {
//		fprintf(stderr, _("Status(R/W):\n"));
//		snd_pcm_status_dump(status, log);
//	}
	DPRINT("[%s] read/write error, state = %s \n" , __FUNCTION__, snd_pcm_state_name(snd_pcm_status_get_state(status)));
	return -1;
}

/* I/O suspend handler */
static int suspend(void)
{
	int res;

	if (!quiet_mode)
		fprintf(stderr, _("Suspended. Trying resume. ")); fflush(stderr);

	while ((res = snd_pcm_resume(handle)) == -EAGAIN)
		sleep(1) ;	/* wait until suspend flag is released */

	if (res < 0) {
		 if (!quiet_mode)
		 	fprintf(stderr, _("Failed. Restarting stream. ")); 
		fflush(stderr);

		if ((res = snd_pcm_prepare(handle)) < 0) {
			DPRINT("[%s] suspend: prepare error: %s \n" , __FUNCTION__, snd_strerror(res));
			return -1;
		}
	}
	 return 0;
}


/*
 *  write function
 */
static ssize_t pcm_write(unsigned char *data, size_t count)
{
	ssize_t r;
	ssize_t result = 0;

	if (sleep_min == 0 &&
	    count < chunk_size) {
		snd_pcm_format_set_silence(hwparams.format, data + count * bits_per_frame / 8, (chunk_size - count) * hwparams.channels);
		count = chunk_size;
	}
	while (count > 0) {
		r = snd_pcm_writei(handle, data, count);

		if (r == -EAGAIN || (r >= 0 && (size_t)r < count)) {
 			snd_pcm_wait(handle, 1000);
		} else if (r == -EPIPE) {
 			if (xrun()<-1){return -1;}
		} else if (r == -ESTRPIPE) {
 			if (suspend()<0){return -1;}
		} else if (r < 0) {
 			DPRINT("[%s] write error: %s \n" , __FUNCTION__, snd_strerror(r));
			return -1;
		} 
		if (r > 0) {
//			if (verbose > 1)
//				compute_max_peak(data, r * hwparams.channels);
			result += r;
			count -= r;
			data += r * bits_per_frame / 8;
		}
	}
	return result;
}

/*
 *  read function
 */

    

 
/* calculate the data count to read from/to dsp */
static unsigned long calc_count(void)
{
	unsigned long count;

	if (timelimit == 0) {
		count = pbrec_count;
	} else {
		count = snd_pcm_format_size(hwparams.format, hwparams.rate * hwparams.channels);
		count *= (unsigned long)timelimit;
	}
	return count < pbrec_count ? count : pbrec_count;
}

 
  
/* playing raw data */
static int playback_go(int fd, size_t loaded, unsigned long count, int rtype, char *name)
{
	int l, r;
	unsigned long written = 0;
	unsigned long c;
	int ret=0;
	//header(rtype, name);
	set_params();

	while (loaded > chunk_bytes && written < count) {
		if (pcm_write(audiobuf + written, chunk_size) <= 0)
			return -1;
		written += chunk_bytes;
		loaded -= chunk_bytes;
	}
	if (written > 0 && loaded > 0)
		memmove(audiobuf, audiobuf + written, loaded);

	l = loaded;
	while (written < count) {
		do {
			c = count - written;
			if (c > chunk_bytes)
				c = chunk_bytes;
			c -= l;

			if (c == 0)
				break;
			r = safe_read(fd, audiobuf + l, c);
			if (r < 0) {
				perror(name);
				return -1;
			}
			fdcount += r;
			if (r == 0)
				break;
			l += r;
			
		} while (sleep_min == 0 && (size_t)l < chunk_bytes);
		l = l * 8 / bits_per_frame;
		if (quiet_mode) {ret=1;goto _end;}
 
		r = pcm_write(audiobuf, l);
 
		if (r != l)	break;
		r = r * bits_per_frame / 8;
		written += r;
		l = 0;
	}
_end:
  		memset( audiobuf, 0, chunk_bytes ) ;
        pcm_write( audiobuf, 1) ;
        usleep( 100000 ) ;
		return ret;
 	//snd_pcm_drain(handle);
}

/*
 *  let's play or capture it (capture_type says VOC/WAVE/raw)
 */

static int playback(char *name)
{
 	size_t dta;
	ssize_t dtawave;
	int ret=-1;
	pbrec_count = LLONG_MAX;
	fdcount = 0;
	if (!name) return -1;
 
	if ((fd = open(name, O_RDONLY, 0)) == -1) 
	{
 
 	  	DPRINT("[%s] error opening playback file  = %s \n\n\n", __FUNCTION__,name);
		perror(name);
		return -1;
	}
  
	/* read the file header */
	dta = sizeof(AuHeader);

	if ((size_t)safe_read(fd, audiobuf, dta) != dta) {
		DPRINT("[%s] read error \n", __FUNCTION__) ;
		return -1;
	}
 
	dta = sizeof(VocHeader);
	if ((size_t)safe_read(fd, audiobuf + sizeof(AuHeader),
		 dta - sizeof(AuHeader)) != dta - sizeof(AuHeader)) {
		DPRINT("[%s] read error \n", __FUNCTION__) ;
		return -1;
	}
 
	/* read bytes for WAVE-header */
	if ((dtawave = test_wavefile(fd, audiobuf, dta)) >= 0) {
		pbrec_count = calc_count();
 		ret = playback_go(fd, dtawave, pbrec_count, FORMAT_WAVE, name);
	}  
        
	if (fd != 0) close(fd);
 
	return ret;
}
#include "sc1445x_alsa_ioctl.h"
 
static int set_audio_mode(void )
{
	 int res = 0 ;
	 sc1445x_audio_mode_t am ;

	 am.mode = 3 ;
	 res = snd_hwdep_open( &hwdephandle, "hw:0,0", O_RDWR ) ;
	 if( res < 0 )   return -1;


	 res = snd_hwdep_ioctl( hwdephandle, SNDRV_SC1445x_SET_MODE, &am ) ;
	 if( res < 0 ) {
	  fprintf( stderr, "could not set mode to %d: %s\n",
		  am.mode, snd_strerror( res ) ) ;
	 }
	 res = snd_hwdep_close( hwdephandle ) ;
	 
	 return res ;
}
static int restore_audio_mode(void )
{
 int res = 0 ;
 sc1445x_audio_mode_t am ;

 am.mode = 0 ;
 res = snd_hwdep_open( &hwdephandle, "hw:0,0", O_RDWR ) ;
 if( res < 0 )   return -1;


 res = snd_hwdep_ioctl( hwdephandle, SNDRV_SC1445x_SET_MODE, &am ) ;
 if( res < 0 ) {
  fprintf( stderr, "could not set mode to %d: %s\n",
      am.mode, snd_strerror( res ) ) ;
 }
 res = snd_hwdep_close( hwdephandle ) ;
 
 return res ;
}

  /*
static void header(int rtype, char *name)
{
	if (!quiet_mode) {
		if (! name)
			name = (stream == SND_PCM_STREAM_PLAYBACK) ? "stdout" : "stdin";
		fprintf(stderr, "%s %s '%s' : ",
			(stream == SND_PCM_STREAM_PLAYBACK) ? _("Playing") : _("Recording"),
			gettext(N_("WAVE")),
			name);
		fprintf(stderr, "%s, ", snd_pcm_format_description(hwparams.format));
		fprintf(stderr, _("Rate %d Hz, "), hwparams.rate);
		if (hwparams.channels == 1)
			fprintf(stderr, _("Mono"));
		else if (hwparams.channels == 2)
			fprintf(stderr, _("Stereo"));
		else
			fprintf(stderr, _("Channels %i"), hwparams.channels);
		fprintf(stderr, "\n");
	}
}*/

/* peak handler */
/*
static void compute_max_peak(unsigned char *data, size_t count)
{
	signed int val, max, max_peak = 0, perc;
	static	int	run = 0;
	size_t ocount = count;
	int	format_little_endian = snd_pcm_format_little_endian(hwparams.format);	

	switch (bits_per_sample) {
	case 8: {
		signed char *valp = (signed char *)data;
		signed char mask = snd_pcm_format_silence(hwparams.format);
		while (count-- > 0) {
			val = *valp++ ^ mask;
			val = abs(val);
			if (max_peak < val)
				max_peak = val;
		}
		break;
	}
	case 16: {
		signed short *valp = (signed short *)data;
		signed short mask = snd_pcm_format_silence_16(hwparams.format);
		signed short sval;

		count /= 2;
		while (count-- > 0) {
			if (format_little_endian)
				sval = __le16_to_cpu(*valp);
			else	sval = __be16_to_cpu(*valp);
			sval = abs(sval) ^ mask;
			if (max_peak < sval)
				max_peak = sval;
			valp++;
		}
		break;
	}
	case 24: {
		unsigned char *valp = data;
		signed int mask = snd_pcm_format_silence_32(hwparams.format);

		count /= 3;
		while (count-- > 0) {
			if (format_little_endian) {
				val = valp[0] | (valp[1]<<8) | (valp[2]<<16);
			} else {
				val = (valp[0]<<16) | (valp[1]<<8) | valp[2];
			}
			// Correct signed bit in 32-bit value 
			if (val & (1<<(bits_per_sample-1))) {
				val |= 0xff<<24;	// Negate upper bits too  
			}
			val = abs(val) ^ mask;
			if (max_peak < val)
				max_peak = val;
			valp += 3;
		}
		break;
	}
	case 32: {
		signed int *valp = (signed int *)data;
		signed int mask = snd_pcm_format_silence_32(hwparams.format);
		count /= 4;
		while (count-- > 0) {
			if (format_little_endian)
				val = __le32_to_cpu(*valp);
			else	val = __be32_to_cpu(*valp);
			val = abs(val) ^ mask;
			if (max_peak < val)
				max_peak = val;
			valp++;
		}
		break;
	}
	default:
		if (run == 0) {
			fprintf(stderr, _("Unsupported bit size %d.\n"), bits_per_sample);
			run = 1;
		}
		return;
	}
	max = 1 << (bits_per_sample-1);
	if (max <= 0)
		max = 0x7fffffff;

	if (bits_per_sample > 16)
		perc = max_peak / (max / 100);
	else
		perc = max_peak * 100 / max;

	if(verbose<=2) {
		static int maxperc=0;
		static time_t t=0;
		const time_t tt=time(NULL);
		if(tt>t) {
			t=tt;
			maxperc=0;
		}
		if(perc>maxperc)
			maxperc=perc;

		putchar('\r');
		for (val = 0; val <= perc / 2 && val < 50; val++)
			putchar('#');
		for (; val < maxperc / 2 && val < 50; val++)
			putchar(' ');
		putchar('+');
		for (++val; val < 50; val++)
			putchar(' ');

		printf("| %02i%%", maxperc);
		if (perc>99)
			printf(_(" !clip  "));

			fflush(stdout);
	}
	else if(verbose==3) {
		printf(_("Max peak (%li samples): 0x%08x "), (long)ocount, max_peak);
		for (val = 0; val < 20; val++)
			if (val <= perc / 5)
				putchar('#');
			else
				putchar(' ');
		printf(" %i%%\n", perc);
	}
}
*/
