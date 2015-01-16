#include "fluidsynth_priv.h"
#include "fluid_midi.h"
#include <sndfile.h>
#include <string.h>
#include <ladspa.h>
#include <dssi.h>
  
typedef struct event_table_t{
    snd_seq_event_t *events;
    int length;
    size_t last_nframe;
} event_table_t;
typedef struct read_midi_ctx_t {
    fluid_player_t  *player;
    fluid_track_t   *track;
} read_midi_ctx_t;

event_table_t *event_table;

int
fluid_log(int level, const char* fmt, ...) 
{

}
//{{{OLD
//fluid_midi_event_t *
//new_fluid_midi_event ()
//{
//    fluid_midi_event_t* evt; //oh it's just the name
//    evt = FLUID_NEW(fluid_midi_event_t);
//    if (evt == NULL) {
//        FLUID_LOG(FLUID_ERR, "Out of memory");
//        return NULL;
//    }
//    evt->dtime = 0;
//    evt->type = 0;
//    evt->channel = 0;
//    evt->param1 = 0;
//    evt->param2 = 0;
//    evt->next = NULL; //so this looks like it's set up to be a linked list, right? each event contains pointer to the next yes
//    evt->paramptr = NULL;
//    return evt;
//}
//}}}

void insert_event(event_table_t *event_table, snd_seq_event_t *event){
  event_table->events = realloc(event_table->events, (event_table->length + 1) * sizeof(snd_seq_event_t));
  memcpy(&event_table->events[event_table->length], event, sizeof(snd_seq_event_t));
  event_table->length += 1;
}
void delete_event(event_table_t *event_table, snd_seq_event_t *event){
  size_t i;
  for (i=0; i< event_table->length; i++){
    if(compare_events(&event_table->events[i], event)){ 
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
      memcpy(&event_table->events[i], event, sizeof(snd_seq_event_t)); 
    }
  }
}

//{{{ type conversion stuff
//struct _fluid_midi_event_t {
//  fluid_midi_event_t* next; /* Link to next event */ 
//  void *paramptr;           /* Pointer parameter (for SYSEX data), size is stored to param1, param2 indicates if pointer should be freed (dynamic if TRUE) */
//  unsigned int dtime;       /* Delay (ticks) between this and previous event. midi tracks. */
//  unsigned int param1;      /* First parameter */ DONE
//  unsigned int param2;      /* Second parameter */ DONE
//  unsigned char type;       /* MIDI event type */ DONE
//  unsigned char channel;    /* MIDI channel */ DONE
//};
//  on_event.type = SND_SEQ_EVENT_NOTEON;
//  on_event.data.note.channel = 0; // this one  oh ok, so we save this data, and don't worry about it until we get to run_synth? looks so ok perfect
//  on_event.data.note.note = midi_note;
//  on_event.data.note.velocity = midi_velocity;
//  on_event.time.tick = 0;
//  }}}
void convert_event_format(fluid_midi_event_t *from, snd_seq_event_t *to){
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
  to->data.note.channel = from->channel; 
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
  printf("event_type: %s", note_event);
  printf("channel: %d ", event->data.note.channel);
  printf("note: %d ", event->data.note.note);
  printf("velocity: %d ", event->data.note.velocity);
  printf("\n");
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
  event_table->last_nframe = (player->deltatime * track->ticks) * 44100 / 1000;
  size_t nframes = event_table->last_nframe - last_nframe;
  //ok so I have no idea how to get the linker to work when we put this code into cli-dssi-host, should we look at that next? sure
  //FIXME msec
  /*printf("track=%02d msec=%05d ticks=%05u dtime=%05u next=%05u type=Ox%x\n", 
         track->num, 
         track->ticks * player->deltatime,
         track->ticks, 
         event->dtime, 
         track->ticks + event->dtime,
         event->type); */
  
  convert_event_format(event, &seq_event);
//  print_snd_seq_event(&seq_event);


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
  printf("run_synth(instancehandle, %u,\n", nframes);
  print_event_table(event_table);  
  printf(", %u)\n", event_table->length);
//weird, negative numbers type problem...is it because we need to print as %u? or something similar? might be  
//ok so it's the event table that we pass in, right? yes
//      descriptor->run_synth(instanceHandle, //instance handle is taken care of for us, current_event is our event_table, right? event_table->events
//			    player->msec,  //like this? not really
//			    event_table->events,
//			    event_table->length);
//
//			    so with nfraes, we need to track how many samples we should generate from last get_events call. so it should be something like:
// ok something like this wow, ok, do we need print this now too, or should we try hooking it up? let's print it first
}


//{{{    mrswatson time calculation
//    case TIME_DIVISION_TYPE_TICKS_PER_BEAT: {
//            double ticksPerSecond = (double)timeDivision * getTempo() / 60.0;
//            double sampleFramesPerTick = getSampleRate() / ticksPerSecond;
//            currentTimeInSampleFrames += (long)(unpackedVariableLength * sampleFramesPerTick);
//        }
//}}}


void read_midi_file2(char *filename){
  int i;
  fluid_player_t *player;
  fluid_playlist_item playlist_item;
  read_midi_ctx_t ctx;

  event_table = malloc(sizeof (event_table_t));
  event_table->events = NULL;
  event_table->length = 0;
  event_table->last_nframe = 0;

  playlist_item.filename = filename;
 // 
 // player = (fluid_player_t *)new_fluid_player();
  player = FLUID_NEW(fluid_player_t);
  player->playback_callback = &get_events; //how do I pass it to the callback? no way to do that, we can only pass one variable as "data"
  player->playback_userdata = (void *)&ctx; // here. so i guess need to make a structure and put player, track pointers into it.
  ctx.player = player;
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


int main(void){
 char *filename = "example.mid";
 read_midi_file2(filename);
}
