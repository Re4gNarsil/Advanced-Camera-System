// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "CameraProjectCharacter.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "CameraCharacter/CameraSpringArm.h"
#include "TimerManager.h"

//////////////////////////////////////////////////////////////////////////
// ACameraProjectCharacter

ACameraProjectCharacter::ACameraProjectCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;
	
	// Create a camera boom (pulls in towards the player if there is a collision)
	OurCameraSpringArm = CreateDefaultSubobject<UCameraSpringArm>(TEXT("CameraBoom"));
	OurCameraSpringArm->SetupAttachment(RootComponent);
	OurCameraSpringArm->TargetArmLength = 200.0f; // The camera follows at this distance behind the character	
	OurCameraSpringArm->SetRelativeLocation(CameraArmLocation);
	OurCameraSpringArm->ActualSocketOffset = CameraSocketOffset;
	OurCameraSpringArm->ExtraArmRotation = CameraExtraRotation;
	OurCameraSpringArm->bUsePawnControlRotation = true; // Rotate the arm based on the controller
	

	// Create a follow camera
	OurCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	OurCamera->SetupAttachment(OurCameraSpringArm, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	OurCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm
	
	bUseControllerRotationYaw = true;

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)

}

//////////////////////////////////////////////////////////////////////////
// Input

void ACameraProjectCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("ToggleCameraControl", IE_Pressed, this, &ACameraProjectCharacter::ToggleCameraControlOn);
	PlayerInputComponent->BindAction("ToggleCameraControl", IE_Released, this, &ACameraProjectCharacter::ToggleCameraControlOff);

	PlayerInputComponent->BindAction("ToggleCameraSide", IE_Released, this, &ACameraProjectCharacter::ToggleCameraSide);

	PlayerInputComponent->BindAxis("MoveForward", this, &ACameraProjectCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ACameraProjectCharacter::MoveRight);
	PlayerInputComponent->BindAxis("ZoomIn", this, &ACameraProjectCharacter::ZoomIn);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &ACameraProjectCharacter::TurnRight);
	PlayerInputComponent->BindAxis("TurnRate", this, &ACameraProjectCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &ACameraProjectCharacter::LookUp);
	PlayerInputComponent->BindAxis("LookUpRate", this, &ACameraProjectCharacter::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &ACameraProjectCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &ACameraProjectCharacter::TouchStopped);

	// VR headset functionality
	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &ACameraProjectCharacter::OnResetVR);
}

// Called when the game starts or when spawned
void ACameraProjectCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (!OurCameraSpringArm) {
		UE_LOG(LogTemp, Error, TEXT("Spring Arm Failure"));
		OurCameraSpringArm = FindComponentByClass<UCameraSpringArm>();
	}
	DesiredArmLocation = CameraArmLocation;
	DesiredSocketOffset = CameraSocketOffset;

	//GetWorldTimerManager().SetTimer(RandomChanges, this, &ACameraProjectCharacter::RandomlyChangeCamera, 3.f, true);
}

void ACameraProjectCharacter::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void ACameraProjectCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
		Jump();
}

void ACameraProjectCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
		StopJumping();
}

void ACameraProjectCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ACameraProjectCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void ACameraProjectCharacter::MoveForward(float Value)
{
	if (!bAllowPlayerInputs) { return; }
	if (!bControllingCamera) {
		if ((Controller != NULL) && (Value != 0.0f))
		{
			// find out which way is forward
			const FRotator Rotation = Controller->GetControlRotation();
			const FRotator YawRotation(0, Rotation.Yaw, 0);

			// get forward vector
			const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
			AddMovementInput(Direction, Value);
		}
	}
	else
	{
		OurCameraSpringArm->ActualSocketOffset += FVector(0, 0, Value);
	}
}

void ACameraProjectCharacter::MoveRight(float Value)
{
	if (!bAllowPlayerInputs) { return; }
	if (!bControllingCamera) {
		if ((Controller != NULL) && (Value != 0.0f))
		{
			// find out which way is right
			const FRotator Rotation = Controller->GetControlRotation();
			const FRotator YawRotation(0, Rotation.Yaw, 0);

			// get right vector 
			const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
			// add movement in that direction
			AddMovementInput(Direction, Value);
		}
	}
	else
	{
		OurCameraSpringArm->ActualSocketOffset += FVector(0, Value, 0);
	}
}


void ACameraProjectCharacter::TurnRight(float Rate)
{
	//If the camera is relocating we may want to disable all player inputs to keep it from being interrupted

	if (!bAllowPlayerInputs) { return; }
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ACameraProjectCharacter::LookUp(float Rate)
{
	if (!bAllowPlayerInputs) { return; }
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void ACameraProjectCharacter::ZoomIn(float Rate)
{
	if (!bAllowPlayerInputs) { return; }

	// Move camera spring arm socket forward
	if (bControllingCamera) { OurCameraSpringArm->ActualSocketOffset += FVector(Rate * 10, 0, 0); }
}

void ACameraProjectCharacter::ZoomOut(float Rate)
{
	if (!bAllowPlayerInputs) { return; }

	// Move camera spring arm socket back
	if (bControllingCamera) { OurCameraSpringArm->ActualSocketOffset += FVector(Rate * 10, 0, 0); }
}

void ACameraProjectCharacter::ToggleCameraControlOn()
{
	// Disable player movement while controlling the camera
	ToggleCharacterSettings(true, true, false);
}

void ACameraProjectCharacter::ToggleCameraControlOff()
{
	// Restore player options and move the camera back to where we want it
	//ToggleCharacterSettings(false, false, false);

	CalculateLongestTime();
	if (AutoAdjustTime > 0) { CalculateSpeedNeeded(-1); }
	GetWorldTimerManager().SetTimer(AdjustCameraTimer, this, &ACameraProjectCharacter::CorrectCameraTransform, GetWorld()->GetDeltaSeconds(), true);
}

void ACameraProjectCharacter::ToggleCharacterSettings(bool bAllowInputs, bool bControlCamera, bool bUseControllerRotation)
{
	bAllowPlayerInputs = bAllowInputs;
	bControllingCamera = bControlCamera;
	bUseControllerRotationYaw = bUseControllerRotation;
}

void ACameraProjectCharacter::ToggleCameraSide()
{
	// Change both the camera spring's arm's relative location, as well as where it's socket offset will be
	// Need to add some slight rotational input to get the socket offset to move properly

	bUsingRightSide = !bUsingRightSide;
	DesiredArmLocation.Y = -DesiredArmLocation.Y;
	if (bUsingRightSide) { 
		DesiredArmLocation.Y = CameraArmLocation.Y;
		DesiredSocketOffset.Y = CameraSocketOffset.Y;
		AddControllerYawInput(.1f * BaseTurnRate * GetWorld()->GetDeltaSeconds());
	}
	else { 
		DesiredArmLocation.Y = (-CameraArmLocation.Y * .75f);
		DesiredSocketOffset.Y = (-CameraSocketOffset.Y * .75f);
		AddControllerYawInput(-.1f * BaseTurnRate * GetWorld()->GetDeltaSeconds());
	}

	CalculateLongestTime();
	if (AutoAdjustTime > 0) { CalculateSpeedNeeded(-1); }
	GetWorldTimerManager().SetTimer(AdjustCameraTimer, this, &ACameraProjectCharacter::CorrectCameraTransform, GetWorld()->GetDeltaSeconds(), true);
}

void ACameraProjectCharacter::CorrectCameraTransform()
{
	if (!OurCameraSpringArm) { UE_LOG(LogTemp, Error, TEXT("Camera Spring Arm Vanished")); }
	else {
		ChangesNeeded = 0 + CorrectCameraRotation() + CorrectCameraLocation() + CorrectSocketLocation() + CorrectExtraRotation();

		if (ChangesNeeded == 0) {
			ToggleCharacterSettings(true, false, true);
			GetWorldTimerManager().ClearTimer(AdjustCameraTimer);
		}
	}
}

// Figure out what change to our camera's transform will take the longest
void ACameraProjectCharacter::CalculateLongestTime()
{
	// If we don't want to correct every aspect of a rotation, remove those differences from our calculations

	FRotator DifferenceInRotation = (Controller->GetDesiredRotation() - GetActorRotation()).GetNormalized();
	if (!bAutoCorrectCameraRotationPitch) { DifferenceInRotation.Pitch = 0; }
	if (!bAutoCorrectCameraRotationYaw) { DifferenceInRotation.Yaw = 0; }
	if (!bAutoCorrectCameraRotationRoll) { DifferenceInRotation.Pitch = 0; }

	// Divide the total distance by the speed at which each change will happen

	float RotationalDifferenceTotal = FMath::Abs(DifferenceInRotation.Pitch) + FMath::Abs(DifferenceInRotation.Yaw) + FMath::Abs(DifferenceInRotation.Roll);
	TimeToRotate = RotationalDifferenceTotal / (AutoTurnRate * BaseTurnRate * .75f);

	FVector LocationDifference = (DesiredArmLocation - OurCameraSpringArm->GetRelativeLocation()) * AutoCorrectCameraLocation;
	float LocationDistance = FVector::Dist(FVector::ZeroVector, LocationDifference);
	TimeToMove = LocationDistance / (AutoMoveRate * .75f);

	FVector OffsetDifference = (DesiredSocketOffset - OurCameraSpringArm->ActualSocketOffset) * AutoCorrectSocketOffset;
	float OffsetDistance = FVector::Dist(FVector::ZeroVector, OffsetDifference);
	TimeToShift = OffsetDistance / (AutoMoveRate * .75f);

	FRotator DifferenceInExtraRotation = (CameraExtraRotation - OurCameraSpringArm->ExtraArmRotation).GetNormalized();
	float ExtraRotationDifferenceTotal = FMath::Abs(DifferenceInExtraRotation.Pitch) + FMath::Abs(DifferenceInExtraRotation.Yaw) + FMath::Abs(DifferenceInExtraRotation.Roll);
	TimeToChange = ExtraRotationDifferenceTotal / (AutoTurnRate * BaseTurnRate * .75f);

	if (TimeToRotate > LongestTimeNeeded) { LongestTimeNeeded = TimeToRotate; }
	if (TimeToMove > LongestTimeNeeded) { LongestTimeNeeded = TimeToMove; }
	if (TimeToShift > LongestTimeNeeded) { LongestTimeNeeded = TimeToShift; }
	if (TimeToChange > LongestTimeNeeded) { LongestTimeNeeded = TimeToChange; }
}

// If we want to adjust our camera's transform in a set amount of time, adjust our movement speed to finish in the allotted time
void ACameraProjectCharacter::CalculateSpeedNeeded(float DesiredTime)
{
	float RequiredTime = (DesiredTime > 0) ? DesiredTime : AutoAdjustTime;
	SpeedNeeded = LongestTimeNeeded / RequiredTime;
}

int ACameraProjectCharacter::CorrectCameraRotation()
{
	// If we don't want to change part of the rotation, remove it from the equation

	FRotator DifferenceInRotation = (Controller->GetDesiredRotation() - GetActorRotation()).GetNormalized();
	if (!bAutoCorrectCameraRotationPitch) { DifferenceInRotation.Pitch = 0; }
	if (!bAutoCorrectCameraRotationYaw) { DifferenceInRotation.Yaw = 0; }
	if (!bAutoCorrectCameraRotationRoll) { DifferenceInRotation.Pitch = 0; }

	float RotationalDifferenceTotal = FMath::Abs(DifferenceInRotation.Pitch) + FMath::Abs(DifferenceInRotation.Yaw) + FMath::Abs(DifferenceInRotation.Roll);

	// If we're close enough, set the rotation to exactly what we want

	if (RotationalDifferenceTotal <= (AutoTurnRate * BaseTurnRate * GetWorld()->GetDeltaSeconds() * TimeToRotate * SpeedNeeded) / LongestTimeNeeded)
	{
		Controller->SetControlRotation(GetActorRotation().GetNormalized());
	}
	else {
		float PitchRatio = (DifferenceInRotation.Pitch / RotationalDifferenceTotal);
		float YawRatio   = (DifferenceInRotation.Yaw   / RotationalDifferenceTotal);
		float RollRatio  = (DifferenceInRotation.Roll  / RotationalDifferenceTotal);

		// We need to change the control rotation, rather than adding the value inputs, in order to get it to sync with our other changes correctly
		// By multiplying by TimeToRotate and dividing by LongestTimeNeeded we'll ensure movement and rotation finishes at the same time

		float MovementAmount = (AutoTurnRate * BaseTurnRate * GetWorld()->GetDeltaSeconds() * TimeToRotate * SpeedNeeded * .75f) / LongestTimeNeeded;
		FRotator AddedRotation = (FRotator(-PitchRatio, -YawRatio, RollRatio) * MovementAmount).GetNormalized();

		UE_LOG(LogTemp, Warning, TEXT("%f   %f   %f"), TimeToRotate, SpeedNeeded, LongestTimeNeeded);

		Controller->SetControlRotation(Controller->GetDesiredRotation() + AddedRotation);
		return 1;
	}
	return 0;
}

int ACameraProjectCharacter::CorrectCameraLocation()
{
	//Again, if we don't want to automatically fix a portion of our camera's offset, then remove it from the calculations

	FVector CurrentPosition = OurCameraSpringArm->GetRelativeLocation();
	FVector PositionDifference = DesiredArmLocation - CurrentPosition;
	FVector DesiredPosition = CurrentPosition + (PositionDifference * AutoCorrectCameraLocation);

	float Distance = FVector::Dist(DesiredPosition, CurrentPosition);

	// If we're close enough, set the location to exactly what we want
	// Need to make sure distance isn't really small since sometimes TimeToMove will get set to 0 for some odd reason

	if ((Distance <= (AutoMoveRate * GetWorld()->GetDeltaSeconds() * TimeToMove * SpeedNeeded) / LongestTimeNeeded) || (Distance < .1f))
	{
		OurCameraSpringArm->SetRelativeLocation(DesiredArmLocation);
	}
	else {
		FVector Direction = (DesiredPosition - CurrentPosition).GetSafeNormal();

		// By multiplying by TimeToMove and dividing by LongestTimeNeeded we'll ensure movement and rotation finishes at the same time
		
		float MovementAmount = ((AutoMoveRate * GetWorld()->GetDeltaSeconds() * TimeToMove * SpeedNeeded * .75f) / LongestTimeNeeded);
		FVector NewPosition = CurrentPosition + (Direction * MovementAmount);

		OurCameraSpringArm->SetRelativeLocation(NewPosition);
		return 1;
	}
	return 0;
}

int ACameraProjectCharacter::CorrectSocketLocation()
{
	FVector CurrentPosition = OurCameraSpringArm->ActualSocketOffset;
	FVector PositionDifference = DesiredSocketOffset - CurrentPosition;
	FVector DesiredPosition = CurrentPosition + (PositionDifference * AutoCorrectSocketOffset);

	float Distance = FVector::Dist(DesiredPosition, CurrentPosition);
	if ((Distance <= (AutoMoveRate * GetWorld()->GetDeltaSeconds() * TimeToShift * SpeedNeeded) / LongestTimeNeeded) || (Distance < .1f))
	{
		OurCameraSpringArm->ActualSocketOffset = DesiredSocketOffset;
	}
	else {
		FVector Direction = (DesiredPosition - CurrentPosition).GetSafeNormal();
		float MovementAmount = (AutoMoveRate * GetWorld()->GetDeltaSeconds() * TimeToShift * SpeedNeeded * .75f) / LongestTimeNeeded;

		FVector NewPosition = CurrentPosition + (Direction * MovementAmount);

		UE_LOG(LogTemp, Warning, TEXT("%f   %f   %f"), TimeToShift, SpeedNeeded, LongestTimeNeeded);

		OurCameraSpringArm->ActualSocketOffset = NewPosition;
		return 1;
	}
	return 0;
}

int ACameraProjectCharacter::CorrectExtraRotation()
{
	FRotator DifferenceInRotation = (CameraExtraRotation - OurCameraSpringArm->ExtraArmRotation).GetNormalized();
	float ExtraRotationDifferenceTotal = FMath::Abs(DifferenceInRotation.Pitch) + FMath::Abs(DifferenceInRotation.Yaw) + FMath::Abs(DifferenceInRotation.Roll);

	if (ExtraRotationDifferenceTotal <= (AutoTurnRate * BaseTurnRate * GetWorld()->GetDeltaSeconds() * TimeToChange * SpeedNeeded) / LongestTimeNeeded)
	{
		OurCameraSpringArm->ExtraArmRotation = CameraExtraRotation;
	}
	else {
		float PitchRatio = (DifferenceInRotation.Pitch / ExtraRotationDifferenceTotal);
		float YawRatio   = (DifferenceInRotation.Yaw   / ExtraRotationDifferenceTotal);
		float RollRation = (DifferenceInRotation.Roll  / ExtraRotationDifferenceTotal);
		float MovementAmount = (AutoTurnRate * BaseTurnRate * GetWorld()->GetDeltaSeconds() * TimeToChange * SpeedNeeded * .75f) / LongestTimeNeeded;

		FRotator Rotation = (FRotator(PitchRatio, YawRatio, RollRation) * MovementAmount).GetNormalized();
		OurCameraSpringArm->ExtraArmRotation += Rotation;
		return 1;
	}
	return 0;
}

void ACameraProjectCharacter::ChangeCameraSocketLocation(FVector NewLocation, bool bIsRelative, float DesiredMovementTime, bool bTakeControl)
{
	// If there is any reason oo change the camera's position (relative or otherwise) this makes it easy
	// The camera will move smoothly to it's new location and rotation after this is called

	DesiredSocketOffset = (bIsRelative) ? NewLocation : NewLocation - OurCameraSpringArm->GetComponentLocation();

	CalculateLongestTime();
	if (AutoAdjustTime > 0 || DesiredMovementTime > 0) { CalculateSpeedNeeded(DesiredMovementTime); }
	ToggleCharacterSettings(false, true, false);

	CalculateLongestTime();
	GetWorldTimerManager().SetTimer(AdjustCameraTimer, this, &ACameraProjectCharacter::CorrectCameraTransform, GetWorld()->GetDeltaSeconds(), true);
}

void ACameraProjectCharacter::ChangeCameraArmRotation(FRotator NewRotation, bool bIsRelative, float DesiredRotationTime, bool bTakeControl)
{
	CameraExtraRotation = (bIsRelative) ? NewRotation : NewRotation - Controller->GetDesiredRotation();

	CalculateLongestTime();
	if (AutoAdjustTime > 0 || DesiredRotationTime > 0) { CalculateSpeedNeeded(DesiredRotationTime); }
	ToggleCharacterSettings(false, true, false);

	CalculateLongestTime();
	GetWorldTimerManager().SetTimer(AdjustCameraTimer, this, &ACameraProjectCharacter::CorrectCameraTransform, GetWorld()->GetDeltaSeconds(), true);
}

void ACameraProjectCharacter::RandomlyChangeCamera()
{
	float RanX = FMath::RandRange(-120, 120);
	float RanY = FMath::RandRange(-120, 120);
	float RanZ = FMath::RandRange(-120, 120);

	float RanPitch = FMath::RandRange(-120, 120);
	float RanYaw = FMath::RandRange(-120, 120);
	float RanRoll = FMath::RandRange(-120, 120);

	ChangeCameraArmRotation(FRotator(RanPitch, RanYaw, RanRoll).GetNormalized());
	ChangeCameraSocketLocation(FVector(RanX, RanY, RanZ));
}