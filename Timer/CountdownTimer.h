#pragma once

#include <functional>

class CountdownTimer {
public:
	bool Init(unsigned sec, unsigned min = 0, unsigned hours = 0);
	void SetCountForward(bool forward);

	unsigned GetSeconds() const;
	unsigned GetMinutes() const;

	CString GetRemaingTimeAsString();

	bool Start(std::function<void()> whenDone);
	bool Pause();
	bool Resume();
	void Stop();

	bool IsRunning() const;
	bool IsExpired() const;
	bool IsPaused() const;

private:
	unsigned _hours{ 0 }, _minutes{ 1 }, _seconds{ 0 };
	unsigned _totalSec{ 0 };
	DWORD64 _current, _pause;
	bool _running{ false }, _paused{ false };
	bool _expired{ false };
};

