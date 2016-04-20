// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "ZanshinBasicPickup.generated.h"

UCLASS()
class ZANSHIN_API AZanshinBasicPickup : public AActor
{
	GENERATED_BODY()

protected:
	//////////////////////////////////////////////////////////////////////////
	// Components

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Root, meta = (AllowPrivateAccess = "true"))
	class USceneComponent* Root;

	// Sphere collision component
	UPROPERTY(VisibleDefaultsOnly, Category = Pickup)
	class USphereComponent* SphereComponent;	

	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class USkeletalMeshComponent* BaseMesh;

	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class USkeletalMeshComponent* TopMesh;
	
public:	

	//////////////////////////////////////////////////////////////////////////
	// Constructor

	// Sets default values for this actor's properties
	AZanshinBasicPickup();

	UPROPERTY(EditAnywhere, Category = Pickup)
	float TimeToRespawn;

	UPROPERTY(EditAnywhere, Category = Gameplay)
	FName ItemAttachPoint;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Particle)
	class UParticleSystemComponent* Particles;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	UFUNCTION(BlueprintCallable, Category = "Pickup")
	void OnOverlapBegin(class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	virtual void Activate();
	virtual void Deactivate();

protected:

	virtual void ActiveEffect(class AActor* OtherActor);

private:
	
	bool bIsActive;
	float TimeCounter;
};
