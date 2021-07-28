#include "pch.h"
#include "CountdownTimer.h"

bool CountdownTimer::Init(unsigned sec, unsigned min, unsigned hours) {
	_seconds = sec;
	_minutes = min;
	_hours = hours;

	_running = _paused = false;

	_totalSec = _hours * 3600 + _minutes * 60 + _seconds;
	if (_totalSec == 0)
		return false;

	return true;
}

unsigned CountdownTimer::GetSeconds() const {
	return _seconds;
}

unsigned CountdownTimer::GetMinutes() const {
	return _minutes;
}

CString CountdownTimer::GetRemaingTimeAsString() {
	auto tick = _paused ? _pause : ::GetTickCount64();
	auto sec = _running ? (tick - _current) / 1000 : 0;
	unsigned min = 0;
	if (sec > _totalSec) {
		sec = 0;
		_expired = true;
	}
	else {
		sec = _totalSec - sec;
		min = (unsigned)(sec / 60);
		sec -= min * 60;
	}
	CString text;
	text.Format(L"%02d:%02d", min, sec);

	return text;
}

bool CountdownTimer::Start(std::function<void()> whenDone) {
	if (_running) {
		if (_paused)
			Resume();
		return true;
	}

	_running = true;
	_paused = false;
	_expired = false;
	_current = ::GetTickCount64();
	return true;
}

bool CountdownTimer::Pause() {
	if (!_running)
		return false;
	if (_paused)
		return true;

	_paused = true;
	_pause = ::GetTickCount64();
	return true;
}

bool CountdownTimer::Resume() {
	if (!_paused)
		return false;

	_paused = false;
	_current += (::GetTickCount64() - _pause);

	return true;
}

void CountdownTimer::Stop() {
	_running = false;
	_expired = false;
	Init(_seconds, _minutes, _hours);
}

bool CountdownTimer::IsRunning() const {
	return _running;
}

bool CountdownTimer::IsExpired() const {
	return _expired;
}

bool CountdownTimer::IsPaused() const {
	return _paused;
}
