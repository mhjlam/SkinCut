#include "StopWatch.hpp"

#include <iomanip>
#include <functional>

#include "Util.hpp"


using namespace SkinCut;


StopWatch::StopWatch(ClockType ct) : m_ClockType(ct) {
	QueryPerformanceFrequency(&m_Frequency);
	m_TimeSplits = std::map<std::string, Split>();
}

StopWatch::StopWatch(std::string id, ClockType ct) : m_ClockType(ct) {
	QueryPerformanceFrequency(&m_Frequency);
	m_TimeSplits = std::map<std::string, Split>();

	Start(id);
}

StopWatch::~StopWatch() {
	m_TimeSplits.clear();
}

void StopWatch::Start(std::string id) {
	// disallow empty ids
	if (id.empty()) { return; }

	// see if record exists already
	auto record = m_TimeSplits.find(id);
	if (record != m_TimeSplits.end()) { return; }

	// add new record
	long long time = GetTime();
	m_TimeSplits.insert(make_pair(id, Split(time)));
}

void StopWatch::Stop(std::string id) {
	// attempt to find record
	auto record = m_TimeSplits.find(id);
	if (record == m_TimeSplits.end()) { return; }

	// determine clock frequency
	long long freq = 1L;
	if (m_ClockType == ClockType::QPC_MS) {
		freq = m_Frequency.QuadPart / 1000L;
	}
	else if (m_ClockType == ClockType::QPC_US) {
		freq = m_Frequency.QuadPart / 1000000L;
	}

	// determine elapsed time
	long long time = GetTime();
	long long diff = time - record->second.Start;
	record->second.Elapsed = diff / freq;
}


void StopWatch::Reset() {
	m_TimeSplits.clear();
	m_TimeSplits = std::map<std::string, Split>();
}

void StopWatch::Reset(std::string id, bool start) {
	// attempt to find record
	auto record = m_TimeSplits.find(id);
	if (record == m_TimeSplits.end()) { 
		return;
	}

	// reset clock
	record->second.Start = (start) ? GetTime() : 0L;
	record->second.Elapsed = 0L;
}

long long StopWatch::ElapsedTime(std::string id) {
	// attempt to find record
	auto record = m_TimeSplits.find(id);
	if (record == m_TimeSplits.end()) { 
		return 0L;
	}

	// return elapsed time
	return record->second.Elapsed;
}

void StopWatch::Report(bool terse, bool totalOnly) {
	long long total = 0;

	for (auto it = m_TimeSplits.begin(); it != m_TimeSplits.end(); ++it) {
		auto record = m_TimeSplits.find(it->first);
		total += record->second.Elapsed;

		if (!totalOnly) { Report(it->first, terse); }
	}

	std::stringstream ss;

	if (terse) {
		ss << total;
		Util::ConsoleMessage(ss.str());
		Util::ConsoleMessage();
		return;
	}

	std::string unit = (m_ClockType == ClockType::QPC_US || m_ClockType == ClockType::CHRONO_US) ? " us" : " ms";
	ss << "Total: " << total << unit;

	Util::ConsoleMessage(ss.str());
	Util::ConsoleMessage();
}

void StopWatch::Report(std::string id, bool terse) {
	auto record = m_TimeSplits.find(id);
	if (record == m_TimeSplits.end()) { return; }

	if (terse) {
		std::stringstream ss;
		ss << record->second.Elapsed;
		Util::ConsoleMessage(ss.str());
		return;
	}

	std::string unit = (m_ClockType == ClockType::QPC_US || m_ClockType == ClockType::CHRONO_US) ? " us" : " ms";

	std::stringstream ss;
	ss << id << ": " << record->second.Elapsed << unit;
	Util::ConsoleMessage(ss.str());
}

long long StopWatch::GetTime() {
	switch (m_ClockType) {
		case ClockType::CHRONO_MS: {
			auto now = std::chrono::high_resolution_clock::now().time_since_epoch();
			return std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
		}

		case ClockType::CHRONO_US: {
			auto now = std::chrono::high_resolution_clock::now().time_since_epoch();
			return std::chrono::duration_cast<std::chrono::microseconds>(now).count();
		}

		case ClockType::QPC_MS:
		case ClockType::QPC_US:
		default: {
			LARGE_INTEGER qpc;
			QueryPerformanceCounter(&qpc);
			return qpc.QuadPart;
		}
	}
}
