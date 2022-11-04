#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>
#include <fcntl.h>
#include "synth.h"
#include "guitar_voice.h"

#define SAMPLING_FREQ 44100
#define FRAMES 24
#define CHANNELS 8
#define PLUCK_LEN 22500
#define FADE_LENGTH 2000

void sample_create(struct sample_create_s *, int, double *, FILE*);
int sample_create_add(struct sample_create_s *, struct note_s *, int, int);
int sample_create_remove(struct sample_create_s *, int);
int soundcard_output(snd_pcm_t *, double *);
static void * keyboard_input(void * args);

int main(int argc, char * argv[]){
	int i, j, rc;
	int guitar_on = 1;
	int input_flg[3] = {0,0,0};

	// Choose input selection -------------------------------------------------------------
		if(argc < 2) {
			input_flg[2] = 0;
		} else {
			input_flg[2] = atoi(argv[1]);
		}


	// Initialize the sound card ----------------------------------------------------------
		snd_pcm_t * handle;
		snd_pcm_uframes_t frames = FRAMES;
		rc = snd_pcm_open(&handle, "default",SND_PCM_STREAM_PLAYBACK,0);
		if(rc < 0){
			fprintf(stderr, "can't open the default pcm device:%s\n",snd_strerror(rc));
			exit(1);
		}

		// Hardware
		rc = soundcard_init(SAMPLING_FREQ, frames, handle);
		if(rc != 0){
			fprintf(stderr, "Couldn't configue the soundcard hw params\n");
			exit(1);
		}

		// Software
		rc = alsa_sw_init(handle);
		if(rc != 0){
			fprintf(stderr, "Couldnt set the ALSA software params\n");
			exit(1);
		}

	// Initialize sample creators ---------------------------------------------------------
		struct sample_create_s samp_create[CHANNELS];
		struct note_s note[88];
		double output[FRAMES];

		note_init(&note[0]);
		
		for(i=0;i<CHANNELS;i++){
			samp_create[i].active_flag = 0;
			samp_create[i].channel = malloc(FRAMES * sizeof(double));
			samp_create[i].note_num = -1;
		}

	// Open the Plucked data --------------------------------------------------------------
		double pluck[PLUCK_LEN];
		FILE * f_ptr = fopen("ExcitePlucked.txt","r");
		if(f_ptr == NULL){
			fprintf(stderr, "Pluck data couldn't be read from file\n");
			exit(1);
		}
		char pluck_str[15];
		for(i=0;i<PLUCK_LEN;i++){
			fscanf(f_ptr, "%s", pluck_str);
			pluck[i] = atof(pluck_str);
		}
		fclose(f_ptr);


	// Start the other threads ------------------------------------------------------------
		pthread_t keyboard_input_tid;
		rc = pthread_create(&keyboard_input_tid, NULL, keyboard_input, &input_flg[0]);
		if(rc != 0){
			fprintf(stderr, "Couldn't communicate with MIDI input device\n");
			fprintf(stderr, "\t - Thread could not start\n");
		}

	// Do the debugging stuff
		FILE * debug_ptr = fopen("debug_output", "w");
		
	// Synthesis Loop ---------------------------------------------------------------------
		int note_num;
		int sc_flag;
		int max_cycle_count;
		int max_cycle_num;

		while(guitar_on){

		// Check for new Input
			if(input_flg[1] != 0){
				input_flg[0] = 1;
				note_num = (0xFF & input_flg[1] - 21);
				if(note[note_num].defined == 1){
					if((0xFF00 & input_flg[1]) != 0){
						max_cycle_count = 0;
						for(i=0;i<CHANNELS;i++){
							if(samp_create[i].active_flag == 0) break;
							if(samp_create[i].cycle_count > max_cycle_count){
								max_cycle_count = samp_create[i].cycle_count;
								max_cycle_num = i;
							}
						}

						if(i==8){
							 sample_create_add(&samp_create[0],&note[0],note_num,max_cycle_num);
						} else {
							 sample_create_add(&samp_create[0],&note[0],note_num,i);
							printf("debug - Channel %d is activated\n",i);
						}
	
					} else {
						for(i=0;i<CHANNELS;i++){
							if(samp_create[i].note_num == note_num){
								//sample_create_remove(&samp_create[0], i);
								samp_create[i].fade = FADE_LENGTH -1;
								printf("debug - channel %d has ended\n",i);
							}
						}
					}
				} else {
					printf("debug - That note is not available\n");
				}		
			input_flg[1] = 0;
		}

		// Assume no active sample creators
		sc_flag = 0;

		// Run active sample creators
		for(i=0;i<CHANNELS;i++){
			if(samp_create[i].active_flag == 1){
				sample_create(&samp_create[0],i,&pluck[0], debug_ptr);
				sc_flag = 1;
				for(j=0;j<FRAMES;j++){
					output[j] += samp_create[i].channel[j];
				}
				if(samp_create[i].fade < 0){
					sample_create_remove(&samp_create[0],i);
				}
			}
		}

		// Outpt to sound card
			if(sc_flag == 1){
				rc = soundcard_output(handle, &output[0]);
				for(i=0;i<FRAMES;i++) output[i] = 0;
			}

		}

	printf("Hello, World!\n");
	return 0;

}

void sample_create(struct sample_create_s * sc, int this, double * pluck,FILE * debug_ptr){
	int rc, i, k;
	double temp1, tempin, temp2;
	double b[2] = {0.8995, 0.1087};
	double a[2] = {1.0, 0.0136};

	for(k=0;k<FRAMES;k++){
		temp1 = 0;

	// Interpolate Fractional Delays
		for(i=0;i<4;i++){
			temp1+=sc[this].delay[sc[this].note.N - (4-i) -1] * sc[this].note.c[i];
		}
	// Send the output
		sc[this].channel[k] = temp1 * sc[this].note.g1;

	// Scale output at note end
		if(sc[this].fade < FADE_LENGTH){
			if(sc[this].fade > 0){
				sc[this].channel[k] *= (double) sc[this].fade / (double) FADE_LENGTH;
			} else {
				sc[this].channel[k] = 0;
			}
			sc[this].fade --;
		}
	
	// Filter Output for Feedback
		tempin = temp1;
		temp1 = b[0] * temp1 + sc[this].z_1;
		sc[this].z_1 = b[1] * tempin - a[1]* temp1;

	// Determine Excitation
		if(sc[this].pluck_ptr < PLUCK_LEN){
			temp2 = sc[this].note.g2 * pluck[sc[this].pluck_ptr];
			sc[this].pluck_ptr++;
		} else {
			temp2 = 0;
		}
	// Update the delay lines
		temp1 += temp2 - sc[this].delay_excit[sc[this].note.M-1];
		for(i=sc[this].note.M-1; i > 0;i--){
			sc[this].delay_excit[i] = sc[this].delay_excit[i-1];
		}
		sc[this].delay_excit[0] = temp2;
		for(i=sc[this].note.N-1; i > 0;i--){
			sc[this].delay[i] = sc[this].delay[i-1];
		}
		sc[this].delay[0] = temp1;


		if(sc[this].cycle_count < 44100000){
			sc[this].cycle_count ++;
		} else {
			sc[this].cycle_count  = 441000;
		}
	}

}


int sample_create_add(struct sample_create_s * sample_create, struct note_s * note, int note_num, int new){

	int i;

	sample_create[new].active_flag = 1;
	sample_create[new].note_num = note_num;
	sample_create[new].note = note[note_num];
	sample_create[new].cycle_count = 0;
	sample_create[new].z_1 = 0;
	sample_create[new].pluck_ptr = 0;
	sample_create[new].fade = FADE_LENGTH + 1;
	// malloc and zero the delay lines
	sample_create[new].delay = malloc(note[note_num].N * sizeof(double));
	if(sample_create[new].delay == 0){
		fprintf(stderr, "Couldn't allocate memory for delay line\n");
		fprintf(stderr, "You have memory issues - exitting ..\n");
		exit(1);
	}
	for(i = 0; i < note[note_num].N; i++){
		sample_create[new].delay[i] = 0;
	}

	sample_create[new].delay_excit = malloc(note[note_num].M * sizeof(double));
	if(sample_create[new].delay_excit == 0){
		fprintf(stderr, "Couldn't allocate memory for delay excit line\n");
		fprintf(stderr, "You have memory issues - exitting ..\n");
	}
	for(i=0;i<note[note_num].M;i++){
		sample_create[new].delay_excit[i] = 0;
	}

	// 0 the channel
	for(i=0;i<FRAMES;i++){
		sample_create[new].channel[i] = 0;
	}
	
	return 0;

}



int sample_create_remove(struct sample_create_s * sample_create, int this){
	sample_create[this].active_flag = 0;
	sample_create[this].note_num = -1;
	return 0;
}


int soundcard_output(snd_pcm_t * handle, double * output){
	int sample;
	int amp = 10000;
	int i;
	char buffer[FRAMES * 4];

	// Check for volume file
	double volume;
	char c[3];
	FILE * f_ptr = fopen("volume", "r");
	if(f_ptr == NULL){
		fprintf(stderr, "Couldnt Open the volume control file\n");
		fprintf(stderr, "using default volume = full blast\n");
		volume = 1;
 	} else {
		fscanf(f_ptr, "%s", c);
		fclose(f_ptr);
		volume = atof(c)/100.0;
	}

	for(i=0;i<FRAMES;i++){
		sample = volume * amp * output[i];
		buffer[0+i*4] = sample & 0xFF;
		buffer[1+i*4] = (sample & 0xFF00) >> 8;
		buffer[2+i*4] = sample & 0xFF;
		buffer[3+i*4] = (sample & 0xFF00) >> 8;
	}
	i = snd_pcm_writei(handle, buffer, FRAMES);
	if(i<0){
		snd_pcm_prepare(handle);
		fprintf(stderr, "x-run occured\n");
	}
	return i;
}


	



static void * keyboard_input(void * args){
	int * main_flg = (int*) args;
	int * KB_flg = (int *) args +1;
	int * input_select = (int*) args + 2;
	int midi;
	unsigned char  mididata[3];
	int midirx;
	
	FILE * volume_ptr;
	FILE * proc_msg_ptr;


	if(*input_select == 1){
		midi = open("/dev/rfcomm0", O_RDWR | O_NOCTTY | O_NDELAY);
	} else {
		midi = open("/dev/dmmidi1", O_RDWR | O_NOCTTY | O_NDELAY);
	}
	
	if(midi == -1){
		perror("Open Port: Unable to open MIDI port\n");
		exit(1);
	}
	else fcntl(midi, F_SETFL, 0);

	while(1){
		midirx = read(midi, mididata, 1);
		if(mididata[0] == 0xFE){

		} else if(mididata[0] > 127 && mididata[0] < 133){
			switch(mididata[0]){	// switch instruments
				case 128 : // piano
					printf("User Selected Piano\n");
					proc_msg_ptr = fopen("proc_msg", "w");
					if(proc_msg_ptr != 0){
						usleep(10000);
						close(midi);
						fprintf(proc_msg_ptr, "i %d\n", mididata[0]);
						fclose(proc_msg_ptr);
						exit(0);
					} else {
						fprintf(stderr, "Couldn't Switch Processes\n");
					}
					break;

				case 129 : // flute
					printf("User Selected Flute\n");
					proc_msg_ptr = fopen("proc_msg", "w");
					if(proc_msg_ptr != 0){
						usleep(10000);
						close(midi);
						fprintf(proc_msg_ptr, "i %d\n", mididata[0]);
						fclose(proc_msg_ptr);
						exit(0);
					} else {
						fprintf(stderr, "Couldn't Switch Processes\n");
					}
					break;

				case 130 : // harpsichord
					printf("User Selected Harpsichord\n");
					proc_msg_ptr = fopen("proc_msg", "w");
					if(proc_msg_ptr != 0){
						usleep(10000);
						close(midi);
						fprintf(proc_msg_ptr, "i %d\n", mididata[0]);
						fclose(proc_msg_ptr);
						exit(0);
					} else {
						fprintf(stderr, "Couldn't Switch Processes\n");
					}
					break;

				case 131: // Clarinet
					printf("User Selected Clarinet\n");
					proc_msg_ptr = fopen("proc_msg", "w");
					if(proc_msg_ptr != 0){
						usleep(10000);
						close(midi);
						fprintf(proc_msg_ptr, "i %d\n", mididata[0]);
						fclose(proc_msg_ptr);
						exit(0);
					} else {
						fprintf(stderr, "Couldn't Switch Processes\n");
					}
					break;

				case 132: // Guitar
					printf("User Selected Guitar\n");
					break;
			}
		} else {
			midirx = read(midi, &mididata[1],2);
			if(mididata[1] == 7){
				volume_ptr = fopen("volume", "w");
				proc_msg_ptr = fopen("proc_msg", "w");
				if(volume_ptr != 0){
					fprintf(volume_ptr, "%d\n",mididata[2]);
					fclose(volume_ptr);
				} else {
					fprintf(stderr, "didn't write the volume file\n");
				}
				if(proc_msg_ptr != 0){
					fprintf(proc_msg_ptr, "v %d\n", mididata[2]);
					fclose(proc_msg_ptr);
				} else {
					fprintf(stderr, "couldnt write the process message file\n");
				}
			} else {
				*KB_flg = (mididata[2] << 8) | mididata[1];
				while(*main_flg == 0) usleep(1000);
				*main_flg = 0;
			}
		}
	}
}


