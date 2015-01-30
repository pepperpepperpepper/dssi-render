SUBDIRS = midi

bin_PROGRAMS = dssi-render

AM_CFLAGS = -Wall -std=c99 $(DSSI_CFLAGS) $(SNDFILE_CFLAGS) $(ALSA_CFLAGS)
AM_LIBS = $(DSSI_LIBS) $(SNDFILE_LIBS) $(ALSA_LIBS)

dssi_render_SOURCES = dssi-render.c dssi-render.h ladspa_run_synth.c
dssi_render_LDADD = $(AM_LIBS) -lm -ldl midi/libfluidmidi.la

EXTRA_DIST = lts.prs
