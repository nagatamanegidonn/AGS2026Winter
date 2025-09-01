#include <DxLib.h>
#include "Measure.h"

Measure* Measure::mInstance = nullptr;

void Measure::CreateInstance(void)
{
	if (mInstance == nullptr)
	{
		mInstance = new Measure();
	}
	mInstance->Init();
}

Measure& Measure::GetInstance(void)
{
	return *mInstance;
}

void Measure::Init(void)
{
	mInitTime = std::chrono::system_clock::now();
	mStartTime = std::chrono::system_clock::now();
	mPreFrameTime = std::chrono::system_clock::now();
	mCntTotalFrame = 0;
}

void Measure::Start(void)
{
	mStartTime = std::chrono::system_clock::now();
	mTotalTime =
		std::chrono::duration_cast<std::chrono::nanoseconds>(mStartTime - mInitTime).count() / 1000000000.0;
	mCntTotalFrame++;

	auto deltaTime =
		std::chrono::duration_cast<std::chrono::nanoseconds>(mStartTime - mPreFrameTime).count() / 1000000000.0;
	mPreFrameTime = mStartTime;

	if (mAverage.size() >= AVG_FRAME)
	{
		mAverage.erase(mAverage.begin());
	}
	mAverage.push_back(deltaTime);

}

void Measure::Watch(std::string name)
{

	if (mWatchTimes.count(name) == 0)
	{
		mWatchTimes.emplace(name, 0.0);
	}
	else
	{
		auto nowTime = std::chrono::system_clock::now();
		auto deltaTime = 
			std::chrono::duration_cast<std::chrono::nanoseconds>(nowTime - mStartTime).count() / 1000000000.0;
		mWatchTimes[name] += deltaTime;
		mStartTime = nowTime;
	}

}

void Measure::Draw(void)
{

#ifdef _DEBUG

	if (IS_DISABLE)
	{
		return;
	}

	const int HEIGHT = 20;
	int y = 0;

	y += HEIGHT;
	DrawFormatString(0, y, 0xffffff, L"%-15s : %.3f", L"é¿çséûä‘", mTotalTime);

	y += HEIGHT;
	DrawFormatString(0, y, 0xffffff, L"%-15s : %d", L"é¿çsÉtÉåÅ[ÉÄ", mCntTotalFrame);

	double avg = 0.0;
	for (auto d : mAverage)
	{
		avg += d;
	}
	double fps = 1.0 / (avg / static_cast<double>(AVG_FRAME));

	y += HEIGHT;
	DrawFormatString(0, y, 0xffffff, L"%-15s : %.3f", L"ïΩãœÇeÇoÇr", fps);

	for (const auto& watch : mWatchTimes)
	{
		y += HEIGHT;
		DrawFormatString(0, y, 0xffffff, L"%-15s : %.3f", watch.first.c_str(), watch.second);
	}

#endif // _DEBUG

}

void Measure::Reset(void)
{
	mWatchTimes.clear();
}

Measure::Measure(void)
{
	mInitTime = std::chrono::system_clock::now();
	mCntTotalFrame = 0;
	mTotalTime = 0.0;
}

Measure::~Measure(void)
{
	delete mInstance;
}
