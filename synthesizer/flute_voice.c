#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <alsa/asoundlib.h>
#include <pthread.h>
#include <fcntl.h>
#include "flute_voice.h"
#include "synth.h"

#define SAMPLING_FREQ 44100;
#define FRAMES 24
#define CHANNELS 8
#define PI 3.14159265359
#define FADE_LENGTH 2000

int sample_create_add(struct sample_create_s *, struct note_s *, int, int);
int sample_create_remove(struct sample_create_s *, int);
//void flute_sample_create(struct sample_create_s *, int);
void flute_sample_create(struct sample_create_s *, int, FILE *);
int soundcard_output(snd_pcm_t * handle, double * output);
static void *keyboard_input(void * args);
static void *keep_alive(void * args);

// Main
int main(int argc, char * argv[]){
	int i, j, rc;					// indices & results check flag
	int flute_on = 1;			// Flag for continued flute operation
	int input_flg[3] = {0,0,0};	// indicates new input

	// Choose input selections
	if(argc < 2){
		input_flg[2] = 0;
	} else {
		input_flg[2] = atoi(argv[1]);
	} 


	// Initialize sound card ------------------------------------------------------------
		snd_pcm_t * handle;					// Soundcard handle
		snd_pcm_uframes_t frames = FRAMES;	// Frames per Period (for ALSA)
	
		rc = snd_pcm_open(&handle, "default", SND_PCM_STREAM_PLAYBACK,0);
		if(rc < 0) {
			fprintf(stderr, "Can't Open the default PCM device:%s\n",snd_strerror(rc));
			exit(1);
		}
		
		// Hardware
		rc = soundcard_init(44100, frames, handle);
		if(rc != 0){
			fprintf(stderr, "Coudn't configure the soundcard hardware params\n");
			exit(1);
		}
		
		// Software
		rc = alsa_sw_init(handle);
		if(rc != 0) {
			fprintf(stderr,"failed at setting software parameters\n");
			exit(1);
		}
		
		
		
	// Initialize sample_creators -------------------------------------------------------
		struct sample_create_s sample_create[CHANNELS];
		struct note_s note[88];
		double output[FRAMES];
		
		note_init(&note[0]);
		
		for(i=0;i<CHANNELS;i++){
			//sample_create[i].Flute = flute_init();
			sample_create[i].active_flag = 0;
			sample_create[i].channel = malloc(FRAMES * sizeof(double));
			
		}
		

	
		
	// Start the other threads ----------------------------------------------------------
		pthread_t keyboard_input_tid;
		pthread_t keep_alive_tid;
		
		rc = pthread_create(&keyboard_input_tid, NULL, keyboard_input, &input_flg[0]);
		if(rc != 0){
			fprintf(stderr, "cannot communicate with keyboard - thread cannot start\n");
			exit(1);
		}
		

	// DEBUGGING VARIABLES --------------------------------------------------------------
		FILE * debug_ptr = fopen("output_debug", "w");
		if(debug_ptr == NULL){
			fprintf(stderr, "Couldn't open the output_debug file.. CCheck it exsists\nAre you sure you want to debug now..\n");
			exit(1);
		}
		

	// Synthesis Loop -------------------------------------------------------------------
		int note_num;	// New MIDI note
		int sc_flag;	// Flag for if there is active sample creators running
		int velocity;
		int max_cycle_count;
		int max_cycle_num;
	
		while(flute_on){
		
		// Check for new input
			if(input_flg[1] != 0){
				input_flg[0] = 1;						//acknowlege
				note_num = (0xFF & input_flg[1] - 21);	// get note number
				
				if((0xFF00 & input_flg[1]) != 0){	// Key Press
				// Choose new input
					max_cycle_count = 0;
					if(note[note_num].defined == 1){
						for(i=0;i<CHANNELS;i++){
							if(sample_create[i].active_flag == 0) break;
							if(sample_create[i].cycle_count > max_cycle_count){
								max_cycle_count = sample_create[i].cycle_count;
								max_cycle_num = i;
							}
						}

						
						if(i == 8){
							rc = sample_create_add(&sample_create[0], &note[0], note_num, max_cycle_num);
							printf("not enough free channels - wrapping the old\n");
						} else {
							printf("%d sample creator added note %d\n", i, note_num);
							rc = sample_create_add(&sample_create[0], &note[0], note_num, i);
						}
					} else {
						printf("Note is not defined\n");
					}
						
				} else {							// Key Release
					for(i=0;i<CHANNELS;i++){
						if(sample_create[i].note_num == note_num){
							printf("%d sample creator removed note %d\n", i, note_num);
							sample_create[i].fade = FADE_LENGTH-1;
						}
					}
				}
				input_flg[1] = 0;
				
				
				
			}
		
		// Assume no active sample creators
			sc_flag = 0;
		
		// Run the sample creators
			for(i=0;i<CHANNELS;i++){
				if(sample_create[i].active_flag == 1){
					flute_sample_create(&sample_create[0],i,debug_ptr);
					sc_flag = 1;
					for(j=0;j<FRAMES;j++){ // Mix the samples
						output[j] += sample_create[i].channel[j];
					}
					if(sample_create[i].fade < 0){
						sample_create_remove(&sample_create[0],i);
					}
					
				}
			}
			
		// Output To soundcard
			if(sc_flag = 1){
			// output via sound card
				rc = soundcard_output(handle, &output[0]);
				
			// Re-0 the buffer
				for(i=0;i<FRAMES;i++){
					output[i] = 0;
				}
			
			}

	}

}


// Functions
//void flute_sample_create(struct sample_create_s * sc, int this){
void flute_sample_create(struct sample_create_s * sc, int this, FILE * debug_ptr){
	double temp[6] = {0,0,0,0,0,0};
	int temp1, temp2;
	int k,j,i;
	double voiced, kn_temp, a, rand_num, kvib_temp;

	// DEBUG VARIABLES
	double D_PBRANCH1[5];
	double D_ZBRANCH[5];
	double D_TEMP0[5];
	double D_PBRANCH2[5];
	double D_DUP_TMP1[5];
	double D_DLO_TMP2[5];
	double D_TEMP4[5];
	char * string = malloc(20*sizeof(char));


	for(k=0;k<FRAMES;k++){
		
		// Boundary Layer Loss Filter Calculations
		temp[5] = sc[this].delay_up[0];
		sc[this].delay_up[0] = sc[this].Flute.BoundaryNum[0]*sc[this].delay_up[0] + sc[this].z_boundary[0][0];
		sc[this].z_boundary[0][0] = sc[this].Flute.BoundaryNum[1]*temp[5] - sc[this].Flute.BoundaryDen[1]*sc[this].delay_up[0]+sc[this].z_boundary[0][1];
		sc[this].z_boundary[0][1] = sc[this].Flute.BoundaryNum[2]*temp[5] - sc[this].Flute.BoundaryDen[2]*sc[this].delay_up[0];
		
		
		
		temp[5] = sc[this].delay_low[55];
		sc[this].delay_low[55] = sc[this].Flute.BoundaryNum[0]*sc[this].delay_low[55] + sc[this].z_boundary[1][0];
		sc[this].z_boundary[1][0] = sc[this].Flute.BoundaryNum[1]*temp[5] - sc[this].Flute.BoundaryDen[1]*sc[this].delay_low[55] + sc[this].z_boundary[1][1];
		sc[this].z_boundary[1][1] = sc[this].Flute.BoundaryNum[2]*temp[5] - sc[this].Flute.BoundaryDen[2]*sc[this].delay_low[55];
		
		
		// Open End Filter Calculations
		temp[5] = sc[this].delay_up[55];
		sc[this].delay_low[0] = sc[this].Flute.OpenEndNum[0] * temp[5] + sc[this].z_open[0];
		sc[this].z_open[0] = sc[this].Flute.OpenEndNum[1]*temp[5] - sc[this].Flute.OpenEndDen[1]*sc[this].delay_low[0]+sc[this].z_open[1];
		sc[this].z_open[1] = -sc[this].Flute.OpenEndDen[2] * sc[this].delay_low[0];
		
		
		// Tone hole Calculations
		temp[3] = 0;
		
		for(j=0;j<sc[this].Flute.NumToneHoles;j++){

		  temp1 = sc[this].Flute.Dsum[j]-1;
		  temp2 = 55 - sc[this].Flute.Dsum[j];
		    
		    if(sc[this].note.Finger[j] == 1){
			
			// Tone Hole Reflection Filter using direct form II transposed
			temp[5] = sc[this].p_branch[j];
			sc[this].p_branch[j] = sc[this].Flute.ClosedToneHoleNum[0] * sc[this].p_branch[j] + sc[this].z_branch[j];
			sc[this].z_branch[j] = sc[this].Flute.ClosedToneHoleNum[1] * temp[5] - sc[this].Flute.ClosedToneHoleDen[1] * sc[this].p_branch[j];
			temp[0] = sc[this].Flute.ToneHoler0 * (sc[this].delay_up[temp1] + sc[this].delay_low[temp2] - 2 * sc[this].p_branch[j]);
			sc[this].p_branch[j] = temp[0] + sc[this].delay_up[temp1] + sc[this].delay_low[temp2] - sc[this].p_branch[j];
			
		    } else {
			
			// Tone Hole Reflection Filter using direct form II transposed
			temp[5] = sc[this].p_branch[j];
			sc[this].p_branch[j] = sc[this].Flute.OpenToneHoleNum[0] * sc[this].p_branch[j] + sc[this].z_branch[j];
			sc[this].z_branch[j] = sc[this].Flute.OpenToneHoleNum[1] * temp[5] - sc[this].Flute.OpenToneHoleDen[1] * sc[this].p_branch[j];
			temp[0] = sc[this].Flute.ToneHoler0 * (sc[this].delay_up[temp1] + sc[this].delay_low[temp2] - 2 * sc[this].p_branch[j]);
			sc[this].p_branch[j] = temp[0] + sc[this].delay_up[temp1] + sc[this].delay_low[temp2] - sc[this].p_branch[j];
			
		    }
		    
		    if(sc[this].p_branch[j] > 25){
			sc[this].p_branch[j] = 25;
		    } else if (sc[this].p_branch[j] < -25){
			sc[this].p_branch[j] = -25;
		    }
		    
		    sc[this].delay_up[temp1]  += temp[0];
		    sc[this].delay_low[temp2] += temp[0];
		    temp[3] += (sc[this].p_branch[j] / (2 * sc[this].Flute.NumToneHoles));
		   
		    
		}
		
		// Excitation Calculations

		if( sc[this].delay_low[55] > 6){
		    sc[this].delay_low[55] = 6;
		} else if(sc[this].delay_low[55] < -6){
		    sc[this].delay_low[55] = -6;
		}
		
	    
	    // Feedback to excitation
	    voiced = (sc[this].delay_low[55] + sc[this].Flute.kdfb * sc[this].f[0]) * sc[this].Flute.kv;
	    
	    // Increment input linearly from 0 to kn
	    if (sc[this].cycle_count < 300) {
		kn_temp = sc[this].Flute.kn * (double) (sc[this].cycle_count+1) / 1000.0;
	    } else {
		kn_temp = sc[this].Flute.kn;
	    }
	    
	    // LPF noise input to avoid quick variations
	    srand(time(NULL)*rand_num);
	    rand_num = (double)(rand() % 1000000) / 1000000.0;
		rand_num *= 0.9; //Seeing if this fixes the high pitch whistel
	    sc[this].noise_filt[0] = 0.3 * rand_num - 0.3 * sc[this].noise_filt[1];
	    
	    // Calculation of excitation signal with feedback elements and envelope
	    a = sc[this].noise_filt[0] * kn_temp + voiced;
	    
	    // Increase vibrato gain until it reaches max at 1 second
	    if(sc[this].cycle_count < 44100){
		kvib_temp = (double) (sc[this].cycle_count+1)/44100.0 * sc[this].Flute.kvib;
	    } else {
		kvib_temp = sc[this].Flute.kvib;
	    }
	    
	    // Vibrator effect envelope
	    sc[this].delay_jet[0] = a*(1+kvib_temp * cos(2.0*3.14159265359*sc[this].Flute.fvib*(double)sc[this].cycle_count/44100.0));
	    
	    // Calculation at jet non linearity to simulate breath
	    sc[this].c[0] = sc[this].Flute.JetGain*tanh(sc[this].delay_jet[sc[this].note.Njet-1]);

	    // DC Killer used to remove DC content of excitation
	    sc[this].f[0] = sc[this].c[0] - sc[this].c[1] + 0.995 * sc[this].f[1];
	    
	    // Calculate output Double filtered for extra lpf
	    //sc[this].channel[k] = sc[this].delay_up[55] + temp[3];
		sc[this].channel[k] = (sc[this].delay_up[55] + temp[3]) * 0.1 + 0.9 * sc[this].old_out;
		sc[this].old_out = sc[this].channel[k];

		// Scale output for end:
		if(sc[this].fade < FADE_LENGTH){
			if(sc[this].fade > 0){
				sc[this].channel[k] *= (double) sc[this].fade/(double) FADE_LENGTH;
			} else {
				sc[this].channel[k] = 0;
			}
			sc[this].fade --;
		}
    
	    // update the delay lines' pointers
	   for(i=55;i>0;i--){
		   sc[this].delay_up[i] = sc[this].delay_up[i-1];
	   }
	   sc[this].delay_up[0] = sc[this].f[0] + sc[this].Flute.kfb * sc[this].delay_low[55];
	   
	   for(i=55;i>0;i--){
		   sc[this].delay_low[i] = sc[this].delay_low[i-1];
	   }
	   sc[this].delay_low[0] = 0;
	   
	   for(i=sc[this].note.Njet-1;i>0;i--){
		   sc[this].delay_jet[i] = sc[this].delay_jet[i-1];
	   }
	   sc[this].delay_jet[0] = 0;
	   
	    // Update Filter variables
	    sc[this].noise_filt[1]  = sc[this].noise_filt[0];
	    sc[this].c[1] = sc[this].c[0];
	    sc[this].f[1] = sc[this].f[0];
	    
	    // Update the cycle count while (hopefully) avoiding an overflow
		// This will mess up if you try to play CHANNEL notes for more than 1000 secs
		if(sc[this].cycle_count < 44100000){
			sc[this].cycle_count ++;
		} else {
			sc[this].cycle_count = 88200;
		}
		
	}
	
}

int sample_create_add(struct sample_create_s * sample_create, struct note_s * note, int note_num, int new){
	int i;
	
	sample_create[new].active_flag = 1;
	sample_create[new].note_num = note_num;
	sample_create[new].note = note[note_num];
	sample_create[new].fade = FADE_LENGTH + 1;
	sample_create[new].old_out = 0.0;
	
	// Initialize Flute model
	if(note_num < 57){
	        sample_create[new].Flute = flute_1_init();
	} else {
	        sample_create[new].Flute = flute_init();
	}
	
	// 0 the delay lines
	for(i=0;i<sample_create[new].Flute.DelayTotal;i++){
		sample_create[new].delay_up[i]  = 0;
		sample_create[new].delay_low[i] = 0;
	}
	
	// create the jet delay line
	sample_create[new].delay_jet = malloc(note[note_num].Njet * sizeof(double));
	if(sample_create[new].delay_jet == NULL){
		fprintf(stderr,"Cannot allocate memory for jet delay line\n");
		exit(1);
	}
	for(i=0;i<note[note_num].Njet;i++){
		sample_create[new].delay_jet[i] = 0;
	}
	
	// 0 the channel
	for(i=0;i<FRAMES;i++){
		sample_create[new].channel[i] = 0;
	}
	
	// 0 all other variables
	for(i=0;i<6;i++){
		sample_create[new].z_branch[i] = 0;
		sample_create[new].p_branch[i] = 0;
	}
	
	for (i=0;i<2;i++){
		sample_create[new].z_open[i] = 0;
		sample_create[new].z_boundary[0][i] = 0;
		sample_create[new].z_boundary[1][i] = 0;
		sample_create[new].noise_filt[i] = 0;
		sample_create[new].f[i] = 0;
		sample_create[new].c[i] = 0;
	}
	
	sample_create[new].dup_zer = 0; 
	sample_create[new].dup_dto = sample_create[new].Flute.DelayTotal - 1;
	sample_create[new].dlo_zer = 0; 
	sample_create[new].dlo_dto = sample_create[new].Flute.DelayTotal - 1;
	sample_create[new].djt_zer = 0; 
	sample_create[new].djt_njt = sample_create[new].note.Njet - 1;
	
	sample_create[new].cycle_count = 0;
	
	return 0;
}

int sample_create_remove(struct sample_create_s * sample_create, int this){
	sample_create[this].active_flag = 0;	
	sample_create[this].note_num = 9999; // stops from trying to eliminate already stopped channels
	return 0;

}

int soundcard_output(snd_pcm_t * handle, double * output){
	int sample;
	int amp = 1000;	// TODO: Change it to volume control
	int i;
	char buffer[FRAMES * 4];
	
	// Chech if file volume
	double volume;
	char c[3];
	FILE * f_ptr = fopen("volume", "r");
	if(f_ptr == NULL){
		fprintf(stderr, "couldn't open the volume control file\n");
		fprintf(stderr, "using defualt = full blast\n");
		volume = 1;
	} else {
		fscanf(f_ptr, "%s",c);
		fclose(f_ptr);
		volume = atof(c)/100.0;
	}
	
	for(i=0;i<FRAMES;i++){
		
		sample = volume*amp*output[i];
		buffer[0+i*4] = sample&0xFF;		// This line over-writes the index at i=18
		buffer[1+i*4] = (sample&0xFF00)>> 8;	
		buffer[2+i*4] = sample&0xFF;
		buffer[3+i*4] = (sample&0xFF00)>> 8;	
	}
	
	i = snd_pcm_writei(handle,buffer,FRAMES);
	if(i<0){
		printf("under-run\n");
		snd_pcm_prepare(handle);// Fixes the buffer after a problem
	}
	return i;
}
	
// Threads

static void *keyboard_input(void * args){
	int * main_flg = (int *) args;
	int * KB_flg = (int * ) args + 1;
	int * input_select = (int *) args + 2;
	int midi;
	unsigned char mididata[3];
	int midirx;
	
	FILE * volume_ptr;
	FILE * proc_msg_ptr;


	if(*input_select == 1){
		midi = open("/dev/rfcomm0", O_RDWR | O_NOCTTY | O_NDELAY);
	} else {
		midi = open("/dev/midi1", O_RDWR | O_NOCTTY | O_NDELAY);
	}
	if (midi == -1){
		perror("Open Port: Unable to open MIDI port\n");
		exit(1);
	}
	else fcntl(midi, F_SETFL,0);
	
	while(1){
		midirx = read(midi, mididata, 1);
		if(mididata[0] == 0xFE){

		} else if(mididata[0] > 127 && mididata[0] < 133){
			switch(mididata[0]){ // Parse for new insturment here
				case 128: // piano
					printf("User Selected piano\n");
					proc_msg_ptr = fopen("proc_msg", "w");
					if(proc_msg_ptr != 0){
						usleep(10000);
						close(midi);
						fprintf(proc_msg_ptr, "i %d\n", mididata[0]);
						fclose(proc_msg_ptr);
						exit(0);
					} else {
						fprintf(stderr, "Couldn't switch instruments\n");
					}
					break;

				case 129: // flute
					printf("User Selected Flute\n");
					break;

				case 130: // harpsichord
					printf("User Selected Harpsichord\n");
					proc_msg_ptr = fopen("proc_msg", "w");
					if(proc_msg_ptr != 0){
						usleep(10000);
						close(midi);
						fprintf(proc_msg_ptr, "i %d\n", mididata[0]);
						fclose(proc_msg_ptr);
						exit(0);
					} else {
						fprintf(stderr, "Couldn't switch instruments\n");
					}
					break;

				case 131: // Clarinet
					printf("User selected Clarinet\n");
					proc_msg_ptr = fopen("proc_msg", "w");
					if(proc_msg_ptr != 0){
						usleep(10000);
						close(midi);
						fprintf(proc_msg_ptr, "i %d\n", mididata[0]);
						fclose(proc_msg_ptr);
						exit(0);
					} else {
						fprintf(stderr, "Couldn't switch instruments\n");
					}
					break;

				case 132:
					printf("User Selected GUITAR\n");
					proc_msg_ptr = fopen("proc_msg", "w");
					if(proc_msg_ptr != 0){
						usleep(10000);
						close(midi);
						fprintf(proc_msg_ptr, "i %d\n", mididata[0]);
						fclose(proc_msg_ptr);
						exit(0);
					} else {
						fprintf(stderr, "Couldn't switch instruments\n");
					}
					break;
			}
		} else {
			midirx = read(midi, &mididata[1], 2);
			if(mididata[1] == 7){
				volume_ptr = fopen("volume","w");
				proc_msg_ptr = fopen("proc_msg","w");
				if(volume_ptr != 0){
					fprintf(volume_ptr, "%d\n", mididata[2]);
					fclose(volume_ptr);
				} else {
					fprintf(stderr, "Couldn't write the volume to file\n");
				}
				if(proc_msg_ptr != 0){
					fprintf(proc_msg_ptr, "v %d\n", mididata[2]);
					fclose(proc_msg_ptr);
				} else {
					fprintf(stderr, "didn't get the process message written\n");
				}
			} else {
				*KB_flg = (mididata[2] << 8) | mididata[1];
				while(*main_flg == 0) usleep(1000);
				*main_flg = 0;
			}
		}
	}
	
}

static void *keep_alive(void * args){
	int * flute_on = (int *) args;
	FILE * f_ptr;
	char c;
	
	while(1){
		f_ptr = fopen("config","r");
		if(f_ptr == 0){
			fprintf(stderr,"Keep-Alive can't open the config file \n");
		}
		fscanf(f_ptr,"%c",&c);
		fclose(f_ptr);
		if(c == '0') *flute_on = 0;
		sleep(2);
	}
}

    
