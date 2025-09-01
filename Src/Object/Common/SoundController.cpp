#include "../../Application.h"

#include "SoundController.h"

SoundController::SoundController(void)
{
}
SoundController::~SoundController(void)
{
}

void SoundController::Add(int type, std::wstring str, float vol)
{
	auto res = std::make_unique<Sound>(Sound::TYPE::SOUND_2D, str);
	res->ChengeMaxVolume(vol);
	soundMap_.emplace(type, std::move(res));
}

void SoundController::Init(void)
{
	
}

void SoundController::Release(void)
{
	for (auto& p : soundMap_)
	{
		p.second->Release();  // サウンドのリソースを解放

		// p.second が動的に確保されたオブジェクトであれば、以下のようにメモリ解放
		// delete p.second; // 必要に応じて、メモリを解放する
	}

	soundMap_.clear();  // マップをクリア
}

void SoundController::Destroy(void)
{
	Release();
}

bool SoundController::Play(int src, Sound::TIMES times, bool isForce)
{
	const auto& lPair = soundMap_.find(src);
	if (lPair != soundMap_.end())
	{
		if (!lPair->second->CheckLoad())
		{
			lPair->second->Load();
		}
		return lPair->second->Play(times, isForce);
	}
	return false;
}

bool SoundController::Play(int src, Sound::TIMES times, VECTOR pos, float radius)
{
	const auto& lPair = soundMap_.find(src);
	if (lPair != soundMap_.end())
	{
		if (!lPair->second->CheckLoad())
		{
			lPair->second->Load();
		}
		return lPair->second->Play(pos, radius, times);
	}
	return false;
}

void SoundController::Stop(int src)
{
	const auto& lPair = soundMap_.find(src);
	if (lPair != soundMap_.end())//指定のものが存在するか？
	{
		return lPair->second->Stop();
	}
}

void SoundController::AllStop(void)
{
	// soundMap_ のすべての要素に対して Stop() を呼び出す
	for (auto& lPair : soundMap_)
	{
		lPair.second->Stop();
	}
}

bool SoundController::CheckMove(int src)
{
	const auto& lPair = soundMap_.find(src);
	if (lPair != soundMap_.end())
	{
		return lPair->second->CheckMove();
	}
	return false;
}

void SoundController::ChengeVolume(int src, float per)
{
	const auto& lPair = soundMap_.find(src);
	if (lPair != soundMap_.end())
	{
		return lPair->second->ChengeVolume(per);
	}
}
