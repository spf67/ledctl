/*
 * ledctld.c
 *
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <pthread.h>

#include "ledctld.h"
#include "worker.h"

static char	*dir = LEDCTL_DIR;		/* рабочий каталог */
static char	*pid_file = LEDCTL_PIDFILE;	/* файл для pid процесса */
static char	*fifo_cmd = LEDCTL_FIFOCMD;	/* FIFO-канал для соединения */

static void
usage (char *progname)
{
	fprintf (stderr,
		 "Usage: %s [-p pid_file] [-f fifo_cmd]\n",
		 progname);

	exit (0);
}

static void
sighandler (int sig)
{
	unlink (fifo_cmd);
	unlink (pid_file);

	exit (sig);
}

int
main (int argc, char *argv[])
{
	FILE		*fd;
	struct stat	st;
	pid_t		pid;
	int		rc;
	char		*p;

	/* каталог можно задать через окружение */
	p = getenv ("LEDCTL_DIR");
	if (p)
		dir = p;

	/* Обработка опций командной строки */
	while ((rc = getopt (argc, argv, "p:f:")) != -1)
	{
		switch (rc)
		{
			case 'p':
				pid_file = optarg;
				break;

			case 'f':
				fifo_cmd = optarg;
				break;

			default:
				usage (argv[0]);
		}
	}
	argc -= optind;
	argv += optind;

	/* Переход в рабочий каталог */
	rc = chdir (dir);
	if (rc)
	{
		perror (dir);
		return errno;
	}

	/*
	 * Процесс для работы с устройством должен быть один.
	 * Проверяем pid-файл.
	 */
	fd = fopen (pid_file, "r");
	if (fd)
	{
		rc = fscanf (fd, "%u", &pid);
		if (rc != 1)
		{
			perror ("fscanf(pid_file)");
			return errno;
		}
		fclose (fd);

		rc = kill (pid, 0);
		if (rc == 0)
		{
			fprintf (stderr,
				 "Process pid=%d already exists\n",
				 pid);
			return EEXIST;
		}

		unlink (pid_file);
	}

	/* Откроем pid-файл */
	fd = fopen (pid_file, "w");
	if (fd == NULL)
	{
		perror ("fopen(pid_file)");
		return errno;
	}

#if 0
	/* Стать даемоном */
	rc = daemon (1, 1);
	if (rc)
	{
		perror ("daemon");
		return errno;
	}
#endif

	/* Запишем свой pid */
	pid = getpid ();
	fprintf (fd, "%u\n", pid);
	fclose (fd);

	/* Обработка сигналов */
	signal (SIGINT, sighandler);

	/*
	 * Удаление командного FIFO-канала.
	 * Если его нет, то ошибка игнорируется.
	 * Если удалить не удалось, то эту ошибку поймаем на создании канала.
	 */
	unlink (fifo_cmd);

	/* Создание командного FIFO-канала */
	umask (0006);
	rc = mkfifo (fifo_cmd, 0620);
	if (rc)
	{
		perror (fifo_cmd);
		return errno;
	}

	/*
	 * Основной цикл.
	 * Принимаются команды connect и запускается
	 * отдельный обработчик клиента.
	 */
	while (1)
	{
		pthread_t	thread;
		char		buf[STRING_MAX];
		char		fifo_client[PATH_CLIENT_MAX];

		/* Командный FIFO-канал открывается для чтения */
		fd = fopen (fifo_cmd, "r");
		if (fd == NULL)
		{
			perror ("fopen(fifo_cmd)");
			return errno;
		}

		while ((p = fgets (buf, sizeof (buf), fd)) != NULL)
		{
			rc = sscanf (buf, "connect %30[-_0-9A-Za-z]",
				     fifo_client);
			if (rc != 1)
				/*
				 * Комманда не connect
				 * или имя файла содержит недопустимые символы.
				 * Пропускаем...
				 */
				continue;

			/* Скопируем имя клиентского FIFO-канала */
			p = malloc (strlen (fifo_client) + 1);
			if (p == NULL)
			{
				perror ("malloc");
				continue;
			}
			strcpy (p, fifo_client);

			/* Запуск обработчика в отдельном потоке */
			rc = pthread_create (&thread, NULL, worker, p);
			if (rc)
			{
				perror ("pthread_create");
				continue;
			}

			/* Окончание потока не обрабатывается */
			rc = pthread_detach (thread);
			if (rc)
				perror ("pthread_detach");
		}

	}

	return 0;
}
