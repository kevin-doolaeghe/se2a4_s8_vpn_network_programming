#
# Makefile pour generer les executables du pont virtuel
#

OBJS_SWITCH=libnet.o virtual_bridge.o
OBJS_CLIENT=libnet.o virtual_client.o

CFLAGS += -Wall -DDEBUG

all: virtual_bridge virtual_client

#
# La cible de nettoyage
#

clean: 
	rm -f core *.o virtual_bridge virtual_client

#
# Les cibles
#

virtual_bridge:	$(OBJS_SWITCH)
	$(CC) $(CFLAGS) -o $@ $(OBJS_SWITCH)
virtual_client:	$(OBJS_CLIENT)
	$(CC) $(CFLAGS) -o $@ $(OBJS_CLIENT)

libnet.o: libnet.c libnet.h
virtual_bridge.o: virtual_bridge.c libnet.h
virtual_client.o: virtual_client.c libnet.h
