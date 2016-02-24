#pragma once
#include <chrono>
namespace Library
{
	class GameTime final
	{
	public:
		GameTime();
		GameTime(const GameTime& rhs);
		GameTime& operator=(const GameTime& rhs);

		const std::chrono::high_resolution_clock::time_point& CurrentTime() const;
		void SetCurrentTime(const std::chrono::high_resolution_clock::time_point& currentTime);

		const std::chrono::milliseconds& TotalGameTime() const;
		void SetTotalGameTime(const std::chrono::milliseconds& totalTime);

		const std::chrono::milliseconds& ElapsedGameTime() const;
		void SetElapsedGameTime(const std::chrono::milliseconds& elapsedTime);

		void Update();
	private:
		std::chrono::high_resolution_clock::time_point mCurrentTime;
		std::chrono::milliseconds mTotalTime;
		std::chrono::milliseconds mElapsedTime;
	};
}


