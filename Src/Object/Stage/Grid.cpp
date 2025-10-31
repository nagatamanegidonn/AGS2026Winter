#include <DxLib.h>
#include "../../Manager/ResourceManager.h"
#include "../../Manager/SceneManager.h"
#include "../../Utility/AsoUtility.h"

#include "../Common/Transform.h"
#include "Grid.h"

Grid::Grid(void)
{
}

Grid::~Grid(void)
{
}

void Grid::Init(void)
{
	//※モデルのテクスチャの参照先がおかしいの白く描画されます


}

void Grid::Update(void)
{
}

void Grid::Draw(void)
{
	DrawDebug();
}
void Grid::DrawDebug(void)
{

	// 【練習】最初の１本
	//VECTOR sPos = { 0.0f, 0.0f, 0.0f };
	//VECTOR ePos = { HLEN, 0.0f, 0.0f };
	//VECTOR sPos = { -HLEN, 0.0f, 0.0f };
	//VECTOR ePos = {  HLEN, 0.0f, 0.0f };
	//DrawLine3D(sPos, ePos, 0xff0000);

	// XZ基本軸(グリッド)
	float num;

	//今回回転させたい回転量をクォータニオンで作る
	Quaternion rotPow = Quaternion();

	float dirPow = 360.0f / (HNUM * 2);
	VECTOR dir = AsoUtility::DIR_R;

	VECTOR sPos = AsoUtility::VECTOR_ZERO;
	VECTOR ePos;
	VECTOR eAgoPos = AsoUtility::VECTOR_ZERO;

	DrawSphere3D(sPos, 20.0f, 10, 0xff0000, 0xff0000, true);


	
	//□
	for (int i = -HNUM; i < HNUM; i++)
	{

		num = static_cast<float>(i);

		// X軸(赤)
		sPos = { -HLEN, 0.0f, num * TERM };
		ePos = { HLEN, 0.0f, num * TERM };
		DrawLine3D(sPos, ePos, 0xff0000);
		DrawSphere3D(ePos, 20.0f, 10, 0xff0000, 0xff0000, true);

		// Z軸(青)
		sPos = { num * TERM, 0.0f, -HLEN };
		ePos = { num * TERM, 0.0f,  HLEN };
		DrawLine3D(sPos, ePos, 0x0000ff);
		DrawSphere3D(ePos, 20.0f, 10, 0x0000ff, 0x0000ff, true);

	}

	// Y軸(緑)
	sPos = { 0.0f, -HLEN, 0.0f };
	ePos = { 0.0f,  HLEN, 0.0f };
	DrawLine3D(sPos, ePos, 0x00ff00);
	DrawSphere3D(ePos, 20.0f, 10, 0x00ff00, 0x00ff00, true);

	return;


	//〇
	for (int i = -HNUM; i < HNUM; i++)
	{

		float rad = AsoUtility::Deg2RadF(dir.y);
		float cosA = cosf(rad);
		float sinA = sinf(rad);

		// Y軸回転行列を使って新しい座標を計算
		float newX = dir.x * cosA + dir.z * sinA;
		float newZ = -dir.x * sinA + dir.z * cosA;

		ePos = { newX, 0.0f, newZ };
		ePos = VScale(ePos, STAGE_RADIUS);

		DrawLine3D(sPos, ePos, 0xff0000);
		DrawSphere3D(ePos, 20.0f, 10, 0x0000ff, 0x0000ff, true);

		if (!AsoUtility::EqualsVZero(eAgoPos))
		{
			DrawLine3D(ePos, eAgoPos, 0x0000ff);
		}
		eAgoPos = ePos;

		dir.y += dirPow;
	}

	// Y軸(緑)
	sPos = { 0.0f, -HLEN, 0.0f };
	ePos = { 0.0f,  HLEN, 0.0f };
	DrawLine3D(sPos, ePos, 0x00ff00);
	DrawSphere3D(ePos, 20.0f, 10, 0x00ff00, 0x00ff00, true);


	return;

}

void Grid::Release(void)
{
}
