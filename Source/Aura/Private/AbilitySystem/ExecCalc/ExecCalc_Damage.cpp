// Copyright Shavi


#include "AbilitySystem/ExecCalc/ExecCalc_Damage.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/AuraAttributeSet.h"

struct AuraDamageStatics
{
	DECLARE_ATTRIBUTE_CAPTUREDEF(Armor)
	AuraDamageStatics()
	{
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, Armor, Target, false);
	}
};

static const AuraDamageStatics& DamageStatics()
{
	static AuraDamageStatics DStatics;
	return DStatics;
}

UExecCalc_Damage::UExecCalc_Damage()
{
	RelevantAttributesToCapture.Add(DamageStatics().ArmorDef);
}

void UExecCalc_Damage::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, OUT FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const UAbilitySystemComponent* SourceASC = ExecutionParams.GetSourceAbilitySystemComponent();
	const UAbilitySystemComponent* TargetASC = ExecutionParams.GetTargetAbilitySystemComponent();

	const AActor* SourceAvatar = SourceASC ? SourceASC->GetAvatarActor() : nullptr;
	const AActor* TargetAvatar = TargetASC ? TargetASC->GetAvatarActor() : nullptr;

	// 1. Получаем ссылку на спецификацию эффекта (Spec), который сейчас выполняется.
	// В Spec хранится вся информация: уровень способности, контекст, захваченные теги и т.д.
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

	// 2. Извлекаем контейнеры тегов Атакующего (Source) и Жертвы (Target).
	// Мы используем GetAggregatedTags(), чтобы собрать все теги: и те, что были в Spec, и те, что висят на самих персонажах.
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	// 3. Создаем структуру параметров для оценки (Evaluate).
	// Система GAS должна знать текущие теги обеих сторон, так как они могут влиять на расчет 
	// (например, "Если цель в огне, игнорировать 10% брони").
	FAggregatorEvaluateParameters EvaluationParameters;
	EvaluationParameters.SourceTags = SourceTags;
	EvaluationParameters.TargetTags = TargetTags;

	// 4. Объявляем переменную, куда будет записан результат вычисления брони.
	float Armor = 0.f;

	// 5. Самый важный момент: Пытаемся вычислить величину атрибута Armor, который мы "захватили" ранее в DamageStatics.
	// Мы передаем ArmorDef (определение захвата), параметры с тегами и ссылку на переменную Armor, куда запишется результат.
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().ArmorDef, EvaluationParameters, Armor);

	// 6. Математическая страховка: гарантируем, что броня не будет отрицательной (ниже 0).
	// Даже если дебаффы ушли в минус, для формул мы берем минимум 0.
	Armor = FMath::Max<float>(0.f, Armor);

	// 7. Инкремент (временный или тестовый код). 
	// Ты просто увеличиваешь значение брони на 1. Обычно здесь идет сложная формула урона.
	++Armor;

	// 8. Создаем объект "вычисленных данных модификатора".
	// Мы говорим: "Мы хотим изменить свойство ArmorProperty (броня цели), используя операцию Additive (сложение), 
	// и прибавить к нему наше вычисленное значение Armor".
	const FGameplayModifierEvaluatedData EvaluatedData(DamageStatics().ArmorProperty, EGameplayModOp::Additive, Armor);

	// 9. Отправляем результат в выходные параметры.
	// Это сообщает системе GAS: "Расчет окончен, примени этот модификатор к конечному результату".
	OutExecutionOutput.AddOutputModifier(EvaluatedData);
}
