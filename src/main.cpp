#include <string>
#include <iostream>
#include "AL/alc.h"
#include "AL/al.h"
#include "AL/alext.h"
#include <vector>
#include <inttypes.h>
#include "sndfile.h"
#include <time.h>

ALuint AddSound(const char* filename)
{
    ALenum err, format;
    ALuint buff;
    SNDFILE* sndfile;
    SF_INFO sfinfo;
    short* membuf = nullptr;
    sf_count_t numFrames;
    ALsizei numBytes;

    sndfile = sf_open(filename, SFM_READ, &sfinfo);
    if (!sndfile)
    {
        std::cout << "Couldn't open the audio " << filename << std::endl;
        return 0;
    }

    format = AL_NONE;
    if (sfinfo.channels == 1)
        format = AL_FORMAT_MONO16;
    else if (sfinfo.channels == 2)
        format = AL_FORMAT_STEREO16;
    else if (sfinfo.channels == 3)
    {
        if (sf_command(sndfile, SFC_WAVEX_GET_AMBISONIC, NULL, 0) == SF_AMBISONIC_B_FORMAT)
            format = AL_FORMAT_BFORMAT2D_16;
    }
    else if (sfinfo.channels == 4)
    {
        if (sf_command(sndfile, SFC_WAVEX_GET_AMBISONIC, NULL, 0) == SF_AMBISONIC_B_FORMAT)
            format = AL_FORMAT_BFORMAT3D_16;
    }

    if(!format)
    {
        std::cout << "Unsupported channel count of " << filename << std::endl;
        sf_close(sndfile);
        return 0;
    }

//	membuf = static_cast<short*>(malloc((size_t)(sfinfo.frames * sfinfo.channels) * sizeof(short)));
	membuf = new short[(sfinfo.frames * sfinfo.channels)];

    numFrames = sf_readf_short(sndfile, membuf, sfinfo.frames);
    if (numFrames < 1)
    {
	    delete membuf;
        sf_close(sndfile);
        std::cout << "Failed to read samples of " << filename << std::endl;
        return 0;
    }

    numBytes = (ALsizei)(numFrames * sfinfo.channels) * (ALsizei)sizeof(short);

    buff = 0;
    alGenBuffers(1, &buff);
    alBufferData(buff, format, membuf, numBytes, sfinfo.samplerate);
    delete membuf;
    sf_close(sndfile);

    err = alGetError();
    if (err != AL_NO_ERROR)
    {
        std::cout << "OpenAL Error : " << alGetString(err) << std::endl;
        if (buff && alIsBuffer(buff))
            alDeleteBuffers(1, &buff);
        return 0;
    }

    return buff;
}


int main(int argc, char** argv)
{
    std::cout << "oui" << std::endl;
    static ALCdevice* cDevice = alcOpenDevice(nullptr);
    if (!cDevice)
        throw("Couldn't get the sound device");

    static ALCcontext* cContext = alcCreateContext(cDevice, nullptr);
    if (!cContext)
        throw("Couldn't create a context");

    if (!alcMakeContextCurrent(cContext))
        throw ("Couldn't make the context current");

    const ALCchar* name = nullptr;
    if (alcIsExtensionPresent(cDevice, "ALC_ENUMERATE_ALL_EXT"))
        name = alcGetString(cDevice, ALC_ALL_DEVICES_SPECIFIER);
    if (!name || alcGetError(cDevice) != AL_NO_ERROR)
        name = alcGetString(cDevice, ALC_DEVICE_SPECIFIER);
    std::cout << name << std::endl;

    std::vector<ALuint> SoundsBuffer;
	if(argc == 1)
        SoundsBuffer.push_back(AddSound("es.ogg"));
	else
		SoundsBuffer.push_back(AddSound(argv[1]));

	float gain = 0.2f;
	if(argc == 3)
		gain = std::stof(argv[2]);

    ALuint sourceID = 0;
    float pitch = 1.f;
    float pos[3] = {0, 0, 0};
    float velo[3] = {0,0,0};
    bool loop = true;
    ALuint currBuff = 0;


    alGenSources(1, &sourceID);
    alSourcef(sourceID, AL_PITCH, pitch);
    alSourcef(sourceID, AL_GAIN, gain);
    alSource3f(sourceID, AL_POSITION, pos[0], pos[1], pos[2]);
    alSource3f(sourceID, AL_VELOCITY,velo[0], velo[1], velo[2]);
    alSourcei(sourceID, AL_LOOPING, loop);
    alSourcei(sourceID, AL_BUFFER, currBuff);

    if (currBuff != SoundsBuffer[0])
    {
        currBuff = SoundsBuffer[0];
        alSourcei(sourceID, AL_BUFFER, (ALint)currBuff);
    }

    alSourcePlay(sourceID);

    ALint s = AL_PLAYING;
    printf("PLAY\n");
    while(s == AL_PLAYING && alGetError() == AL_NO_ERROR)
    {
        printf("In WHILE\n");
        alGetSourcei(sourceID, AL_SOURCE_STATE, &s);
    }
    printf("FINISH\n");
    return 0;
}