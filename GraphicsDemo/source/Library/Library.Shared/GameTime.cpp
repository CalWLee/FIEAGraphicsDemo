#pragma once
#include "pch.h"
namespace Library
{
	GameTime::GameTime()
		: mTotalTime(0), mElapsedTime(0), mCurrentTime(std::chrono::high_resolution_clock::now())
	{
	}

	GameTime::GameTime(const GameTime& rhs)
		: mTotalTime(rhs.mTotalTime), mElapsedTime(rhs.mElapsedTime), mCurrentTime(rhs.mCurrentTime)
	{

	}

	GameTime& GameTime::operator=(const GameTime& rhs)
	{
		if (this != &rhs)
		{
			mTotalTime = rhs.mTotalTime;
			mElapsedTime = rhs.mElapsedTime;
			mCurrentTime = rhs.mCurrentTime;
		}

		return *this;
	}

	void GameTime::Update()
	{
		mElapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>((std::chrono::high_resolution_clock::now() - mCurrentTime));
		mCurrentTime = std::chrono::high_resolution_clock::now();

	}

	const std::chrono::high_resolution_clock::time_point& GameTime::CurrentTime() const
	{
		return mCurrentTime;
	}

	void GameTime::SetCurrentTime(const std::chrono::high_resolution_clock::time_point& currentTime)
	{
		mCurrentTime = currentTime;
	}

	const std::chrono::milliseconds& GameTime::TotalGameTime() const
	{
		return mTotalTime;
	}

	void GameTime::SetTotalGameTime(const std::chrono::milliseconds& totalTime)
	{
		mTotalTime = totalTime;
	}


	const std::chrono::milliseconds& GameTime::ElapsedGameTime() const
	{
		return mElapsedTime;
	}

	void GameTime::SetElapsedGameTime(const std::chrono::milliseconds& elapsedTime)
	{
		mElapsedTime = elapsedTime;
	}

}