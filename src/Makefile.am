SUBDIRS = midi

bin_PROGRAMS = DSSI-render

AM_CFLAGS = -Wall -std=c99 $(DSSI_CFLAGS) $(SNDFILE_CFLAGS) $(ALSA_CFLAGS)
AM_LIBS = $(DSSI_LIBS) $(SNDFILE_LIBS) $(ALSA_LIBS)

DSSI_render_SOURCES = DSSI-render.c DSSI-render.h ladspa_run_synth.c
DSSI_render_LDADD = $(AM_LIBS) -lm -ldl midi/libfluidmidi.la

EXTRA_DIST = lts.prs
