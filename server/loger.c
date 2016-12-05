/*
 * loger.c
 *
 */
#include <sys/types.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>

#include "loger.h"
#include "ledctld.h"

void
loger (const char *format, ...)
{
	static FILE	*log;
	va_list		ap;
	time_t		clock;
	struct tm	tm;
	char		strtm[sizeof("YYYY-MM-DD hh:mm:ss")];

	if (log == NULL)
	{
		log = fopen (LEDCTL_LOGFILE, "a");
		if (log == NULL)
		{
			perror ("fopen(log)");
			exit (errno);
		}
	}

	time (&clock);
	localtime_r (&clock, &tm);
	strftime (strtm, sizeof (strtm), "%F %T", &tm);
	fprintf (log, "%s ", strtm);

	va_start (ap, format);
	vfprintf (log, format, ap);
	va_end (ap);

	fflush (log);
}
