dist_bin_SCRIPTS =  pnml2pins
dist_bin_SCRIPTS += pnml2pins.py

bin_SCRIPTS = pnml2pins
CLEANFILES = $(bin_SCRIPTS)

EXTRA_DIST =  pnml2pins.in
EXTRA_DIST += pnml-pins.c 
EXTRA_DIST += pnml-pins.h

commondir = $(datadir)/$(PACKAGE)/common
common_DATA = pnml-pins.c pnml-pins.h

do_subst = sed -e 's,[@]pythonInstall[@],$(PYTHON),g' \
	-e 's,[@]pnml2pinsDataDir[@],$(datadir)/$(PACKAGE)/common,g' \
	-e 's,[@]pnml2pinsInstallDir[@],$(bindir),g' \
	-e 's,[@]LTSminInstallDir[@],$(LTSMIN_CFLAGS),g' \
	-e 's,[@]CCOpts[@],$(CFLAGS),g' \
	-e 's,[@]TheCC[@],$(CC),g'

pnml2pins: pnml2pins.in Makefile
	$(do_subst) < $(srcdir)/pnml2pins.in > pnml2pins
	chmod +x pnml2pins
