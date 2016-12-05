INCLUDE		= -I.
CFLAGS		= -O0 -g $(INCLUDE)
LDFLAGS		= -O0 -g -pthread

TARGET		= ledctld

SRC		=  ledctld.c
SRC		+= worker.c
SRC		+= device.c
SRC		+= loger.c

OBJ		= $(SRC:.c=.o)
DEPS		= $(SRC:.c=.d)

$(TARGET): $(OBJ)

-include $(DEPS)

%.d: %.c
	$(CC) -M -MF $@ $(CFLAGS) -c $<

clean:
	rm -f $(TARGET) $(OBJ) $(DEPS)
