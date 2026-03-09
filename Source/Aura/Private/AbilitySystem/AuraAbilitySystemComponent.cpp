// Copyright Shavi


#include "AbilitySystem/AuraAbilitySystemComponent.h"

//After all Actor Info set we binding our callback methods to delegates
void UAuraAbilitySystemComponent::AbilityActorInfoSet()
{
	/** Called on server whenever a GE is applied to self. This includes instant and duration based GEs. */
	OnGameplayEffectAppliedDelegateToSelf.AddUObject(this, &UAuraAbilitySystemComponent::EffectApplied);
}
void UAuraAbilitySystemComponent::AddCharacterAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupAbilities)
{
	for (TSubclassOf<UGameplayAbility> AbilityClass : StartupAbilities)
	{
		//Creating a spec of ability
		FGameplayAbilitySpec AbilitySpec= FGameplayAbilitySpec(AbilityClass, 1);
		//Inherent function that grants/gives ability
		//GiveAbility(AbilitySpec);
		GiveAbilityAndActivateOnce(AbilitySpec);
	}
}
//Callback method that activates after any Effect applies to self
void UAuraAbilitySystemComponent::EffectApplied(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayEffectSpec& EffectSpec, FActiveGameplayEffectHandle ActiveEffectHandle)
{
	FGameplayTagContainer TagContainer;
	EffectSpec.GetAllAssetTags(TagContainer);
	//Любой класс, который привяжется к нашему делегату, получит TagContainer со всей информацией
	EffectAssetTags.Broadcast(TagContainer);
}
