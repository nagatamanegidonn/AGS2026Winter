#include "../../Utility/AsoUtility.h"

#include "../../Net/NetManager.h"

#include "../Common/AnimationController.h"
#include "../Common/EffectController.h"
#include "../Common/SoundController.h"

#include "../Common/Collider/Capsule.h"

#include "../Stage/Planet.h"

#include "Hit/HitPart.h"
#include "Hit/HitDamage.h"
#include "EnemyBase.h"

namespace
{
	constexpr int DRAW_UP_SCALE = 30;// モデルに埋まらないよう
	constexpr float HALF_SCALE = 0.5;
	// 判定用の移動量
	constexpr float COL_CHECK_UP_POW = 10.0f * 2.0f;
	constexpr float COL_CHECK_DOWN_POW = 10.0f;
}

EnemyBase::EnemyBase(void)
	: createNo_(0)
	, followTime_(0.0f)
	, follow_(nullptr)
	, gravHitPosDown_(AsoUtility::VECTOR_ZERO)
	, gravHitPosUp_(AsoUtility::VECTOR_ZERO)
	, hitDamePos_(AsoUtility::VECTOR_ZERO)
	, hpMax_(0)
	, hp_(0)
	, key_(0)
{
}

EnemyBase::~EnemyBase(void)
{
}

void EnemyBase::Init(void)
{
}

void EnemyBase::Update(void)
{
}

void EnemyBase::Draw(void)
{
}

void EnemyBase::SetFollow(const Transform* follow)
{
}

bool EnemyBase::IsTargetInFOV(const VECTOR& followPos, float fovDeg)
{
	VECTOR toTarget = VSub(followPos, transform_.pos);
	toTarget.y = 0.0f; // 水平方向だけで判定
	toTarget = VNorm(toTarget);

	VECTOR forward = transform_.GetForward();
	forward.y = 0.0f;
	forward = VNorm(forward);

	float dot = VDot(forward, toTarget);					// コサイン角
	float angleRad = acosf(std::clamp(dot, -1.0f, 1.0f));	// 安定化
	float angleDeg = AsoUtility::Rad2DegF(angleRad);

	// 視野角の半分以内なら true
	return angleDeg <= (fovDeg * HALF_SCALE);
}
void EnemyBase::AddHitPart(int& model, std::wstring boneName, float rad, float rate)
{
	auto part = std::make_unique<HitPart>(model, boneName, rad, rate);
	hitParts_.emplace_back(std::move(part));
}

// Colliderによる判定
#pragma region 判定系

void EnemyBase::CollisionStageCapsule(void)
{
	// カプセルを移動させる
	Transform trans = Transform(transform_);
	trans.pos = movedPos_;
	trans.Update();
	Capsule cap = Capsule(*capsule_, trans);
	// カプセルとの衝突判定(主にステージ)
	for (const auto& c : colliders_)
	{
		auto hits = MV1CollCheck_Capsule(
			c.lock()->modelId_, -1,
			cap.GetPosTop(), cap.GetPosDown(), cap.GetRadius());

		if (c.lock()->type_ == Collider::TYPE::STAGE)
		{
			// 衝突した複数のポリゴンと衝突回避するまで、
			// プレイヤーの位置を移動させる
			for (int i = 0; i < hits.HitNum; i++)
			{
				auto hit = hits.Dim[i];

				// 地面と異なり、衝突回避位置が不明なため、何度か移動させる
				// この時、移動させる方向は、移動前座標に向いた方向であったり、
				// 衝突したポリゴンの法線方向だったりする
				for (int tryCnt = 0; tryCnt < 10; tryCnt++)
				{
					// 再度、モデル全体と衝突検出するには、効率が悪過ぎるので、
					// 最初の衝突判定で検出した衝突ポリゴン1枚と衝突判定を取る
					int pHit = HitCheck_Capsule_Triangle(
						cap.GetPosTop(), cap.GetPosDown(), cap.GetRadius(),
						hit.Position[0], hit.Position[1], hit.Position[2]);

					if (pHit)
					{
						// 法線の方向にちょっとだけ移動させる
						movedPos_ = VAdd(movedPos_, VScale(hit.Normal, 1.0f));
						// カプセルも一緒に移動させる
						trans.pos = movedPos_;
						trans.Update();
						continue;
					}
					break;
				}
			}
		}
		else if (c.lock()->type_ == Collider::TYPE::WALL)
		{
			// 衝突した複数のポリゴンと衝突回避するまで、
			// プレイヤーの位置を移動させる
			for (int i = 0; i < hits.HitNum; i++)
			{
				auto hit = hits.Dim[i];

				// 地面と異なり、衝突回避位置が不明なため、何度か移動させる
				// この時、移動させる方向は、移動前座標に向いた方向であったり、
				// 衝突したポリゴンの法線方向だったりする
				for (int tryCnt = 0; tryCnt < 10; tryCnt++)
				{
					// 再度、モデル全体と衝突検出するには、効率が悪過ぎるので、
					// 最初の衝突判定で検出した衝突ポリゴン1枚と衝突判定を取る
					int pHit = HitCheck_Capsule_Triangle(
						cap.GetPosTop(), cap.GetPosDown(), cap.GetRadius(),
						hit.Position[0], hit.Position[1], hit.Position[2]);

					if (pHit)
					{
						//法線方向高さなし
						VECTOR nor = hit.Normal;
						nor.y = 0.0f;

						// 法線の方向にちょっとだけ移動させる
						movedPos_ = VAdd(movedPos_, nor);
						// カプセルも一緒に移動させる
						trans.pos = movedPos_;
						trans.Update();
						continue;
					}
					break;
				}
			}
		}
		// 検出した地面ポリゴン情報の後始末
		MV1CollResultPolyDimTerminate(hits);
	}
}

void EnemyBase::CollisionGravity(void)
{
	// ジャンプ量を加算
	movedPos_ = VAdd(movedPos_, jumpPow_);

	// 重力方向
	VECTOR dirGravity = AsoUtility::DIR_D;

	// 重力方向の反対
	VECTOR dirUpGravity = AsoUtility::DIR_U;

	// 重力の強さ
	float gravityPow = Planet::DEFAULT_GRAVITY_POW;

	gravHitPosUp_ = VAdd(movedPos_, VScale(dirUpGravity, gravityPow));
	gravHitPosUp_ = VAdd(gravHitPosUp_, VScale(dirUpGravity, COL_CHECK_UP_POW));
	gravHitPosDown_ = VAdd(movedPos_, VScale(dirGravity, COL_CHECK_DOWN_POW));
	for (const auto& c : colliders_)
	{
		// 地面との衝突
		auto hit = MV1CollCheck_Line(
			c.lock()->modelId_, -1, gravHitPosUp_, gravHitPosDown_);

		// 最初は上の行のように実装して、木の上に登ってしまうことを確認する
		if (hit.HitFlag > 0 && VDot(dirGravity, jumpPow_) > 0.9f)
		{
			// 衝突地点から、少し上に移動
			movedPos_ = VAdd(hit.HitPosition, VScale(dirUpGravity, 2.0f));

			// ジャンプリセット
			jumpPow_ = AsoUtility::VECTOR_ZERO;
		}
	}
}

int EnemyBase::DamageUpdate(const std::wstring bone)
{
	auto& nIns = NetManager::GetInstance();
	auto& users = nIns.GetNetUsers();

	// ダメージ処理
	int dame = 0;
	for (auto& user : users)
	{
		const int userDame = nIns.GetNetMonsDamage(user.first, createNo_);
		// 合計ダメージに加算
		dame += nIns.GetNetMonsDamage(user.first, createNo_);
		// ダメージを受けていたなら
		if (userDame > 0)
		{
			bool isEnd = false;
			for (auto& hitdamage : hitdamages_)
			{
				// 表示終了しているものがあるなら
				if (hitdamage->GetState() == HitDamage::STATE::END)
				{
					hitdamage->Init(userDame);
					isEnd = true;
					break;
				}
			}
			if (!isEnd)
			{
				auto part = std::make_unique<HitDamage>(transform_.modelId, bone, userDame);
				hitdamages_.emplace_back(std::move(part));
			}
		}
	}
	return dame;
}

#pragma endregion

void EnemyBase::TargetRotate(const VECTOR& traPos, float rate)
{
	// ターゲット方向ベクトルを計算
	VECTOR toTarget = VSub(traPos, transform_.pos);
	toTarget.y = 0.0f;
	toTarget = VNorm(toTarget);

	VECTOR forward = transform_.GetForward();
	forward.y = 0.0f;
	forward = VNorm(forward);

	// 内積から角度を求める
	float dot = VDot(forward, toTarget);
	dot = std::clamp(dot, -1.0f, 1.0f);
	float angleRad = acosf(dot);

	// 右回りか左回りかを判定するために外積を計算
	float crossY = forward.x * toTarget.z - forward.z * toTarget.x;
	// 外積のY成分が正なら右回り、負なら左回り
	if (crossY > 0.0f)
	{
		angleRad = -angleRad;
	}

	// ラジアンを度に変換
	float angleDeg = AsoUtility::Rad2DegF(angleRad);

	const float maxTurnDeg = 3.0f;	// 1フレームあたりの最大回転角度
	const float deadZoneDeg = 1.0f;	// デッドゾーン(この角度以下は回転しない)

	if (std::abs(angleDeg) > deadZoneDeg)
	{
		float clampedDeg = std::clamp(angleDeg, -maxTurnDeg, maxTurnDeg);
		clampedDeg *= rate;

		playerRotY_ = playerRotY_.Mult(
			Quaternion::AngleAxis(
				AsoUtility::Deg2RadF(clampedDeg), AsoUtility::AXIS_Y
			));
	}
}

void EnemyBase::DrawFOV(float fovDeg, float radius, int rayCount, unsigned int color)
{
	//原点
	VECTOR origin = transform_.pos;
	origin.y += DRAW_UP_SCALE;

	VECTOR forward = transform_.GetForward();
	forward.y = 0.0f;
	forward = VNorm(forward);

	float halfFov = fovDeg * HALF_SCALE;
	float angleStep = fovDeg / (rayCount - 1);

	for (int i = 0; i < rayCount; ++i)
	{
		float angle = -halfFov + angleStep * i;
		float rad = AsoUtility::Deg2RadF(angle);

		// Y軸回転の方向ベクトルを作成
		VECTOR dir = {
			forward.x * cosf(rad) - forward.z * sinf(rad),
			0.0f,
			forward.x * sinf(rad) + forward.z * cosf(rad)
		};

		VECTOR end = VAdd(origin, VScale(dir, radius));
		end.y += DRAW_UP_SCALE;

		DrawLine3D(origin, end, color);
	}
}
