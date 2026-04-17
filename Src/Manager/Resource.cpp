#include <DxLib.h>
#include <EffekseerForDXLib.h>
#include "Resource.h"

Resource::Resource(void)
	:
	type_(TYPE::NONE),
	path_(L""),
	numX_(-1),
	numY_(-1),
	sizeX_(-1),
	sizeY_(-1),
	handleId_(-1),
	handleIds_(nullptr)
{
}

Resource::Resource(TYPE type, const std::wstring& path)
	:
	type_(type),
	path_(path),
	numX_(-1),
	numY_(-1),
	sizeX_(-1),
	sizeY_(-1),
	handleId_(-1),
	handleIds_(nullptr)
{
}

Resource::Resource(TYPE type, const std::wstring& path, int numX, int numY, int sizeX, int sizeY)
	:
	type_(type),
	path_(path),
	numX_(numX),
	numY_(numY),
	sizeX_(sizeX),
	sizeY_(sizeY),
	handleId_(-1),
	handleIds_(nullptr)
{
}

Resource::~Resource(void)
{
}

void Resource::Load(void)
{

	switch (type_)
	{
	case Resource::TYPE::IMG:
		// 画像
		handleId_ = LoadGraph(path_.c_str());
		break;

	case Resource::TYPE::IMGS:
		// 複数画像
		handleIds_ = new int[numX_ * numY_];
		LoadDivGraph(
			path_.c_str(),
			numX_ * numY_,
			numX_, numY_,
			sizeX_, sizeY_,
			&handleIds_[0]);
		break;

	case Resource::TYPE::MODEL:
	{
		auto it = g_modelCache.find(path_);
		if (it != g_modelCache.end()) {
			// すでに読み込まれていれば複製して使う
			handleId_ = MV1DuplicateModel(it->second);
		}
		else {
			// 初回読み込みならキャッシュしておく
			int baseHandle = MV1LoadModel(path_.c_str());
			g_modelCache[path_] = baseHandle;
			handleId_ = MV1DuplicateModel(baseHandle);
		}

		// 削除時に複製分を消すため保存（元のは g_modelCache が保持）
		duplicateModelIds_.push_back(handleId_);
	}
		// モデル
		//handleId_ = MV1LoadModel(path_.c_str());
		break;

	case Resource::TYPE::EFFEKSEER:

		handleId_ = LoadEffekseerEffect(path_.c_str());
		break;

	}

}

void Resource::Release(void)
{

	switch (type_)
	{
	case Resource::TYPE::IMG:
		DeleteGraph(handleId_);
		break;

	case Resource::TYPE::IMGS:
	{
		int num = numX_ * numY_;
		for (int i = 0; i < num; i++)
		{
			DeleteGraph(handleIds_[i]);
		}
		delete[] handleIds_;
	}
		break;

	case Resource::TYPE::MODEL:
	{
		MV1DeleteModel(handleId_);
		for (auto id : duplicateModelIds_) {
			if (id != -1) {
				MV1DeleteModel(id); // 削除
			}
		}
		duplicateModelIds_.clear();
	}
		break;

	case Resource::TYPE::EFFEKSEER:

		DeleteEffekseerEffect(handleId_);
		break;

	}

}

void Resource::CopyHandle(int* imgs)
{

	if (handleIds_ == nullptr)
	{
		return;
	}

	int num = numX_ * numY_;
	for (int i = 0; i < num; i++)
	{
		imgs[i] = handleIds_[i];
	}

}
