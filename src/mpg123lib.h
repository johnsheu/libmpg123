#ifndef MPG123_LIB_H
#define MPG123_LIB_H

/* not decided... how anonymous should the handle be? */
struct frame;
typedef struct frame mpg123_handle;

struct mpg123_parameter
{
	int verbose;    /* verbose level */
#define MPG123_FORCE_MONO   0x7  /*     0111 */
#define MPG123_MONO_LEFT    0x1  /*     0001 */
#define MPG123_MONO_RIGHT   0x2  /*     0010 */
#define MPG123_MONO_MIX     0x4  /*     0100 */
#define MPG123_FORCE_STEREO 0x8  /*     1000 */
#define MPG123_FORCE_8BIT   0x10 /* 00010000 */
#define MPG123_QUIET        0x20 /* 00100000 suppress any printouts (overrules verbose) */
#define MPG123_GAPLESS      0x40 /* 01000000 flag always defined... */
#define MPG123_NO_RESYNC    0x80 /* 10000000 disable resync stream after error */
	int flags; /* combination of above */
	long force_rate;
	int down_sample;
	int rva; /* (which) rva to do: 0: nothing, 1: radio/mix/track 2: album/audiophile */
#ifdef OPT_MULTI
	char* cpu; /* chosen optimization, can be NULL/""/"auto"*/
#endif
	long halfspeed;
};

/* non-threadsafe init/exit, call _once_ */
void mpg123_init(void);
void mpg123_exit(void);
/* threadsafely create and delete handles */
mpg123_handle* mpg123_new(void);
void mpg123_delete(mpg123_handle *mh);

/* set/get output parameters, default 16bit stereo */
#define MPG123_ANY -1 /* Do not enforce this output parameter. */
int mpg123_output    (mpg123_handle *mh, int  format, int  channels, long rate);
int mpg123_get_output(mpg123_handle *mh, int *format, int *channels, long *rate);

int mpg123_open_feed  (mpg123_handle *mh);
int mpg123_open_stream(mpg123_handle *mh, char *url);
int mpg123_open_fd    (mpg123_handle *mh, int fd);

void mpg123_close(mpg123_handle *mh);


/* replacement for mpglib's decodeMP3, same usage */
#define MPG123_ERR -1
#define MPG123_OK  0
#define MPG123_NEED_MORE -10
int mpg123_decode(mpg123_handle *mh, unsigned char *inmemory,int inmemsize, unsigned char *outmemory,int outmemsize,int *done);
void mpg123_track(mpg123_handle *mh); /* clear out buffers/state for track change */



/* missing various functions to change properties: RVA, equalizer */
/* also: functions to access properties: ID3, RVA, ... */
#endif
