#include <DxLib.h>
#include "../../Manager/SceneManager.h"
#include "AnimationController.h"


AnimationController::AnimationController(int modelId)
{
	modelId_ = modelId;

	playType_ = -1;
	isLoop_ = false;

	isStop_ = false;
	switchLoopReverse_ = 0.0f;
	endLoopSpeed_ = 0.0f;
	stepEndLoopStart_ = 0.0f;
	stepEndLoopEnd_ = 0.0f;

	hitStopTime_ = 0.0f;
	isHitStop_ = false;
}

AnimationController::~AnimationController(void)
{
	// アニメーションモデルはキャッシュされているので、解放してはいけない
	// for (const auto& anim : animations_) {
	//     MV1DeleteModel(anim.second.model); // ←これを削除 or コメントアウト
	// }
}

void AnimationController::Add(int type, const std::wstring& path, float speed, int animNo
	, float sTime, float eTime)
{

	Animation anim;

	//anim.model = MV1LoadModel(path.c_str());
	// すでに同じ path が読み込まれていれば、それを再利用
	if (animModelCache_.count(path)) {
		anim.model = animModelCache_[path];
	}
	else {
		anim.model = MV1LoadModel(path.c_str());
		animModelCache_[path] = anim.model;
	}
	anim.animIndex = type;
	anim.speed = speed;

	anim.attachNo = animNo;



	if (animations_.count(type) == 0)
	{
		// 入れ替え
 		animations_.emplace(type, anim);
	}
	else//基本使わない
	{
		// 追加
		//animations_[type].model = anim.model;
		animations_[type].animIndex = anim.animIndex;
		animations_[type].attachNo = anim.attachNo;
		animations_[type].totalTime = anim.totalTime;
	}

	//逆さいせいか判定
	if (sTime > eTime && eTime != -1.0f)
	{
		animations_[type].switchLoopReverse_ = -1.0f;
	}

	animations_[type].startStep = sTime;
	animations_[type].endStep = eTime;




}
//ブレンド設定
void AnimationController::SetIsBlend(int type, bool isBlend, float blendSpeed)
{
	animations_[type].isBlend = isBlend;
	animations_[type].blendSpeed = blendSpeed;

}


void AnimationController::Play(int type, bool isLoop,
	bool isStop, bool isForce)
{
	//if (blend_.isBlending) return; // ブレンド中なら変更不可
	if (blend_.isBlending) {
		if (blend_.data.back().animType == type)
		{
			return; // ブレンド中なら変更不可
		}
	}

	if (playType_ == type && !isForce) return;

	//ここで防止：同じアニメかつ非ループ中、かつ未終了なら無視
	/*if (playType_ == type && !isForce && !isLoop_ && !IsEnd()) {
		return;
	}*/

	if (playType_ != -1 && !isForce&& animations_[type].isBlend)
	{
		auto bData = BlendData();
		// ★この行の直前で現在の attachNo を保存！
		bData.fromAttachNo = playAnim_.attachNo;
		bData.animType = type;
		bData.blendRate = 1.0f;
		if (blend_.data.size() >= 1)
		{
			float mRate_ = 0.0f;
			for (auto& data : blend_.data)
			{
				mRate_ += data.blendRate;
			}
			bData.blendRate -= mRate_;
		}

		blend_.data.push_back(bData);

		//printfDx(L"データ数(attachNo=%d,ブレンド割合%.2f)\n", blend_.data.size(), blend_.data.back().blendRate);


		// ↓ここで playAnim_ が新しいものに上書きされる
		playAnim_ = animations_[type];

		//アタッチナンバーの取得
		int attachNo = GetAttrchNo(type);

		//アニメーションの総時間を獲得
		float animTotal = MV1GetAttachAnimTotalTime(modelId_, attachNo);
		if (animTotal <= 0.0f) {
			//printfDx(L"Warning: totalTime is invalid (attachNo=%d totalTime=%f)\n", attachNo, animTotal);
			// 適当な仮の時間を設定する（もしくは return でもOK）
			animTotal = 1.0f;
		}

		blend_.toAttachNo = attachNo;

		blend_.isBlending = true;

		blendSpeed = animations_[type].blendSpeed;

		playAnim_.attachNo = attachNo;
		playAnim_.step = animations_[type].startStep;

		if (animations_[type].switchLoopReverse_ >= 0.0f)
		{
			//endStepを調べ－１じゃないならそれを入れる
			playAnim_.totalTime = animations_[type].endStep > -1.0f ? animations_[type].endStep : animTotal;
		}
		else
		{
			playAnim_.totalTime = animations_[type].endStep;
		}

		// アニメーションループ
		isLoop_ = isLoop;

		// アニメーションしない
		isStop_ = isStop;

		//再生変数
		stepEndLoopStart_ = animations_[type].startStep;
		stepEndLoopEnd_ = playAnim_.totalTime;
		switchLoopReverse_ = animations_[type].switchLoopReverse_;
	}
	else
	{

		if (playType_ != -1)
		{
			// モデルから現在のアニメーションを外す
			if (playAnim_.attachNo != -1) {
				MV1DetachAnim(modelId_, playAnim_.attachNo);
				playAnim_.attachNo = -1;
			}
		}

		// アニメーション種別を変更
		playType_ = type;
		playAnim_ = animations_[type];

		// 初期化
		playAnim_.step = animations_[type].startStep;

		//// モデルにアニメーションを付ける
		playAnim_.attachNo = GetAttrchNo(type);


		// アニメーション総時間の取得
		//endStepを調べ－１じゃないならそれを入れる
		if (animations_[type].switchLoopReverse_ >= 0.0f)
		{
			playAnim_.totalTime = animations_[type].endStep > -1.0f ? animations_[type].endStep : MV1GetAttachAnimTotalTime(modelId_, playAnim_.attachNo);
		}
		else
		{
			playAnim_.totalTime = animations_[type].endStep;
		}

		// アニメーションループ
		isLoop_ = isLoop;

		// アニメーションしない
		isStop_ = isStop;

		//再生変数
		stepEndLoopStart_ = animations_[type].startStep;
		stepEndLoopEnd_ = playAnim_.totalTime;

		switchLoopReverse_ = animations_[type].switchLoopReverse_;
	}

}

void AnimationController::Update(void)
{
	//再生速度
	float deltaTime = SceneManager::GetInstance().GetDeltaTime();

	// ブレンド処理
	if (blend_.isBlending) {
		//ブレンド進行
		blend_.data.front().blendRate -= blend_.blendSpeed * deltaTime * blendSpeed;

		//ブレンド終了判定
		if (blend_.data.back().blendRate <= 0.0f) {
			blend_.data.back().blendRate = 0.0f;
			blend_.isBlending = false;

			// 前のアニメを外す
			MV1DetachAnim(modelId_, blend_.data.back().fromAttachNo);

			// ブレンド終了時に playAnim_ に確定させる
			playAnim_.attachNo = blend_.toAttachNo;

			// 再生アニメーションタイプを更新（Playの変更許可に必要）
			playType_ = playAnim_.animIndex;

			blend_.data.pop_front();  // 古いものから削除

		}
		else if(blend_.data.front().blendRate <= 0.0f) {
			blend_.data.front().blendRate = 0.0f;

			// 前のアニメを外す
			MV1DetachAnim(modelId_, blend_.data.front().fromAttachNo);

			blend_.data.pop_front();  // 古いものから削除
			// 再生アニメーションタイプを更新（Playの変更許可に必要）
			//playType_ = playAnim_.animIndex;
		}

		float mRate_ = 0.0f;
		for (auto& data : blend_.data)
		{
			MV1SetAttachAnimBlendRate(modelId_, data.fromAttachNo, data.blendRate);
		
			mRate_ += data.blendRate;
			
		}

		//最新アニメーション
		MV1SetAttachAnimBlendRate(modelId_, blend_.toAttachNo, 1.0f - mRate_);

		
	}

	//アニメーション再生
	if (!isStop_) {
		playAnim_.step += (deltaTime * playAnim_.speed * switchLoopReverse_);

		bool isEnd = false;
		//再生方向が正か負か
		if (switchLoopReverse_ > 0.0f)
		{
			// 通常再生の場合
			if (playAnim_.step > playAnim_.totalTime)
			{
				isEnd = true;
			}
		}
		else
		{
			// 逆再生の場合
			if (playAnim_.step < playAnim_.totalTime)
			{
				isEnd = true;
			}
		}

		//アニメーションが終わったら
		if (isEnd) {
			//ループするか
			if (isLoop_) {

				//再生方向の変更がだいぶ変わっているのでこのままでおｋ
				playAnim_.step = stepEndLoopStart_;
				playAnim_.totalTime = stepEndLoopEnd_;
				//printfDx("正方向のループ\n", stepEndLoopEnd_);

			}
			else {
				isStop_ = true; // 再生停止
				//printfDx("ループなし\n", stepEndLoopEnd_);

				//playAnim_.step = stepEndLoopEnd_;
				//printfDx("Warning:  playAnim_.step=%f\n", stepEndLoopEnd_);
			}
		}
	}

	//デバッグ
	if (playAnim_.attachNo == -1) {
		printfDx(L"Warning: playAnim_ にアニメがアタッチされていません\n");
		return;
	}

	//ヒットストップ処理
	if (isHitStop_)
	{
		hitStopTime_ -= 0.01f;
		if (hitStopTime_ <= 0.0f)
		{
			isHitStop_ = false;
			hitStopTime_ = 0.0f;
		}
		return;
	}

	//アニメションアタッチ
	MV1SetAttachAnimTime(modelId_, playAnim_.attachNo, playAnim_.step);

	// Root移動を打ち消す補正を入れる（ここが重要）
	int rootFrame = MV1SearchFrame(modelId_, L"mixamorig:Hips");
	localPos_ = MV1GetAttachAnimFrameLocalPosition(modelId_, playAnim_.attachNo, rootFrame);
	
	if (blend_.isBlending)
	{
		VECTOR mlocalPos = { 0.0f,0.0f,0.0f };
		float mRate_ = 0.0f;
		for (auto& data : blend_.data)
		{
			mlocalPos = VAdd(mlocalPos,
				VScale(MV1GetAttachAnimFrameLocalPosition(modelId_, data.fromAttachNo, rootFrame), data.blendRate));

			mRate_ += data.blendRate;
		}


		VECTOR blendLpos = MV1GetAttachAnimFrameLocalPosition(modelId_, blend_.toAttachNo, rootFrame);

		localPos_ = VAdd(mlocalPos, VScale(blendLpos, 1.0f - mRate_));

	}

	//MATRIX invRootTrans = MGetTranslate(VScale(rootPos, -1.0f));
	//MV1SetAttachAnimMatrix(modelId_, playAnim_.attachNo, invRootTrans);
}

void AnimationController::SetEndLoop(float startStep, float endStep, float speed)
{
	stepEndLoopStart_ = startStep;
	stepEndLoopEnd_ = endStep;
	endLoopSpeed_ = speed;
}

int AnimationController::GetPlayType(void) const
{
	return playType_;
}

bool AnimationController::IsEnd(void) const
{

	bool ret = false;

	if (isLoop_)
	{
		// ループ設定されているなら、
		// 無条件で終了しないを返す
		return ret;
	}

	if (switchLoopReverse_ > 0.0f)
	{
		if (playAnim_.step >= playAnim_.totalTime)
		{
			// 再生時間を過ぎたらtrue
			return true;
		}
	}
	else
	{
		// 逆再生の場合
		if (playAnim_.step <= playAnim_.totalTime)
		{
			// 再生時間を過ぎたらtrue
			return true;
		}
	}

	return ret;

}

void AnimationController::SetHitStop(float time)
{
	hitStopTime_ = time;
	isHitStop_ = true;
}

int AnimationController::GetAttrchNo(int animType)
{
	auto& playAnim = animations_[animType];

   	int animIdx = 0;
	if (MV1GetAnimNum(playAnim.model) > 2)
	{
		// アニメーションが複数保存されていたら、番号1を指定
		animIdx = animations_[animType].attachNo;
	}
	else if (MV1GetAnimNum(playAnim.model) > 1)
	{
		// アニメーションが複数保存されていたら、番号1を指定
		animIdx = 1;
	}

	int attachNo = MV1AttachAnim(modelId_, animIdx, playAnim.model);
	if (attachNo == -1) {
		printfDx(L"MV1AttachAnim failed (modelId=%d animIdx=%d)\n", modelId_, animIdx);
		return -1;
	}
	return attachNo;
}
