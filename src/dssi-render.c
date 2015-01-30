#define CLI_FUNCS

#include "dssi-render.h"
#include "ladspa_run_synth.h"
#include "midi/midi_loader.h" 

#define SAMPLE_RATE 44100
#define DEBUG 1

void
print_usage(void) {
  fprintf(stderr, "Render a midi file with a DSSI instrument.\n");
  fprintf(stderr, "Usage:\n");
  fprintf(stderr, "$ %s <dssi_plugin.so>[%c<label>]\n", my_name, LABEL_SEP);
  fprintf(stderr, "  [-p [<bank>%c]<preset>] "
	  "(use -p -1 for default port values;\n           "
	  "-p -2 for random values; omit -p to read port values from stdin)\n",
	  BANK_SEP);
  fprintf(stderr, "  [-i <midifile.mid>] (default == \"test.mid\")\n");
  fprintf(stderr, "  [-o <output_file.wav>] (default == \"output.wav\")\n");
  fprintf(stderr, "  [-c <no_channels>] (default == 1; use -c -1 to use plugin's channel count)\n");
  fprintf(stderr, "  [-d <project_directory>]\n");
  fprintf(stderr, "  [-k <configure_key>%c<value>] ...\n", KEYVAL_SEP);
  fprintf(stderr, "  [-b] (clip out-of-bounds values, including Inf and NaN, to within bounds\n       (calls exit()) if -b is omitted)\n");
  exit(1);
}

int file_exist (char *filename)
{
  struct stat   buffer;   
  return (stat (filename, &buffer) == 0);
}
  
const DSSI_Descriptor *descriptor;
LADSPA_Handle instanceHandle;
SNDFILE *outfile;
char *output_file = "output.wav"; //ask anton...needs malloc?
char *midi_filename = "test.mid";
  int clip = 0;
  int have_warned = 0;
  int nchannels = 1;

// check later {{{  
DSSI_Descriptor_Function descfn;
  void *pluginObject;

  SF_INFO outsfinfo;

  char **configure_key = NULL;
  char **configure_val = NULL;
  char *directory = NULL;
  char *dllName = NULL;
  char *label;
  char *projectDirectory = NULL;

  int in, out, controlIn, controlOut;
  int ins, outs, controlIns, controlOuts;
  port_vals_source_t src = from_stdin;
  int midi_velocity = 127;
  int bank = 0;
  int program_no = 0;
  int nkeys = 0;


  size_t length = SAMPLE_RATE;
  size_t release_tail = -1;
  size_t nframes = 256; //here I guess? yeah ok
  size_t items_written = 0;
  
  float **pluginInputBuffers, **pluginOutputBuffers;
  float *pluginControlIns, *pluginControlOuts;
//}}}

void connect_ports(void){
  in = out = controlIn = controlOut = 0;
  for (int j = 0; j < descriptor->LADSPA_Plugin->PortCount; j++) {  
    /* j is LADSPA port number */
    
    LADSPA_PortDescriptor pod =
      descriptor->LADSPA_Plugin->PortDescriptors[j];
    
    if (LADSPA_IS_PORT_AUDIO(pod)) {
      
      if (LADSPA_IS_PORT_INPUT(pod)) {
	descriptor->LADSPA_Plugin->connect_port
	  (instanceHandle, j, pluginInputBuffers[in++]);
	
      } else if (LADSPA_IS_PORT_OUTPUT(pod)) {
	descriptor->LADSPA_Plugin->connect_port
	  (instanceHandle, j, pluginOutputBuffers[out++]);
      }
      
    } else if (LADSPA_IS_PORT_CONTROL(pod)) {
      
      if (LADSPA_IS_PORT_INPUT(pod)) {
	
	descriptor->LADSPA_Plugin->connect_port
	  (instanceHandle, j, &pluginControlIns[controlIn++]);
	
      } else if (LADSPA_IS_PORT_OUTPUT(pod)) {
	descriptor->LADSPA_Plugin->connect_port
	  (instanceHandle, j, &pluginControlOuts[controlOut++]);
      }
    }
  }  /* 'for (j...'  LADSPA port number */
}
  
int
main(int argc, char **argv) {

  my_name = basename(argv[0]);

  sample_rate = SAMPLE_RATE;
  
  /* Probably an unorthodox srandom() technique... */
  struct timeval tv;
  struct timezone tz;
  gettimeofday(&tv, &tz);
  srandom(tv.tv_sec + tv.tv_usec);

  if (argc < 2) {
    print_usage();
  }

  /* dll name is argv[1]: parse dll name, plus a label if supplied */
  parse_keyval(argv[1], LABEL_SEP, &dllName, &label);

  for (int i = 2; i < argc; i++) {
    if (DEBUG) {
      fprintf(stderr, "%s: processing options: argv[%d] = %s\n", 
	      my_name, i, argv[i]);
    }

    /* Deal with flags */
    if (!strcmp(argv[i], "-b")) {
      clip = 1;
      continue;
    } else {
      /* It's not a flag, so expect option + argument */
      if (argc <= i + 1) print_usage();
    }

    if (!strcmp(argv[i], "-c")) {
      nchannels = strtol(argv[++i], NULL, 0);
    } else if (!strcmp(argv[i], "-o")) {
      output_file = argv[++i];
    } else if (!strcmp(argv[i], "-i")) {
      midi_filename = argv[++i];
    } else if (!strcmp(argv[i], "-d")) {
      projectDirectory = argv[++i];
    } else if (!strcmp(argv[i], "-k")) {
      configure_key = realloc(configure_key, (nkeys + 1) * sizeof(char *));
      configure_val = realloc(configure_val, (nkeys + 1) * sizeof(char *));
      parse_keyval(argv[++i], KEYVAL_SEP, &configure_key[nkeys], 
		   &configure_val[nkeys]);
      nkeys++;
      
    } else if (!strcmp(argv[i], "-p")) {
//FIXME needs port preset
      char *first_str;
      char *second_str;
      parse_keyval(argv[++i], BANK_SEP, &first_str, &second_str);
      if (second_str) {
	bank = strtol(first_str, NULL, 0);
	program_no = strtol(second_str, NULL, 0);
      } else {
	program_no = strtol(first_str, NULL, 0);
	bank = 0;
      }
      if (program_no == -1) {
	src = from_defaults;
      } else if (program_no == -2) {
	src = from_random;
      } else {
	src = from_preset;
      }
    } else {
      fprintf(stderr, "%s: Error: Unknown option: %s\n", my_name, argv[i]);
      print_usage();
    }
  }

  //make sure input and output exist
  if(!file_exist(midi_filename)){
    fprintf(stderr, "ERROR:Could not find %s\n Please specify an input file with -i\n", midi_filename);
    exit(1);
  }
  if(!file_exist(output_file)){
    fprintf(stderr, "ERROR:Could not find %s\n Please specify an output file with -o\n", output_file);
    exit(1);
  }

  if (DEBUG) {
    for (int i = 0; i < nkeys; i++) {
      printf("key %d: %s; value: %s\n", i, configure_key[i],
	     configure_val[i]);
    }
  }

  if (DEBUG) {
    fprintf(stderr, "%s: Cmd-line args ok\n", my_name);
  }

  directory = load(dllName, &pluginObject, 0);
  if (!directory || !pluginObject) {
    fprintf(stderr, "\n%s: Error: Failed to load plugin library \"%s\"\n", 
	    my_name, dllName);
    return 1;
  }
                
  descfn = (DSSI_Descriptor_Function)dlsym(pluginObject, 
					   "dssi_descriptor");

  if (!descfn) {
    fprintf(stderr, "%s: Error: Not a DSSI plugin\n", my_name);
    exit(1);
  }

  /* Get the plugin descriptor and check the run_synth*() function
   * exists */
  int j = 0;
  descriptor = NULL;
  const DSSI_Descriptor *desc;
	
  while ((desc = descfn(j++))) {
    if (!label ||
	!strcmp(desc->LADSPA_Plugin->Label, label)) {
      descriptor = desc;
      break;
    }
  }

  if (!descriptor) {
    fprintf(stderr, 
	    "\n%s: Error: Plugin label \"%s\" not found in library \"%s\"\n",
	    my_name, label ? label : "(none)", dllName);
    return 1;
  }

  if (!descriptor->run_synth 
      && !descriptor->run_multiple_synths) {
    fprintf(stderr, "%s: Error: No run_synth() or run_multiple_synths() method in plugin\n", my_name);
    exit(1);
  }

  if (!label) {
    label = strdup(descriptor->LADSPA_Plugin->Label);
  }

  /* Count number of i/o buffers and ports required */
  ins = 0;
  outs = 0;
  controlIns = 0;
  controlOuts = 0;
 
  for (int j = 0; j < descriptor->LADSPA_Plugin->PortCount; j++) {
    LADSPA_PortDescriptor pod =
      descriptor->LADSPA_Plugin->PortDescriptors[j];

    if (LADSPA_IS_PORT_AUDIO(pod)) {
	    
      if (LADSPA_IS_PORT_INPUT(pod)) ++ins;
      else if (LADSPA_IS_PORT_OUTPUT(pod)) ++outs;
	    
    } else if (LADSPA_IS_PORT_CONTROL(pod)) {
	    
      if (LADSPA_IS_PORT_INPUT(pod)) ++controlIns;
      else if (LADSPA_IS_PORT_OUTPUT(pod)) ++controlOuts;
    }
  }

  if (!outs) {
    fprintf(stderr, "%s: Error: no audio output ports\n", my_name);
    exit(1);
  }
  if (nchannels == -1) {
    nchannels = outs;
  }

  /* Create buffers */

  pluginInputBuffers = (float **)malloc(ins * sizeof(float *));
  pluginControlIns = (float *)calloc(controlIns, sizeof(float));

  pluginOutputBuffers = (float **)malloc(outs * sizeof(float *));
  pluginControlOuts = (float *)calloc(controlOuts, sizeof(float));

  for (int i = 0; i < outs; i++) {
    pluginOutputBuffers[i] = NULL;
  }


  /* Instantiate plugin */

  instanceHandle = descriptor->LADSPA_Plugin->instantiate
    (descriptor->LADSPA_Plugin, sample_rate);
  if (!instanceHandle) {
    fprintf(stderr, 
	    "\n%s: Error: Failed to instantiate instance %d!, plugin \"%s\"\n",
	    my_name, 0, label);
    return 1;
  }

  /* Connect ports */
//this works a bit weird for some plugins...FIXME

  connect_ports();

  /* Set the control port values */

  if (src == from_preset) {
    /* Set the ports according to a preset */
    if (descriptor->select_program) {
      descriptor->select_program(instanceHandle, bank, program_no);
    }
  } else {
    /* Assign values to control ports: defaults, random, or from stdin */
    controlIn = 0;
    for (int j = 0; j < descriptor->LADSPA_Plugin->PortCount; j++) {  
      /* j is LADSPA port number */
      
      LADSPA_PortDescriptor pod =
	descriptor->LADSPA_Plugin->PortDescriptors[j];
      
      if (LADSPA_IS_PORT_CONTROL(pod) && LADSPA_IS_PORT_INPUT(pod)) {
	LADSPA_Data val;
	if (src == from_defaults) {
	  val = get_port_default(descriptor->LADSPA_Plugin, j);
	} else if (src == from_stdin) {
	  scanf("%f", &val);
	} else if (src == from_random) {
	  val = get_port_random(descriptor->LADSPA_Plugin, j);
	}
	pluginControlIns[controlIn] = val;
	controlIn++;
      }
    }
  }
	

  /* It can happen that a control port is set wrongly after
   * select_program(): for example xsynth-dssi does not set its tuning
   * port in the select_program() call (which makes sense: xsynth
   * users might want to be able to keep their current tuning while
   * changing presets).  Here, if we call select_program() we'll get
   * tuning = 0.0, and we don't get any sound. There might be other
   * bad effects in other cases.  One solution is to read all the
   * control-in values and if they're not in range, reset them using
   * get_default().
   */

  controlIn = 0;
  for (int j = 0; j < descriptor->LADSPA_Plugin->PortCount; j++) {  
    /* j is LADSPA port number */
      
    LADSPA_PortDescriptor pod =
      descriptor->LADSPA_Plugin->PortDescriptors[j];
    
    if (LADSPA_IS_PORT_CONTROL(pod) && LADSPA_IS_PORT_INPUT(pod)) {

      LADSPA_PortRangeHintDescriptor prhd =
	descriptor->LADSPA_Plugin->PortRangeHints[j].HintDescriptor;      
      const char * pname = descriptor->LADSPA_Plugin->PortNames[j];
      LADSPA_Data lb = descriptor->LADSPA_Plugin->
	PortRangeHints[j].LowerBound;
      LADSPA_Data ub = descriptor->LADSPA_Plugin->
	PortRangeHints[j].UpperBound;
      LADSPA_Data val = pluginControlIns[controlIn];
      LADSPA_Data def = get_port_default(descriptor->LADSPA_Plugin, j);

      if ((LADSPA_IS_HINT_BOUNDED_BELOW(prhd) && val < lb) ||
	  (LADSPA_IS_HINT_BOUNDED_ABOVE(prhd) && val > ub)) {
	fprintf(stderr, 
		"%s: Warning: port %d (%s) was %.3f, overriding to %.3f\n",
		my_name, j, pname, val, def);
	pluginControlIns[controlIn] = def;
      }
      if (DEBUG) {
	fprintf(stderr, "port %3d; prhd %3d; lb %6.2f; ub %6.2f; val %6.2f (%s)\n",
		j, prhd, lb, ub, val, pname);
      }
      controlIn++;
    }
  }

  /* Activate */

  if (descriptor->LADSPA_Plugin->activate) {
    descriptor->LADSPA_Plugin->activate(instanceHandle);
  }


  /* Configure */

  if (projectDirectory && descriptor->configure) {
    char *rv = descriptor->configure(instanceHandle,
				     DSSI_PROJECT_DIRECTORY_KEY,
				     projectDirectory);
    if (rv) {
      fprintf(stderr, 
	      "%s: Warning: plugin doesn't like project directory: \"%s\"\n",
	      my_name, rv);
    }
  }
  if (nkeys && descriptor->configure) {
    for (int i = 0; i < nkeys; i++) {
      char *rv = descriptor->configure(instanceHandle,
				       configure_key[i],
				       configure_val[i]);
      if (rv) {
	fprintf(stderr, 
		"%s: Warning: plugin doesn't like "
		"configure key-value pair: \"%s\"\n", 
		my_name, rv);
    exit(1);
      }
    }
  }

  /* Open sndfile */

  outsfinfo.samplerate = sample_rate;
  outsfinfo.channels = nchannels;
  outsfinfo.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
  outsfinfo.frames = length;
    
  outfile = sf_open(output_file, SFM_WRITE, &outsfinfo);
  if (!outfile) {
    fprintf(stderr, "%s: Error: Not able to open output file %s.\n", 
	    my_name, output_file);
    fprintf(stderr, "%s: %s\n", my_name, sf_strerror(outfile));
    return 1;
  }

  ladspa_run_synth();  

    
  sf_close(outfile);
    
  /* Clean up */

  if (descriptor->LADSPA_Plugin->deactivate) {
    descriptor->LADSPA_Plugin->deactivate(instanceHandle);
  }
  
  if (descriptor->LADSPA_Plugin->cleanup) {
    descriptor->LADSPA_Plugin->cleanup(instanceHandle);
  }

  return 0;
}
