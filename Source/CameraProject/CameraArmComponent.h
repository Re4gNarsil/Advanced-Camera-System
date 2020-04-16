// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Camera/CameraComponent.h"
#include "CameraArmComponent.generated.h"


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class CAMERAPROJECT_API UCameraArmComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCameraArmComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	void PositionOurCamera();

public:	
	// Called every frame
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera", meta = (AllowPrivateAccess = "true"))
		float DesiredCameraDistance = 200;

private:
	UCameraComponent* OurCamera;

	AActor* OurOwner;

	bool TryingToReset = true;

	FVector DesiredLocalLocation;
	FRotator DesiredLocalRotation;
		
};
