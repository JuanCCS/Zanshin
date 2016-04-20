// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Pawn.h"
#include "ZanshinDrone.generated.h"

UCLASS()
class ZANSHIN_API AZanshinDrone : public AZanshinNetRepCharacter
{
	GENERATED_BODY()

	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class USphereComponent* SphereComponent;

public:
	// Sets default values for this pawn's properties
	AZanshinDrone(const class FObjectInitializer& ObjectInitializer);

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;

	UPROPERTY(EditAnywhere, Category = Behavior)
	class UBehaviorTree* BehaviorTree;

	UFUNCTION(BlueprintCallable, Category = "Damage")
	void OnHit(class AActor* OtherActor, class UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
};