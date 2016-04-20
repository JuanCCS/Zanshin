// Fill out your copyright notice in the Description page of Project Settings.

#include "Zanshin.h"
#include "ZanshinSwitchCover.h"
#include "ZanshinCover.h"
#include "Arrow/ZanshinBasicArrow.h"

// Sets default values
AZanshinSwitchCover::AZanshinSwitchCover()
{
	SwitchBoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComp"));
	SwitchBoxComponent->BodyInstance.SetCollisionProfileName("IgnoreOnlyPawn");
	SwitchBoxComponent->OnComponentHit.AddDynamic(this, &AZanshinSwitchCover::OnHit);		// set up a notification for when this component hits something blocking
	SwitchBoxComponent->SetNetAddressable();
	SwitchBoxComponent->SetIsReplicated(true);

	RootComponent = SwitchBoxComponent;
	RootComponent->SetNetAddressable();
	RootComponent->SetIsReplicated(true);

 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AZanshinSwitchCover::BeginPlay()
{
	Super::BeginPlay();
	
	Reset();
}

// Called every frame
void AZanshinSwitchCover::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );
}

void AZanshinSwitchCover::OnHit(AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (bCanActive)
	{
		Active();
		
		AZanshinBasicArrow* Arrow = Cast<AZanshinBasicArrow>(OtherActor);
		if (Arrow != nullptr) {
			AZanshinCover* Owner = Cast<AZanshinCover>(CoverOwner);
			if (Owner != nullptr) {
				Owner->SendClick();
			}
		}
	}
}

void AZanshinSwitchCover::Reset() { bCanActive = true; }

void AZanshinSwitchCover::Active() { bCanActive = false; }

//////////////////////////////////////////////////////////////////////////
// Getter and Setter

void AZanshinSwitchCover::SetCover(AActor* Cover)
{
	CoverOwner = Cover;
	RootComponent->AttachTo(Cover->GetRootComponent(), FName(*Cover->GetRootComponent()->GetName()), EAttachLocation::KeepWorldPosition);
}