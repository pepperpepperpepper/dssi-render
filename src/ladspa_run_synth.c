#include "dssi-render.h"
#include "midi/midi_loader.h" 

  /* Instead of creating an alsa midi input, we fill in two events
   * note-on and note-off */
inline int
min(int x, int y) {
  return (x < y) ? x : y;
}
//is there any way to test a single event in the event table from here? hmm not sure

void ladspa_run_sample_callback(event_table_t *event_table, void *userdata){
//so something weird with nframes we pass, a bit strange code in trivial_synth too, I thought that run_synth need number of frames to  calulate, but it seems it is multiplying number of events and number of frames to calulate. need to check documentation about to to actually run this thing. right trivial_synth is supplied as an example from the people who wrote dssi, most likely it's cli-dssi-host that is the source of the confusion b utc ould believe
//what if we set the number of frames to a fixed number, did that work? yes already tried that, wo rk partially, only with on_event. so i guess only thing left to check is to supply on_event and change only one field like note in it with data from event_table, just to find out what is going on. well one thing, not sure if this is the best, but I could generate a much simpler midi file for testing purposes, with two notes, or do you think that's a waste? well it should work too, checking it here will be just faster
//well one quick fix we can try to run, overwise need to check it all again
  snd_seq_event_t on_event, off_event, *current_event;
  on_event.type = SND_SEQ_EVENT_NOTEON;
  on_event.data.note.channel = 0;
  on_event.data.note.note = 2 ;//midi_note;
  on_event.data.note.velocity = 64;// midi_velocity;
  on_event.time.tick = 0;

  off_event.type = SND_SEQ_EVENT_NOTEOFF;
  off_event.data.note.channel = 0;
  off_event.data.note.note = midi_note;
  off_event.data.note.off_velocity = midi_velocity;
  off_event.time.tick = 0;
if(event_table->length >=1){
  //on_event.data.note.note = event_table->events->data.note.note;
  //on_event.data.note.velocity = event_table->events->data.note.velocity;
}// nope i removed those changes, should be correct notes
  nframes = event_table->nframes_since_last;
  nframes = nframes > 0 ? nframes : 1;
  for (int i = 0; i < outs; i++) {
    pluginOutputBuffers[i] = (float *)realloc(pluginOutputBuffers[i], nframes * sizeof(float)); // yeah here it is, small buffer also nframes is a size_t, that would limit it? or not really not really ok so that can get pretty big? ye2^32
    memset(pluginOutputBuffers[i], 0, nframes * sizeof(float));
}
  connect_ports();
//should we try exiting right after runsynth?  or is there not much code afterwards?

//nframes = event_table->nframes_since_last;

    if (descriptor->run_synth) {
      descriptor->run_synth(instanceHandle,
	        nframes,		    
			    event_table->events,
			    event_table->length
      );
/*if(event_table->events){
printf("on_event: ");
print_snd_seq_event(&on_event);
printf("event_table: ");
print_snd_seq_event(event_table->events);
}*/
    } else if (descriptor->run_multiple_synths) {
      descriptor->run_multiple_synths(1,
				      &instanceHandle,
    //        event_table->nframes_since_last, 
	  nframes,		    
            &event_table->events,
            &event_table->length);
    }
//so we need to test it now? yes
    /* Interleaving for libsndfile. */
    float sf_output[nchannels * nframes];
    for (int i = 0; i < nframes; i++) {
      /* First, write all the obvious channels */
      for (int j = 0; j < min(outs, nchannels); j++) {
	/* If outs > nchannels, we *could* do mixing - but don't. */
	sf_output[i * nchannels + j] = pluginOutputBuffers[j][i];
      }
      /* Then, if user wants *more* output channels than there are
       * audio output ports (ie outs < nchannels), copy the last audio
       * out to all the remaining channels. If outs >= nchannels, this
       * loop is never entered. */
      for (int j = outs; j < nchannels; j++) {
	sf_output[i * nchannels + j] = pluginOutputBuffers[outs - 1][i];
      }
    }
//clip = 1;  // FIXME
    if (clip) {
      for (int i = 0; i < nframes * nchannels; i++) {
	if (!finite(sf_output[i])) {
	  if (!have_warned) {
	    have_warned = 1;
	    fprintf(stderr, 
		    "%s: Warning: clipping NaN or Inf in synthesized data\n", 
		    my_name);
	  }
	  if (sf_output[i] < 0.0f) {
	    sf_output[i] = -1.0f;
	  } else {
	    sf_output[i] = 1.0f;
	  }
	} else {
	  if (sf_output[i] < -1.0f) {
	    if (!have_warned) {
	      have_warned = 1;
	      fprintf(stderr, 
		      "%s: Warning: clipping out-of-bounds value in synthesized data\n", 
		      my_name);
	    }
	    sf_output[i] = -1.0f;
	  } else if (sf_output[i] > 1.0f) {
	    if (!have_warned) {
	      have_warned = 1;
	      fprintf(stderr, 
		      "%s: Warning: clipping out-of-bounds value in synthesized data\n", 
		      my_name);
	    }
	    sf_output[i] = 1.0f;
	  }
	}
      }
    } else {
      for (int i = 0; i < nframes * nchannels; i++) {
	if (!finite(sf_output[i])) {
	  fprintf(stderr, "%s: Error: NaN or Inf in synthesized data\n",
		  my_name);
	  exit(1);
	}
	if (sf_output[i] > 1.0f
	    || sf_output[i] < -1.0f) {
	  fprintf(stderr, "%s: Error: sample data out of bounds\n",
		  my_name);
	  exit(1);
	}
      }
    }

    /* Write the audio */
    if ((items_written = sf_writef_float(outfile, 
					 sf_output,
					 nframes)) != nframes) {
      fprintf(stderr, "%s: Error: can't write data to output file %s\n", 
	      my_name, output_file);
      fprintf(stderr, "%s: %s\n", my_name, sf_strerror(outfile));
      return;
    }
  fprintf(stdout, "%s: Wrote %d frames to %s\n", 
	  my_name, items_written, output_file);
    
/*    total_written += items_written;
    if (release_tail >= 0) {
      if (total_written > length + release_tail) {
	finished = 1;
      }
    } else {
      if (total_written > length 
	  && is_silent(sf_output, nframes * nchannels)) {
	finished = 1;
      } else if (total_written > MAX_LENGTH * sample_rate) {
	/ * The default sineshaper patch never releases, after a note-off,
	 * to silence. So truncate. This is sineshaper 0.3.0 (so maybe it's 
	 * different in the new version) and here I mean the default
	 * patch as returned by the get_port_default() function, not the 
	 * default set by the sineshaper UI.
	 /
	finished = 1;
	fprintf(stderr, "%s: Warning: truncating after writing %d frames\n", 
		my_name, total_written);
      }
    }
  }
*/
}

void ladspa_run_synth(void){

//  ctx.player = player;
//  ctx.callback = callback;
//  ctx.callback_userdata = callback_userdata;
//     ctx.track = player->track[i];// here. so i guess need to make a structure and put player, track pointers into it.
  load_midi_file(midi_filename, ladspa_run_sample_callback, NULL);
}

