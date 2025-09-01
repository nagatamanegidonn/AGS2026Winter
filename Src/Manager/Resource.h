#pragma once
#include <string>
#include <vector>
#include <map>

class Resource
{

public:
	
	// リソースタイプ
	enum class TYPE
	{
		NONE,
		IMG,
		IMGS,
		MODEL,
		EFFEKSEER
	};

	// コンストラクタ
	Resource(void);
	// コンストラクタ
	Resource(TYPE type, const std::wstring& path);
	// コンストラクタ(IMGS用)
	Resource(TYPE type, const std::wstring& path, int numX, int numY, int sizeX, int sizeY);

	// デストラクタ
	~Resource(void);

	// 読み込み
	void Load(void);

	// 解放
	void Release(void);

	// 複数画像ハンドルを別配列にコピー
	void CopyHandle(int* imgs);

	std::map<std::wstring, int> g_modelCache;

	// リソースタイプ
	TYPE type_;

	// リソースの読み込み先
	std::wstring path_;

	// 画像とモデルのハンドルID
	int handleId_;

	// IMGS::LoadDivGraph用
	int* handleIds_;
	int numX_;
	int numY_;
	int sizeX_;
	int sizeY_;

	// モデル複製用
	std::vector<int> duplicateModelIds_;

};
