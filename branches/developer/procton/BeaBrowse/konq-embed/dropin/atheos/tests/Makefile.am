
SUBDIRS = .

INCLUDES = -I$(srcdir)/.. $(QT_INCLUDES)

commonsrc = main.cpp
commonlib = ../libatheosdropin.la
commondepend = TWindow.h
commonldflags = -lbe -lqt

bin_PROGRAMS = lineedit
lineedit_SOURCES =  lineedit.cpp \
                 $(commonsrc) 
lineedit_LDADD = ../libatheosdropin.la
lineedit_LDFLAGS = -lbe -lqt

lineedit$(EXEEXT): $(lineedit_OBJECTS) $(test1_DEPENDENCIES) $(commondepend)
	@rm -f $@
	$(CXXLINK)  $(lineedit_OBJECTS) $(lineedit_LDADD) $(lineedit_LDFLAGS)

KDE_OPTIONS = qtonly
