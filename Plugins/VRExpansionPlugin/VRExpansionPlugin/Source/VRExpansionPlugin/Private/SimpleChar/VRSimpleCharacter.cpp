// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "SimpleChar/VRSimpleCharacter.h"
//#include "Runtime/Engine/Private/EnginePrivate.h"

AVRSimpleCharacter::AVRSimpleCharacter(const FObjectInitializer& ObjectInitializer)
 : Super(ObjectInitializer.SetDefaultSubobjectClass<UVRSimpleCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))

{

	// Remove the movement jitter with slow speeds
	FRepMovement& MovementRep = GetReplicatedMovement_Mutable();
	MovementRep.LocationQuantizationLevel = EVectorQuantization::RoundTwoDecimals;

	VRMovementReference = Cast<UVRBaseCharacterMovementComponent>(GetMovementComponent());
	if (VRMovementReference)
		VRMovementReference->bApplyAdditionalVRInputVectorAsNegative = false;

	VRSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("VR Scene Component"));

	if (VRSceneComponent)
	{
		VRSceneComponent->SetupAttachment(NetSmoother);
		VRSceneComponent->SetRelativeLocation(FVector(0, 0, -96));
	}

	//VRReplicatedCamera = CreateDefaultSubobject<UReplicatedVRCameraComponent>(TEXT("VR Replicated Camera"));
	if (VRReplicatedCamera && VRSceneComponent)
	{
		VRReplicatedCamera->bOffsetByHMD = true;
		VRReplicatedCamera->SetupAttachment(VRSceneComponent);
	}

	/*VRHeadCollider = CreateDefaultSubobject<UCapsuleComponent>(TEXT("VR Head Collider"));
	if (VRHeadCollider && VRReplicatedCamera)
	{
		VRHeadCollider->SetCapsuleSize(20.0f, 25.0f);
		VRHeadCollider->SetupAttachment(VRReplicatedCamera);
	}*/

//	ParentRelativeAttachment = CreateDefaultSubobject<UParentRelativeAttachmentComponent>(TEXT("Parent Relative Attachment"));
	if (ParentRelativeAttachment && VRReplicatedCamera && VRSceneComponent)
	{
		ParentRelativeAttachment->bOffsetByHMD = true;
		ParentRelativeAttachment->SetupAttachment(VRSceneComponent);
	}

	if (LeftMotionController)
	{
		LeftMotionController->bOffsetByHMD = true;

		if (VRSceneComponent)
		{
			LeftMotionController->SetupAttachment(VRSceneComponent);
		}
	}

	//RightMotionController = CreateDefaultSubobject<UGripMotionControllerComponent>(TEXT("Right Grip Motion Controller"));
	if (RightMotionController)
	{
		RightMotionController->bOffsetByHMD = true;

		if (VRSceneComponent)
		{
			RightMotionController->SetupAttachment(VRSceneComponent);
		}
	}
}

void AVRSimpleCharacter::BeginPlay()
{
	Super::BeginPlay();

	// I am re-forcing these to true here
	// The editor loses these values sometimes when copying over from the std character
	// And I like copying since it saves me hours of work.

	if(LeftMotionController)
		LeftMotionController->bOffsetByHMD = true;

	if(RightMotionController)
		RightMotionController->bOffsetByHMD = true;

	if(ParentRelativeAttachment)
		ParentRelativeAttachment->bOffsetByHMD = true;

	if(VRReplicatedCamera)
		VRReplicatedCamera->bOffsetByHMD = true;

}

FVector AVRSimpleCharacter::GetTeleportLocation(FVector OriginalLocation)
{
	//FVector modifier = VRRootReference->GetVRLocation_Inline() - this->GetActorLocation();
	//modifier.Z = 0.0f; // Null out Z
	//return OriginalLocation - modifier;
	
	return OriginalLocation;
}

bool AVRSimpleCharacter::TeleportTo(const FVector& DestLocation, const FRotator& DestRotation, bool bIsATest, bool bNoCheck)
{
	bool bTeleportSucceeded = Super::TeleportTo(DestLocation + FVector(0,0,GetCapsuleComponent()->GetScaledCapsuleHalfHeight()), DestRotation, bIsATest, bNoCheck);

	if (bTeleportSucceeded)
	{
		NotifyOfTeleport();
	}

	return bTeleportSucceeded;
}

bool AVRSimpleCharacter::ServerMoveVR_Validate(float TimeStamp, FVector_NetQuantize10 InAccel, FVector_NetQuantize100 ClientLoc, FVRConditionalMoveRep ConditionalReps, FVector_NetQuantize100 LFDiff, uint8 MoveFlags, FVRConditionalMoveRep2 MoveReps, uint8 ClientMovementMode)
{
	return ((UVRSimpleCharacterMovementComponent*)GetCharacterMovement())->ServerMoveVR_Validate(TimeStamp, InAccel, ClientLoc, ConditionalReps, LFDiff, MoveFlags, MoveReps, ClientMovementMode);
}

bool AVRSimpleCharacter::ServerMoveVRDual_Validate(float TimeStamp0, FVector_NetQuantize10 InAccel0, uint8 PendingFlags, uint32 View0, FVRConditionalMoveRep OldConditionalReps, FVector_NetQuantize100 OldLFDiff, float TimeStamp, FVector_NetQuantize10 InAccel, FVector_NetQuantize100 ClientLoc, FVRConditionalMoveRep ConditionalReps, FVector_NetQuantize100 LFDiff, uint8 NewFlags, FVRConditionalMoveRep2 MoveReps, uint8 ClientMovementMode)
{
	return ((UVRSimpleCharacterMovementComponent*)GetCharacterMovement())->ServerMoveVRDual_Validate(TimeStamp0, InAccel0, PendingFlags, View0, OldConditionalReps, OldLFDiff, TimeStamp, InAccel, ClientLoc, ConditionalReps, LFDiff, NewFlags, MoveReps, ClientMovementMode);
}

bool AVRSimpleCharacter::ServerMoveVRDualHybridRootMotion_Validate(float TimeStamp0, FVector_NetQuantize10 InAccel0, uint8 PendingFlags, uint32 View0, FVRConditionalMoveRep OldConditionalReps, FVector_NetQuantize100 OldLFDiff, float TimeStamp, FVector_NetQuantize10 InAccel, FVector_NetQuantize100 ClientLoc, FVRConditionalMoveRep ConditionalReps, FVector_NetQuantize100 LFDiff, uint8 NewFlags, FVRConditionalMoveRep2 MoveReps, uint8 ClientMovementMode)
{
	return ((UVRSimpleCharacterMovementComponent*)GetCharacterMovement())->ServerMoveVRDualHybridRootMotion_Validate(TimeStamp0, InAccel0, PendingFlags, View0, OldConditionalReps, OldLFDiff, TimeStamp, InAccel, ClientLoc, ConditionalReps, LFDiff, NewFlags, MoveReps, ClientMovementMode);
}

void AVRSimpleCharacter::ServerMoveVRDual_Implementation(
	float TimeStamp0,
	FVector_NetQuantize10 InAccel0,
	uint8 PendingFlags,
	uint32 View0,
	FVRConditionalMoveRep OldConditionalReps,
	FVector_NetQuantize100 OldLFDiff,
	float TimeStamp,
	FVector_NetQuantize10 InAccel,
	FVector_NetQuantize100 ClientLoc,
	FVRConditionalMoveRep ConditionalReps,
	FVector_NetQuantize100 LFDiff,
	uint8 NewFlags,
	FVRConditionalMoveRep2 MoveReps,
	uint8 ClientMovementMode)
{
	((UVRSimpleCharacterMovementComponent*)GetCharacterMovement())->ServerMoveVRDual_Implementation(
		TimeStamp0,
		InAccel0,
		PendingFlags,
		View0,
		OldConditionalReps,
		OldLFDiff,
		TimeStamp,
		InAccel,
		ClientLoc,
		ConditionalReps,
		LFDiff,
		NewFlags,
		MoveReps,
		ClientMovementMode);
}

void AVRSimpleCharacter::ServerMoveVRDualHybridRootMotion_Implementation(
	float TimeStamp0,
	FVector_NetQuantize10 InAccel0,
	uint8 PendingFlags,
	uint32 View0,
	FVRConditionalMoveRep OldConditionalReps,
	FVector_NetQuantize100 OldLFDiff,
	float TimeStamp,
	FVector_NetQuantize10 InAccel,
	FVector_NetQuantize100 ClientLoc,
	FVRConditionalMoveRep ConditionalReps,
	FVector_NetQuantize100 LFDiff,
	uint8 NewFlags,
	FVRConditionalMoveRep2 MoveReps,
	uint8 ClientMovementMode)
{
	((UVRSimpleCharacterMovementComponent*)GetCharacterMovement())->ServerMoveVRDualHybridRootMotion_Implementation(
		TimeStamp0,
		InAccel0,
		PendingFlags,
		View0,
		OldConditionalReps,
		OldLFDiff,
		TimeStamp,
		InAccel,
		ClientLoc,
		ConditionalReps,
		LFDiff,
		NewFlags,
		MoveReps,
		ClientMovementMode);
}

void AVRSimpleCharacter::ServerMoveVR_Implementation(
	float TimeStamp,
	FVector_NetQuantize10 InAccel,
	FVector_NetQuantize100 ClientLoc,
	FVRConditionalMoveRep ConditionalReps,
	FVector_NetQuantize100 LFDiff,
	uint8 MoveFlags,
	FVRConditionalMoveRep2 MoveReps,
	uint8 ClientMovementMode)
{
	((UVRSimpleCharacterMovementComponent*)GetCharacterMovement())->ServerMoveVR_Implementation(
		TimeStamp,
		InAccel,
		ClientLoc,
		ConditionalReps,
		LFDiff,
		MoveFlags,
		MoveReps,
		ClientMovementMode);
}