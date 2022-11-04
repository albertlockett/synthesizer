#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <alsa/asoundlib.h>
#include <pthread.h>
#include <fcntl.h>
#include "clarinet_voice.h"
#include "synth.h"

#define SAMPLING_FREQ 44100
#define FRAMES 24
#define CHANNELS 8
#define FADE_LENGTH 2000

void clarinet_sample_create(struct sample_create_s *, int);
int sample_create_remove(struct sample_create_s *, int);
int sample_create_add(struct sample_create_s * , struct note_s *, int , int);
static void * keyboard_input(void *);


int main(int argc, char* argv[]){
    int i, j, rc;
    int clarinet_on = 1;
    int input_flg[3] = {0,0,0};
    
    // Choose input selections ----------------------------------------------------
        if (argc < 2){
            input_flg[2] = 0;
        } else {
            input_flg[2] = atoi(argv[1]);
        }
    
    
    // Initialize the sound card --------------------------------------------------
        snd_pcm_t * handle;
        snd_pcm_uframes_t frames = FRAMES;	// Frames per Period (for ALSA)
        
        rc = snd_pcm_open(&handle, "default", SND_PCM_STREAM_PLAYBACK,0);
        if(rc < 0) {
            fprintf(stderr, "Can't Open the default PCM device:%s\n",snd_strerror(rc));
            exit(1);
        }
        
        // Hardware
        rc = soundcard_init(SAMPLING_FREQ, frames, handle);
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
    
    // Initalize sample_creators --------------------------------------------------
        struct sample_create_s sample_create[CHANNELS];
        struct note_s note[88];
        double output[FRAMES];
        
        note_init(&note[0]);
    
        for(i=0;i<CHANNELS;i++){
            sample_create[i].Clarinet = clarinet_init();
            sample_create[i].active_flag = 0;
            sample_create[i].channel = malloc(FRAMES*sizeof(double));
            
        }
    
    // Start the keyboard input thread --------------------------------------------
        pthread_t keyboard_input_tid;
        
        rc = pthread_create(&keyboard_input_tid, NULL, keyboard_input, &input_flg[0]);
        if(rc != 0){
            fprintf(stderr, "cannot communicate with keyboard - thread cannot start\n");
            exit(1);
        }
    
    // Synthesis Loop ------------------------------------------------------------
        int note_num;
        int sc_flag;
        int max_cycle_count;
        int max_cycle_num;
        while(clarinet_on){
            
            // Check for new input
			if(input_flg[1] != 0){
				input_flg[0] = 1;						//acknowlege
				note_num = (0xFF & input_flg[1] - 21);	// get note number
            
                if((0xFF00 & input_flg[1]) != 0){	// Key Press
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
							printf("Creating sampler using channel %d\n",i);
							printf("%d sample creator added note %d\n", i, note_num);
							rc = sample_create_add(&sample_create[0], &note[0], note_num, i);
						}
                        
                    } else {
                        printf("Note is note defined\n");
                    }
                    
                } else { // Key Release
                    for(i=0;i<CHANNELS;i++){
						if(sample_create[i].note_num == note_num){
							printf("%d sample creator removed note %d\n", i, note_num);
							//rc = sample_create_remove(&sample_create[0],i);
							sample_create[i].fade = FADE_LENGTH - 1;
							printf("channel %d was removed\n",i);
						}
					}
                }
                input_flg[1] = 0;
            }
            
            // Assume no active sample creators
                sc_flag = 0;
            
            // run the sample creators
                for(i=0;i<CHANNELS;i++){
                    if(sample_create[i].active_flag == 1){
                        clarinet_sample_create(&sample_create[0],i);
                        sc_flag = 1;
                        for(j=0;j<FRAMES;j++){
                            output[j] += sample_create[i].channel[j];
                        }
                    }
					if(sample_create[i].fade < 0){
						sample_create_remove(&sample_create[0],i);
					}
                }
            
            // Send Samples to sound Card
                if(sc_flag = 1){
                    // output via sound card
                    rc = soundcard_output(handle, &output[0]);
                    
                    // Re-0 the buffer
                    for(i=0;i<FRAMES;i++){
                        output[i] = 0;
                    }
                    
                }
        
        }
    
    
    
    return 0;
}

void clarinet_sample_create(struct sample_create_s * sc, int this){
    double temp[6] = {0,0,0,0,0,0};
    int temp1, temp2;
    int i,j,k;
    double  pinc = 0.01;
    double p_delta, refl;
    
    for(k=0;k<FRAMES;k++){
        // Boundary-Layer Loss Filter Calcualtions
        temp[5] = sc[this].delay_up[0];
        sc[this].delay_up[0] = sc[this].Clarinet.BoundaryNum[0]*sc[this].delay_up[0] + sc[this].z_boundary[0][0];
        sc[this].z_boundary[0][0] = sc[this].Clarinet.BoundaryNum[1]*temp[5] - sc[this].Clarinet.BoundaryDen[1]*sc[this].delay_up[0]+sc[this].z_boundary[0][1];
        sc[this].z_boundary[0][1] = sc[this].Clarinet.BoundaryNum[2]*temp[5] - sc[this].Clarinet.BoundaryDen[2]*sc[this].delay_up[0];
        
        temp[5] = sc[this].delay_low[83];
        sc[this].delay_low[83] = sc[this].Clarinet.BoundaryNum[0]*sc[this].delay_low[83] + sc[this].z_boundary[1][0];
        sc[this].z_boundary[1][0] = sc[this].Clarinet.BoundaryNum[1]*temp[5] - sc[this].Clarinet.BoundaryDen[1]*sc[this].delay_low[83] + sc[this].z_boundary[1][1];
        sc[this].z_boundary[1][1] = sc[this].Clarinet.BoundaryNum[2]*temp[5] - sc[this].Clarinet.BoundaryDen[2]*sc[this].delay_low[83];
        
        // Open End Filter Calculations
        temp[5] = sc[this].delay_up[83];
        sc[this].delay_low[0] = sc[this].Clarinet.OpenEndNum[0] * temp[5] + sc[this].z_open[0];
        sc[this].z_open[0] = sc[this].Clarinet.OpenEndNum[1]*temp[5] - sc[this].Clarinet.OpenEndDen[1]*sc[this].delay_low[0]+sc[this].z_open[1];
        sc[this].z_open[1] = -sc[this].Clarinet.OpenEndDen[2] * sc[this].delay_low[0];
        
        // Tone hole Calculations
        temp[3] = 0;
        
        for(j=0;j<sc[this].Clarinet.NumToneHoles;j++){
            
            temp1 = sc[this].Clarinet.Dsum[j]-1;
            temp2 = 83 - sc[this].Clarinet.Dsum[j];
            if(sc[this].note.Finger[j] == 1){
                
                // Tone Hole Reflection Filter using direct form II transposed
                temp[5] = sc[this].p_branch[j];
                sc[this].p_branch[j] = sc[this].Clarinet.ClosedToneHoleNum[0] * sc[this].p_branch[j] + sc[this].z_branch[j];
                
                sc[this].z_branch[j] = sc[this].Clarinet.ClosedToneHoleNum[1] * temp[5] - sc[this].Clarinet.ClosedToneHoleDen[1] * sc[this].p_branch[j];
                temp[0] = sc[this].Clarinet.ToneHoler0 * (sc[this].delay_up[temp1] + sc[this].delay_low[temp2] - 2 * sc[this].p_branch[j]);
                sc[this].p_branch[j] = temp[0] + sc[this].delay_up[temp1] + sc[this].delay_low[temp2] - sc[this].p_branch[j];
                
            } else {
                // Tone Hole Reflection Filter using direct form II transposed
                temp[5] = sc[this].p_branch[j];
                sc[this].p_branch[j] = sc[this].Clarinet.OpenToneHoleNum[0] * sc[this].p_branch[j] + sc[this].z_branch[j];
                
                sc[this].z_branch[j] = sc[this].Clarinet.OpenToneHoleNum[1] * temp[5] - sc[this].Clarinet.OpenToneHoleDen[1] * sc[this].p_branch[j];
                temp[0] = sc[this].Clarinet.ToneHoler0 * (sc[this].delay_up[temp1] + sc[this].delay_low[temp2] - 2 * sc[this].p_branch[j]);
                sc[this].p_branch[j] = temp[0] + sc[this].delay_up[temp1] + sc[this].delay_low[temp2] - sc[this].p_branch[j];
            }
            
            if(sc[this].p_branch[j] > 1){
                sc[this].p_branch[j] = 1;
            } else if (sc[this].p_branch[j] < -1){
                sc[this].p_branch[j] = -1;
            }
            
            sc[this].delay_up[temp1]  += temp[0];
            sc[this].delay_low[temp2] += temp[0];
            temp[3] += (sc[this].p_branch[j] / (2 * sc[this].Clarinet.NumToneHoles));
        
        }
        
        // Excitation Calculations
        if( sc[this].delay_low[83] > 6){
            sc[this].delay_low[83] = 6;
        } else if(sc[this].delay_low[83] < -6){
            sc[this].delay_low[83] = -6;
        }
        
        // Excitation Calculations
        p_delta = 0.5*sc[this].p_oc - sc[this].delay_low[83];
        
        // Pressure dependant reflection coefficient
        refl = 0.7*0.4 + p_delta;
        if(refl > 1.0) refl = 1;
        
        // The system output is determined at the input to the air column. The oral cavity is
        // incremented until it reaches the desired value
        if(sc[this].p_oc < 1) sc[this].p_oc += pinc;
        
		// Calculate output
        sc[this].channel[k] = sc[this].delay_up[0] + sc[this].delay_low[83] + temp[3]/2.0;
		// Sclae output for end
		if(sc[this].fade < FADE_LENGTH){
			if(sc[this].fade > 0){
				sc[this].channel[k] *= (double) sc[this].fade / (double) FADE_LENGTH;
			} else {
				sc[this].channel[k] = 0;
			}
			sc[this].fade --;
		}

		if(sc[this].note_num == 27) sc[this].channel[k] *= .6;
        
        // Shift delay lines
        for(j=83;j>0;j--){
            sc[this].delay_up[j]  = sc[this].delay_up[j-1];
            sc[this].delay_low[j] = sc[this].delay_low[j-1];
        }
        sc[this].delay_low[0] = 0;
        sc[this].delay_up[0] = 0.5*sc[this].p_oc - p_delta*refl;
		
		sc[this].cycle_count ++;
	}
}


int sample_create_add(struct sample_create_s * sample_create, struct note_s * note, int note_num, int new){
    
    int i;
    
    sample_create[new].active_flag = 1;
    sample_create[new].note_num = note_num;
    sample_create[new].note = note[note_num];
    sample_create[new].fade = FADE_LENGTH + 1;
    // 0 the channel
    for(i=0;i<FRAMES;i++){
        sample_create[new].channel[i] = 0.0;
    }
    
    // Set other varaiables to initail value
    for(i=0;i<84;i++){
        sample_create[new].delay_up[i] = 0.0;
        sample_create[new].delay_low[i] = 0.0;
    }
    
    for(i=0;i<2;i++){
        sample_create[new].z_open[i] = 0.0;
        sample_create[new].z_boundary[0][i] = 0.0;
        sample_create[new].z_boundary[1][i] = 0.0;
    }
    
    for(i=0;i<11;i++){
        sample_create[new].z_branch[i] = 0.0;
        sample_create[new].p_branch[i] = 0.0;
    }
    
    sample_create[new].p_oc = 0.0;
    
    return 0;
}


int sample_create_remove(struct sample_create_s * sample_create, int this){
    sample_create[this].active_flag = 0;
    sample_create[this].note_num = 9999;
    return 0;
    
}

    
int soundcard_output(snd_pcm_t * handle, double * output){
        int sample;
        int amp = 8000;	// TODO: Change it to volume control
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
    
static void * keyboard_input(void * args){
    int * main_flg = (int *) args;
    int * KB_flg = (int *) args + 1;
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
            switch(mididata[0]){
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
                    printf("User Selected flute\n");
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
                case 130: // harpsicord
                    printf("User Selected harpsichord\n");
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
                    printf("User Selected Clarinet\n");
                    break;
                case 132: // Guitar
                    printf("User Selected guitar\n");
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




