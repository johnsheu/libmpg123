#include "mpg123lib.h"
#include "mpg123.h"
#include "genre.h"

static void utf8_ascii(mpg123_string *dest, mpg123_string *source);

/* print tags... limiting the UTF-8 to ASCII */
void print_id3_tag(mpg123_handle *mh, int long_id3)
{
	char genre_from_v1 = 0;
	enum { TITLE=0, ARTIST, ALBUM, COMMENT, YEAR, GENRE, FIELDS } ti;
	mpg123_string tag[FIELDS];
	mpg123_id3v1 *v1;
	mpg123_id3v2 *v2;
	/* no memory allocated here, so return is safe */
	for(ti=0; ti<FIELDS; ++ti) mpg123_init_string(&tag[ti]);

	/* extract the data */
	mpg123_id3(mh, &v1, &v2);
	/* Only work if something there... */
	if(v1 == NULL && v2 == NULL) return;
	if(v2 != NULL) /* fill from ID3v2 data */
	{
		utf8_ascii(&tag[TITLE],   &v2->title);
		utf8_ascii(&tag[ARTIST],  &v2->artist);
		utf8_ascii(&tag[ALBUM],   &v2->album);
		utf8_ascii(&tag[COMMENT], &v2->comment);
		utf8_ascii(&tag[YEAR],    &v2->year);
		utf8_ascii(&tag[GENRE],   &v2->genre);
	}
	if(v1 != NULL) /* fill gaps with ID3v1 data */
	{
		/* I _could_ skip the recalculation of fill ... */
		if(!tag[TITLE].fill)
		{
			if(tag[TITLE].size >= 31 || mpg123_resize_string(&tag[TITLE], 31))
			{
				strncpy(tag[TITLE].p,v1->title,30);
				tag[TITLE].p[30] = 0;
				tag[TITLE].fill = strlen(tag[TITLE].p) + 1;
			}
		}
		if(!tag[ARTIST].fill)
		{
			if(tag[ARTIST].size >= 31 || mpg123_resize_string(&tag[ARTIST],31))
			{
				strncpy(tag[ARTIST].p,v1->artist,30);
				tag[ARTIST].p[30] = 0;
				tag[ARTIST].fill = strlen(tag[ARTIST].p) + 1;
			}
		}
		if(!tag[ALBUM].fill)
		{
			if(tag[ALBUM].size >= 31 || mpg123_resize_string(&tag[ALBUM],31))
			{
				strncpy(tag[ALBUM].p,v1->album,30);
				tag[ALBUM].p[30] = 0;
				tag[ALBUM].fill = strlen(tag[ALBUM].p) + 1;
			}
		}
		if(!tag[COMMENT].fill)
		{
			if(tag[COMMENT].size >= 31 || mpg123_resize_string(&tag[COMMENT],31))
			{
				strncpy(tag[COMMENT].p,v1->comment,30);
				tag[COMMENT].p[30] = 0;
				tag[COMMENT].fill = strlen(tag[COMMENT].p) + 1;
			}
		}
		if(!tag[YEAR].fill)
		{
			if(tag[YEAR].size >= 5 || mpg123_resize_string(&tag[YEAR],5))
			{
				strncpy(tag[YEAR].p,v1->year,4);
				tag[YEAR].p[4] = 0;
				tag[YEAR].fill = strlen(tag[YEAR].p) + 1;
			}
		}
		/*
			genre is special... v1->genre holds an index, id3v2 genre may contain indices in textual form and raw textual genres...
		*/
		if(!tag[GENRE].fill)
		{
			if(tag[GENRE].size >= 31 || mpg123_resize_string(&tag[GENRE],31))
			{
				if(v1->genre <= genre_count)
				{
					strncpy(tag[GENRE].p, genre_table[v1->genre], 30);
				}
				else
				{
					strncpy(tag[GENRE].p,"Unknown",30);
				}
				tag[GENRE].p[30] = 0;
				tag[GENRE].fill = strlen(tag[GENRE].p) + 1;
				genre_from_v1 = 1;
			}
		}
	}

	if(tag[GENRE].fill && !genre_from_v1)
	{
		/*
			id3v2.3 says (id)(id)blabla and in case you want ot have (blabla) write ((blabla)
			also, there is
			(RX) Remix
			(CR) Cover
			id3v2.4 says
			"one or several of the ID3v1 types as numerical strings"
			or define your own (write strings), RX and CR 

			Now I am very sure that I'll encounter hellishly mixed up id3v2 frames, so try to parse both at once.
		*/
		mpg123_string tmp;
		mpg123_init_string(&tmp);
		debug1("interpreting genre: %s\n", tag[GENRE].p);
		if(mpg123_copy_string(&tag[GENRE], &tmp))
		{
			size_t num = 0;
			size_t nonum = 0;
			size_t i;
			enum { nothing, number, outtahere } state = nothing;
			tag[GENRE].fill = 0; /* going to be refilled */
			/* number\n -> id3v1 genre */
			/* (number) -> id3v1 genre */
			/* (( -> ( */
			for(i = 0; i < tmp.fill; ++i)
			{
				debug1("i=%lu", (unsigned long) i);
				switch(state)
				{
					case nothing:
						nonum = i;
						if(tmp.p[i] == '(')
						{
							num = i+1; /* number starting as next? */
							state = number;
							debug1("( before number at %lu?", (unsigned long) num);
						}
						/* you know an encoding where this doesn't work? */
						else if(tmp.p[i] >= '0' && tmp.p[i] <= '9')
						{
							num = i;
							state = number;
							debug1("direct number at %lu", (unsigned long) num);
						}
						else state = outtahere;
					break;
					case number:
						/* fake number alert: (( -> ( */
						if(tmp.p[i] == '(')
						{
							nonum = i;
							state = outtahere;
							debug("no, it was ((");
						}
						else if(tmp.p[i] == ')' || tmp.p[i] == '\n' || tmp.p[i] == 0)
						{
							if(i-num > 0)
							{
								/* we really have a number */
								int gid;
								char* genre = "Unknown";
								tmp.p[i] = 0;
								gid = atoi(tmp.p+num);

								/* get that genre */
								if(gid >= 0 && gid <= genre_count) genre = genre_table[gid];
								debug1("found genre: %s", genre);

								if(tag[GENRE].fill) mpg123_add_string(&tag[GENRE], ", ");
								mpg123_add_string(&tag[GENRE], genre);
								nonum = i+1; /* next possible stuff */
								state = nothing;
								debug1("had a number: %i", gid);
							}
							else
							{
								/* wasn't a number, nonum is set */
								state = outtahere;
								debug("no (num) thing...");
							}
						}
						else if(!(tmp.p[i] >= '0' && tmp.p[i] <= '9'))
						{
							/* no number at last... */
							state = outtahere;
							debug("nothing numeric here");
						}
						else
						{
							debug("still number...");
						}
					break;
					default: break;
				}
				if(state == outtahere) break;
			}
			if(nonum < tmp.fill-1)
			{
				if(tag[GENRE].fill) mpg123_add_string(&tag[GENRE], ", ");
				mpg123_add_string(&tag[GENRE], tmp.p+nonum);
			}
		}
		mpg123_free_string(&tmp);
	}

	if(long_id3)
	{
		fprintf(stderr,"\n");
		/* print id3v2 */
		/* dammed, I use pointers as bool again! It's so convenient... */
		fprintf(stderr,"\tTitle:   %s\n", tag[TITLE].fill ? tag[TITLE].p : "");
		fprintf(stderr,"\tArtist:  %s\n", tag[ARTIST].fill ? tag[ARTIST].p : "");
		fprintf(stderr,"\tAlbum:   %s\n", tag[ALBUM].fill ? tag[ALBUM].p : "");
		fprintf(stderr,"\tYear:    %s\n", tag[YEAR].fill ? tag[YEAR].p : "");
		fprintf(stderr,"\tGenre:   %s\n", tag[GENRE].fill ? tag[GENRE].p : "");
		fprintf(stderr,"\tComment: %s\n", tag[COMMENT].fill ? tag[COMMENT].p : "");
		fprintf(stderr,"\n");
	}
	else
	{
		/* We are trying to be smart here and conserve vertical space.
		   So we will skip tags not set, and try to show them in two parallel columns if they are short, which is by far the	most common case. */
		/* one _could_ circumvent the strlen calls... */
		if(tag[TITLE].fill && tag[ARTIST].fill && strlen(tag[TITLE].p) <= 30 && strlen(tag[TITLE].p) <= 30)
		{
			fprintf(stderr,"Title:   %-30s  Artist: %s\n",tag[TITLE].p,tag[ARTIST].p);
		}
		else
		{
			if(tag[TITLE].fill) fprintf(stderr,"Title:   %s\n", tag[TITLE].p);
			if(tag[ARTIST].fill) fprintf(stderr,"Artist:  %s\n", tag[ARTIST].p);
		}
		if(tag[COMMENT].fill && tag[ALBUM].fill && strlen(tag[COMMENT].p) <= 30 && strlen(tag[ALBUM].p) <= 30)
		{
			fprintf(stderr,"Comment: %-30s  Album:  %s\n",tag[COMMENT].p,tag[ALBUM].p);
		}
		else
		{
			if(tag[COMMENT].fill)
				fprintf(stderr,"Comment: %s\n", tag[COMMENT].p);
			if(tag[ALBUM].fill)
				fprintf(stderr,"Album:   %s\n", tag[ALBUM].p);
		}
		if(tag[YEAR].fill && tag[GENRE].fill && strlen(tag[YEAR].p) <= 30 && strlen(tag[GENRE].p) <= 30)
		{
			fprintf(stderr,"Year:    %-30s  Genre:  %s\n",tag[YEAR].p,tag[GENRE].p);
		}
		else
		{
			if(tag[YEAR].fill)
				fprintf(stderr,"Year:    %s\n", tag[YEAR].p);
			if(tag[GENRE].fill)
				fprintf(stderr,"Genre:   %s\n", tag[GENRE].p);
		}
	}
	for(ti=0; ti<FIELDS; ++ti) mpg123_free_string(&tag[ti]);
}


static void utf8_ascii(mpg123_string *dest, mpg123_string *source)
{
	size_t spos = 0;
	size_t dlen = 1; /* one paranoia 0 */
	char *p;
	/* Find length, continuation bytes don't count. */
	for(spos=0; spos < source->fill; ++spos)
	if((source->p[spos] & 0xc0) == 0x80) continue;
	else ++dlen;

	if(!mpg123_resize_string(dest, dlen)){ mpg123_free_string(dest); return; }

	/* Just ASCII, we take it easy. */
	p = dest->p;
	for(spos=0; spos < source->fill; ++spos)
	{
		/* utf8 continuation byte bo, lead!*/
		if((source->p[spos] & 0xc0) == 0x80) continue;
		/* utf8 lead byte, no, cont! */
		else if(source->p[spos] & 0x80) *p = '*';
		else *p = source->p[spos];
		++p; /* next output char */
	}
	dest->p[dest->size] = 0;
	dest->fill = dest->size;
}
