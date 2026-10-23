#pragma once

class CountdownTimer {
public:
	bool Init(unsigned sec, unsigned min = 0);

	unsigned GetSeconds() const;
	unsigned GetMinutes() const;

	// returns true only on the call where the countdown reaches zero
	bool Update();
	CString GetRemainingTimeAsString() const;

	bool Start();
	bool Pause();
	bool Resume();
	void Stop();

	bool IsRunning() const;
	bool IsExpired() const;
	bool IsPaused() const;

private:
	unsigned GetElapsedSeconds() const;

	unsigned _minutes{ 1 }, _seconds{ 0 };
	unsigned _totalSec{ 0 };
	DWORD64 _current, _pause;
	bool _running{ false }, _paused{ false };
	bool _expired{ false };
};

