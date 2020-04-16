// Fill out your copyright notice in the Description page of Project Settings.


#include "CameraCharacter.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "CameraArmComponent.h"

// Sets default values
ACameraCharacter::ACameraCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	//BaseTurnRate = 45.f;
	//BaseLookUpRate = 45.f;

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
	OurCameraArm = CreateDefaultSubobject<UCameraArmComponent>(TEXT("CameraArm"));
	OurCameraArm->SetupAttachment(RootComponent);
	OurCameraArm->DesiredCameraDistance = 200.0f; // The camera follows at this distance behind the character
	//OurCameraArm->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	OurCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	OurCamera->SetupAttachment(OurCameraArm); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	OurCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character)
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)


	// Create a camera anchor for the boom to attach to so we can adjust where the camera actually is
	//UCameraComponent* OurCamera = this->FindComponentByClass<UCameraComponent>();

	//PointComponent = CreateDefaultSubobject<USceneComponent>(TEXT("CameraPoint"));
	//PointComponent->SetupAttachment(FollowCamera);
	//PointComponent->SnapTo(FollowCamera);

	//PointComponent->SetRelativeLocation(FVector::ZeroVector);
	


}

// Called when the game starts or when spawned
void ACameraCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ACameraCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ACameraCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

