#pragma once

#include <vector>

class Chime {
public:
	Chime();

	void Play();
	void Stop();

private:
	std::vector<uint8_t> _wave;
};
