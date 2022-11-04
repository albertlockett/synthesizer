// a header file to handle more general Synthesizer stuff
//

#include <alsa/asoundlib.h>
#include <stdlib.h>


// Functions --------------------------------------------------------

int soundcard_init(int sampling_rate, snd_pcm_uframes_t frame_size, snd_pcm_t *handle){
	// Used to initialize the soundcard:
	int dir = 0;
	int rc = 0;
	int val;
	snd_pcm_hw_params_t * params;
	snd_pcm_uframes_t tmp_frames = 2375; // best is 2375

	// Setup the hardware parameters:
	snd_pcm_hw_params_alloca(&params);
	snd_pcm_hw_params_any(handle, params);
	snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
	snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S16_LE);
	snd_pcm_hw_params_set_channels(handle, params, 2);
	snd_pcm_hw_params_set_rate_near(handle, params, &sampling_rate, &dir);
	snd_pcm_hw_params_set_period_size_near(handle, params, &frame_size, &dir);
	
	
	
	rc = snd_pcm_hw_params_set_buffer_size_near(handle, params, &tmp_frames);
	if(rc != 0) printf("problem setting the buffer size\n");
	
	// debugging the problem of the delay: buffer size hypothesis
	//printf("HARDWARE PARAMETERS ---\n");
	
	rc = snd_pcm_hw_params_get_buffer_time(params, &val, &dir);
	if(rc != 0) printf("problem getting buff time\n");
	//printf("buffer time\t-\t%d\n",val);
	
	rc = snd_pcm_hw_params_get_buffer_time_max(params, &val, &dir);
	if(rc != 0) printf("problem getting max buff time\n");
	//printf("max buff time\t-\t%d\n",val);
	
	rc = snd_pcm_hw_params_get_buffer_time_min(params, &val, &dir);
	if(rc != 0) printf("problem getting min buff time\n");
	//printf("min buff time\t-\t%d\n",val);
	
	rc = snd_pcm_hw_params_get_buffer_size(params, &tmp_frames);
	if(rc != 0) printf("problem getting buff size\n");
	//printf("buffer size\t-\t%u\n",&tmp_frames);
	
	rc = snd_pcm_hw_params_get_buffer_size_min(params, &tmp_frames);
	if(rc != 0) printf("problem getting max buff size\n");
	//printf("min buff size\t-\t%u\n",&tmp_frames);
	
	rc = snd_pcm_hw_params_get_buffer_size_max(params, &tmp_frames);
	if(rc != 0) printf("problem getting min buff size\n");
	//printf("max buff size\t-\t%u\n",&tmp_frames);
	


	// Write Parametrs to driver:
	rc = snd_pcm_hw_params(handle, params);
	if(rc < 0){
		fprintf(stderr, "unable to write hardware parameters: %s\n"
			,snd_strerror(rc));
		return(1);
	}
	
	return 0;

}

int alsa_sw_init(snd_pcm_t * handle){
	int rc = 0;
	snd_pcm_sw_params_t * sw_params;
	
	// tmp vars
	snd_pcm_uframes_t tmp_frames = 15;
	
	
	// Allocate memory for the parameters
	snd_pcm_sw_params_alloca(&sw_params);
	
	// Get current sw parameters	
	rc = snd_pcm_sw_params_current(handle, sw_params);
	if(rc < 0){
		fprintf(stderr, "can't get ALSA sw params, %s\n", snd_strerror(rc));
		return(1);
	}
	
	
	// Add the config code here
	//printf("SOFTWARE PARAMETERS ---\n");
	snd_pcm_sw_params_get_start_threshold(sw_params, &tmp_frames);
	//printf("Start threshold\t-\t%d frames\n",tmp_frames);
	
	snd_pcm_sw_params_get_stop_threshold(sw_params, &tmp_frames);
	//printf("Stop threshold\t-\t%d frames\n",tmp_frames);
	
	//printf("resetting stop threshold\n");
	//tmp_frames = tmp_frames/1.3;
	//snd_pcm_sw_params_set_stop_threshold(handle, sw_params,tmp_frames);
	
	//snd_pcm_sw_params_get_stop_threshold(sw_params, &tmp_frames);
	//printf("Stop threshold\t-\t%d frames\n",tmp_frames);
	
	snd_pcm_sw_params_get_silence_threshold(sw_params, &tmp_frames);
	//printf("Silence thresh\t-\t%d frames\n",tmp_frames);
	
	snd_pcm_sw_params_get_silence_size(sw_params, &tmp_frames);
	//printf("Silence size\t-\t%d frames\n",tmp_frames);
	
	
	// Write parameters to driver
	rc = snd_pcm_sw_params(handle, sw_params);
	if(rc < 0){
		fprintf(stderr, "can't write ALSA sw params, %s\n",snd_strerror(rc));
		return (1);
	}
	
	
	
	return 0;
}
