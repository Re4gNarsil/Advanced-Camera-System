// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CameraProjectCharacter.generated.h"

UCLASS(config=Game)
class ACameraProjectCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	class UCameraSpringArm* OurCameraSpringArm;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* OurCamera;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	ACameraProjectCharacter();

	void ToggleCameraControlOn();

	void ToggleCameraControlOff();

	void ToggleCharacterSettings(bool bAllowInputs, bool bControlCamera, bool bUseControllerRotation);

	void ToggleCameraSide();

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category= "Camera")
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	float BaseLookUpRate;

	// If we want to move in a set amount of time, set this value to 0 or greater; otherwise, the transform will change at an adjustable speed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
		float AutoAdjustTime = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
		float AutoTurnRate = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
		float AutoMoveRate = 700;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
		FRotator CameraExtraRotation = FRotator(0, 0, 0);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
		FVector CameraArmLocation = FVector(0, 20, 80);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
		FVector CameraSocketOffset = FVector(0, 60, 20);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
		float MaxCameraDistance = 500;

	/**    */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Auto Correct")
		bool bAutoCorrectCameraRotationYaw = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Auto Correct")
		bool bAutoCorrectCameraRotationPitch = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Auto Correct")
		bool bAutoCorrectCameraRotationRoll = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Auto Correct")
		FVector AutoCorrectCameraLocation = FVector(1, 1, 1);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Auto Correct")
		FVector AutoCorrectSocketOffset = FVector(1, 1, 1);

	bool bControllingCamera = false;
	bool bAllowPlayerInputs = true;

protected:

	/** Resets HMD orientation in VR. */
	void OnResetVR();

	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	/** 
	 * Called via input to turn at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUp(float Rate);

	void TurnRight(float Rate);

	void LookUpAtRate(float Rate);

	void ZoomIn(float Rate);

	void ZoomOut(float Rate);

	/** Handler for when a touch input begins. */
	void TouchStarted(ETouchIndex::Type FingerIndex, FVector Location);

	/** Handler for when a touch input stops. */
	void TouchStopped(ETouchIndex::Type FingerIndex, FVector Location);

	void CorrectCameraTransform();

	void CalculateLongestTime();

	void CalculateSpeedNeeded(float DesiredTime);

	int CorrectCameraRotation();

	int CorrectCameraLocation();

	int CorrectSocketLocation();

	int CorrectExtraRotation();

	void ChangeCameraSocketLocation(FVector NewLocation, bool bIsRelative = true, float DesiredMovementTime = -1, bool bTakeControl = true);

	void ChangeCameraArmRotation(FRotator NewRotation, bool bIsRelative = true, float DesiredRotationTime = -1, bool bTakeControl = true);

	FTimerHandle RandomChanges;
	void RandomlyChangeCamera();

	FVector DesiredSocketOffset;
	FVector DesiredArmLocation;

	FTimerHandle AdjustCameraTimer;

	bool bUsingRightSide = true;

	int ChangesNeeded = 0;

	float LongestTimeNeeded = 0;

	float TimeToRotate = 0;
	float TimeToMove = 0;
	float TimeToShift = 0;
	float TimeToChange = 0;

	float SpeedNeeded = 1;
protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class UCameraSpringArm* GetCameraBoom() const { return OurCameraSpringArm; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return OurCamera; }
};

