// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "ZanshinSwitchCover.generated.h"

UCLASS()
class ZANSHIN_API AZanshinSwitchCover : public AActor
{
	GENERATED_BODY()

	//////////////////////////////////////////////////////////////////////////
	// Components

	// Sphere collision component
	UPROPERTY(VisibleDefaultsOnly, Category = Switch)
	class UBoxComponent* SwitchBoxComponent;
	
public:	
	//////////////////////////////////////////////////////////////////////////
	// Constructor

	// Sets default values for this actor's properties
	AZanshinSwitchCover();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	/** called when projectile hits something */
	UFUNCTION(BlueprintCallable, Category = "Active")
	void OnHit(AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	void Reset();

	//////////////////////////////////////////////////////////////////////////
	// Getter and Setter

	void SetCover(AActor* Cover);

private:

	bool bCanActive;
	AActor* CoverOwner;

	void Active();
};