// Copyright Shavi


#include "AbilitySystem/AuraAbilitySystemComponent.h"

//After all Actor Info set we binding our callback methods to delegates
void UAuraAbilitySystemComponent::AbilityActorInfoSet()
{
	/** Called on server whenever a GE is applied to self. This includes instant and duration based GEs. */
	OnGameplayEffectAppliedDelegateToSelf.AddUObject(this, &UAuraAbilitySystemComponent::EffectApplied);
}
//Callback method that activates after any Effect applies to self
void UAuraAbilitySystemComponent::EffectApplied(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayEffectSpec& EffectSpec, FActiveGameplayEffectHandle ActiveEffectHandle)
{
	FGameplayTagContainer TagContainer;
	EffectSpec.GetAllAssetTags(TagContainer);
	//Любой класс, который привяжется к нашему делегату, получит TagContainer со всей информацией
	EffectAssetTags.Broadcast(TagContainer);
}
