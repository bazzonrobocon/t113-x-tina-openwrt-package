#include "media_spectrum.h"
#include "speex/speex_preprocess.h"

static spectrum_t mspectrum;

void media_spectrum_clear(void)
{
	spectrum_t *sp = &mspectrum;

	if(sp->handle)
	{
		speex_preprocess_state_destroy(sp->handle);
	}
	if(sp->ps_buffer)
	{
		free(sp->ps_buffer);
	}
	if(sp->pcm_data)
	{
		free(sp->pcm_data);
	}
	memset(sp, 0, sizeof(spectrum_t));
}

void media_spectrum_updata(AudioPcmData *audio_info)
{
	int frame_size;
	spectrum_t *sp = &mspectrum;

    if ((sp->init_flag == 1) && ((sp->samplerate != audio_info->samplerate)
        || (sp->channels != audio_info->channels)
        || (sp->size != audio_info->nSize)))
    {
        media_spectrum_clear();
    }

    sp->samplerate = audio_info->samplerate;
    sp->channels = audio_info->channels;
    sp->size = audio_info->nSize;
	if(sp->init_flag)
	{
		goto END;
	}
	if(!audio_info->pData)
	{
		goto END;
	}
	frame_size = audio_info->nSize/(audio_info->channels*sizeof(short));
	sp->handle = speex_preprocess_state_init(frame_size, audio_info->samplerate);
	speex_preprocess_ctl(sp->handle, SPEEX_PREPROCESS_GET_PSD_SIZE, &sp->ps_size);
	if(!sp->ps_buffer)
	{
		sp->ps_buffer = (int *)malloc(sp->ps_size * sizeof(int));
	}
	if(!sp->pcm_data)
	{
		sp->pcm_data = malloc(audio_info->nSize);
	}
	sp->init_flag = 1;

END:
	if(sp->pcm_data)
	{
		memcpy(sp->pcm_data, audio_info->pData, audio_info->nSize);
	}

	return;
}

int *media_get_spectrum(void)
{
	spectrum_t *sp = &mspectrum;

	if(!sp->init_flag)
	{
		return NULL;
	}
	speex_preprocess_estimate_update(sp->handle, (spx_int16_t *)sp->pcm_data);
	speex_preprocess_ctl(sp->handle, SPEEX_PREPROCESS_GET_PSD, sp->ps_buffer);

#if 0
	FILE *fd;
	char buffer[32];
	fd = fopen("/tmp/spectrum.log", "w+");
	for(int i=0; i<sp->ps_size; i++){
		memset(buffer, 0, sizeof(buffer));
		sprintf(buffer, "%u ", sp->ps_buffer[i]);
		fwrite(buffer,1, strlen(buffer),fd);
	}
	memset(buffer, 0, sizeof(buffer));
	sprintf(buffer, "\n");
	fwrite(buffer,1, strlen(buffer),fd);
	fclose(fd);
#endif

	return sp->ps_buffer;
}

int media_get_spectrum_size(void)
{
	spectrum_t *sp = &mspectrum;
	return sp->ps_size;
}
