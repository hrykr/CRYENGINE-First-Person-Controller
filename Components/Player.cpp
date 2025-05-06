// Copyright 2017-2020 Crytek GmbH / Crytek Group. All rights reserved.
#include "StdAfx.h"
#include "Player.h"
#include "GamePlugin.h"

#include <CrySchematyc/Env/Elements/EnvComponent.h>
#include <CryCore/StaticInstanceList.h>
#include <CrySchematyc/Env/IEnvRegistrar.h>

#include <DefaultComponents/Cameras/CameraComponent.h>
#include <DefaultComponents/Input/InputComponent.h>
#include <DefaultComponents/Physics/CharacterControllerComponent.h>
#include <DefaultComponents/Geometry/AdvancedAnimationComponent.h>

namespace
{
	static void RegisterPlayerComponent(Schematyc::IEnvRegistrar& registrar)
	{
		Schematyc::CEnvRegistrationScope scope = registrar.Scope(IEntity::GetEntityScopeGUID());
		{
			Schematyc::CEnvRegistrationScope componentScope = scope.Register(SCHEMATYC_MAKE_ENV_COMPONENT(CPlayerComponent));
		}
	}

	CRY_STATIC_AUTO_REGISTER_FUNCTION(&RegisterPlayerComponent);
}

CPlayerComponent::CPlayerComponent() :
	m_pCameraComponent(nullptr),
	m_pInputComponent(nullptr),
	m_pCharacterComponent(nullptr),
	m_currentYaw(IDENTITY),
	m_currentPitch(0.f),
	m_movementDelta(ZERO),
	m_mouseDeltaRotation(ZERO),
	m_currentPlayerState(DEFAULT_STATE),
	m_currentStance(DEFAULT_STANCE),
	m_desiredStance(DEFAULT_STANCE),
	m_walkSpeed(DEFAULT_SPEED_WALKING),
	m_sprintSpeed(DEFAULT_SPEED_SPRINTING),
	m_jumpHeight(DEFAULT_JUMP_HEIGHT),
	m_rotationSpeed(DEFAULT_ROTATION_SPEED),
	m_cameraOffsetStanding(Vec3(0.f, 0.f, DEFAULT_CAMERA_HEIGHT_STANDING)),
	m_cameraEndOffset(Vec3(0.f, 0.f, DEFAULT_CAMERA_HEIGHT_STANDING)),
	m_cameraOffsetCrouching(Vec3(0.f, 0.f, DEFAULT_CAMERA_HEIGHT_CROUCHING)),
	m_capsuleHeightStanding(DEFAULT_CAPSULE_HEIGHT_STANDING),
	m_capsuleHeightCrouching(DEFAULT_CAPSULE_HEIGHT_CROUCHING),
	m_capsuleGroundOffset(DEFAULT_CAPSULE_GROUND_OFFSET),
	m_rotationLimitsMaxPitch(DEFAULT_ROT_LIMIT_PITCH_MAX),
	m_rotationLimitsMinPitch(DEFAULT_ROT_LIMIT_PITCH_MIN)
{

}

void CPlayerComponent::Initialize()
{
	m_pCameraComponent = m_pEntity->GetOrCreateComponent<Cry::DefaultComponents::CCameraComponent>();
	m_pInputComponent = m_pEntity->GetOrCreateComponent<Cry::DefaultComponents::CInputComponent>();
	m_pCharacterComponent = m_pEntity->GetOrCreateComponent<Cry::DefaultComponents::CCharacterControllerComponent>();
	m_pAdvancedAnimationComponent = m_pEntity->GetOrCreateComponent<Cry::DefaultComponents::CAdvancedAnimationComponent>();

	Reset();
}

void CPlayerComponent::Reset() {
	// Reset Input
	m_movementDelta = ZERO;
	m_mouseDeltaRotation = ZERO;
	m_currentYaw = Quat::CreateRotationZ(m_pEntity->GetWorldRotation().GetRotZ());
	m_currentPitch = 0.f;

	InitializeInput();

	// Reset Player State
	m_currentPlayerState = EPlayerState::Walking;
	m_currentStance = EPlayerStance::Standing;
	m_desiredStance = m_currentStance;

	// Reset Camera Lerp
	m_cameraEndOffset = m_cameraOffsetStanding;
}

void CPlayerComponent::ReCenterCollider() {
	static bool skip = false;
	if (skip) {
		skip = false;
		return;
	}

	auto pCharacterController = m_pEntity->GetComponent<Cry::DefaultComponents::CCharacterControllerComponent>();
	if (pCharacterController == nullptr) {
		return;
	}

	const auto& physParams = pCharacterController->GetPhysicsParameters();
	float heightOffset = physParams.m_height * 0.5f;

	if (physParams.m_bCapsule) {
		heightOffset = heightOffset * 0.5f + physParams.m_radius * 0.5f;
	}

	pCharacterController->SetTransformMatrix(Matrix34(IDENTITY, Vec3(0.f, 0.f, 0.005f + heightOffset)));

	skip = true;

	pCharacterController->Physicalize();
}

void CPlayerComponent::InitializeInput()
{

	// Movement
	m_pInputComponent->RegisterAction("player", "moveforward", [this](int activationMode, float value) {m_movementDelta.y = value;  });
	m_pInputComponent->BindAction("player", "moveforward", eAID_KeyboardMouse, eKI_W);

	m_pInputComponent->RegisterAction("player", "moveback", [this](int activationMode, float value) {m_movementDelta.y = -value;  });
	m_pInputComponent->BindAction("player", "moveback", eAID_KeyboardMouse, eKI_S);

	m_pInputComponent->RegisterAction("player", "moveright", [this](int activationMode, float value) {m_movementDelta.x = value;  });
	m_pInputComponent->BindAction("player", "moveright", eAID_KeyboardMouse, eKI_D);

	m_pInputComponent->RegisterAction("player", "moveleft", [this](int activationMode, float value) {m_movementDelta.x = -value;  });
	m_pInputComponent->BindAction("player", "moveleft", eAID_KeyboardMouse, eKI_A);

	m_pInputComponent->RegisterAction("player", "movegamepad_x", [this](int activationMode, float value) { m_movementDelta.x = value; });
	m_pInputComponent->BindAction("player", "movegamepad_x", eAID_XboxPad, eKI_XI_ThumbLX);
	m_pInputComponent->BindAction("player", "movegamepad_x", eAID_PS4Pad, eKI_XI_ThumbLX);

	m_pInputComponent->RegisterAction("player", "movegamepad_y", [this](int activationMode, float value) { m_movementDelta.y = value; });
	m_pInputComponent->BindAction("player", "movegamepad_y", eAID_XboxPad, eKI_XI_ThumbLY);
	m_pInputComponent->BindAction("player", "movegamepad_y", eAID_PS4Pad, eKI_XI_ThumbLY);

	// Sprint
	m_pInputComponent->RegisterAction("player", "sprint", [this](int activationMode, float value) {
		if (activationMode == eAAM_OnPress) {
			m_currentPlayerState = EPlayerState::Sprinting;
		}
		else if (activationMode == eAAM_OnRelease) {
			m_currentPlayerState = EPlayerState::Walking;
		}
	});
	m_pInputComponent->BindAction("player", "sprint", eAID_KeyboardMouse, eKI_LShift, true, true, false);
	m_pInputComponent->BindAction("player", "sprint", eAID_XboxPad, eKI_XI_ThumbL, true, true, false);
	m_pInputComponent->BindAction("player", "sprint", eAID_PS4Pad, eKI_XI_ThumbL, true, true, false);

	// Crouch
	m_pInputComponent->RegisterAction("player", "crouch", [this](int activationMode, float value) {
		if (activationMode != eAAM_OnPress)
			return;

		switch (m_desiredStance)
		{
		case EPlayerStance::Standing: {
			m_desiredStance = EPlayerStance::Crouching;
		} break;
		case EPlayerStance::Crouching: {
			m_desiredStance = EPlayerStance::Standing;
		} break;
		}
	});
	m_pInputComponent->BindAction("player", "crouch", eAID_KeyboardMouse, eKI_C, true, false, false);
	m_pInputComponent->BindAction("player", "crouch", eAID_XboxPad, eKI_XI_B, true, false, false);
	m_pInputComponent->BindAction("player", "crouch", eAID_PS4Pad, eKI_XI_B, true, false, false);

	// Jump
	m_pInputComponent->RegisterAction("player", "jump", [this](int activationMode, float value) {
		if (m_pCharacterComponent->IsOnGround()) {
			m_pCharacterComponent->AddVelocity(Vec3(0.f, 0.f, m_jumpHeight));
		}
	});
	m_pInputComponent->BindAction("player", "jump", eAID_KeyboardMouse, eKI_Space, true, false, false);
	m_pInputComponent->BindAction("player", "jump", eAID_XboxPad, eKI_XI_A, true, false, false);
	m_pInputComponent->BindAction("player", "jump", eAID_PS4Pad, eKI_XI_A, true, false, false);

	// Mouse Rotation
	m_pInputComponent->RegisterAction("player", "yaw", [this](int activationMode, float value) {m_mouseDeltaRotation.y = -value;  });
	m_pInputComponent->BindAction("player", "yaw", eAID_KeyboardMouse, eKI_MouseY);

	m_pInputComponent->RegisterAction("player", "yaw_gamepad", [this](int activationMode, float value) {m_mouseDeltaRotation.y = value;  });
	m_pInputComponent->BindAction("player", "yaw_gamepad", eAID_XboxPad, eKI_XI_ThumbRY);
	m_pInputComponent->BindAction("player", "yaw_gamepad", eAID_PS4Pad, eKI_XI_ThumbRY);

	m_pInputComponent->RegisterAction("player", "pitch", [this](int activationMode, float value) {m_mouseDeltaRotation.x = -value;  });
	m_pInputComponent->BindAction("player", "pitch", eAID_KeyboardMouse, eKI_MouseX);
	m_pInputComponent->BindAction("player", "pitch", eAID_XboxPad, eKI_XI_ThumbRX);
	m_pInputComponent->BindAction("player", "pitch", eAID_PS4Pad, eKI_XI_ThumbRX);
}

Cry::Entity::EventFlags CPlayerComponent::GetEventMask() const
{
	return
		Cry::Entity::EEvent::GameplayStarted |
		Cry::Entity::EEvent::Update |
		Cry::Entity::EEvent::Reset | Cry::Entity::EEvent::EditorPropertyChanged |
		Cry::Entity::EEvent::PhysicalTypeChanged;
}

void CPlayerComponent::ProcessEvent(const SEntityEvent& eventParam)
{
	switch (eventParam.event)
	{
	case Cry::Entity::EEvent::GameplayStarted: {
		Reset();
	} break;
	case Cry::Entity::EEvent::Update: {

		const float frametime = eventParam.fParam[0];

		UpdateMovement();
		UpdateRotation();
		UpdateCamera(frametime);

		TryUpdateStance();

	} break;
	case Cry::Entity::EEvent::PhysicalTypeChanged: {
		ReCenterCollider();
	} break;
	case Cry::Entity::EEvent::EditorPropertyChanged:
	case Cry::Entity::EEvent::Reset: {
		Reset();
	} break;
	}
}

void CPlayerComponent::TryUpdateStance() {

	if (m_desiredStance == m_currentStance) {
		return;
	}

	IPhysicalEntity* pPhysEnt = m_pEntity->GetPhysicalEntity();

	if (pPhysEnt == nullptr) {
		return;
	}

	const float radius = m_pCharacterComponent->GetPhysicsParameters().m_radius * 0.5f;
	float height = 0.f;
	Vec3 camOffset = ZERO;

	switch (m_desiredStance)
	{
	case EPlayerStance::Standing: {

		height = m_capsuleHeightStanding;
		camOffset = m_cameraOffsetStanding;

		primitives::capsule capsule;
		capsule.axis.Set(0, 0, 1);

		capsule.center = m_pEntity->GetWorldPos() + Vec3(0, 0, m_capsuleGroundOffset + radius + height * 0.5f);
		capsule.r = radius;
		capsule.hh = height * 0.5f;

		if (IsCapsuleIntersectingGeometry(capsule)) {
			return;
		}

	} break;
	case EPlayerStance::Crouching: {
		height = m_capsuleHeightCrouching;
		camOffset = m_cameraOffsetCrouching;
	} break;
	}

	pe_player_dimensions playerDimensions;
	pPhysEnt->GetParams(&playerDimensions);

	playerDimensions.heightCollider = m_capsuleGroundOffset + radius + height * 0.5f;
	playerDimensions.sizeCollider = Vec3(radius, radius, height * 0.5f);
	m_cameraEndOffset = camOffset;
	m_currentStance = m_desiredStance;
	pPhysEnt->SetParams(&playerDimensions);

}

void CPlayerComponent::UpdateMovement() {
	Vec3 velocity = Vec3(m_movementDelta.x, m_movementDelta.y, 0.0f);
	velocity.Normalize();
	float playerMoveSpeed = m_currentPlayerState == EPlayerState::Walking ? m_walkSpeed : m_sprintSpeed;
	m_pCharacterComponent->SetVelocity(m_pEntity->GetWorldRotation() * velocity * playerMoveSpeed);
}

void CPlayerComponent::UpdateRotation() {
	m_currentYaw *= Quat::CreateRotationZ(m_mouseDeltaRotation.x * m_rotationSpeed);
	m_pEntity->SetRotation(m_currentYaw);
}

void CPlayerComponent::UpdateCamera(float frametime) {
	m_currentPitch = crymath::clamp(m_currentPitch + m_mouseDeltaRotation.y * m_rotationSpeed, m_rotationLimitsMinPitch, m_rotationLimitsMaxPitch);

	Vec3 currentCameraOffset = m_pCameraComponent->GetTransformMatrix().GetTranslation();
	currentCameraOffset = Vec3::CreateLerp(currentCameraOffset, m_cameraEndOffset, 10.0f * frametime);

	Matrix34 finalCamMatrix;
	finalCamMatrix.SetTranslation(currentCameraOffset);
	finalCamMatrix.SetRotation33(Matrix33::CreateRotationX(m_currentPitch));
	m_pCameraComponent->SetTransformMatrix(finalCamMatrix);
}

bool CPlayerComponent::IsCapsuleIntersectingGeometry(const primitives::capsule& capsule) const {
	IPhysicalEntity* pPhysEnt = m_pEntity->GetPhysicalEntity();
	if (pPhysEnt == nullptr)
		return false;

	IPhysicalWorld::SPWIParams pwiParams;
	pwiParams.itype = capsule.type;
	pwiParams.pprim = &capsule;

	pwiParams.pSkipEnts = &pPhysEnt;
	pwiParams.nSkipEnts = 1;

	intersection_params intersectionParams;
	intersectionParams.bSweepTest = false;
	pwiParams.pip = &intersectionParams;

	const int contactCount = static_cast<int>(gEnv->pPhysicalWorld->PrimitiveWorldIntersection(pwiParams));
	return contactCount > 0;
}