// Copyright 2017-2020 Crytek GmbH / Crytek Group. All rights reserved.
#pragma once

namespace primitives {
	struct capsule;
}

namespace Cry::DefaultComponents {
	class CCameraComponent;
	class CInputComponent;
	class CCharacterControllerComponent;
	class CAdvancedAnimationComponent;
}

class CPlayerComponent final : public IEntityComponent
{
public:

	enum class EPlayerState {
		Walking, Sprinting
	};

	enum class EPlayerStance {
		Standing, Crouching
	};

private:

	static constexpr float DEFAULT_SPEED_WALKING = 3;
	static constexpr float DEFAULT_SPEED_SPRINTING = 5;
	static constexpr float DEFAULT_JUMP_HEIGHT = 3;
	static constexpr float DEFAULT_ROTATION_SPEED = 0.002f;
	static constexpr float DEFAULT_CAMERA_HEIGHT_STANDING = 1.8f;
	static constexpr float DEFAULT_CAMERA_HEIGHT_CROUCHING = 1.2f;
	static constexpr float DEFAULT_CAPSULE_HEIGHT_STANDING = 1;
	static constexpr float DEFAULT_CAPSULE_HEIGHT_CROUCHING = 0.5f;
	static constexpr float DEFAULT_CAPSULE_GROUND_OFFSET = 0.2f;
	static constexpr float DEFAULT_ROT_LIMIT_PITCH_MAX = 1.5f;
	static constexpr float DEFAULT_ROT_LIMIT_PITCH_MIN = -0.85f;
	static constexpr EPlayerState DEFAULT_STATE = EPlayerState::Walking;
	static constexpr EPlayerStance DEFAULT_STANCE = EPlayerStance::Standing;

public:
	CPlayerComponent();
	virtual ~CPlayerComponent() override {};

	virtual void Initialize() override;

	virtual Cry::Entity::EventFlags GetEventMask() const override;
	virtual void ProcessEvent(const SEntityEvent& event) override;

	// Reflect type to set a unique identifier for this component
	static void ReflectType(Schematyc::CTypeDesc<CPlayerComponent>& desc)
	{
		desc.SetGUID("{63F4C0C6-32AF-4ACB-8FB0-57D45DD14725}"_cry_guid);
		desc.AddMember(&CPlayerComponent::m_walkSpeed, 'pws', "playerwalkspeed", "Player Walk Speed", "Player walking speed", DEFAULT_SPEED_WALKING);
		desc.AddMember(&CPlayerComponent::m_sprintSpeed, 'prs', "playersprintspeed", "Player Sprint Speed", "Player sprinting speed", DEFAULT_SPEED_SPRINTING);
		desc.AddMember(&CPlayerComponent::m_jumpHeight, 'pjh', "playerjumpheight", "Player Jump Height", "Player jump height", DEFAULT_JUMP_HEIGHT);
		desc.AddMember(&CPlayerComponent::m_rotationSpeed, 'pros', "playerrotationspeed", "Mouse Rotation Speed", "Mouse rotation speed", DEFAULT_ROTATION_SPEED);
		desc.AddMember(&CPlayerComponent::m_cameraOffsetStanding, 'cams', "cameraoffsetstanding", "Camera Offset Standing", "Camera offset while standing", Vec3(0.f, 0.f, DEFAULT_CAMERA_HEIGHT_STANDING));
		desc.AddMember(&CPlayerComponent::m_cameraOffsetCrouching, 'camc', "cameraoffsetcrouching", "Camera Offset Crouching", "Camera offset while crouching", Vec3(0.f, 0.f, DEFAULT_CAMERA_HEIGHT_CROUCHING));
		desc.AddMember(&CPlayerComponent::m_capsuleHeightStanding, 'caps', "capsuleheightstanding", "Capsule Height Standing", "Capsule collider height while standing", DEFAULT_CAPSULE_HEIGHT_STANDING);
		desc.AddMember(&CPlayerComponent::m_capsuleHeightCrouching, 'capc', "capsuleheightcrouching", "Capsule Height Crouching", "Capsule collider height while crouching", DEFAULT_CAPSULE_HEIGHT_CROUCHING);
		desc.AddMember(&CPlayerComponent::m_capsuleGroundOffset, 'capo', "capsulegroundoffset", "Capsule Ground Offset", "Capsule collider ground offset", DEFAULT_CAPSULE_GROUND_OFFSET);
		desc.AddMember(&CPlayerComponent::m_rotationLimitsMaxPitch, 'cpm', "camerapitchmax", "Camera Max Pitch", "Maximum value of camera pitch", DEFAULT_ROT_LIMIT_PITCH_MAX);
		desc.AddMember(&CPlayerComponent::m_rotationLimitsMinPitch, 'cpmi', "camerapitchmin", "Camera Min Pitch", "Minimum value of camera pitch", DEFAULT_ROT_LIMIT_PITCH_MIN);
	}
protected:

	void InitializeInput();
	void Reset();
	void UpdateMovement();
	void UpdateRotation();
	void UpdateCamera(float frametime);
	void ReCenterCollider();
	void TryUpdateStance();
	bool IsCapsuleIntersectingGeometry(const primitives::capsule& capsule) const;

private:

	// Component References

	Cry::DefaultComponents::CCameraComponent* m_pCameraComponent;
	Cry::DefaultComponents::CInputComponent* m_pInputComponent;
	Cry::DefaultComponents::CCharacterControllerComponent* m_pCharacterComponent;
	Cry::DefaultComponents::CAdvancedAnimationComponent* m_pAdvancedAnimationComponent;



	// Runtime Variables

	Quat m_currentYaw;
	float m_currentPitch;
	Vec2 m_movementDelta;
	Vec2 m_mouseDeltaRotation;
	EPlayerState m_currentPlayerState;
	EPlayerStance m_currentStance;
	EPlayerStance m_desiredStance;
	Vec3 m_cameraEndOffset;



	// Component Properties

	float m_walkSpeed; // Player Walking Speed
	float m_sprintSpeed; // Player Sprinting Speed
	float m_jumpHeight; // Player Jump Height
	float m_rotationSpeed; // Mouse Rotation Speed
	Vec3 m_cameraOffsetStanding; // Camera Offset While Standing
	Vec3 m_cameraOffsetCrouching; // Camera Offset While Crouching
	float m_capsuleHeightStanding; // Capsule Collider Height While Standing
	float m_capsuleHeightCrouching; // Capsule Collider Height While Crouching
	float m_capsuleGroundOffset; // Capsule Collider Ground Offset
	float m_rotationLimitsMaxPitch; // Maximum Rotation Pitch Value
	float m_rotationLimitsMinPitch; // Minimum Rotation Pitch Value
};
