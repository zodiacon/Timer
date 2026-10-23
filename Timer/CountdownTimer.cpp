#include "pch.h"
#include "CountdownTimer.h"

bool CountdownTimer::Init(unsigned sec, unsigned min) {
	_seconds = sec;
	_minutes = min;

	_running = _paused = _expired = false;

	_totalSec = _minutes * 60 + _seconds;
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

DWORD64 CountdownTimer::GetElapsedMilliseconds() const {
	if (!_running)
		return 0;
	auto tick = _paused ? _pause : ::GetTickCount64();
	return tick - _current;
}

unsigned CountdownTimer::GetElapsedSeconds() const {
	return (unsigned)(GetElapsedMilliseconds() / 1000);
}

double CountdownTimer::GetProgress() const {
	if (_totalSec == 0)
		return 0;
	auto progress = GetElapsedMilliseconds() / (_totalSec * 1000.0);
	return progress > 1 ? 1 : progress;
}

bool CountdownTimer::Update() {
	if (!_running || _expired || GetElapsedSeconds() < _totalSec)
		return false;

	_expired = true;
	return true;
}

CString CountdownTimer::GetRemainingTimeAsString() const {
	auto elapsed = GetElapsedSeconds();
	auto sec = elapsed >= _totalSec ? 0 : _totalSec - elapsed;
	CString text;
	text.Format(L"%02u:%02u", sec / 60, sec % 60);

	return text;
}

bool CountdownTimer::Start() {
	// an expired timer starts over
	if (_running && !_expired) {
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
	if (!_running || _expired)
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
	Init(_seconds, _minutes);
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
