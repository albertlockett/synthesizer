// a header file to handle more general Synthesizer stuff
//

#include <alsa/asoundlib.h>
#include <stdlib.h>


// Functions --------------------------------------------------------

int soundcard_init(int sampling_rate, snd_pcm_uframes_t frame_size, snd_pcm_t *handle){
	// Used to initialize the soundcard:
	int dir = 0;
	int rc = 0;
	snd_pcm_hw_params_t * params;


	// Setup the hardware parameters:
	snd_pcm_hw_params_alloca(&params);
	snd_pcm_hw_params_any(handle, params);
	snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
	snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S16_LE);
	snd_pcm_hw_params_set_channels(handle, params, 2);
	snd_pcm_hw_params_set_rate_near(handle, params, &sampling_rate, &dir);
	snd_pcm_hw_params_set_period_size_near(handle, params, &frame_size, &dir);

	// Write Parametrs to driver:
	rc = snd_pcm_hw_params(handle, params);
	if(rc < 0){
		fprintf(stderr, "unable to write hardware parameters: %s\n"
			,snd_strerror(rc));
		return(1);
	}
	
	return 0;

}
