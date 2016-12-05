/*
 * worker.c
 *
 */
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>

#include "ledctld.h"
#include "worker.h"
#include "loger.h"
#include "device.h"

static const char * const ret_ok = "OK";
static const char * const ret_failed = "FAILED";

static void
chomp (char *str)
{
	int	l;

	if (str == NULL || *str == '\0')
		return;

	l = strlen (str);
	if (str[--l] == '\n')
		str[l] = '\0';
}

static int
mkarg (char *str, char *argv[], int argv_len)
{
	int	argc = 0;
	bool	inword = false;

	if (str == NULL)
		return argc;

	while (*str)
	{
		switch (*str)
		{
			case ' ':
			case '\t':
			case '\r':
			case '\n':
				*str = '\0';
				if (inword)
					inword = false;
				break;
			default:
				if (!inword)
				{
					argv[argc] = str;
					inword = true;
					if (++argc >= argv_len)
						goto done;
				}
		}

		++str;
	}

done:
	argv[argc] = NULL;
	return argc;
}

void *
worker (void *arg)
{
	char	*client = arg;
	FILE	*qfd, *afd;
	char	fname[PATH_CLIENT_MAX], errbuf[STRING_MAX];
	char	request[STRING_MAX], answer[STRING_MAX];

	loger ("%s: started\n", client);

	/*
	 * Для взаимодействия с клиентом открываются два FIFO-канала.
	 * Первый - для получения команд. (q)
	 * Второй - для ответов. (a)
	 * Каналы должны быть подготовлены клиентом.
	 */
	/* Канал команд */
	snprintf (fname, sizeof (fname), "%s.q", client);
	qfd = fopen (fname, "r");
	if (qfd == NULL)
	{
		strerror_r (errno, errbuf, sizeof (errbuf));
		loger ("%s: ERROR fopen(%s): %s\n", client, fname, errbuf);
		goto out1;
	}
	setvbuf (qfd, NULL, _IONBF, 0);

	/* Канал ответов */
	snprintf (fname, sizeof (fname), "%s.a", client);
	afd = fopen (fname, "w");
	if (afd == NULL)
	{
		strerror_r (errno, errbuf, sizeof (errbuf));
		loger ("%s: ERROR fopen(%s): %s\n", client, fname, errbuf);
		goto out2;
	}
	setvbuf (afd, NULL, _IONBF, 0);

	/* В канал ответов отправляется сообщение о готовности */
	snprintf (answer, sizeof (answer), "%s", ret_ok);
	fprintf (afd, "%s\n", answer);

	/*
	 * Основной цикл.
	 * Из канала команд получаем строку, разбиваем на аргументы,
	 * делаем запрос к оборудованию.
	 */
	while (fgets (request, sizeof (request), qfd) != NULL)
	{
		const char	*ret_code = ret_failed;
		char		*rs = NULL;
		int		argc, rc;
		char		*argv[DEVICE_ARG_MAX + 1];
		char		str[STRING_MAX];

		chomp (request);
		loger ("%s: request: %s\n", client, request);

		if (request[0])
		{
			argc = mkarg (request, argv, DEVICE_ARG_MAX);
			if (argc)
			{
				rs = str;
				rc = device (argc, argv, &rs, sizeof (str));
				if (rc)
					ret_code = ret_failed;
				else
					ret_code = ret_ok;
			}
		}

		if (rs)
			snprintf (answer, sizeof (answer),
				  "%s %s", ret_code, rs);
		else
			snprintf (answer, sizeof (answer),
				  "%s", ret_code);

		fprintf (afd, "%s\n", answer);
		loger ("%s: answer: %s\n", client, answer);
	}

	fclose (afd);
out2:
	fclose (qfd);
out1:
	loger ("%s: exited\n", client);
	free (client);
	return NULL;
}
