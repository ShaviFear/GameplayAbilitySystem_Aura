// Copyright Shavi

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h" 
#include "AbilitySystemComponent.h"
#include "AuraAttributeSet.generated.h"

/*#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) - этот макрос при компиляции автоматически сгенерирует следующие функции:

GetHealth() — возвращает текущее значение(float).

GetHealthAttribute() — возвращает сам объект атрибута.

SetHealth(float NewVal) — устанавливает значение.

InitHealth(float NewVal) — инициализирует значение.*/

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 *
 */

UCLASS()
class AURA_API UAuraAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
public:
	UAuraAttributeSet();

	/** Регистрирует переменные для сетевой репликации (передачи данных от сервера к клиентам). */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Вызывается ПЕРЕД изменением атрибута. Идеальное место для "клампинга" (ограничения значений, например, чтобы Health не стало > MaxHealth). */
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Health, Category = "Vital Attributes")
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, Health);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxHealth, Category = "Vital Attributes")
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, MaxHealth);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Mana, Category = "Vital Attributes")
	FGameplayAttributeData Mana;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, Mana);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxMana, Category = "Vital Attributes")
	FGameplayAttributeData MaxMana;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, MaxMana);


	/* * OnRep-функции (RepNotify).
	 * Вызываются на клиентах, когда сервер прислал новое значение атрибута.
	 * Нужны, чтобы система способностей (GAS) на клиенте узнала об изменениях и обновила UI.
	 */
	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldHealth) const;

	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth) const;

	UFUNCTION()
	void OnRep_Mana(const FGameplayAttributeData& OldMana) const;

	UFUNCTION()
	void OnRep_MaxMana(const FGameplayAttributeData& OldMaxMana) const;


};

