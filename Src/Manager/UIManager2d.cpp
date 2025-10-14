#include"../../Utility/Utility.h"
#include "UIManager2d.h"

UIManager2d* UIManager2d::instance_ = nullptr;

void UIManager2d::CreateInstance(void)
{
	if (instance_ == nullptr) {
		instance_ = new UIManager2d();
	}
}

UIManager2d& UIManager2d::GetInstance(void)
{
	return *instance_;
}

void UIManager2d::Add(const std::string& _name, const int _imgHndl, const UI_DIRECTION_2D _type, const UI_DRAW_DIMENSION _dimension)
{
	//要素があるとき
	if (direcInfoes_.contains(_name)) {
		//今ある分回す
		for (auto& direc : direcInfoes_[_name]) {
			if (GetDirectionGroup(direc.type) == GetDirectionGroup(_type)) {
				return;
			}
		}
	}

	//画像の格納
	images_.emplace(_name, _imgHndl);

	//基礎情報初期化
	UIInfo info = {};
	info.dimension = _dimension;
	info.pos = Utility::VECTOR_INIT;
	info.scl = 1.0f;
	info.deg = 0.0f;
	info.alpha = ALPHA_MAX;

	//基礎情報追加
	infoes_.emplace(_name, info);

	//通常描画なら演出追加の必要はない
	if (_type == UI_DIRECTION_2D::NOMAL)return;

	//演出処理の追加
	PushUIDirection(_name, _type);
	
}

void UIManager2d::DeleteUI(const std::string& _name)
{
	images_.erase(_name);
	infoes_.erase(_name);
	if (direcInfoes_.contains(_name)) {
		direcInfoes_.erase(_name);
	}
}

void UIManager2d::DeleteUI(const std::vector<std::string> _names)
{
	for (auto& name : _names) {
		images_.erase(name);
		infoes_.erase(name);
		if (direcInfoes_.contains(name)) {
			direcInfoes_.erase(name);
		}
	}
}

void UIManager2d::PushUIDirection(const std::string& _name, UI_DIRECTION_2D _type)
{
	//演出情報初期化
	DirectionInfo direcInfo = {};
	direcInfo.type = _type;
	direcInfo.pow = 0.0f;
	direcInfo.acc = 0.0f;
	direcInfo.max = 0.0f;
	direcInfo.min = 0.0f;

	//演出情報の追加
	direcInfoes_[_name].push_back(direcInfo);

	//更新の追加
	UI_DIRECTION_GROUP group = GetDirectionGroup(_type);
	if (group == UI_DIRECTION_GROUP::MOVE) {
		updates_[_name].push_back(&UIManager2d::Move);
	}
	if (group == UI_DIRECTION_GROUP::ZOOM) {
		updates_[_name].push_back(&UIManager2d::Zoom);
	}
	if (group == UI_DIRECTION_GROUP::ROTATION) {
		updates_[_name].push_back(&UIManager2d::Rotation);
	}
	if (group == UI_DIRECTION_GROUP::GRADUALLY) {
		updates_[_name].push_back(&UIManager2d::AlphaAcc);
	}
}

void UIManager2d::PopUIDirection(const std::string& _name)
{
	updates_[_name].pop_back();
	direcInfoes_[_name].pop_back();
}

void UIManager2d::PopUIDirection(const std::string& _name, UI_DIRECTION_GROUP _group)
{
	bool haveNomal = false;
	int foundCnt_ = 0;
	for (auto& direcInfo : direcInfoes_[_name]) {
		if (GetDirectionGroup(direcInfo.type) == _group)break;
		if (direcInfo.type == UI_DIRECTION_2D::NOMAL)haveNomal = true;
		foundCnt_++;
	}

	direcInfoes_[_name].erase(direcInfoes_[_name].begin() + foundCnt_);
	if (haveNomal)foundCnt_--;
	updates_[_name].erase(updates_[_name].begin() + foundCnt_);
}

void UIManager2d::SetUIInfo(const std::string& _name, const VECTOR _pos, const float _scale, const float _deg, const float _alpha)
{
	infoes_[_name].pos = _pos;
	infoes_[_name].scl = _scale;
	infoes_[_name].deg = _deg;
	infoes_[_name].alpha = _alpha;
}

void UIManager2d::SetUIDirectionPram(const std::string& _name, const UI_DIRECTION_GROUP _group, const float _acc, const float _max, const float _min)
{
	for (auto& info:direcInfoes_[_name]) {
		
		auto group = GetDirectionGroup(info.type);
		//設定するパラメータの意味が違ったら
		if (group != _group)continue;
		info.acc = _acc;
		info.max = _max;
		info.min = _min;
		info.isFinish = false;

		//加速が負の方向の物たちは先に設定しておく
		auto direcType = info.type;
		if (direcType == UI_DIRECTION_2D::MOVE_UP ||
			direcType == UI_DIRECTION_2D::MOVE_LEFT ||
			direcType == UI_DIRECTION_2D::ROT_RIGHT ||
			direcType == UI_DIRECTION_2D::ZOOM_OUT ||
			direcType == UI_DIRECTION_2D::GRAD_DISAP) {
			info.acc *= -1.0f;
		}
	}

	
}

void UIManager2d::SetPos(const std::string& _name, const VECTOR& _pos)
{
	//動きのエフェクトがあるかを検出
	bool isMoveDirec = false;
	UI_DIRECTION_2D moveType = UI_DIRECTION_2D::NOMAL;
	for (auto& info : direcInfoes_[_name]) {
		if (GetDirectionGroup(info.type) == UI_DIRECTION_GROUP::MOVE) {
			isMoveDirec = true;
			moveType = info.type;
			break;
		}
	}
	
	//動きがある場合
	if (isMoveDirec) {
		//動きの方向には同期しない
		//上下移動ならば
		if (moveType == UI_DIRECTION_2D::MOVE_DOWN ||
			moveType == UI_DIRECTION_2D::MOVE_UP ||
			moveType == UI_DIRECTION_2D::UP_DOWN) {
			//Y以外を同期
			infoes_[_name].pos.x = _pos.x;
			infoes_[_name].pos.z = _pos.z;
		}
		//左右移動ならば
		if (moveType == UI_DIRECTION_2D::MOVE_LEFT ||
			moveType == UI_DIRECTION_2D::MOVE_RIGHT ||
			moveType == UI_DIRECTION_2D::LEFT_RIGHT) {
			//X軸は同期しない
			infoes_[_name].pos.y = _pos.y;
			infoes_[_name].pos.z = _pos.z;
		}
	}
	else {
		//すべて同期
		infoes_[_name].pos = _pos;
	}
	
}

void UIManager2d::SetScale(const std::string& _name, const float& _scl)
{
	infoes_[_name].scl = _scl;
}

void UIManager2d::SetAlpha(const std::string& _name, const float& _alpha)
{
	infoes_[_name].alpha = _alpha;
}

void UIManager2d::SetImage(const std::string& _name, const int _imageHndl)
{
	//画像の差し替え
	images_[_name] = _imageHndl;
}

void UIManager2d::Update(const std::string _name)
{
	int cnt = 0;
	for (auto& update : updates_[_name]) {
		(this->*update)(_name, direcInfoes_[_name][cnt]);
		cnt++;
	}
}

void UIManager2d::Update(const std::vector<std::string> _names)
{
	for (auto& name : _names) {
		int cnt = 0;
		for (auto& update : updates_[name]) {
			(this->*update)(name,direcInfoes_[name][cnt]);
			cnt++;
		}
	}
}

void UIManager2d::Draw(const std::string _name)
{
	auto info = infoes_[_name];
	//うっすら黒くする
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, static_cast<int>(info.alpha));
	if (info.dimension == UI_DRAW_DIMENSION::DIMENSION_2) {
		DrawRotaGraph(static_cast<int>(info.pos.x), static_cast<int>(info.pos.y), info.scl, info.deg / 180.0f, images_[_name], true);
	}
	else {
		DrawBillboard3D(info.pos, 0.5f, 0.5f, info.scl, info.deg*DX_PI_F / 180.0f,  images_[_name],  false);
	}
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

}

void UIManager2d::Draw(const std::vector<std::string> _names)
{
	for (auto& name : _names) {
		Draw(name);	//単体描画に渡す。
	}
}

void UIManager2d::Relese(void)
{
	images_.clear();
	infoes_.clear();
	direcInfoes_.clear();
	updates_.clear();
}

void UIManager2d::Destroy(void)
{
	Relese();
	delete instance_;
}

void UIManager2d::ResetUpdate(const std::string _name, const UI_DIRECTION_GROUP _group)
{
	//更新の数分回す
	for (auto& direction : direcInfoes_[_name]) {
		//該当する更新を見つけたら
		if (GetDirectionGroup(direction.type) == _group) {
			//その更新が終了したかのフラグを返す。
			direction.pow = 0.0f;
		}
	}
}

const bool UIManager2d::IsFinishDirection(const std::string _name, const UI_DIRECTION_GROUP _group)
{
	//ループじゃないことが前提
	if (!IsLoopUpdate(_name, _group)) 
	{
		//更新の数分回す
		for (auto& direction : direcInfoes_[_name]) {
			//該当する更新を見つけたら
			if (GetDirectionGroup(direction.type) == _group) {
				//その更新が終了したかのフラグを返す。
				return direction.isFinish;
			}
		}
	}
	//ループ・そもそも該当するものがなかった時用
	return false;
}

const bool UIManager2d::IsLoopUpdate(const std::string _name, const UI_DIRECTION_GROUP _group)
{
	//情報分回す
	for (auto& direction : direcInfoes_[_name]) {
		//該当の更新のとき判定を加える。
		if (GetDirectionGroup(direction.type) == _group) {
			//ループならtrueを返す。
			if (direction.type == UI_DIRECTION_2D::UP_DOWN ||
				direction.type == UI_DIRECTION_2D::LEFT_RIGHT ||
				direction.type == UI_DIRECTION_2D::ZOOM_INOUT ||
				direction.type == UI_DIRECTION_2D::ROT_CRADLE ||
				direction.type == UI_DIRECTION_2D::FLASHING)
			{
				return true;
			}
		}
	}
	return false;
}

const VECTOR UIManager2d::GetDrawPos(const std::string _name) const
{
	return infoes_.at(_name).pos;
}

const float UIManager2d::GetDrawScale(const std::string _name) const
{
	return infoes_.at(_name).scl;
}

const float UIManager2d::GetDrawAlpha(const std::string _name) const
{
	return infoes_.at(_name).alpha;
}


const UIManager2d::UI_DIRECTION_GROUP UIManager2d::GetDirectionGroup(const std::string _name)
{
	return GetDirectionGroup(direcInfoes_[_name][0].type);
}

const UIManager2d::UI_DIRECTION_GROUP UIManager2d::GetDirectionGroup(const UI_DIRECTION_2D _type)
{
	UI_DIRECTION_GROUP ret = UI_DIRECTION_GROUP::NONE;

	//移動
	if (_type == UI_DIRECTION_2D::MOVE_UP	||
		_type == UI_DIRECTION_2D::MOVE_DOWN ||
		_type == UI_DIRECTION_2D::MOVE_LEFT ||
		_type == UI_DIRECTION_2D::MOVE_RIGHT||
		_type == UI_DIRECTION_2D::UP_DOWN	||
		_type == UI_DIRECTION_2D::LEFT_RIGHT) {

		ret = UI_DIRECTION_GROUP::MOVE;
	}

	//拡大縮小
	if (_type == UI_DIRECTION_2D::ZOOM_IN || _type == UI_DIRECTION_2D::ZOOM_OUT || _type == UI_DIRECTION_2D::ZOOM_INOUT) {
		ret = UI_DIRECTION_GROUP::ZOOM;
	}

	//回転
	if (_type == UI_DIRECTION_2D::ROT_LEFT || _type == UI_DIRECTION_2D::ROT_RIGHT || _type == UI_DIRECTION_2D::ROT_CRADLE) {
		ret = UI_DIRECTION_GROUP::ROTATION;
	}

	//透明度
	if (_type == UI_DIRECTION_2D::GRAD_AP || _type == UI_DIRECTION_2D::GRAD_DISAP || _type == UI_DIRECTION_2D::FLASHING) {
		ret = UI_DIRECTION_GROUP::GRADUALLY;
	}

	return ret;
}

void UIManager2d::Move(const std::string& _name, DirectionInfo& _direcInfo)
{
	VECTOR afterPos = infoes_[_name].pos;
	auto direcType = _direcInfo.type;

	//上下
	if (direcType == UI_DIRECTION_2D::MOVE_UP || 
		direcType == UI_DIRECTION_2D::MOVE_DOWN || 
		direcType == UI_DIRECTION_2D::UP_DOWN) {

		//移動量加算
		afterPos.y += _direcInfo.acc;
		_direcInfo.pow += _direcInfo.acc;

		//移動上限・下限の設定
		if (_direcInfo.pow > _direcInfo.max ||
			_direcInfo.pow < _direcInfo.min) {
			
			//上下移動繰り返しのとき
			if (direcType == UI_DIRECTION_2D::UP_DOWN) {
				//加算方向を逆に
				_direcInfo.acc *= -1.0f;
			}
			else {
				//終了
				_direcInfo.isFinish = true;
				//動かさない
				return;
			}
			
		}
	}

	//左右
	if (direcType == UI_DIRECTION_2D::MOVE_LEFT ||
		direcType == UI_DIRECTION_2D::MOVE_RIGHT ||
		direcType == UI_DIRECTION_2D::LEFT_RIGHT) {

		//移動量加算
		afterPos.x += _direcInfo.acc;
		_direcInfo.pow += _direcInfo.acc;

		//移動上限・下限の設定
		if (_direcInfo.pow > _direcInfo.max ||
			_direcInfo.pow < _direcInfo.min) {

			//上下移動繰り返しのとき
			if (direcType == UI_DIRECTION_2D::LEFT_RIGHT) {
				//加算方向を逆に
				_direcInfo.acc *= -1.0f;
			}
			else {
				//終了
				_direcInfo.isFinish = true;
				//動かさない
				return;
			}

		}
	}

	//移動処理
	infoes_[_name].pos = afterPos;
}

void UIManager2d::Zoom(const std::string& _name, DirectionInfo& _direcInfo)
{
	float afterScl = infoes_[_name].scl;
	auto direcType = _direcInfo.type;

	afterScl += _direcInfo.acc;
	_direcInfo.pow += _direcInfo.acc;

	//制限
	//移動上限・下限の設定
	if (afterScl > _direcInfo.max ||
		afterScl < _direcInfo.min) {
		//繰り返し処理ならば加算方向を逆に
		if (_direcInfo.type == UI_DIRECTION_2D::ZOOM_INOUT) {
			_direcInfo.acc *= -1.0f;
		}
		else {
			//終了
			_direcInfo.isFinish = true;
			//これ以上変更を加えない
			if (_direcInfo.type == UI_DIRECTION_2D::ZOOM_IN) {
				infoes_[_name].scl = _direcInfo.max;
			}
			else if (_direcInfo.type == UI_DIRECTION_2D::ZOOM_OUT) {
				infoes_[_name].scl = _direcInfo.min;
			}
			return;
		}
	}


	//移動処理
	infoes_[_name].scl = afterScl;
}

void UIManager2d::Rotation(const std::string& _name, DirectionInfo& _direcInfo)
{
	float afterDeg = infoes_[_name].deg;
	auto direcType = _direcInfo.type;

	afterDeg += _direcInfo.acc;
	_direcInfo.pow += _direcInfo.acc;

	//制限
	//移動上限・下限の設定
	if (_direcInfo.pow > _direcInfo.max ||
		_direcInfo.pow < _direcInfo.min) {
		//繰り返し処理ならば加算方向を逆に
		if (_direcInfo.type == UI_DIRECTION_2D::ROT_CRADLE) {
			_direcInfo.acc *= -1.0f;
		}
	}


	//移動処理
	infoes_[_name].deg = afterDeg;
}

void UIManager2d::AlphaAcc(const std::string& _name, DirectionInfo& _direcInfo)
{
	float afterAlpha = infoes_[_name].alpha;
	auto direcType = _direcInfo.type;

	afterAlpha += _direcInfo.acc;
	_direcInfo.pow += _direcInfo.acc;

	//制限
	//移動上限・下限の設定
	if (afterAlpha > _direcInfo.max ||
		afterAlpha < _direcInfo.min) {
		//繰り返し処理ならば加算方向を逆に
		if (_direcInfo.type == UI_DIRECTION_2D::FLASHING) {
			_direcInfo.acc *= -1.0f;
		}
		else {
			//終了
			_direcInfo.isFinish = true;
			//これ以上変更を加えない
			return;
		}
	}


	//移動処理
	infoes_[_name].alpha = afterAlpha;
}

