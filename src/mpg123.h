#ifndef MPG123_LIB_H
#define MPG123_LIB_H

#ifdef BUILD_MPG123_DLL
/* The dll exports. */
#define EXPORT __declspec(dllexport)
#else
#ifdef LINK_MPG123_DLL
/* The exe imports. */
#define EXPORT __declspec(dllimport)
#else
/* Nothing on normal/UNIX builds */
#define EXPORT
#endif
#endif

#include <stdlib.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/* not decided... how anonymous should the handle be? */
struct mpg123_handle_struct;
struct mpg123_pars_struct;
typedef struct mpg123_handle_struct mpg123_handle;
typedef struct mpg123_pars_struct   mpg123_pars;

/* non-threadsafe init/exit, call _once_ */
EXPORT int  mpg123_init(void);
EXPORT void mpg123_exit(void);
/* Create a handle with optional choice of decoder (named by a string).
   and optional retrieval of an error code to feed to mpg123_plain_strerror().
   Optional means: Any of or both the parameters may be NULL.
   The handle creation is successful when a non-NULL pointer is returned. */
EXPORT mpg123_handle *mpg123_new(const char* decoder, int *error);
/* Create a handle with preset parameters. */
EXPORT mpg123_handle *mpg123_parnew(mpg123_pars *mp, const char* decoder, int *error);
/* Delete handle, mh is either a valid mpg123 handle or NULL. */
EXPORT void mpg123_delete(mpg123_handle *mh);

/* Return NULL-terminated array of generally available decoder names... */
EXPORT char **mpg123_decoders();
/* ...or just the actually supported (by CPU) decoders. */
EXPORT char **mpg123_supported_decoders();
EXPORT int mpg123_decoder(mpg123_handle *mh, const char* decoder);

enum mpg123_errors
{
	MPG123_OK=0, MPG123_BAD_OUTFORMAT, MPG123_BAD_CHANNEL, MPG123_BAD_RATE,
	MPG123_ERR_16TO8TABLE, MPG123_BAD_PARAM, MPG123_BAD_BUFFER,
	MPG123_OUT_OF_MEM, MPG123_NOT_INITIALIZED, MPG123_BAD_DECODER, MPG123_BAD_HANDLE,
	MPG123_NO_BUFFERS, MPG123_BAD_RVA, MPG123_NO_GAPLESS, MPG123_NO_SPACE,
	MPG123_BAD_TYPES, MPG123_BAD_BAND, MPG123_ERR_NULL, MPG123_ERR_READER,
	MPG123_NO_SEEK_FROM_END, MPG123_BAD_WHENCE
};
/* Give string describing that error errcode means. */
EXPORT const char* mpg123_plain_strerror(int errcode);
/* Give string describing what error has occured in the context of handle mh.
   When a function operating on an mpg123 handle returns MPG123_ERR, you should check for the actual reason via
   char *errmsg = mpg123_strerror(mh)
   This function will catch mh == NULL and return the message for MPG123_BAD_HANDLE. */
EXPORT const char* mpg123_strerror(mpg123_handle *mh);
/* Return the plain errcode intead of a string. */
EXPORT int         mpg123_errcode(mpg123_handle *mh);

/* 16 or 8 bits, signed or unsigned... all flags fit into 8 bits, float/double are not yet standard and special anyway */
#define MPG123_ENC_16     0x40 /* 0100 0000 */
#define MPG123_ENC_SIGNED 0x80 /* 1000 0000 */
#define MPG123_ENC_8(f)   (!((f) & MPG123_ENC_16)) /* it's 8bit encoding of not 16bit, this changes in case float output will be integrated in the normal library */

#define MPG123_ENC_SIGNED_16    (MPG123_ENC_16|MPG123_ENC_SIGNED|0x10) /* 1101 0000 */
#define MPG123_ENC_UNSIGNED_16  (MPG123_ENC_16|0x20)                   /* 0110 0000 */
#define MPG123_ENC_UNSIGNED_8   0x01                                   /* 0000 0001 */
#define MPG123_ENC_SIGNED_8     (MPG123_ENC_SIGNED|0x02)               /* 1000 0010 */
#define MPG123_ENC_ULAW_8       0x04                                   /* 0000 0100 */
#define MPG123_ENC_ALAW_8       0x08                                   /* 0000 1000 */
#define MPG123_ENC_ANY ( MPG123_ENC_SIGNED_16  | MPG123_ENC_UNSIGNED_16 | \
                         MPG123_ENC_UNSIGNED_8 | MPG123_ENC_SIGNED_8    | \
                         MPG123_ENC_ULAW_8 | MPG123_ENC_ALAW_8 | MPG123_ENC_ANY )

/* They can be combined into one number to indicate mono and stereo... */
#define MPG123_MONO   1
#define MPG123_STEREO 2

/* 8000, 11025, 12000, 16000, 22050, 24000, 32000, 44100, 48000 or _one_ custom rate <=96000 */
#define MPG123_RATES     9 /* A future library version may not have less! */
EXPORT extern const long mpg123_rates[MPG123_RATES];
#define MPG123_ENCODINGS 6 /* A future library version may not have less! */
EXPORT extern const int  mpg123_encodings[MPG123_ENCODINGS];

/* Accept no output format at all, use before specifying supported formats with mpg123_format */
EXPORT int mpg123_format_none(mpg123_handle *mh);
/* Accept all formats (also any custom rate you may set) -- this is default. */
EXPORT int mpg123_format_all(mpg123_handle *mh);
/*
	Setting audio format support in detail:
	rateindex: Index in rates list...
	Negative rate index chooses the custom one.
	channels: combination of MPG123_STEREO and MPG123_MONO
	encodings: combination of accepted encodings for rate and channels, p.ex MPG123_ENC_SIGNED16|MPG123_ENC_ULAW_8
*/
EXPORT int mpg123_format(mpg123_handle *mh, int rateindex, int channels, int encodings); /* 0 is good, -1 is error */
/* Check if a specific format at a specific rate is supported.
   Returns 0 for no support (includes invalid parameters), MPG123_STEREO, MPG123_MONO or MPG123_STEREO|MPG123_MONO. */
EXPORT int mpg123_format_support(mpg123_handle *mh, int ratei, int enci); /* Indices of rate and encoding! */
/* Get the current output format. */
EXPORT int mpg123_getformat(mpg123_handle *mh, long *rate, int *channels, int *encoding);

/* various flags */
#define MPG123_FORCE_MONO   0x7  /*     0111 */
#define MPG123_MONO_LEFT    0x1  /*     0001 */
#define MPG123_MONO_RIGHT   0x2  /*     0010 */
#define MPG123_MONO_MIX     0x4  /*     0100 */
#define MPG123_FORCE_STEREO 0x8  /*     1000 */
#define MPG123_FORCE_8BIT   0x10 /* 00010000 */
#define MPG123_QUIET        0x20 /* 00100000 suppress any printouts (overrules verbose) */
#define MPG123_GAPLESS      0x40 /* 01000000 flag always defined... */
#define MPG123_NO_RESYNC    0x80 /* 10000000 disable resync stream after error */
/* RVA choices */
#define MPG123_RVA_OFF   0
#define MPG123_RVA_MIX   1
#define MPG123_RVA_ALBUM 2
#define MPG123_RVA_MAX   MPG123_RVA_ALBUM
enum mpg123_parms
{
	MPG123_VERBOSE,        /* set verbosity value for enabling messages to stderr, >= 0 makes sense */
	MPG123_FLAGS,          /* set all flags, p.ex val = MPG123_GAPLESS|MPG123_MONO_MIX */
	MPG123_ADD_FLAGS,      /* add some flags */
	MPG123_FORCE_RATE,     /* when value > 0, force output rate to that value */
	MPG123_DOWN_SAMPLE,    /* 0=native rate, 1=half rate, 2=quarter rate */
	MPG123_RVA,            /* one of the RVA choices above */
	MPG123_DOWNSPEED,      /* play a frame <n> times */
	MPG123_UPSPEED,        /* play every <n>th frame */
	MPG123_START_FRAME,    /* start with this frame (skip frames before that) */ 
	MPG123_DECODE_FRAMES,  /* decode only this number of frames */
	MPG123_ICY_INTERVAL,   /* stream contains ICY metadata with this interval */
	MPG123_OUTSCALE        /* the scale for output samples (amplitude) */
};
/* This sets, for a specific handle, a specific parameter (key chosen from the above list), to the specified value.
   TODO: Assess the possibilities and troubles of changing parameters during playback. */
EXPORT int mpg123_param   (mpg123_handle *mh, int key, long value, double fvalue);
EXPORT int mpg123_getparam(mpg123_handle *mh, int key, long *val,  double *fval);
/* Direct access to a parameter set without full handle around it. */
EXPORT mpg123_pars *mpg123_new_pars(int *error);
EXPORT void         mpg123_delete_pars(mpg123_pars* mp);
EXPORT int mpg123_par   (mpg123_pars *mp, int key, long value, double fvalue);
EXPORT int mpg123_getpar(mpg123_pars *mp, int key, long *val, double *fval);


#define MPG123_LEFT  1
#define MPG123_RIGHT 2
/* Channel can be MPG123_LEFT, MPG123_RIGHT or MPG123_LEFT|MPG123_RIGHT for both.
   Band is an eq band from 0 to 31, val the (linear) factor. */
EXPORT int mpg123_eq(mpg123_handle *mh, int channel, int band, double val);
EXPORT int mpg123_reset_eq(mpg123_handle *mh); /* all back to 1 */

/* Change output volume including the RVA setting, vol<0 just applies (a possibly changed) RVA setting. */
EXPORT int mpg123_volume(mpg123_handle *mh, double vol);
EXPORT int mpg123_volume_change(mpg123_handle *mh, double change);
/* Return current volume setting, the actual value due to RVA, the RVA adjustment itself.
   It's all as double float value to abstract the sample format.
   Oh, and the volume values are linear factors / amplitudes  (not percent) and the RVA value is in decibel. */
EXPORT int mpg123_getvolume(mpg123_handle *mh, double *base, double *really, double *rva_db);

/* The current position in samples. One the next read, you'd get that sample. */
EXPORT off_t mpg123_tell(mpg123_handle *mh);
/* The next read will give you data from this frame. */
EXPORT off_t mpg123_tellframe(mpg123_handle *mh);
/* If possible, tell the full (expected) length of current track in samples. */
EXPORT off_t mpg123_length(mpg123_handle *mh);
/* Info about current and remaining frames/seconds.
   You provide an offset (in frames) from now and a number of output bytes served by mpg123 but not yet played.
   You get the projected current frame and seconds, as well as the remaining frames/seconds.
   This does _not_ care about skipped samples due to gapless playback. */
EXPORT int mpg123_position( mpg123_handle *mh, off_t frame_offset, off_t buffered_bytes,
                            off_t *current_frame,     off_t *frames_left,
/*                            off_t *current_samples,   off_t *samples_left ); */
double *current_seconds, double *seconds_left);
/* Time (seconds) per frame; <0 is error. */
EXPORT double mpg123_tpf(mpg123_handle *mh);

/* The open functions reset stuff and make a new, different stream possible - even if there isn't actually a resource involved like with open_feed. */
EXPORT int mpg123_open     (mpg123_handle *mh, char *url); /* a file or http url */
EXPORT int mpg123_open_feed(mpg123_handle *mh);            /* prepare for direct feeding */
EXPORT int mpg123_open_fd  (mpg123_handle *mh, int fd);    /* use an already opened file descriptor */
/* reading samples / triggering decoding, possible return values: */
/* MPG123_OK on success */
#define MPG123_ERR -1 /* in general, functions return that on error */
/* special status valuea */
#define MPG123_NEED_MORE  -10 /* For feed: "Feed me more!" */
#define MPG123_NEW_FORMAT -11 /* Output format will be different on next call. */
#define MPG123_DONE       -12 /* Track ended. */
/* Read from stream and decode up to outmemsize bytes. Returns a code from above and the number of decoded bytes in *done. */
EXPORT ssize_t mpg123_read(mpg123_handle *mh, unsigned char *outmemory, size_t outmemsize, size_t *done);
/* Same as above but with feeding input data (when inmemory != NULL).
   This is very close to a drop-in replacement for old mpglib.
   When you give zero-sized output buffer the input will be parsed until decoded data is available.
   That enables you to get NEW_FORMAT (and query it) without taking decoded data. */
EXPORT int mpg123_decode(mpg123_handle *mh, unsigned char *inmemory, size_t inmemsize, unsigned char *outmemory, size_t outmemsize, size_t *done);
/* Decode only one frame (or read a frame and return after setting a new format), update num to latest decoded frame index. */
EXPORT int mpg123_decode_frame(mpg123_handle *mh, off_t *num, unsigned char **audio, size_t *bytes);

/* Get and reset the clip count. */
EXPORT long mpg123_clip(mpg123_handle *mh);

/* Well, what do you think? Closes the resource, if libmpg123 opened it. */
EXPORT int mpg123_close(mpg123_handle *mh);

/* The seek stuff needs more thought; it's going to be sample-accurate and I need a way for feeding.
   So: SEEK STUFF WILL CHANGE! */

EXPORT off_t mpg123_timeframe(mpg123_handle *mh, double sec);
EXPORT int mpg123_print_index(mpg123_handle *fr, FILE* out);

/*
	Seeking in MPEG files/streams: modelled after the standard fseek (or fseeko).
	- set whence to SEEK_SET, SEEK_CUR or SEEK_END (not guaranteed to work for all streams, of course)
	- returning resulting offset >= 0 or MPG123_ERR (-1)
	mpg123_feedseek() gives also an input data offset that it expects to be present the next time data is fed to mpg123_decode().
	Still wondering: long or off_t ??

	Trying to code it so that no decoding happens during seek (but some pre-decoding may be needed after seek).
	Sample-accurate seek depends on the gapless code being in effect.
	Without that, we only get frame-accurate.
*/
EXPORT off_t mpg123_seek      (mpg123_handle *mh, off_t sampleoff, int whence);
EXPORT off_t mpg123_feedseek  (mpg123_handle *mh, off_t sampleoff, int whence, off_t *input_offset);
/* in/output offset in MPEG frames instead of samples */
EXPORT off_t mpg123_seek_frame(mpg123_handle *mh, off_t frameoff,  int whence);

enum mpg123_vbr  { MPG123_CBR=0, MPG123_VBR, MPG123_ABR };
struct mpg123_frameinfo
{
	enum {MPG123_1_0 = 0, MPG123_2_0, MPG123_2_5 } version;
	int layer; /* Well... 1, 2 or 3  */
	long rate; /* The sampling rate. */
	/* "Stereo", "Joint-Stereo", "Dual-Channel", "Single-Channel" ... so mode != MPG213_M_MONO means two channels. */
	enum { MPG123_M_STEREO=0, MPG123_M_JOINT, MPG123_M_DUAL, MPG123_M_MONO } mode;
	int mode_ext;
	int framesize;
#define MPG123_CRC       1
#define MPG123_COPYRIGHT 2
#define MPG123_PRIVATE   4
#define MPG123_ORIGINAL  8
	int flags;
	int emphasis;
	int bitrate;
	int abr_rate;
	enum mpg123_vbr vbr;
};

EXPORT int mpg123_info(mpg123_handle *mh, struct mpg123_frameinfo *mi);
/* Scan through file (if seekable) or just the first frame (without decoding, for non-seekable) and return various information.
   That could include format, length, padding, ID3, ... */
/* int mpg123_scan(mpg123_handle *mh, struct mpg123_info *mi); */

EXPORT size_t mpg123_safe_buffer(); /* Get the safe output buffer size for all cases (when you want to replace the internal buffer) */
EXPORT size_t mpg123_outblock(mpg123_handle *mh); /* The max size of one frame's decoded output with current settings. */
EXPORT int mpg123_replace_buffer(mpg123_handle *mh, unsigned char *data, size_t size);

/* 128 bytes of ID3v1 - Don't take anything for granted (like string termination)! */
typedef struct
{
	char tag[3];         /* "TAG", the classic intro */
	char title[30];      /* title string  */
	char artist[30];     /* artist string */
	char album[30];      /* album string */
	char year[4];        /* year string */
	char comment[30];    /* comment string */
	unsigned char genre; /* genre code */
} mpg123_id3v1;

/* A safer string, also can hold a number of null-terminated strings. */
typedef struct 
{
	char* p;     /* pointer to the string data */
	size_t size; /* raw number of bytes allocated */
	size_t fill; /* number of used bytes (including closing zero byte) */
} mpg123_string;

/* A little string library, it's not strictly mpeg decoding, but the funcitons are there. */
EXPORT void mpg123_init_string  (mpg123_string* sb);
EXPORT void mpg123_free_string  (mpg123_string* sb);
/* returning 0 on error, 1 on success */
EXPORT int  mpg123_resize_string(mpg123_string* sb, size_t news);
EXPORT int  mpg123_copy_string  (mpg123_string* from, mpg123_string* to);
EXPORT int  mpg123_add_string   (mpg123_string* sb, char* stuff);
EXPORT int  mpg123_set_string   (mpg123_string* sb, char* stuff);

typedef struct
{
	unsigned char version; /* 3 or 4 for ID3v2.3 or ID3v2.4 */
	/* The ID3v2 text frames are allowed to contain multiple strings.
	   So check for null bytes until you reach the mpg123_string fill.
	   All text is encoded in UTF-8 */
	mpg123_string title;
	mpg123_string artist;
	mpg123_string album;
	mpg123_string year;    /* be ready for 20570! */
	mpg123_string comment;
	mpg123_string genre;   /* The genre string(s) may very well need postprocessing, esp. for ID3v2.3 . */
} mpg123_id3v2;

/* Query if there is (new) meta info, be it ID3 or ICY (or something new in future).
   The check function returns a combination of these flags: */
#define MPG123_ID3     0x3 /* 0011 There is some ID3 info. Also matches 0010 or NEW_ID3. */
#define MPG123_NEW_ID3 0x1 /* 0001 There is ID3 info that changed since last call to mpg123_id3. */
#define MPG123_ICY     0xc /* 1100 There is some ICY info. Also matches 0100 or NEW_ICY.*/
#define MPG123_NEW_ICY 0x4 /* 0100 There is ICY info that changed since last call to mpg123_icy. */
EXPORT int mpg123_meta_check(mpg123_handle *mh); /* On error (no valid handle) just 0 is returned. */
/* Point v1 and v2 to existing data structures wich may change on any next read/decode function call.
   Return value is MPG123_OK or MPG123_ERR, v1 and/or v2 can be set to NULL when there is no corresponding data. */
EXPORT int mpg123_id3(mpg123_handle *mh, mpg123_id3v1 **v1, mpg123_id3v2 **v2);
EXPORT int mpg123_icy(mpg123_handle *mh, char **icy_meta); /* same for ICY meta string */

/* missing various functions to change properties: RVA, equalizer */
/* also: functions to access properties: RVA, equalizer... */

#ifdef __cplusplus
}
#endif

#endif
