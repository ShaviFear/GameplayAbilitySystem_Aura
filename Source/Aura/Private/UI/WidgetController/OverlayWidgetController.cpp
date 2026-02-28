// Copyright Shavi


#include "UI/WidgetController/OverlayWidgetController.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"

//Broadcasting initial values from Attribute set of owner to his widget for example
void UOverlayWidgetController::BroadcastInitialValues()
{
    // Мы должны привести AttributeSet к нашему AuraAttributeSet
    // AttributeSet уже есть в базовом классе AuraWidgetController
    const UAuraAttributeSet* AuraAS = CastChecked<UAuraAttributeSet>(AttributeSet);

    // Теперь мы берем данные из РЕАЛЬНОГО сета нашего персонажа
    OnHealthChanged.Broadcast(AuraAS->GetHealth());
    OnMaxHealthChanged.Broadcast(AuraAS->GetMaxHealth());
    OnManaChanged.Broadcast(AuraAS->GetMana());
    OnMaxManaChanged.Broadcast(AuraAS->GetMaxMana());
}

void UOverlayWidgetController::BindCallbacksToDependencies()
{
    const UAuraAttributeSet* AuraAS = CastChecked<UAuraAttributeSet>(AttributeSet);

    /**
    1. AuraAS->GetHealthAttribute() : Макрос ATTRIBUTE_ACCESSORS создал эту функцию.Она возвращает не само число(здоровье), 
    а описание того, что такое "Здоровье" для движка.
    2. GetGameplayAttributeValueChangeDelegate(...) : Ты обращаешься к AbilitySystemComponent(ASC) и говоришь : 
    «Дай мне "колокольчик", который звенит каждый раз, когда меняется атрибут Здоровья».
    3 AddLambda([this](const FOnAttributeChangeData& Data) : Ты привязываешь(подписываешь) свою лямбда функцию к этому колокольчику.
    4. Теперь : Как только ASC изменит здоровье(от урона или хила), он автоматически вызовет твою функцию HealthChanged.
    */
    AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AuraAS->GetHealthAttribute()).AddLambda(
        [this](const FOnAttributeChangeData& Data)
        {
            OnHealthChanged.Broadcast(Data.NewValue);
        });

    AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AuraAS->GetMaxHealthAttribute()).AddLambda(
        [this](const FOnAttributeChangeData& Data)
        {
            OnMaxHealthChanged.Broadcast(Data.NewValue);
        });

    AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AuraAS->GetManaAttribute()).AddLambda(
        [this](const FOnAttributeChangeData& Data)
        {
            OnManaChanged.Broadcast(Data.NewValue);
        });

    AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AuraAS->GetMaxManaAttribute()).AddLambda(
        [this](const FOnAttributeChangeData& Data)
        {
            OnMaxManaChanged.Broadcast(Data.NewValue);
        });

    Cast<UAuraAbilitySystemComponent>(AbilitySystemComponent)->EffectAssetTags.AddLambda(
        [this](const FGameplayTagContainer& AssetTags /*Мы получаём AssetTags из делегата EffectAssetTags*/)
        {
            for (const FGameplayTag& Tag : AssetTags)
            {
                // For example, say that Tag = Message.HealthPotion
                // "Message.HealthPotion".MatchesTag("Message") will return True, "Message".MessageTag(Message.HealthPotion) return False
                FGameplayTag MessageTag = FGameplayTag::RequestGameplayTag(FName("Message"));
                
                if (Tag.MatchesTag(MessageTag))
                {
                    const FUIWidgetRow* Row = GetDataTableRowByTag<FUIWidgetRow>(MessageWidgetDataTable, Tag);
                    MessageWidgetRowDelegate.Broadcast(*Row);
                }
            }
        }
    );
}

/**
Почему это сделано именно так? (Архитектура)
Ты выстроил цепочку связей:

AttributeSet меняет значение Health.

AbilitySystemComponent замечает это и «звенит в колокольчик».

WidgetController (C++) слышит этот звон, ловит новое значение в функции HealthChanged.

WidgetController делает Broadcast своего динамического делегата.

Blueprint Виджета (который подписан на этот делегат в редакторе) получает новое число и обновляет полоску здоровья (ProgressBar).
*/
