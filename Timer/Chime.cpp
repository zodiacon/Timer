#include "pch.h"
#include "Chime.h"
#include <mmsystem.h>
#include <cmath>
#include <algorithm>

#pragma comment(lib, "winmm")

namespace {
	constexpr uint32_t SamplesPerSec = 44100;
	constexpr double Duration = 2.5;
	constexpr double Pi = 3.14159265358979323846;

#pragma pack(push, 1)
	struct WaveHeader {
		char Riff[4]{ 'R', 'I', 'F', 'F' };
		uint32_t RiffSize;
		char Wave[4]{ 'W', 'A', 'V', 'E' };
		char Fmt[4]{ 'f', 'm', 't', ' ' };
		uint32_t FmtSize{ 16 };
		uint16_t Format{ WAVE_FORMAT_PCM };
		uint16_t Channels{ 1 };
		uint32_t SampleRate{ SamplesPerSec };
		uint32_t ByteRate{ SamplesPerSec * 2 };
		uint16_t BlockAlign{ sizeof(int16_t) };
		uint16_t BitsPerSample{ 16 };
		char Data[4]{ 'd', 'a', 't', 'a' };
		uint32_t DataSize;
	};
#pragma pack(pop)
}

Chime::Chime() {
	// rising C-E-G arpeggio, each note a soft bell with a few decaying harmonics
	const struct {
		double Freq;
		double Start;
	} notes[] = {
		{ 1046.50, 0.00 },
		{ 1318.51, 0.18 },
		{ 1567.98, 0.36 },
	};
	const struct {
		double Ratio, Amp, Decay;
	} partials[] = {
		{ 1.0, 1.00, 2.5 },
		{ 2.0, 0.35, 5.0 },
		{ 3.0, 0.12, 8.0 },
	};

	const int count = (int)(SamplesPerSec * Duration);
	std::vector<double> samples(count);
	double peak = 0;
	for (int i = 0; i < count; i++) {
		double t = (double)i / SamplesPerSec;
		double v = 0;
		for (auto& n : notes) {
			double nt = t - n.Start;
			if (nt < 0)
				continue;
			double attack = nt < 0.005 ? nt / 0.005 : 1.0;
			for (auto& p : partials)
				v += attack * p.Amp * std::exp(-nt * p.Decay) * std::sin(2 * Pi * n.Freq * p.Ratio * nt);
		}
		// fade out the tail to avoid a click at the end
		double remaining = Duration - t;
		if (remaining < 0.1)
			v *= remaining / 0.1;
		samples[i] = v;
		peak = (std::max)(peak, std::abs(v));
	}

	WaveHeader header;
	header.DataSize = (uint32_t)(count * sizeof(int16_t));
	header.RiffSize = (uint32_t)(sizeof(header) - 8 + header.DataSize);

	_wave.resize(sizeof(header) + header.DataSize);
	memcpy(_wave.data(), &header, sizeof(header));
	auto data = (int16_t*)(_wave.data() + sizeof(header));
	const double scale = 0.5 * 32767 / peak;
	for (int i = 0; i < count; i++)
		data[i] = (int16_t)(samples[i] * scale);
}

void Chime::Play() {
	::PlaySound((PCWSTR)_wave.data(), nullptr, SND_MEMORY | SND_ASYNC | SND_NODEFAULT);
}

void Chime::Stop() {
	::PlaySound(nullptr, nullptr, 0);
}
