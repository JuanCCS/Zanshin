// Fill out your copyright notice in the Description page of Project Settings.

#include "Zanshin.h"
#include "ZanshinSpectatorPawn.h"

AZanshinSpectatorPawn::AZanshinSpectatorPawn(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	//Create a Spring Arm Component
	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraAttachmentArm"));
	SpringArmComponent->bEnableCameraLag = true;
	SpringArmComponent->bEnableCameraRotationLag = true;
	SpringArmComponent->AttachTo(RootComponent);
	SpringArmComponent->TargetArmLength = 0.0f;

	// Create a Camera Component	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->AttachTo(SpringArmComponent, USpringArmComponent::SocketName);
	FirstPersonCameraComponent->RelativeLocation = FVector(0.f, 0.f, 0.f); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;
}

void AZanshinSpectatorPawn::SetupPlayerInputComponent(UInputComponent* InputComponent)
{
	check(InputComponent);

	InputComponent->BindAxis("MoveForward", this, &ADefaultPawn::MoveForward);
	InputComponent->BindAxis("MoveRight", this, &ADefaultPawn::MoveRight);
	InputComponent->BindAxis("MoveUp", this, &ADefaultPawn::MoveUp_World);
	InputComponent->BindAxis("Turn", this, &ADefaultPawn::AddControllerYawInput);
	InputComponent->BindAxis("TurnRate", this, &ADefaultPawn::TurnAtRate);
	InputComponent->BindAxis("LookUp", this, &ADefaultPawn::AddControllerPitchInput);
	InputComponent->BindAxis("LookUpRate", this, &AZanshinSpectatorPawn::LookUpAtRate);

	InputComponent->BindAction("LowSpeedScroll", IE_Pressed, this, &AZanshinSpectatorPawn::LowSpeed);
	InputComponent->BindAction("HighSpeedScroll", IE_Pressed, this, &AZanshinSpectatorPawn::HightSpeed);
}

void AZanshinSpectatorPawn::LookUpAtRate(float Val)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Val * BaseLookUpRate * GetWorld()->GetDeltaSeconds() * CustomTimeDilation);
}

void AZanshinSpectatorPawn::LowSpeed()
{
	UFloatingPawnMovement* fpm = Cast<UFloatingPawnMovement>(GetMovementComponent());
	fpm->MaxSpeed -= 85;
}

void AZanshinSpectatorPawn::HightSpeed()
{
	UFloatingPawnMovement* fpm = Cast<UFloatingPawnMovement>(GetMovementComponent());
	fpm->MaxSpeed += 85;
}