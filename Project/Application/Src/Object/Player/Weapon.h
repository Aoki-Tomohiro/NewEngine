#pragma once
#include "Engine/Framework/Object/IGameObject.h"
#include "Engine/Components/Collision/Collider.h"
#include "Engine/Components/Particle/ParticleManager.h"
#include "Engine/Components/Audio/Audio.h"

class Weapon : public IGameObject, public Collider
{
public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize() override;

	/// <summary>
	/// 更新
	/// </summary>
	void Update() override;

	/// <summary>
	/// 描画
	/// </summary>
	/// <param name="camera"></param>
	void Draw(const Camera& camera) override;

	/// <summary>
	/// スプライトの描画
	/// </summary>
	void DrawUI() override;

	/// <summary>
	/// 座標を設定
	/// </summary>
	/// <param name="translation"></param>
	void SetTranslation(const Vector3& translation) { worldTransform_.translation_ = translation; };

	/// <summary>
	/// 回転を設定
	/// </summary>
	/// <param name="rotation"></param>
	void SetRotation(const Vector3& rotation) { worldTransform_.rotation_ = rotation; };

	/// <summary>
	/// プレイヤーのワールドトランスフォームを設定
	/// </summary>
	/// <param name="playerWorldTransform"></param>
	void SetPlayerWorldTransform(const WorldTransform* playerWorldTransform) { playerWorldTransform_ = playerWorldTransform; };

	/// <summary>
	/// 攻撃フラグを取得
	/// </summary>
	bool GetIsAttack() const { return isAttack_; };

	/// <summary>
	/// 攻撃フラグを設定
	/// </summary>
	/// <param name="isAttack"></param>
	void SetIsAttack(bool isAttack) { isAttack_ = isAttack; };

	/// <summary>
	/// ヒットフラグを取得
	/// </summary>
	/// <returns></returns>
	bool GetIsHit() const { return isHit_; };

	/// <summary>
	/// ヒットフラグを設定
	/// </summary>
	/// <param name="isHit"></param>
	void SetIsHit(bool isHit) { isHit_ = isHit; };

	/// <summary>
	/// ワールド変換データを取得
	/// </summary>
	/// <returns></returns>
	const WorldTransform& GetWorldTransform() const override { return worldTransform_; };

	/// <summary>
	/// 衝突判定
	/// </summary>
	/// <param name="collider"></param>
	void OnCollision(Collider* collider) override;

	/// <summary>
	/// ワールドポジションを取得
	/// </summary>
	/// <returns></returns>
	const Vector3 GetWorldPosition() const override;

	/// <summary>
	/// ImGuiの更新
	/// </summary>
	void UpdateImGui();

private:
	//オーディオ
	Audio* audio_ = nullptr;

	//当たり判定用のワールドトランスフォーム
	WorldTransform worldTransformCollision_{};

	//攻撃フラグ
	bool isAttack_ = false;

	//当たり判定のフラグ
	bool onCollision_ = false;
	bool preOnCollision_ = false;

	//プレイヤーのワールドトランスフォーム
	const WorldTransform* playerWorldTransform_ = nullptr;

	//パーティクルシステム
	ParticleSystem* particleSystem_ = nullptr;
	ParticleSystem* shockWaveParticleSystem_ = nullptr;

	//ヒットフラグ
	bool isHit_ = false;

	//OBB
	OBB obbSize{
	.center{worldTransformCollision_.translation_},
	.orientations{{1.0f,0.0f,0.0f},{0.0f,1.0f,0.0f},{0.0f,0.0f,1.0f}},
	.size{1.0f,1.0f,1.0f}
	};

	//オーディオハンドル
	uint32_t slashAudioHandle_ = 0;
};

