﻿/*
	scanclr: Estimate length (sample count) of a mpeg file and compare to length from exact scan.

	copyright 2009 by the mpg123 project - free software under the terms of the LGPL 2.1
	see COPYING and AUTHORS files in distribution or http://mpg123.org

    CLR example initially written by Malcolm Boczek
    Based on scan.c example initially written by Thomas Orgis
*/

/* Note the lack of error checking here.
   While it would be nicer to inform the user about troubles, libmpg123 is designed _not_ to bite you on operations with invalid handles , etc.
  You just jet invalid results on invalid operations... */
/* Ditto for mpg123clr (MB) */


using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

using mpg123clr;

namespace scanclr
{
    class Program
    {
        static void Main(string[] args)
        {
            if (args.Length < 1)
            {
                Console.WriteLine("\nI will give you the estimated and exact sample lengths of MPEG audio files.\n");
                Console.WriteLine("\nUsage: scanclr <mpeg audio file list>\n\n");
                Console.WriteLine("Press any key to exit:");
                while (Console.Read() == 0) ;

                return;
            }


            mpg123clr.mpg.ErrorCode err;

            err = mpg123.Init();

            mpg123 mp = new mpg123();
            err = mp.New();

            mp.SetParam(mpg123clr.mpg.parms.resync_limit, -1, 0);

            foreach (string name in args)
            {

                err = mp.Open(name);

                long a, b;

                a = mp.Length();

                mp.Scan();

                b = mp.Length();

                mp.Close();

                Console.WriteLine(string.Format("File {0}: estimated {1} vs. scanned {2}", name, a, b));
            }

            Console.WriteLine("\nPress any key to exit:");
            while (Console.Read() == 0) ;

            mp.Dispose();

            mpg123.Exit();
        }
    }
}
