#include "dssi-render.h"
#include "midi/midi_loader.h" 

  /* Instead of creating an alsa midi input, we fill in two events
   * note-on and note-off */
inline int
min(int x, int y) {
  return (x < y) ? x : y;
}
// where is nframes defined?

void ladspa_run_sample_callback(event_table_t *event_table, void *userdata){
  nframes = event_table->nframes_since_last;
  nframes = nframes > 0 ? nframes : 1;
//is it nframes that has the negative value? or event_table length? well they are both size_t, can't be negative. i guess need to add prints to see what's going on
  printf("nframes: 0x%x\n", nframes);
  printf("outs: 0x%x\n", outs);
  for (int i = 0; i < outs; i++) {
    pluginOutputBuffers[i] = (float *)realloc(pluginOutputBuffers[i], nframes * sizeof(float)); 
    printf("i: 0x%x buff: 0x%x\n", i, pluginOutputBuffers[i]);
    memset(pluginOutputBuffers[i], 0, nframes * sizeof(float));
}
  connect_ports();


    if (descriptor->run_synth) {
      descriptor->run_synth(instanceHandle,
	        nframes,		    
			    event_table->events,
			    event_table->length
      );
    } else if (descriptor->run_multiple_synths) {
      descriptor->run_multiple_synths(1,
				      &instanceHandle,
	  nframes,		    
            &event_table->events,
            &event_table->length);
    }
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
    
}


void ladspa_run_synth(void){
  load_midi_file(midi_filename, sample_rate, ladspa_run_sample_callback, NULL);
}

