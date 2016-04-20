// Fill out your copyright notice in the Description page of Project Settings.

#include "Zanshin.h"
#include "BTTask_BB_FindNextPathNode.h"
#include "AI/ZanshinAIController.h"

UBTTask_BB_FindNextPathNode::UBTTask_BB_FindNextPathNode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

EBTNodeResult::Type UBTTask_BB_FindNextPathNode::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AZanshinAIController* MyController = Cast<AZanshinAIController>(OwnerComp.GetAIOwner());
	TArray<ATargetPoint*> LocationsToWalk = MyController->LocationsToWalk;
	if (MyController == NULL || LocationsToWalk.Num() == 0) {
		return EBTNodeResult::Failed;
	}

	//Index
	uint8 PathIndex = MyController->GetPathIndex();
	PathIndex++;
	if (!LocationsToWalk.IsValidIndex(PathIndex)) {
		PathIndex = 0;
	}
	MyController->SetPathIndex(PathIndex);

	ATargetPoint* NewTarget = LocationsToWalk[PathIndex];
	if (NewTarget)
	{
		FVector NewLocation = NewTarget->GetActorLocation();
		OwnerComp.GetBlackboardComponent()->SetValue<UBlackboardKeyType_Vector>(BlackboardKey.GetSelectedKeyID(), NewLocation);

		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;
}