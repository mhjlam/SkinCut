#pragma once

#include <map>
#include <ctime>
#include <chrono>
#include <memory>
#include <sstream>
#include <iostream>

#include <Windows.h>


namespace SkinCut {
	enum class ClockType {
		QPC_MS,
		QPC_US,
		CHRONO_MS,
		CHRONO_US
	};

	class StopWatch {
	public:
		StopWatch(ClockType clockType = ClockType::QPC_US);
		StopWatch(std::string id, ClockType clockType = ClockType::QPC_US);
		~StopWatch();

		void Start(std::string id);
		void Stop(std::string id);
		void Reset();
		void Reset(std::string id, bool start = true);

		long long ElapsedTime(std::string id);

		void Report(bool terse = false, bool totalOnly = false);
		void Report(std::string id, bool terse = false);

	private:
		long long GetTime();

	private:
		struct Split {
			long long Start;
			long long Elapsed;

			Split() {
				Start = 0L;
				Elapsed = 0L;
			}

			Split(long long startTime) {
				Start = startTime;
				Elapsed = 0L;
			}
		};

		ClockType m_ClockType;
		LARGE_INTEGER m_Frequency;
		std::map<std::string, Split> m_TimeSplits;
	};
}
