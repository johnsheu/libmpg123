#include <syn123.h>
#include <out123.h>

#include "compat.h"

int main(int argc, char **argv)
{
	int ret = 0;
	if(argc < 5)
	{
		fprintf( stderr, "Usage: %s <rate> <freq> <block> <speedfactor>"
			" [duration [outfile]]\n"
		,	argv[0] );
		return 1;
	}
	long rate = atol(argv[1]);
	double freq = atof(argv[2]);
	long block = atol(argv[3]);
	size_t bi = 0;
	double factor = atof(argv[4]);
	double duration = argc > 5 ? atof(argv[5]) : 5;
	char *outfile = argc > 6 ? argv[6] : NULL;
	syn123_handle *syn = syn123_new(rate, 1, MPG123_ENC_FLOAT_32, 0, NULL);
	if(!syn)
		return -1;
	out123_handle *out = out123_new();
	if(!out)
		return -2;
	if(out123_open(out, outfile ? "wav" : NULL, outfile))
		return -3;
	if(out123_start(out, rate, 1, MPG123_ENC_FLOAT_32))
		return -3;
	if(syn123_setup_waves(syn, 1, NULL, &freq, NULL, NULL, NULL))
		return -4;
	float *inbuf = malloc(sizeof(float)*block);
	if(!inbuf)
		return -5;
	while(bi*block < duration*rate)
	{
		size_t bytes = syn123_read(syn, inbuf, sizeof(float)*block);
		if(bytes != sizeof(float)*block)
		{
			ret = -14;
			break;
		}
		double outrate = (double)rate/pow(factor, bi);
		if(outrate > LONG_MAX)
			outrate = LONG_MAX;
		if(syn123_setup_resample(syn, rate, (long)outrate, 1, 1))
		{
			ret = -11;
			break;
		}
		size_t outblock = syn123_resample_count(rate, (long)outrate, block);
		if(!outblock)
		{
			ret = -12;
			break;
		}
		float *outbuf = malloc(sizeof(float)*(outblock));
		if(!outbuf)
		{
			ret = -13;
			break;
		}
		size_t outsamples = syn123_resample(syn, outbuf, inbuf, block);
		syn123_amp(outbuf, MPG123_ENC_FLOAT_32, outsamples, syn123_db2lin(-6), 0, NULL, NULL);
		if(out123_play(out, outbuf, sizeof(float)*outsamples) != sizeof(float)*outsamples)
		{
			ret = -10;
			break;
		}
		free(outbuf);
		++bi;
	}
	free(inbuf);
	out123_del(out);
	syn123_del(syn);
	return ret;
}
