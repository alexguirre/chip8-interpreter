#include "Sound.h"
#include <stdexcept>
#include <cmath>

// based on https://stackoverflow.com/a/10111570

CSound::CSound()
	: mV{}
{
	SDL_AudioSpec desiredSpec;
	desiredSpec.freq = Frequency;
	desiredSpec.format = AUDIO_S16SYS;
	desiredSpec.channels = 1;
	desiredSpec.samples = 2048;
	desiredSpec.callback = reinterpret_cast<SDL_AudioCallback>(SdlCallback);
	desiredSpec.userdata = this;

	SDL_AudioSpec obtainedSpec;

	mDeviceID = SDL_OpenAudioDevice(
		nullptr, false, &desiredSpec, &obtainedSpec, 0
	);

	if (!mDeviceID)
	{
		throw std::runtime_error("Failed to open audio device: " + std::string(SDL_GetError()));
	}

	SDL_PauseAudioDevice(mDeviceID, 0);
}

CSound::~CSound()
{
	if (mDeviceID)
	{
		SDL_CloseAudioDevice(mDeviceID);
	}
}

void CSound::Beep(double frequency, std::chrono::milliseconds duration)
{
	SDL_LockAudio();
	mBeeps.push({ frequency, static_cast<std::uint32_t>(duration.count() * Frequency / 1000) });
	SDL_UnlockAudio();
}

void CSound::GenerateSamples(gsl::span<std::int16_t> stream)
{
	std::ptrdiff_t i = 0;
	while (i < stream.size())
	{
		if (mBeeps.empty())
		{
			std::fill(stream.begin(), stream.end(), std::int16_t{ 0 });
			return;
		}

		SBeep& b = mBeeps.front();
		
		std::uint32_t samplesToDo = static_cast<std::uint32_t>(std::min(i + b.SamplesLeft, stream.size()));
		b.SamplesLeft -= samplesToDo - static_cast<std::uint32_t>(i);

		while (i < samplesToDo)
		{
			constexpr double Amplitude = 28000;
			constexpr double Pi = 3.14159265358979323846;
			stream[i] = static_cast<std::int16_t>(Amplitude * std::sin(mV * 2 * Pi / Frequency));
			i++;
			mV += b.Frequency;
		}

		if (b.SamplesLeft == 0)
		{
			mBeeps.pop();
		}
	}
}

void CSound::SdlCallback(CSound* self, std::uint8_t* stream, std::int32_t len)
{
	self->GenerateSamples(
		{ reinterpret_cast<std::int16_t*>(stream), len / 2 }
	);
}
