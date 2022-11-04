// Here is the code that will run the piano instrument of the synthesizer
// It doesn't use threading... I'm counting on it being able to generate
// samples quickly and that users won't notice a lag of 1 period.
//
//
// AGL - MAR 8
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <complex.h>
#include <unistd.h>
#include <alsa/asoundlib.h>
#include <pthread.h>
#include "piano_voice.h"
#include "synth.h"


#define SAMPLING_FREQ 44100		// sampling frequency (samp/sec)
#define FRAMES	24			// frames in a period
#define CHANNELS 8			// channels (concurrent notes)
#define DURATION 3			// length of a note (seconds)
#define SB_DL	8			// number of soundboard delay lines
#define FADE_LENGTH 500			// Length of fade out of note

// Function prototypes
static void *keyboard_input(void * args);
int sample_create_add(struct sample_create_s* , int, struct note_s *, int, int);
int sample_create_remove(struct sample_create_s *, int, int);
void sample_create(struct sample_create_s *, int);
struct sound_board_s sound_board_mix(struct sound_board_s, struct sample_create_s *, double *);
int soundcard_mix(snd_pcm_t *, double*);

// threads
static void *keyboard_input(void * args);
//static void *keep_alive(void * args);


// Main ---------------------------------------------------------------

int main(int argc, char * argv[]){
	
// define general Variables --- 
	int rc, i, j, k;
	int sampling_freq = SAMPLING_FREQ;	// sampling rate
	snd_pcm_t * handle;			// Soundcard Handle
	snd_pcm_uframes_t frames = FRAMES;	// Frames per sample
	int piano_on = 1;			// Switch for piano on/off
	int input_flags[3] = {0,0,0};		// Flags for keyboard input
	pthread_t input_get;			// thread for non-blocking input
	pthread_t keep_alive_t;			// for keeping piano going
	int sc_new = 0, sc_old = 0;		// Indexes for sample creation
	int note_number;	                // note number
	int sc_flag = 0;			// flag for if sample creartors are active
	

// Initialize stuff --- 
	int input_method;
	if(argc < 2){			// Input method control
		input_method = 0;	// could be more efficient
	} else {
		input_method = atoi(argv[1]);
	}
	input_flags[2] = input_method;

	struct sound_board_s sound_board;		// Sound board
	sound_board = sound_board_init(); 	
	
	
	struct note_s notes[88];			// Notes
	note_init(&notes[0]);		
	
	rc = snd_pcm_open(&handle, "default", SND_PCM_STREAM_PLAYBACK, 0);	// Sound card
	if(rc < 0) {
		fprintf(stderr, "Unable to open the default PCM Device: %s\n" ,snd_strerror(rc));
		exit(1);
	}
	rc = soundcard_init(44100, frames, handle); if (rc != 0) exit(1);
	
	
	// init the sw params
	rc = alsa_sw_init(handle);
	if(rc != 0){
		fprintf(stderr,"failed setting sw params\n");
		exit(1);
	}
	
	
	double channel_buff[CHANNELS][2*FRAMES];	// buffer for samples
	struct sample_create_s samp_create[CHANNELS];	// creator of samples
	
	for(i=0;i<CHANNELS;i++){			// link them & init 
		samp_create[i].channel = &channel_buff[i][0];
		samp_create[i].active_flag = 0;
		samp_create[i].cycle_count = 0;
	}	
	
	double output[2*FRAMES]; 			// Output


// Start threads
	rc = pthread_create(&input_get, NULL, keyboard_input, &input_flags[0]); // Input threads
	if(rc != 0){
		fprintf(stderr, "cant make keyboard thread go ...\n"); exit(1);
	}

	
// synth loop 	
	i=0; k = 0;	
	int velocity;
	int min_cycle_count;
	int min_cycle_num;

	while(piano_on){
		
	// Nonblocking keyboard input		
		if(input_flags[1] !=  0){
		// set flag
			input_flags[0] = 1;
			
		// decode note number
			note_number = (0xFF & input_flags[1]) - 21;  
			if((0xFF00 & input_flags[1]) != 0){ // Key press
				if(note_number >= 0 && note_number < 71){
				// choose new input
					min_cycle_count = DURATION*SAMPLING_FREQ;
					for(i=0;i<CHANNELS;i++){
						if(samp_create[i].active_flag == 0) break;
						if(samp_create[i].cycle_count < min_cycle_count){
							min_cycle_count = samp_create[i].cycle_count;
							min_cycle_num = i;
						}		
					}

					velocity = (0xFF00&input_flags[1]) >> 8;
					
					if(i==8){
						sc_new = sample_create_add(&samp_create[0], min_cycle_num, &notes[0], note_number, velocity);
						samp_create[i].note = notes[note_number];
					} else {
						sc_new = sample_create_add(&samp_create[0], i, &notes[0], note_number, velocity);
						samp_create[i].note = notes[note_number];
					}
				}
				input_flags[1] = 0;
				
				
			} else {	// Key release
				
				for(i=0;i<CHANNELS;i++){
					if(samp_create[i].note_num == note_number) {
						samp_create[i].cycle_count = FADE_LENGTH;
						break;
					}
				}
			
				input_flags[1] = 0;
			}
			
			
		}
	
	// reset active sample_creator flag	
		sc_flag = 0; 
	
	// Sample Creation Loop		
		for(i=0;i<CHANNELS;i++){		
			if(samp_create[i].active_flag == 1){
				if(samp_create[i].cycle_count == 0){
					sc_old = sample_create_remove(&samp_create[0], sc_old, i);
				
				} else {
                                        
					sample_create(samp_create, i);  // create a period of samples;
					samp_create[i].cycle_count --;
					
					sc_flag = 1;
				
				}
			}
	
		}

	// Check for active notes
		if(sc_flag == 0){
			snd_pcm_prepare;
		} else {
		// Soundboard the samples
			sound_board = sound_board_mix(sound_board, &samp_create[0], &output[0]);
			
		// Output the samples
			rc = soundcard_output(handle, &output[0]);
		}
	}

        return 0;
}


// Functions:

void sample_create(struct sample_create_s *sc, int current){
	int i,j,k;
	double vinh = 0, v = 0, L = 0, in, in_old;


	for(k=0;k<FRAMES*sc[current].f_sc;k++){ // Generate 1 full period
		in = 0;

		// First two samples of first cycle do this:
		if(sc[current].cycle_count == DURATION*SAMPLING_FREQ/FRAMES){
			if(sc[current].note.M > 1){
				if(k<1) sc[current].vh[0] = sc[current].Vi/2;
			} else {
				if(k<1) sc[current].vh[0] = sc[current].Vi;
			}
		}

		for(i=0;i<sc[current].note.Ns;i++){  // Velocity of string
			sc[current].vin_up[i][0] = sc[current].delay_up[i][sc[current].dup_min1];

			sc[current].vin_up[i][1] = (sc[current].delay_up[i][sc[current].dup_min1]
						 +  sc[current].delay_up[i][sc[current].dup_min2])/2;
			
			sc[current].vin_low[i][0] = sc[current].delay_low[i][sc[current].dlow_min1[i]];
			
			
			sc[current].vin_low[i][1] = (sc[current].delay_low[i][sc[current].dlow_min1[i]]
						  + sc[current].delay_low[i][sc[current].dlow_min2[i]])/2;
			
			
		}
		
		
		if(sc[current].hammer == 0){    // Calculate hammer force:
			for(i=0;i<2;i++){
				
				 // Calculate string input velocity
				 vinh = 0;
				for(j=0;j<sc[current].note.Ns;j++){
					vinh += sc[current].vin_low[j][i] + sc[current].vin_up[j][i];
				}
			   
				 // Calclate velocity of felt
				v=sc[current].vh[0] - vinh - sc[current].Fout[0]/(2*sc[current].note.Ns*sc[current].note.string.Z);
			
				// Calclate del_y
				sc[current].del_y[0] = v/sc[current].note.hammer.fsh + sc[current].del_y[1];
				if(sc[current].del_y[0] < 0) sc[current].del_y[0] = 0;

				// update Fout
				sc[current].Fout[1] = sc[current].Fout[0];

				// Calculate output force of hammer
				sc[current].Fout[0] = sc[current].note.hammer.K 
					* pow(sc[current].del_y[0],sc[current].note.hammer.p);

				// update some variables
				sc[current].vh[1]    = sc[current].vh[0];
				sc[current].del_y[1] = sc[current].del_y[0];
				
				// Recalculate Hammer Velocity
				sc[current].vh[0] = sc[current].vh[1]-sc[current].Fout[1]
							/(sc[current].note.hammer.fsh * sc[current].note.hammer.m);

			}
		}

		L = (sc[current].Fout[0] + sc[current].Fout[1])/2.0;
		if(L < 0.003){
			if(sc[current].note.M > 1) sc[current].hammer = 1;
		}
		
		for(i=0;i<sc[current].note.Ns;i++){
			L += 2*sc[current].note.string.Z*((sc[current].vin_up[i][0] + sc[current].vin_up[i][1])/2 
				+ (sc[current].vin_low[i][0] + sc[current].vin_low[i][1])/2);

		}

		// Update the String Velocities
		for(i=0;i<sc[current].note.Ns;i++){
			sc[current].delay_up[i][sc[current].dup_min1] = 
				L/(2*sc[current].note.Ns*sc[current].note.string.Z) - (sc[current].vin_low[i][0] + sc[current].vin_low[i][1])/2;

			sc[current].delay_low[i][sc[current].dlow_min1[i]] =
				L/(2*sc[current].note.Ns*sc[current].note.string.Z) - (sc[current].vin_up[i][0] + sc[current].vin_up[i][1])/2;
		
		}

		// Filter Strings
		for(i=0;i<sc[current].note.Ns;i++){
			sc[current].temp[i][0][0] = sc[current].delay_up[i][sc[current].dup_mup];

			sc[current].temp[i][1][0] = sc[current].delay_low[i][sc[current].dlow_mlow[i]];

			// Allpass and LPF
			sc[current].temp[i][2][0] = sc[current].temp[i][0][0]*sc[current].note.LP[i].glp 
				- sc[current].temp[i][2][1]*sc[current].note.LP[i].a1;

			for(j=3;j<(3+sc[current].note.M);j++){
				sc[current].temp[i][j][0] = sc[current].note.allpass.a2 * sc[current].temp[i][j-1][0]
							  + sc[current].note.allpass.a1 * sc[current].temp[i][j-1][1]
							  + sc[current].temp[i][j-1][2]
							  - sc[current].note.allpass.a1 * sc[current].temp[i][j][1]
							  - sc[current].note.allpass.a2 * sc[current].temp[i][j][2] ;
			}
			sc[current].temp[i][7][0] = sc[current].note.tunning[i].a1 * sc[current].temp[i][j-1][0] 
						  + sc[current].temp[i][j-1][1]
						  - sc[current].note.tunning[i].a1 * sc[current].temp[i][7][1];

			
			in += (sc[current].delay_up[i][sc[current].dup_mup] * sc[current].note.string.Ref) 
				* sc[current].note.control.scale;
			
			
			sc[current].delay_up[i][sc[current].dup_mup] = -1.0 * sc[current].temp[i][1][0];
			
			sc[current].delay_low[i][sc[current].dlow_mlow[i]] = sc[current].temp[i][7][0]*-sc[current].note.string.tau;
			
			sc[current].temp[i][7][1] = sc[current].temp[i][7][0];

			for(j=1;j<3;j++){
				sc[current].temp[i][2][3-j] = sc[current].temp[i][2][2-j];
				sc[current].temp[i][3][3-j] = sc[current].temp[i][3][2-j];
				sc[current].temp[i][4][3-j] = sc[current].temp[i][4][2-j];
				sc[current].temp[i][5][3-j] = sc[current].temp[i][5][2-j];
				sc[current].temp[i][6][3-j] = sc[current].temp[i][6][2-j];
			}

			 
		}
		
		
		// DEBUG CODE:
		if(in != 0){
			if(sc[current].debug_flag[0] == 0){
				gettimeofday(&sc[current].tv[1], NULL);
				sc[current].t[0] = (double) sc[current].tv[0].tv_sec + (double) sc[current].tv[0].tv_usec/1000000.0;
				sc[current].t[1] = (double) sc[current].tv[1].tv_sec + (double) sc[current].tv[1].tv_usec/1000000.0;
				sc[current].debug_flag[0] = 1;
			}
		}
		
		
		if(sc[current].cycle_count < FADE_LENGTH){
			in *= (double) (sc[current].cycle_count) / (double) FADE_LENGTH;
			
		}
		
		
		switch((int)sc[current].f_sc){	
			case(0):
				sc[current].channel[2*k]     = in;
				sc[current].channel[2*k + 1] = in;
				break;
		
			case(1):
				sc[current].channel[k] = in;
				break;
			
			case(2):
				if((k+1) % 2 == 0){
					sc[current].channel[(k-1)/2] = (in + in_old)/2.0;
				} else {
					in_old = in;
				}
		}

		
		sc[current].dup_mup--;  if(sc[current].dup_mup  < 0) sc[current].dup_mup  = sc[current].note.string.Mup-1;
		sc[current].dup_min1--; if(sc[current].dup_min1 < 0) sc[current].dup_min1 = sc[current].note.string.Mup-1;
		sc[current].dup_min2--;	if(sc[current].dup_min2 < 0) sc[current].dup_min2 = sc[current].note.string.Mup-1;
		sc[current].dup_mzer--;	if(sc[current].dup_mzer < 0) sc[current].dup_mzer = sc[current].note.string.Mup-1;
		for(j=0;j<sc[current].note.Ns;j++){
			sc[current].dlow_mzer[j]--; if(sc[current].dlow_mzer[j] < 0) sc[current].dlow_mzer[j] = sc[current].note.string.Mlow[j] - 1;
			sc[current].dlow_min1[j]--; if(sc[current].dlow_min1[j] < 0) sc[current].dlow_min1[j] = sc[current].note.string.Mlow[j] - 1;
			sc[current].dlow_min2[j]--; if(sc[current].dlow_min2[j] < 0) sc[current].dlow_min2[j] = sc[current].note.string.Mlow[j] - 1;
			sc[current].dlow_mlow[j]--; if(sc[current].dlow_mlow[j] < 0) sc[current].dlow_mlow[j] = sc[current].note.string.Mlow[j] - 1;
		}
		
	}

}

int sample_create_add(struct sample_create_s *sample_create, int  new, struct note_s * notes, int note_num, int velocity){
	// Activates a new sample creator
	int i, j;

	// initialize the sample_create structure		
	sample_create[new].active_flag = 1;	// set active
	sample_create[new].cycle_count = DURATION*SAMPLING_FREQ/FRAMES;
	sample_create[new].note = notes[note_num];
	sample_create[new].f_sc = sample_create[new].note.string.fs / (double) SAMPLING_FREQ;
	sample_create[new].note_num = note_num;
	
	// Input Velocity Calculation
	double v_input = (double) velocity/ (double) 0xFF;
	sample_create[new].Vi = (notes[note_num].control.MaxV - notes[note_num].control.MinV) * v_input + notes[note_num].control.MinV;

	// Allocate memory for delay lines and 0 all the vairables
	sample_create[new].vh[0]        = 0;
	sample_create[new].vh[0]        = 0;
	sample_create[new].hammer       = 0;
	sample_create[new].del_y[0]     = 0;
	sample_create[new].del_y[1]     = 0;
	sample_create[new].Fout[0]      = 0;
	sample_create[new].Fout[1]      = 0;
	for(i=0;i<sample_create[new].note.Ns;i++){
		// allocate memory for upper delay line
		sample_create[new].delay_up[i] = malloc(sizeof(double)*sample_create[new].note.string.Mup);
		if(sample_create[new].delay_up[i] == 0){
			printf("Could't allocate memory for upper delay line %d - %d\n",new, i);
			printf("This error isn't handled, prepare for seg-fault\n");
		}
		// 0 out the upper delay line
		for(j=0;j<sample_create[new].note.string.Mup;j++){
			sample_create[new].delay_up[i][j] = 0;
		}

		// allocate memory for lower delay line
		sample_create[new].delay_low[i] = malloc(sizeof(double)*sample_create[new].note.string.Mlow[i]);
		if(sample_create[new].delay_low[i] == 0){
			printf("Could't allocate memory for lower delay line %d - %d\n",new, i);
			printf("This error isn't handled, prepare for seg-fault\n");
		}
		// 0 out the lower delay line
		for(j=0;j<sample_create[new].note.string.Mlow[i];j++){
			sample_create[new].delay_low[i][j] = 0;
		}
		
		// 0 all other variables
		sample_create[new].vin_up[i][0] = 0;
		sample_create[new].vin_up[i][1] = 0;
		sample_create[new].vin_low[i][0] = 0;
		sample_create[new].vin_low[i][1] = 0;
		sample_create[new].temp[i][0][i] = 0;
		sample_create[new].temp[i][1][i] = 0;
		sample_create[new].temp[i][2][i] = 0;
		sample_create[new].temp[i][3][i] = 0;
		sample_create[new].temp[i][4][i] = 0;
		sample_create[new].temp[i][5][i] = 0;
		sample_create[new].temp[i][6][i] = 0;
		sample_create[new].temp[i][7][i] = 0;
		
	}
	
	sample_create[new].dup_mup  = sample_create[new].note.string.Mup - 1;
	sample_create[new].dup_min1 = sample_create[new].note.string.Min - 1;
	sample_create[new].dup_min2 = sample_create[new].note.string.Min - 2;
	sample_create[new].dup_mzer = 0;
	
	for(j=0;j<sample_create[new].note.Ns;j++){
		sample_create[new].dlow_mzer[j] = 0;
		sample_create[new].dlow_min1[j] = sample_create[new].note.string.Mlow[j] - sample_create[new].note.string.Min - 1;
		sample_create[new].dlow_min2[j] = sample_create[new].note.string.Mlow[j] - sample_create[new].note.string.Min - 2;
		sample_create[new].dlow_mlow[j] = sample_create[new].note.string.Mlow[j] - 1;
		
		
	}

	//DEBUG CODE:
	gettimeofday(&sample_create[new].tv[0], NULL);
	sample_create[new].debug_flag[0] = 0;
	sample_create[new].debug_flag[1] = 0;



	new ++;				// Choose which channel to use next
	if(new == CHANNELS) new = 0;
	return new;
}

int sample_create_remove(struct sample_create_s * sc, int old, int i){
	int j;
	
	sc[i].active_flag = 0;
	for(j=0;j<sc[i].note.Ns;j++){	// FRee the memory from delay lines)
		//free(sc[i].delay_up[j]);
		//free(sc[i].delay_low[j]);
	}
	
	for(j=0;j<FRAMES;j++){
		sc[i].channel[j] = 0;
	}
	
	old ++;
	if(old == CHANNELS) old = 0;
	
	return old;

}

struct sound_board_s sound_board_mix(struct sound_board_s sb, struct sample_create_s * sc, double * output){
	int i,j,k;
	double in, out;
	double mix = 1.0;

	for(k=0;k<FRAMES;k++){
		
	// Mix all the active samples for a given k
		in = 0;
		for(i=0;i<CHANNELS;i++){
			if(sc[i].active_flag == 1){
				in += sc[i].channel[k];
			}
		}
		
	// use the samples to create ISB
		for(i=0;i<SB_DL;i++){
			sb.isb[i] = sb.bsb[i] * in;
			for(j=0;j<SB_DL;j++){
				sb.isb[i] += sb.A[i][j]*sb.osb[0][j];
			}
		}
	
	// Shift the pointers for the delay lines
		for(i=0;i<SB_DL;i++){
			if(sb.dptr[i] == sb.lengths[i] - 1) sb.dptr[i] = 0;
			else 				 sb.dptr[i]++;
		} // now sb.dptr = oldest value in delay line
	
	// recalculate OSB
		sb.osb[0][0] = sb.gsb[0] * sb.d1[sb.dptr[0]] - sb.osb[1][0] *sb.a1sb[0];
		sb.osb[0][1] = sb.gsb[1] * sb.d2[sb.dptr[1]] - sb.osb[1][1] *sb.a1sb[1];
		sb.osb[0][2] = sb.gsb[2] * sb.d3[sb.dptr[2]] - sb.osb[1][2] *sb.a1sb[2];
		sb.osb[0][3] = sb.gsb[3] * sb.d4[sb.dptr[3]] - sb.osb[1][3] *sb.a1sb[3];
		sb.osb[0][4] = sb.gsb[4] * sb.d5[sb.dptr[4]] - sb.osb[1][4] *sb.a1sb[4];
		sb.osb[0][5] = sb.gsb[5] * sb.d6[sb.dptr[5]] - sb.osb[1][5] *sb.a1sb[5];
		sb.osb[0][6] = sb.gsb[6] * sb.d7[sb.dptr[6]] - sb.osb[1][6] *sb.a1sb[6];
		sb.osb[0][7] = sb.gsb[7] * sb.d8[sb.dptr[7]] - sb.osb[1][7] *sb.a1sb[7];
		
	// Update the delay lines
		sb.d1[sb.dptr[0]] = sb.isb[0];
		sb.d2[sb.dptr[1]] = sb.isb[1];
		sb.d3[sb.dptr[2]] = sb.isb[2];
		sb.d4[sb.dptr[3]] = sb.isb[3];
		sb.d5[sb.dptr[4]] = sb.isb[4];
		sb.d6[sb.dptr[5]] = sb.isb[5];
		sb.d7[sb.dptr[6]] = sb.isb[6];
		sb.d8[sb.dptr[7]] = sb.isb[7];
		
	// Calculate output and update OSB (2 birds 1 stone)
		out = 0;
		for(i=0;i<SB_DL;i++){
			out += sb.csb[i] * sb.osb[0][i];
			sb.osb[1][i] = sb.osb[0][i];
		}

	// Update output
		output[k] = mix*200.0*out+(1-mix)*200.0*in;
	
	}
	return sb;
	
}

int soundcard_output(snd_pcm_t * handle, double * output){
	int sample;
	int amp = 8000;	// TODO: Change it to volume control
	int i;
	char buffer[FRAMES * 4];
	
	// Check which volume we should be using
	FILE * f_ptr = fopen("volume","r");
	if(f_ptr == NULL){
		fprintf(stderr,"Couldn't open the volume control file\n");
		exit(1);
	}
	char c[3];
	fscanf(f_ptr,"%s",c);
	fclose(f_ptr);
	double volume = atof(c)/100.0;
	
	for(i=0;i<FRAMES;i++){
		
		sample = volume*amp*output[i];
		buffer[0+i*4] = sample&0xFF;		// This line over-writes the index at i=18
		buffer[1+i*4] = (sample&0xFF00)>> 8;	
		buffer[2+i*4] = sample&0xFF;
		buffer[3+i*4] = (sample&0xFF00)>> 8;	
	}
	
	i = snd_pcm_writei(handle,buffer,FRAMES);
	if(i<0){
		snd_pcm_prepare(handle);// Fixes the buffer after a problem
	}
	return i;
}

// Threads:

static void *keyboard_input(void * args){
	int * main_flg = (int *) args;
	int * KB_flg  = (int *) args+1; 
	int * input_method = (int *) args + 2;

	int midi;
	unsigned char midi_data[3];
	unsigned char mididata[3];
	int midirx; 
	
	char * flute_cmd[] = {"./flute","1",(char*) 0};
	char * harps_cmd[] = {"./harpsichord","1",(char*) 0};
	char * guitar_cmd[] = {"./guitar","1",(char *) 0};

	FILE * volume_ptr;
	FILE * proc_msg_ptr;

	char input = '1';

	// Open the MIDI device:
	if(*input_method == 0){
		midi = open("/dev/dmmidi1", O_RDWR | O_NOCTTY | O_NDELAY);
	} else {
		midi = open("/dev/rfcomm0", O_RDWR | O_NOCTTY | O_NDELAY);
	}
	if (midi == -1) {
			perror("Open port: Unable to open MIDI port\n"); exit(1);
	} 
	else fcntl(midi,F_SETFL,0);

	char * c = malloc (3*sizeof(char));
	// Read MIDI data
	while(1){
		midirx = read(midi, mididata, 1);
		if(mididata[0] == 0xFE){
			// This is the keepalive 

		} else if(mididata[0] > 127 && mididata[0]< 133){
			switch(mididata[0]){
				case 128 :	// piano
					printf("User Selected piano\n");
					break;
				case 129 :	// flute
					printf("User Selected flute\n");
					proc_msg_ptr = fopen("proc_msg", "w");
					if(proc_msg_ptr != 0){
						close(midi);
						usleep(10000);
						fprintf(proc_msg_ptr,"i %d\n", mididata[0]);
						fclose(proc_msg_ptr);
						exit(1);
					} else {
						fprintf(stderr, "Couldn't Switch processes\n");
					}
					break;

				case 130 :  // harpsichord
					printf("User Selected harispchord\n");
					proc_msg_ptr = fopen("proc_msg","w");
					if(proc_msg_ptr != 0){
						close(midi);
						usleep(10000);
						fprintf(proc_msg_ptr, "i %d\n", mididata[0]);
						fclose(proc_msg_ptr);
						exit(0);
					} else {
						fprintf(stderr, "Couldn't switch processes\n");
					}
					break;
	
				case 131 :	// Clarinet
					printf("Clarinet Selected\n");
					proc_msg_ptr = fopen("proc_msg","w");
					if(proc_msg_ptr != 0){
						close(midi);
						usleep(10000);
						fprintf(proc_msg_ptr,"i %d\n", mididata[0]);
						fclose(proc_msg_ptr);
						exit(0);
					} else {
						fprintf(stderr, "Couldn't switch processes\n");
					}
					break;

				case 132:	// Guitar
					printf("Guitar Selected\n");
					proc_msg_ptr = fopen("proc_msg","w");
					if(proc_msg_ptr != 0){
						close(midi);
						usleep(10000);
						fprintf(proc_msg_ptr,"i %d\n", mididata[0]);
						fclose(proc_msg_ptr);
						exit(0);
					} else {
						fprintf(stderr, "Couldn't Switch processes\n");
					}
					break;
			}
		} else {
			midirx = read(midi, &mididata[1], 2);
			if(mididata[1] == 7){
				volume_ptr = fopen("volume","w");
				proc_msg_ptr = fopen("proc_msg","w");			
				if(volume_ptr != 0){
					fprintf(volume_ptr, "%d\n",mididata[2]);
					fclose(volume_ptr);
				} else {
					fprintf(stderr,"didn't get the volume done\n");
				}			
				if(proc_msg_ptr != 0){
					fprintf(proc_msg_ptr, "v %d\n",mididata[2]);
					fclose(proc_msg_ptr);
				} else {
					fprintf(stderr,"didn't get the process message done\n");
				}			
			} else {
				c = &mididata[0];
				*KB_flg = (mididata[2] << 8) | mididata[1];
		        while(*main_flg == 0) usleep(1000);
		        *main_flg = 0;
			}
		}
	}
	
	return 0;
}

