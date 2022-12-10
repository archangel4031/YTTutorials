// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "GEExec_ElementDamage.generated.h"

/**
 * 
 */
UCLASS()
class GASASSOCIATE_API UGEExec_ElementDamage : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()
	
public:

	UGEExec_ElementDamage();

	void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecParams, FGameplayEffectCustomExecutionOutput& ExecOutputs) const;
};
