// Fill out your copyright notice in the Description page of Project Settings.


#include "CameraArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "DrawDebugHelpers.h"
#include "CollisionQueryParams.h"
#include "WorldCollision.h"
#include "Engine/World.h"

// Sets default values for this component's properties
UCameraArmComponent::UCameraArmComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostPhysics;

	bAutoActivate = true;


	// Create a camera anchor for the boom to attach to so we can adjust where the camera actually is

	// ...
}


// Called when the game starts
void UCameraArmComponent::BeginPlay()
{
	Super::BeginPlay();

	OurOwner = GetOwner();

	OurCamera = Cast<UCameraComponent>(GetChildComponent(0)); //Cast<UCameraComponent>(GetChildComponent(0));
	OurCamera->RegisterComponent();

	DesiredLocalLocation = GetComponentLocation() - OurOwner->GetActorLocation();
	DesiredLocalRotation = GetComponentRotation() - OurOwner->GetActorRotation();

	PositionOurCamera();

	// ...
	
}

void UCameraArmComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);


	PositionOurCamera();


}

void UCameraArmComponent::PositionOurCamera()
{
	//FVector DesiredArmLocation = 

	//FVector DesiredCameraLocation = GetComponentLocation() + (GetForwardVector().BackwardVector * DesiredCameraDistance);




	//PreviousArmOrigin = ArmOrigin;
	//PreviousDesiredLoc = DesiredLoc;

	// Now offset camera position back along our rotation
	FVector DesiredCameraLocation = GetComponentLocation() + (GetForwardVector().BackwardVector * DesiredCameraDistance);

	// Add socket offset in local space
	//DesiredLoc += FRotationMatrix(DesiredRot).TransformVector(SocketOffset);

	// Do a sweep to ensure we are not penetrating the world
	FVector ResultLoc;
	//if (bDoTrace && (TargetArmLength != 0.0f))
	//{
		//bIsCameraFixed = true;
		FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(SpringArm), false, GetOwner());

		FCollisionQueryParams CameraQueryParams;

		CameraQueryParams.AddIgnoredActor(GetOwner());
		CameraQueryParams.bTraceComplex = false;

		FHitResult CameraHitResult;

		FHitResult SweepResult;

		GetWorld()->LineTraceSingleByChannel(CameraHitResult, GetComponentLocation(), DesiredCameraLocation, ECollisionChannel::ECC_Camera, CameraQueryParams);

		GetWorld()->SweepSingleByChannel(SweepResult, GetComponentLocation(), DesiredCameraLocation, FQuat::Identity, ECollisionChannel::ECC_Camera, FCollisionShape::MakeSphere(12), CameraQueryParams);

		//FVector DesiredLocalOffset = CameraHitResult. - GetComponentLocation();

		DrawDebugSphere(GetWorld(), SweepResult.TraceStart, 40, 16, FColor(0, 255, 0), false, -1, 0, 10);
		DrawDebugSphere(GetWorld(), SweepResult.TraceEnd, 50, 12, FColor(255, 0, 0), false, -1, 0, 10);
		DrawDebugSphere(GetWorld(), SweepResult.ImpactPoint, 60, 8, FColor(0, 0, 255), false, -1, 0, 10);

		OurCamera->SetWorldLocation(CameraHitResult.TraceEnd);

		//ResultLoc = BlendLocations(DesiredLoc, Result.Location, Result.bBlockingHit, DeltaTime);

		//if (ResultLoc == DesiredLoc)
		//{
		//	bIsCameraFixed = false;
		//}




	UE_LOG(LogTemp, Warning, TEXT("%s"), *DesiredCameraLocation.ToString());
	UE_LOG(LogTemp, Error, TEXT("%s"), *OurCamera->GetComponentLocation().ToString());

	DrawDebugLine(GetWorld(), GetComponentLocation(), DesiredCameraLocation, FColor(255, 0, 0), false, -1, 0, 10);
	DrawDebugLine(GetWorld(), GetComponentLocation(), CameraHitResult.Location, FColor(0, 0, 255), false, -1, 0, 10);
	//DrawDebugLine(GetWorld(), GetComponentLocation(), CameraHitResult.ImpactPoint, FColor(0, 255, 0), false, -1, 0, 10);



	//UE_LOG(LogTemp, Warning, TEXT("%s"), *GetComponentLocation().ToString());
	//UE_LOG(LogTemp, Error, TEXT("%s"), *CameraDesiredLocation.ToString());
}