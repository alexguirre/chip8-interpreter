#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_audio.h>
#include <chrono>
#include <cstdint>
#include <gsl/span>
#include <queue>

class CSound
{
private:
	static constexpr std::int32_t Frequency{ 44100 };

	struct SBeep
	{
		double Frequency;
		std::uint32_t SamplesLeft;
	};

	SDL_AudioDeviceID mDeviceID;
	std::queue<SBeep> mBeeps;
	double mV;

public:
	CSound();
	~CSound();

	void Beep(double frequency, std::chrono::milliseconds duration);

private:
	void GenerateSamples(gsl::span<std::int16_t> stream);

	static void SdlCallback(CSound* self, std::uint8_t* stream, std::int32_t len);
};
