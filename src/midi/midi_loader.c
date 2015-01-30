#include "midi_loader.h"  
#define DEBUG 0
event_table_t *event_table;

int
fluid_log(int level, const char* fmt, ...) 
{

}

void insert_event(event_table_t *event_table, snd_seq_event_t *event){
  event_table->events = realloc(event_table->events, (event_table->length + 1) * sizeof(snd_seq_event_t));
  memcpy(&event_table->events[event_table->length], event, sizeof(snd_seq_event_t));
  event_table->length += 1;
}
void delete_event(event_table_t *event_table, snd_seq_event_t *event){
  size_t i;
  for (i=0; i< event_table->length; i++){
    if(compare_events(&event_table->events[i], event)){ 
      if (DEBUG){
        printf("removed_event\n");
      }
      memcpy(&event_table->events[i], &event_table->events[i+1], sizeof(snd_seq_event_t)*(event_table->length - i -1));
      event_table->events = realloc(event_table->events, event_table->length * sizeof(snd_seq_event_t));
      event_table->length--;
      i--;
    }
  }
}
void delete_note_off_events(event_table_t *event_table){
  size_t i;
  for (i=0; i< event_table->length; i++){
    if(event_table->events[i].type == SND_SEQ_EVENT_NOTEOFF){ 
      if(DEBUG){
        printf("removed_note_off_event\n");
      }
      memcpy(&event_table->events[i], &event_table->events[i+1], sizeof(snd_seq_event_t)*(event_table->length - i -1));
      event_table->events = realloc(event_table->events, event_table->length * sizeof(snd_seq_event_t));
      event_table->length--;
      i--;
    }
  }
}

int compare_events(snd_seq_event_t *event1, snd_seq_event_t *event2){
  return (
    (event1->data.note.note    == event2->data.note.note) && 
    (event1->data.note.channel == event2->data.note.channel)
  ) ? 1 : 0;
}


void replace_events(event_table_t *event_table, snd_seq_event_t *event){
  size_t i;
  for (i=0; i< event_table->length; i++){
    if(compare_events(&event_table->events[i], event)){ 
      if (DEBUG){
        printf("replaced_event\n");
      }
      memcpy(&event_table->events[i], event, sizeof(snd_seq_event_t)); 
    }
  }
}

void convert_event_format(fluid_midi_event_t *from, snd_seq_event_t *to){
  memset(to, 0, sizeof(snd_seq_event_t));
//{{{ from->type
   switch(from->type){

    case NOTE_ON: 
      to->type = SND_SEQ_EVENT_NOTEON; 
      to->data.note.note = from->param1;
      to->data.note.velocity = from->param2;
      break;
    case NOTE_OFF: 
      to->type = SND_SEQ_EVENT_NOTEOFF; 
      to->data.note.note = from->param1;
      to->data.note.off_velocity = from->param2; 
      break; 
    /*case FLUID_NONE: to->type = SND_SEQ_EVENT_SYSTEM; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_RESULT; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_NOTE; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_KEYPRESS; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_CONTROLLER; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_PGMCHANGE; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_CHANPRESS; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_PITCHBEND; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_CONTROL14; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_NONREGPARAM; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_REGPARAM; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_SONGPOS; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_SONGSEL; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_QFRAME; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_TIMESIGN; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_KEYSIGN; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_START; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_CONTINUE; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_STOP; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_SETPOS_TICK; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_SETPOS_TIME; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_TEMPO; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_CLOCK; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_TICK; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_QUEUE_SKEW; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_SYNC_POS; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_TUNE_REQUEST; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_RESET; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_SENSING; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_ECHO; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_OSS; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_CLIENT_START; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_CLIENT_EXIT; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_CLIENT_CHANGE; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_PORT_START; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_PORT_EXIT; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_PORT_CHANGE; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_PORT_SUBSCRIBED; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_PORT_UNSUBSCRIBED; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_USR0; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_USR1; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_USR2; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_USR3; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_USR4; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_USR5; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_USR6; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_USR7; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_USR8; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_USR9; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_SYSEX; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_BOUNCE; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_USR_VAR0; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_USR_VAR1; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_USR_VAR2; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_USR_VAR3; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_USR_VAR4; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_NONE; break; one of those constants, we can have a switch in print function to clearly show what event is it*/
   }
//}}}
  //to->data.note.channel = from->channel; 
  to->data.note.channel = 0; // FIXME force channel
  to->time.tick = 0;
}



void print_snd_seq_event(snd_seq_event_t *event){
  char note_event[20];
  switch(event->type){

    case SND_SEQ_EVENT_NOTEON:
      strcpy(note_event,"NOTE_ON"); 
      break;
    case SND_SEQ_EVENT_NOTEOFF:
      strcpy(note_event,"NOTE_OFF");
      break;
    break;
  }
  if (DEBUG){
    printf("event_type: %s", note_event);
    printf("channel: %d ", event->data.note.channel);
    printf("note: %d ", event->data.note.note);
    printf("velocity: %d ", event->data.note.velocity);
    printf("tick: %d ", event->time.tick);
    printf("\n");
  }
}


void print_event_table (event_table_t *event_table){
  
  unsigned int i;
  for(i=0; i< event_table->length; i++){ 
    printf(" - %d: ", i + 1);
    print_snd_seq_event(&event_table->events[i]); 

  }
  printf("--\n");
}




int get_events(void *data, fluid_midi_event_t *event){
  read_midi_ctx_t *ctx = (read_midi_ctx_t *)data;
  fluid_player_t *player = ctx->player;
  fluid_track_t *track = ctx->track;
  snd_seq_event_t seq_event; 

  size_t last_nframe = event_table->last_nframe;
  event_table->last_nframe = (player->deltatime * track->ticks) * 44100 / 1000; // FIXME 44100 to ctx->samplerate
  event_table->nframes_since_last = event_table->last_nframe - last_nframe;

  convert_event_format(event, &seq_event);
  //print_snd_seq_event(&seq_event);
  
  read_midi_callback cb = ctx->callback;
  if(cb){
    //size_t i;
  //  for(i = 0; i < event_table->nframes_since_last / 256; i++){ // FIXME //it could be something weird somewhere in this script though, right? 
//some part that we haven't been looking at at all... well yeah, i think of setting this to run whole nframes, and compiling hexter with debug symbols to check it again in valgrind. what is segfaulting...we don't know which part, right? yeah could it be something like an old printf statement somewhere in the code from cli-dssi-host? hm it actually can be code from cli-dssi which is initalizing plugin for only 256 frames and we are trying to run more inside.. yeah that seems right it saves it in "descriptor" right? yeah some buffer there
      cb(event_table, ctx->callback_userdata);
    //}
  }
 // so i fixed logic a bit, should work now oh ok I have the new file 
  delete_note_off_events(event_table);  

  switch(event->type){ 
    case NOTE_ON:
      insert_event(event_table, &seq_event); 
      break;
    case NOTE_OFF:
      replace_events(event_table, &seq_event);     
      break;
    default:
      break;
  }
  
  if (DEBUG){
    printf("event table last nframe: %u\n", event_table->last_nframe);
    printf("run_synth(instancehandle, %u,\n", event_table->nframes_since_last);
    print_event_table(event_table);  
    printf(", %u)\n", event_table->length);
  }
}




void load_midi_file(char *filename, read_midi_callback callback, void *callback_userdata){  //like that?
  int i;
  fluid_player_t *player;
  fluid_playlist_item playlist_item;
  read_midi_ctx_t ctx;

  event_table = malloc(sizeof (event_table_t));
  event_table->events = NULL;
  event_table->length = 0;
  event_table->last_nframe = 0; 
  event_table->nframes_since_last = 0;
//where is this calculated? 
  playlist_item.filename = filename;
 // 
 // so the only arg to the callback is ctx, right? yes
 // player = (fluid_player_t *)new_fluid_player();
  player = (fluid_player_t *)new_fluid_player();
  player->playback_callback = &get_events; //how do I pass it to the callback? no way to do that, we can only pass one variable as "data"
  player->playback_userdata = (void *)&ctx; // here. so i guess need to make a structure and put player, track pointers into it.
  ctx.player = player;
  ctx.callback = callback;
  ctx.callback_userdata = callback_userdata;
  fluid_player_load(player, &playlist_item);

//
//
  for(i = 0; i < player->ntracks; i++){
     ctx.track = player->track[i];// here. so i guess need to make a structure and put player, track pointers into it.
     fluid_track_send_events(player->track[i], player->synth, player, (unsigned int)-1);
  }

  delete_fluid_player(player);
}
//
//should I grab the run_synth call from the other program just so that we know what it requires? yep
//
// something like this, need to fix it a bit ok I'll load example.mid, should we try it before taking a break from this? well it won't compile i think, get_events should be consistent with callback prototype
//
//EXPLANATION:
//LADSPA_Handle instanceHandle;
//instanceHandle = descriptor->LADSPA_Plugin->instantiate
//
//
//
//int nframes (number of frames)

//  current_event is pointer to on/off event
//  snd_seq_event_t on_event, off_event, *current_event;
//  on_event.type = SND_SEQ_EVENT_NOTEON;
//  on_event.data.note.channel = 0; // this one  oh ok, so we save this data, and don't worry about it until we get to run_synth? looks so ok perfect
//  on_event.data.note.note = midi_note;
//  on_event.data.note.velocity = midi_velocity;
//  on_event.time.tick = 0;
//
//  off_event.type = SND_SEQ_EVENT_NOTEOFF;
//  off_event.data.note.channel = 0;
//  off_event.data.note.note = midi_note;
//  off_event.data.note.off_velocity = midi_velocity; // yeah off_velocity instead
//  off_event.time.tick = 0;


//weird, negative numbers type problem...is it because we need to print as %u? or something similar? might be  
//ok so it's the event table that we pass in, right? yes
//      descriptor->run_synth(instanceHandle, //instance handle is taken care of for us, current_event is our event_table, right? event_table->events
//			    player->msec,  //like this? not really
//			    event_table->events,
//			    event_table->length);
//
//			    so with nfraes, we need to track how many samples we should generate from last get_events call. so it should be something like:
// ok something like this wow, ok, do we need print this now too, or should we try hooking it up? let's print it first

//only thing is, I'm not sure what they mean by start of the block, do they mean start of the track? not sure, i guess to start of events we send to plugin one more time to check this note_on\off
//     * The Events pointer points to a block of EventCount ALSA
//     * sequencer events, which is used to communicate MIDI and related
//     * events to the synth.  Each event is timestamped relative to the
//     * start of the block, (mis)using the ALSA "tick time" field as a
//     * frame count. The host is responsible for ensuring that events
//     * with differing timestamps are already ordered by time.
  //FIXME msec
  /*printf("track=%02d msec=%05d ticks=%05u dtime=%05u next=%05u type=Ox%x\n", 
         track->num, 
         track->ticks * player->deltatime,
         track->ticks, 
         event->dtime, 
         track->ticks + event->dtime,
//ok so do we still have an issue with the ticks calculation? yeah but anyway there is something weird with note generation, if i put event on_event from original code, there is something in output file, if i put our events it's silence
         event->type); */
//{{{    mrswatson time calculation
//    case TIME_DIVISION_TYPE_TICKS_PER_BEAT: {
//            double ticksPerSecond = (double)timeDivision * getTempo() / 60.0;
//            double sampleFramesPerTick = getSampleRate() / ticksPerSecond;
//            currentTimeInSampleFrames += (long)(unpackedVariableLength * sampleFramesPerTick); //so do we need the same calculation, currentTimeinFrames? or do we need deltaframes? what are the units we need exactly? basically need timestamp in msec for each event
//        }
//}}}

/*

get_events original
int get_events(void *data, fluid_midi_event_t *event){
  read_midi_ctx_t *ctx = (read_midi_ctx_t *)data;
  fluid_player_t *player = ctx->player;
  fluid_track_t *track = ctx->track;
  snd_seq_event_t seq_event; 

  size_t last_nframe = event_table->last_nframe;
  event_table->last_nframe = (player->deltatime * track->ticks) * 44100 / 1000; // FIXME 44100 to ctx->samplerate
  event_table->nframes_since_last = event_table->last_nframe - last_nframe;

  
  convert_event_format(event, &seq_event);
  print_snd_seq_event(&seq_event);

  read_midi_callback cb = ctx->callback;
  if(cb){
    size_t i;
    for(i = 0; i < event_table->nframes_since_last; i++){
      cb(event_table, ctx->callback_userdata);
    }
  }

  switch(event->type){ 
    case NOTE_ON:
      insert_event(event_table, &seq_event); 
      break;
    case NOTE_OFF:
      delete_event(event_table, &seq_event);
      break;
    default:
      break;
  }
  printf("event table last nframe: %u\n", event_table->last_nframe);
  printf("run_synth(instancehandle, %u,\n", event_table->nframes_since_last);
  print_event_table(event_table);  
  printf(", %u)\n", event_table->length);

}
*/
