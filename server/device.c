/*
 * device.c
 *
 */

#include <sys/types.h>
#include <stddef.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>

#include "device.h"

#define	OK	0
#define	FAILED	1

typedef	enum { off, on } state_t;
typedef	enum { red, green, blue } color_t;
typedef	int	rate_t;

typedef	int	(*cmd_fun_t) (char *argv[], char **rstr);
typedef struct
{
	char		*name;	/* имя команды */
	int		argc;	/* количестов передаваемых аргументов */
	cmd_fun_t	fun;	/* функция обработки */
} cmd_t;

/*
 * Оборудование.
 */
static struct
{
	state_t	state;
	color_t	color;
	rate_t	rate;
} led;
/*
 * Доступ к оборудованию должен быть, как правило,
 * монопольным. Используем mutex.
 */
static pthread_mutex_t	mutex = PTHREAD_MUTEX_INITIALIZER;

/**
 ** Функции обработки команды.
 ** Принимают вектор аргументов argv.
 ** Возвращают OK или FAILED.
 ** В rstr функция может вернуть указатель на строку.
 **/
static int
cmd_set_led_state (char *argv[], char **rstr)
{
	state_t	state;

	if (rstr)
		*rstr = NULL;

	if (strcmp (argv[0], "off") == 0)
		state = off;
	else if (strcmp (argv[0], "on") == 0)
		state = on;
	else
		return FAILED;

	pthread_mutex_lock (&mutex);
	led.state = state;
	pthread_mutex_unlock (&mutex);

	return OK;
}

static int
cmd_get_led_state (char *argv[], char **rstr)
{
	state_t	state;

	if (rstr == NULL)
		return FAILED;

	pthread_mutex_lock (&mutex);
	state = led.state;
	pthread_mutex_unlock (&mutex);

	switch (state)
	{
		case off:
			*rstr = "off";
			break;
		case on:
			*rstr = "on";
			break;
	}

	return OK;
}

static int
cmd_set_led_color (char *argv[], char **rstr)
{
	color_t	color;

	if (rstr)
		*rstr = NULL;

	if (strcmp (argv[0], "red") == 0)
		color = red;
	else if (strcmp (argv[0], "green") == 0)
		color = green;
	else if (strcmp (argv[0], "blue") == 0)
		color = blue;
	else
		return FAILED;

	pthread_mutex_lock (&mutex);
	led.color = color;
	pthread_mutex_unlock (&mutex);

	return OK;
}

static int
cmd_get_led_color (char *argv[], char **rstr)
{
	color_t	color;

	if (rstr == NULL)
		return FAILED;

	pthread_mutex_lock (&mutex);
	color = led.color;
	pthread_mutex_unlock (&mutex);

	switch (color)
	{
		case red:
			*rstr = "red";
			break;
		case green:
			*rstr = "green";
			break;
		case blue:
			*rstr = "blue";
			break;
	}

	return OK;
}

static int
cmd_set_led_rate (char *argv[], char **rstr)
{
	rate_t	rate;

	if (rstr)
		*rstr = NULL;

	if (strcmp (argv[0], "0") == 0)
		rate = 0;
	else if (strcmp (argv[0], "1") == 0)
		rate = 1;
	else if (strcmp (argv[0], "2") == 0)
		rate = 2;
	else if (strcmp (argv[0], "3") == 0)
		rate = 3;
	else if (strcmp (argv[0], "4") == 0)
		rate = 4;
	else if (strcmp (argv[0], "5") == 0)
		rate = 5;
	else
		return FAILED;

	pthread_mutex_lock (&mutex);
	led.rate = rate;
	pthread_mutex_unlock (&mutex);

	return OK;
}

static int
cmd_get_led_rate (char *argv[], char **rstr)
{
	rate_t	rate;

	if (rstr == NULL)
		return FAILED;

	pthread_mutex_lock (&mutex);
	rate = led.rate;
	pthread_mutex_unlock (&mutex);

	switch (rate)
	{
		case 0:
			*rstr = "0";
			break;
		case 1:
			*rstr = "1";
			break;
		case 2:
			*rstr = "2";
			break;
		case 3:
			*rstr = "3";
			break;
		case 4:
			*rstr = "4";
			break;
		case 5:
			*rstr = "5";
			break;
	}

	return OK;
}

/* Обрабатываемые команды */
static cmd_t	cmd[] =
{
	{
		.name	= "set-led-state",
		.argc	= 1,
		.fun	= cmd_set_led_state,
	},
	{
		.name	= "get-led-state",
		.argc	= 0,
		.fun	= cmd_get_led_state,
	},
	{
		.name	= "set-led-color",
		.argc	= 1,
		.fun	= cmd_set_led_color,
	},
	{
		.name	= "get-led-color",
		.argc	= 0,
		.fun	= cmd_get_led_color,
	},
	{
		.name	= "set-led-rate",
		.argc	= 1,
		.fun	= cmd_set_led_rate
	},
	{
		.name	= "get-led-rate",
		.argc	= 0,
		.fun	= cmd_get_led_rate
	},
	{}
};

/*
 * device	- функция подготовки запроса и выполнение его на оборудовании.
 *
 * Параметры:
 *	argc	- длина вектора аргументов
 *	argv	- вектор аргументов
 *	rstr	- указатель на буфер для возвращаемой строки
 *	rstr_len- размер буфера rstr
 *
 * Если ответа нет, то rstr устанавливается в NULL.
 */
int
device (int argc, char *argv[], char **rstr, int rstr_len)
{
	int	rc = -1;
	int	i;
	char	*answer;

	if (argc == 0)
		goto err;			/* Пришел пустой запрос */

	for (i = 0; cmd[i].name; ++i)
	{
		if (strcmp (argv[0], cmd[i].name) == 0)
		{
			--argc;
			++argv;
			if (argc != cmd[i].argc)
				goto err;

			rc = cmd[i].fun (argv, &answer);
			break;
		}
	}
	if (rc == -1)
		goto err;			/* Команда не найдена */

	if (rstr)
	{
		if (answer != NULL)
			/*
			 * Ответ копируется в предоставленный буфер.
			 *
			 * Потенциально узкое место, так как при высокой
			 * нагрузке параллельных запросов (десятки раз в сек.),
			 * answer может измениться.
			 * Если нагрузки высокие, то answer может быть
			 * указателем из кучи (alloc). В этом случае здесь
			 * нужно будет его освободить (free).
			 *
			 * Лучшим вариантом для высокой нагрузки будет пул
			 * статических строк, одна из которых будет возвращаться
			 * в answer. Тогда копирование ответа успеет закончится
			 * без изменения исходного ответа.
			 */
			snprintf (*rstr, rstr_len, "%s", answer);
		else
			*rstr = NULL;
	}

	return rc;
err:
	if (rstr)
		*rstr = NULL;
	return FAILED;
}
