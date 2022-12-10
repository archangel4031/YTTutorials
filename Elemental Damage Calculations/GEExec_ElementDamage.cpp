// Fill out your copyright notice in the Description page of Project Settings.


#include "GEExec_ElementDamage.h"
#include "GASAttributeSet.h"


struct SourceCapture
{
	DECLARE_ATTRIBUTE_CAPTUREDEF(AttackPower)
	DECLARE_ATTRIBUTE_CAPTUREDEF(Fire)
	DECLARE_ATTRIBUTE_CAPTUREDEF(Air)
	DECLARE_ATTRIBUTE_CAPTUREDEF(Earth)
	DECLARE_ATTRIBUTE_CAPTUREDEF(Water)

	SourceCapture()
	{
		DEFINE_ATTRIBUTE_CAPTUREDEF(UGASAttributeSet, AttackPower, Source, true)
		DEFINE_ATTRIBUTE_CAPTUREDEF(UGASAttributeSet, Fire, Source, true)
		DEFINE_ATTRIBUTE_CAPTUREDEF(UGASAttributeSet, Air, Source, true)
		DEFINE_ATTRIBUTE_CAPTUREDEF(UGASAttributeSet, Earth, Source, true)
		DEFINE_ATTRIBUTE_CAPTUREDEF(UGASAttributeSet, Water, Source, true)
	}
};

struct TargetCapture
{
	DECLARE_ATTRIBUTE_CAPTUREDEF(Health)
	DECLARE_ATTRIBUTE_CAPTUREDEF(Shield)

	TargetCapture()
	{
		DEFINE_ATTRIBUTE_CAPTUREDEF(UGASAttributeSet, Health, Target, true)
		DEFINE_ATTRIBUTE_CAPTUREDEF(UGASAttributeSet, Shield, Target, true)
	}
};

static SourceCapture& GetSourceAttributes()
{
	static SourceCapture SourceCaptureVar;
	return SourceCaptureVar;
}

static TargetCapture& GetTargetAttributes()
{
	static TargetCapture TargetCaptureVar;
	return TargetCaptureVar;
}

UGEExec_ElementDamage::UGEExec_ElementDamage()
{
	//Source Captures
	RelevantAttributesToCapture.Add(GetSourceAttributes().AttackPowerDef);
	RelevantAttributesToCapture.Add(GetSourceAttributes().FireDef);
	RelevantAttributesToCapture.Add(GetSourceAttributes().AirDef);
	RelevantAttributesToCapture.Add(GetSourceAttributes().EarthDef);
	RelevantAttributesToCapture.Add(GetSourceAttributes().WaterDef);

	//Target Captures
	RelevantAttributesToCapture.Add(GetTargetAttributes().HealthDef);
	RelevantAttributesToCapture.Add(GetTargetAttributes().ShieldDef);
}

void UGEExec_ElementDamage::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecParams, FGameplayEffectCustomExecutionOutput& ExecOutputs) const
{
	//Get Spec
	const FGameplayEffectSpec& Spec = ExecParams.GetOwningSpec();

	//Get Source and Target Tags for Buffs and Debuffs
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	//Declare Variables to hold captured Magnitudes
	//
	//Source Variables
	float SourceAttackPower = 0.0f;
	float SourceFire = 0.0f;
	float SourceAir = 0.0f;
	float SourceEarth = 0.0f;
	float SourceWater = 0.0f;

	//Target Variables
	float TargetHealth = 0.0f;
	float TargetShield = 0.0f;

	//Begin capturing attributes
	ExecParams.AttemptCalculateCapturedAttributeMagnitude(GetSourceAttributes().AttackPowerDef, FAggregatorEvaluateParameters(), SourceAttackPower);
	ExecParams.AttemptCalculateCapturedAttributeMagnitude(GetSourceAttributes().FireDef, FAggregatorEvaluateParameters(), SourceFire);
	ExecParams.AttemptCalculateCapturedAttributeMagnitude(GetSourceAttributes().AirDef, FAggregatorEvaluateParameters(), SourceAir);
	ExecParams.AttemptCalculateCapturedAttributeMagnitude(GetSourceAttributes().EarthDef, FAggregatorEvaluateParameters(), SourceEarth);
	ExecParams.AttemptCalculateCapturedAttributeMagnitude(GetSourceAttributes().WaterDef, FAggregatorEvaluateParameters(), SourceWater);

	ExecParams.AttemptCalculateCapturedAttributeMagnitude(GetTargetAttributes().HealthDef, FAggregatorEvaluateParameters(), TargetHealth);
	ExecParams.AttemptCalculateCapturedAttributeMagnitude(GetTargetAttributes().ShieldDef, FAggregatorEvaluateParameters(), TargetShield);

	//Debug Logging
	UE_LOG(LogTemp, Warning, TEXT("Source Attack Power: %f"), SourceAttackPower);
	UE_LOG(LogTemp, Warning, TEXT("Source Fire: %f"), SourceFire);
	UE_LOG(LogTemp, Warning, TEXT("Source Air: %f"), SourceAir);
	UE_LOG(LogTemp, Warning, TEXT("Source Earth: %f"), SourceEarth);
	UE_LOG(LogTemp, Warning, TEXT("Source Water: %f"), SourceWater);

	UE_LOG(LogTemp, Warning, TEXT("Target Health: %f"), TargetHealth);
	UE_LOG(LogTemp, Warning, TEXT("Target Shield: %f"), TargetShield);

	//Perform Calculations
	//
	//If Tags are present on target, we do not count that element damage
	if (TargetTags->HasTagExact(FGameplayTag::RequestGameplayTag("char.immunity.fire")))
	{
		SourceFire = 0.0f;
	}
	if (TargetTags->HasTagExact(FGameplayTag::RequestGameplayTag("char.immunity.air")))
	{
		SourceAir = 0.0f;
	}
	if (TargetTags->HasTagExact(FGameplayTag::RequestGameplayTag("char.immunity.earth")))
	{
		SourceEarth = 0.0f;
	}
	if (TargetTags->HasTagExact(FGameplayTag::RequestGameplayTag("char.immunity.water")))
	{
		SourceWater = 0.0f;
	}

	float FinalDamage = 0.0f;
	//If Target has no shield, we directly affect the health
	if (TargetShield <= 0.0f)
	{
		FinalDamage = TargetHealth - ((SourceAttackPower * (SourceFire / 100.0f)) +
			(SourceAttackPower * (SourceAir / 100.0f)) +
			(SourceAttackPower * (SourceEarth / 100.0f)) +
			(SourceAttackPower * (SourceWater / 100.0f)));

		TargetHealth = FinalDamage;
		UE_LOG(LogTemp, Warning, TEXT("INSIDE IF==> Target Health: %f"), TargetHealth);
	}
	//If Target has some shield we deduct the calculation from shield and health
	else
	{
		FinalDamage = TargetShield - ((SourceAttackPower * (SourceFire / 100.0f)) +
			(SourceAttackPower * (SourceAir / 100.0f)) +
			(SourceAttackPower * (SourceEarth / 100.0f)) +
			(SourceAttackPower * (SourceWater / 100.0f)));

		//If attack completely depletes shield, we subtract the additional damage from health
		if (FinalDamage < 0.0f)
		{
			TargetHealth = TargetHealth - fabs(FinalDamage);
			TargetShield = 0.0f;
			UE_LOG(LogTemp, Warning, TEXT("INSIDE ELSE===>IF==> Target Health: %f"), TargetHealth);
			UE_LOG(LogTemp, Warning, TEXT("INSIDE ELSE===>IF==> Target Shield: %f"), TargetShield);

		}
		//Else simply modify the shield
		else
		{
			TargetShield = FinalDamage;
			UE_LOG(LogTemp, Warning, TEXT("INSIDE ELSE===>ELSE==> Target Shield: %f"), TargetShield);
		}
	}

	//Apply changes to Target
	ExecOutputs.AddOutputModifier(FGameplayModifierEvaluatedData(GetTargetAttributes().HealthProperty, EGameplayModOp::Override, TargetHealth));
	ExecOutputs.AddOutputModifier(FGameplayModifierEvaluatedData(GetTargetAttributes().ShieldProperty, EGameplayModOp::Override, TargetShield));
}

