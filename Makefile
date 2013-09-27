SUBDIRS = \
	src

MAKE = make ${MFLAGS}
INSTALL = /usr/bin/install -c
INSTALLDATA = /usr/bin/install -c -m 644
PREFIX = /usr
BINDIR = $(PREFIX)/sbin

all: allintel

allintel: intel runintel

allarm: arm runarm

install: intel
	$(INSTALL) src/accesscontrol $(BINDIR)/accesscontrol

installarm: arm
	$(INSTALL) src/accesscontrol $(BINDIR)/accesscontrol

runintel:
	@for i in $(SUBDIRS); do \
		echo "Building $@";\
		cd $$i;\
		${MAKE} -f Makefile.intel test;\
		cd ..;\
	done

runarm:
	@for i in $(SUBDIRS); do \
		echo "Building $@";\
		cd $$i;\
		${MAKE} test;\
		cd ..;\
	done


intel:
	@for i in $(SUBDIRS); do \
		echo "Building $@";\
		cd $$i;\
		${MAKE} -f Makefile.intel clean;\
		${MAKE} -f Makefile.intel;\
		cd ..;\
	done

arm:
	@for i in $(SUBDIRS); do \
		echo "Building $@";\
		cd $$i;\
		${MAKE} clean;\
		${MAKE};\
		cd ..;\
	done

clean:
	@for i in $(SUBDIRS); do \
		echo "Building $@";\
		cd $$i;\
		${MAKE} clean;\
		cd ..;\
	done

