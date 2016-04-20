// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "GameMode/ZanshinGameModeTeamDeathMatch.h"
#include "Player/ZanshinPlayerState.h"
#include "ZanshinControlPoint.generated.h"

UCLASS()
class ZANSHIN_API AZanshinControlPoint : public AActor
{
	GENERATED_BODY()

	//////////////////////////////////////////////////////////////////////////
	// Compenents

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Root, meta = (AllowPrivateAccess = "true"))
	class USceneComponent* Root;

	UPROPERTY(VisibleAnywhere, Category = Collider)
	USphereComponent* SphereCollider;

	UPROPERTY(VisibleAnywhere, Category = Collider)
	USphereComponent* TeleportSphereCollider;

	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class USkeletalMeshComponent* BaseMesh;

public:
	// Sets default values for this actor's properties
	AZanshinControlPoint(const class FObjectInitializer& ObjectInitializer);

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(float DeltaSeconds) override;

	float CaptureSpeed = 50.f;

	///////////////////////////////////////////////////////////////////////
	// Sounds

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Capturing Point")
	USoundCue* PointCaptured;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Capturing Point")
	USoundCue* CapturingPoint;

	/* The max treshold of accumulated points. */
	float MaxPoints = 100.f;

	/* Red Team subtracts, Blue Team Adds. Works with only two teams.*/
	UPROPERTY(EditAnywhere, replicated, BlueprintReadWrite, Category = Gameplay)
	float CurrentPoints = 0.f;

	/* An index that denotes the letter of each control point. 0:2 => A:C */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	int32 CapturePointLetter;

	///////////////////////////////////////////////////////////////////////
	// Collision Detection

	UFUNCTION()
	void OnOverlapBegin(class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(class AActor * OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UPROPERTY(BlueprintReadOnly, replicated, Category = Score)
	TeamColorProperty TeamControlling;

	int32 RedTeamPlayersCount = 0;
	int32 BlueTeamPlayersCount = 0;

	void HandleCapture(float DeltaTime);

private:


};