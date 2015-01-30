#include "midi_loader.h"
int main(void){
 char *filename = "example.mid";
 load_midi_file(filename, 44100, NULL, NULL); //do you think this will all compile? hope so
}
