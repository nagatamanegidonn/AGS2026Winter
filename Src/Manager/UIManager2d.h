#pragma once
#include<string>
#include<vector>
#include<map>
#include<DxLib.h>

class UIManager2d
{
public:
	//UIの演出
	enum class UI_DIRECTION_2D {
		NOMAL,		//通常
		MOVE_UP,	//上移動
		MOVE_DOWN,	//下移動
		MOVE_LEFT,	//左移動
		MOVE_RIGHT,	//右移動
		UP_DOWN,	//上下移動(ループ)
		LEFT_RIGHT,	//左右移動(ループ)
		ZOOM_IN,	//拡大
		ZOOM_OUT,	//縮小
		ZOOM_INOUT,	//拡大縮小(ループ)
		ROT_RIGHT,	//右回転
		ROT_LEFT,	//左回転
		ROT_CRADLE,	//左右に回転(ループ)
		GRAD_DISAP,	//徐々に消える
		GRAD_AP,	//徐々に現れる
		FLASHING,	//点滅(ループ)
	};

	/// <summary>
	/// 項目分類(１項目につき一つまでを許可)
	/// </summary>
	enum class UI_DIRECTION_GROUP {
		NONE,
		MOVE,		//移動
		ZOOM,		//拡大縮小
		ROTATION,	//回転
		GRADUALLY,	//透明・不透明(Transpercentがもともと使われていたので「徐々に」という英単語にした)
	};

	enum class UI_DRAW_DIMENSION {
		DIMENSION_2,
		DIMENSION_3,
	};

	/// <summary>
	/// 描画に必要なもの
	/// </summary>
	struct UIInfo {
		UI_DRAW_DIMENSION dimension;
		VECTOR pos;	//位置
		float scl;	//大きさ
		float deg;	//角度
		float alpha;//透明度
	};

	//演出一つにつき必要なもの
	struct DirectionInfo {
		UI_DIRECTION_2D type;	//演出種類
		float pow;	//今までの加算量（主にRotationとMoveで使用、開始時その状態を０として扱うため上限下限に注意が必要）
		float acc;	//加算量
		float max;	//最大値
		float min;	//最小値
		bool isFinish;//終了したか
	};

	static constexpr float ALPHA_MAX = 255.0f;
	static constexpr float ALPHA_MIN = 0.0f;

	// インスタンスの生成
	static void CreateInstance(void);
	// インスタンスの取得
	static UIManager2d& GetInstance(void);

	/// <summary>
	/// 描画情報の登録
	/// </summary>
	/// <param name="_name">登録名</param>
	/// <param name="_imgHndl">描画する画像</param>
	/// <param name="_type">演出</param>
	void Add(const std::string& _name, const int _imgHndl, const UI_DIRECTION_2D _type,const UI_DRAW_DIMENSION _dimension);

	/// <summary>
	/// 要素の消去
	/// </summary>
	/// <param name="_name">登録名</param>
	void DeleteUI(const std::string& _name);				//単体
	void DeleteUI(const std::vector<std::string> _names);	//複数

	/// <summary>
	/// 演出追加
	/// </summary>
	/// <param name="_name">登録名</param>
	/// <param name="_type">追加する演出</param>
	void PushUIDirection(const std::string& _name, UI_DIRECTION_2D _type);
	/// <summary>
	/// 直近で追加した演出を削除
	/// </summary>
	/// <param name="_name"></param>
	void PopUIDirection(const std::string& _name);
	void PopUIDirection(const std::string& _name, UI_DIRECTION_GROUP _group);

	/// <summary>
	/// 描画基礎情報設定
	/// </summary>
	/// <param name="_name">登録名</param>
	/// <param name="_pos">描画位置(中心座標)</param>
	/// <param name="_scale">大きさ</param>
	/// <param name="_deg">角度</param>
	/// <param name="_alpha">透明度</param>
	void SetUIInfo(const std::string& _name, const VECTOR _pos, const float _scale = 1.0f, const float _deg = 0.0f, const float _alpha = ALPHA_MAX);

	/// <summary>
	/// UI演出用情報設定
	/// </summary>
	/// <param name="_name">登録名</param>
	/// <param name="_acc">加速度</param>
	/// <param name="_max">最大値(詳細は各項目の更新処理参照)</param>
	/// <param name="_min">最小値(詳細は各項目の更新処理参照)</param>
	void SetUIDirectionPram(const std::string& _name, const UI_DIRECTION_GROUP _group, const float _acc, const float _max, const float _min);

	//パラメータ再設定
	void SetPos(const std::string& _name,const VECTOR& _pos);	//位置
	void SetScale(const std::string& _name,const float& _scl);//拡大率
	void SetAlpha(const std::string& _name,const float& _alpha);	//透明度

	//画像の差し替え
	void SetImage(const std::string& _name, const int _imageHndl);

	//更新
	void Update(const std::string _name);				//単体
	void Update(const std::vector<std::string> _names);	//複数

	//描画
	void Draw(const std::string _name);					//単体
	void Draw(const std::vector<std::string> _names);	//複数

	//解放
	void Relese(void);

	//消去
	void Destroy(void);

	//更新の初期化
	void ResetUpdate(const std::string _name, const UI_DIRECTION_GROUP _group);

	/// <summary>
	/// 特定の更新が終了したか(ループしないものが前提とする)
	/// </summary>
	/// <param name="_name"></param>
	/// <param name="_group"></param>
	/// <returns></returns>
	const bool IsFinishDirection(const std::string _name,const UI_DIRECTION_GROUP _group);

	const bool IsLoopUpdate(const std::string _name, const UI_DIRECTION_GROUP _group);

	const VECTOR GetDrawPos(const std::string _name)const;
	const float GetDrawScale(const std::string _name)const;
	const float GetDrawAlpha(const std::string _name)const;

private:
	/// <summary>
	/// 演出の大まかな種類を取得
	/// </summary>
	/// <param name="_name">登録名</param>
	/// <returns>グループ</returns>
	const UI_DIRECTION_GROUP GetDirectionGroup(const std::string _name);
	//種類版
	const UI_DIRECTION_GROUP GetDirectionGroup(const UI_DIRECTION_2D _type);

	/// <summary>
	/// 移動処理(maxは現在地点から右・下への移動量、minは現在地点から左・上への移動量)
	/// </summary>
	/// <param name="_name">登録名</param>
	void Move(const std::string& _name, DirectionInfo& _direcInfo);

	void Zoom(const std::string& _name, DirectionInfo& _direcInfo);

	/// <summary>
	/// 回転処理(maxは右回転の限度、minは左回転の限度)
	/// </summary>
	/// <param name="_name">登録名</param>
	void Rotation(const std::string& _name, DirectionInfo& _direcInfo);

	/// <summary>
	/// アルファ値調整(maxは不透明度、minは透明度)
	/// </summary>
	/// <param name="_name">登録名</param>
	void AlphaAcc(const std::string& _name, DirectionInfo& _direcInfo);

	//インスタンス用
	static UIManager2d* instance_;

	std::map<std::string, int>images_;				//描画画像格納
	std::map<std::string, UIInfo>infoes_;			//UIの描画に関わる情報
	std::map<std::string, std::vector<DirectionInfo>>direcInfoes_;	//UIの演出に関わる情報


	using Update_UI = void(UIManager2d::*)(const std::string&, DirectionInfo& _direcInfo);
	std::map<std::string, std::vector<Update_UI>>updates_;	//UI更新処理
};

