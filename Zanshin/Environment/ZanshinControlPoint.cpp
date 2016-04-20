// Fill out your copyright notice in the Description page of Project Settings.

#include "Zanshin.h"
#include "ZanshinControlPoint.h"
#include "Player/ZanshinCharacter.h"
#include "Arrow/ZanshinTeleportArrow.h"

// Sets default values
AZanshinControlPoint::AZanshinControlPoint(const class FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	bReplicates = true;
	bNetLoadOnClient = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

	SphereCollider = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere Collider"));
	SphereCollider->AttachTo(RootComponent);
	SphereCollider->SetSphereRadius(250.0f);
	SphereCollider->AttachTo(Root);

	TeleportSphereCollider = CreateDefaultSubobject<USphereComponent>(TEXT("Teleport Sphere Collider"));
	TeleportSphereCollider->AttachTo(RootComponent);
	TeleportSphereCollider->SetSphereRadius(250.0f);
	TeleportSphereCollider->AttachTo(Root);

	BaseMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("BaseMesh"));
	BaseMesh->AttachTo(Root);

 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

void AZanshinControlPoint::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AZanshinControlPoint, TeamControlling);
	DOREPLIFETIME(AZanshinControlPoint, CurrentPoints);

}

// Called when the game starts or when spawned
void AZanshinControlPoint::BeginPlay()
{
	Super::BeginPlay();
	SphereCollider->OnComponentBeginOverlap.AddDynamic(this, &AZanshinControlPoint::OnOverlapBegin);
	SphereCollider->OnComponentEndOverlap.AddDynamic(this, &AZanshinControlPoint::OnOverlapEnd);
}

// Called every frame
void AZanshinControlPoint::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );
	if (RedTeamPlayersCount > 0 || BlueTeamPlayersCount > 0){
		if (RedTeamPlayersCount != BlueTeamPlayersCount)
			HandleCapture(DeltaTime);
	}
}

void AZanshinControlPoint::HandleCapture(float DeltaTime){

	int32 difference = FMath::Abs(RedTeamPlayersCount - BlueTeamPlayersCount);

	if (RedTeamPlayersCount > BlueTeamPlayersCount && CurrentPoints > -MaxPoints){
		CurrentPoints -= CaptureSpeed * DeltaTime;

		if (CurrentPoints < 0)
			TeamColorProperty::CP_None;

		if (CurrentPoints <= -MaxPoints)
		{
			TeamControlling = TeamColorProperty::CP_Red;
			UGameplayStatics::PlaySound2D(GetWorld(), PointCaptured, 0.5f, 1.0f, 0.0f);
		}
	}
	else if (BlueTeamPlayersCount > RedTeamPlayersCount && CurrentPoints < MaxPoints){
		
		CurrentPoints += CaptureSpeed * DeltaTime;
		
		if (CurrentPoints > 0)
			TeamColorProperty::CP_None;
		
		if (CurrentPoints >= MaxPoints)
		{
			TeamControlling = TeamColorProperty::CP_Blue;
			UGameplayStatics::PlaySound2D(GetWorld(), PointCaptured, 0.5f, 1.0f, 0.0f);
		}
	}

	FMath::Clamp(CurrentPoints, -MaxPoints, MaxPoints);
}

///////////////////////////////////////////////////////////////////////
// Collision Detection

void AZanshinControlPoint::OnOverlapBegin(class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (Role < ROLE_Authority)
		return;

	AZanshinCharacter* Player = Cast<AZanshinCharacter>(OtherActor);
	if (Player != nullptr){
		Player->Client_UpdateControlPoints(true, CurrentPoints, CapturePointLetter);
		UGameplayStatics::SpawnSoundAtLocation(GetWorld(), CapturingPoint, Root->RelativeLocation, Root->RelativeRotation, 0.5f, 1.0f, 0.0f);
		switch (Player->CharacterTeam){
			case 1:
				RedTeamPlayersCount++;
				break;
			case 2:
				BlueTeamPlayersCount++;
				break;
			default:
				return;
		}
	}
}

void AZanshinControlPoint::OnOverlapEnd(class AActor * OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AZanshinCharacter* Player = Cast<AZanshinCharacter>(OtherActor);
	if (Player != nullptr){
		Player->Client_UpdateControlPoints(false, CurrentPoints, CapturePointLetter);
		switch (Player->CharacterTeam){
			case 1:
				RedTeamPlayersCount--;
				break;
			case 2:
				BlueTeamPlayersCount--;
				break;
			default:
				return;
		}
	}
}