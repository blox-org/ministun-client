#CROSS=mipsel-uclibc-
CC=$(CROSS)gcc
LD=$(CROSS)ld
AR=$(CROSS)ar
RANLIB=$(CROSS)ranlib
SYSARCH=64

CCFLAGS+=-O2 -Wall -I. -fPIC -Wno-pointer-sign -fomit-frame-pointer #-Wunused-but-set-variable #-DSTUN_BINDREQ_PROCESS
LDFLAGS+=-s -L./
LIBS=libministun.so

PACKAGE=ministun
OBJS=$(PACKAGE).o
INSTALLDIR ?= $(BASEDIR)

all: $(PACKAGE)

$(PACKAGE): $(LIBS) $(OBJS)
	$(CC) $(CCFLAGS) $(LDFLAGS) -lministun -o $(PACKAGE) $(LIBS) client.c 

$(LIBS): ministun.o
	$(CC) $(CCFLAGS) $(LDFLAGS) -shared -Wl,-soname,libministun.so.1 -o libministun.so ministun.o

%.o: %.c
	$(CC) $(CCFLAGS) -c $<

clean:
	rm -f *.o $(PACKAGE) *.so

install: $(PACKAGE)
	-@mkdir -p $(INSTALLDIR)/usr/bin $(INSTALLDIR)/usr/lib$(SYSARCH)/
	-@ln -s libministun.so libministun.so.1
	cp libministun.so libministun.so.1 $(INSTALLDIR)/usr/lib$(SYSARCH)/
	cp ministun $(INSTALLDIR)/usr/bin/
