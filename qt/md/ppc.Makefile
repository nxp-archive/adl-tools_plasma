
#
# `Normal' configuration.
#
CC	      = gcc -ansi -Wall -pedantic

# Allow symbolic register names.
ASFLAGS = -mregnames -g

.s.o:
		$(CC) -c $(ASFLAGS) -o $@ $<


