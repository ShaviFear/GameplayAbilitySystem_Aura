// Copyright Shavi


#include "UI/WidgetController/OverlayWidgetController.h"
#include "AbilitySystem/AuraAttributeSet.h"

//Broadcasting initial values from Attribute set of owner to his widget for example
void UOverlayWidgetController::BroadcastInitialValues()
{
    // Мы должны привести AttributeSet к нашему AuraAttributeSet
    // AttributeSet уже есть в базовом классе AuraWidgetController
    const UAuraAttributeSet* AuraAS = CastChecked<UAuraAttributeSet>(AttributeSet);

    // Теперь мы берем данные из РЕАЛЬНОГО сета нашего персонажа
    OnHealthChanged.Broadcast(AuraAS->GetHealth());
    OnMaxHealthChanged.Broadcast(AuraAS->GetMaxHealth());
}

void UOverlayWidgetController::BindCallbacksToDependencies()
{
    const UAuraAttributeSet* AuraAS = CastChecked<UAuraAttributeSet>(AttributeSet);

    /**
    1. AuraAS->GetHealthAttribute() : Макрос ATTRIBUTE_ACCESSORS создал эту функцию.Она возвращает не само число(здоровье), 
    а описание того, что такое "Здоровье" для движка.
    2. GetGameplayAttributeValueChangeDelegate(...) : Ты обращаешься к AbilitySystemComponent(ASC) и говоришь : 
    «Дай мне "колокольчик", который звенит каждый раз, когда меняется атрибут Здоровья».
    3 .AddUObject(this, &UOverlayWidgetController::HealthChanged) : Ты привязываешь(подписываешь) свою функцию HealthChanged к этому колокольчику.
    4. Теперь : Как только ASC изменит здоровье(от урона или хила), он автоматически вызовет твою функцию HealthChanged.*/
    AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AuraAS->GetHealthAttribute()).AddUObject(this, &UOverlayWidgetController::HealthChanged);
    AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AuraAS->GetMaxHealthAttribute()).AddUObject(this, &UOverlayWidgetController::MaxHealthChanged);
}

/** 
Эта функция — посредник. Она срабатывает «в ответ» на изменения в GAS.
1. сonst FOnAttributeChangeData& Data: Когда GAS вызывает эту функцию, он передает в неё структуру Data.
   В ней лежит информация о том, какое значение было старым, и какое стало новым.
2. OnHealthChanged.Broadcast(Data.NewValue): Твой контроллер берет новое значение из Data.NewValue и пересылает его дальше — в Блюпринт твоего виджета.
*/
void UOverlayWidgetController::HealthChanged(const FOnAttributeChangeData& Data) const
{
    OnHealthChanged.Broadcast(Data.NewValue);
}

void UOverlayWidgetController::MaxHealthChanged(const FOnAttributeChangeData& Data) const
{
    OnMaxHealthChanged.Broadcast(Data.NewValue);
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
