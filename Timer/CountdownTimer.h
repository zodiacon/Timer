#pragma once

class CountdownTimer {
public:
	bool Init(unsigned sec, unsigned min = 0);

	unsigned GetSeconds() const;
	unsigned GetMinutes() const;

	// a stopwatch counts up from zero and never expires; switching resets the timer
	void SetStopwatch(bool stopwatch);
	bool IsStopwatch() const;

	// returns true only on the call where the countdown reaches zero
	bool Update();
	// remaining time for a countdown, elapsed time for a stopwatch
	CString GetTimeAsString() const;
	// fraction of the interval that has elapsed, 0 to 1
	double GetProgress() const;

	bool Start();
	bool Pause();
	bool Resume();
	void Stop();

	bool IsRunning() const;
	bool IsExpired() const;
	bool IsPaused() const;

private:
	DWORD64 GetElapsedMilliseconds() const;
	unsigned GetElapsedSeconds() const;

	unsigned _minutes{ 1 }, _seconds{ 0 };
	unsigned _totalSec{ 0 };
	DWORD64 _current, _pause;
	bool _running{ false }, _paused{ false };
	bool _expired{ false };
	bool _stopwatch{ false };
};

