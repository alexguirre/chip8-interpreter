#pragma once
#include <array>
#include <cstdint>
#include <filesystem>
#include <chrono>
#include <optional>
#include <functional>
#include "Instructions.h"
#include "Constants.h"
#include "Context.h"
#include "Platform.h"

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
		const SInstruction& FindInstruction(std::uint16_t opcode) const;
		std::optional<std::reference_wrapper<const SInstruction>> TryFindInstruction(std::uint16_t opcode) const;

	private:
		void DoCycle();
		void DoTimerTick();
		void DoBeep();
	};
}
