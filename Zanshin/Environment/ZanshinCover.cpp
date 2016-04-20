// Fill out your copyright notice in the Description page of Project Settings.

#include "Zanshin.h"
#include "ZanshinCover.h"
#include "ZanshinSwitchCover.h"
#include "ZanshinCover.h"

// Sets default values
AZanshinCover::AZanshinCover()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AZanshinCover::BeginPlay()
{
	Super::BeginPlay();

	for (auto& Swt : Switches) {
		Swt->SetCover(this);
	}

	MoveLerp = 0;
	ClicksToActivate = 0;
	PointsIndex = 0;

	CountSwitches();
}

// Called every frame
void AZanshinCover::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

	if (bCanMove) {
		Move(DeltaTime);
	}
}

void AZanshinCover::SendClick()
{
	ClicksToActivate--;

	if (ClicksToActivate <= 0) {
		bCanMove = true;

		CountSwitches();

		AngleToRotate = FMath::Abs(Points[PointsIndex]->GetActorRotation().Yaw);
		float AngleHelper = AngleToRotate - 360;
		if (AngleHelper > -180) {
			AngleToRotate = AngleHelper;
		}
	}
}

void AZanshinCover::Move(float DeltaTime)
{
	MoveLerp += DeltaTime;

	FVector NewPosition = FMath::Lerp<FVector>(GetActorLocation(), Points[PointsIndex]->GetActorLocation(), MoveLerp * MoveSpeedMultiplier);
	SetActorLocation(NewPosition);

	FRotator NewRotation = FRotator(GetActorRotation());
	NewRotation.Yaw = FMath::Lerp<float>(GetActorRotation().Yaw, AngleToRotate, MoveLerp * MoveSpeedMultiplier);;
	SetActorRotation(NewRotation);

	float DifferenceLocations = FVector::Dist(GetActorLocation(), Points[PointsIndex]->GetActorLocation());
	if (DifferenceLocations < 10) {
		bCanMove = false;
		MoveLerp = 0;
		PointsIndex++;

		if (PointsIndex >= Points.Num()) {
			PointsIndex = 0;
		}
	}
}

void AZanshinCover::CountSwitches()
{
	ClicksToActivate = 0;

	for (auto& Swt : Switches) {
		ClicksToActivate++;
		Swt->Reset();
	}
}