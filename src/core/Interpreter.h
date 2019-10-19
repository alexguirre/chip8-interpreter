#pragma once
#include "Constants.h"
#include "Context.h"
#include "Instructions.h"
#include "Platform.h"
#include <array>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <optional>

namespace c8
{
	class CInterpreter
	{
	public:
		using Clock = std::chrono::high_resolution_clock;

	private:
		std::shared_ptr<IPlatform> mPlatform;
		SContext mContext;
		Clock::time_point mLastCycleTime;
		Clock::time_point mLastTimerTickTime;
		bool mPaused;

	public:
		CInterpreter(const std::shared_ptr<IPlatform>& platform);

		CInterpreter(CInterpreter&&) = default;
		CInterpreter& operator=(CInterpreter&&) = default;

		CInterpreter(const CInterpreter&) = delete;
		CInterpreter& operator=(const CInterpreter&) = delete;

		inline const SContext& Context() const { return mContext; }
		inline bool IsPaused() const { return mPaused; }

		void Pause(bool pause);
		void Update();
		void Step();

		void LoadProgram(const std::filesystem::path& filePath);
		void LoadState(const std::filesystem::path& filePath);
		void SaveState(const std::filesystem::path& filePath) const;

	private:
		void DoCycle();
		void DoTimerTick();
		void DoBeep();
	};
}
