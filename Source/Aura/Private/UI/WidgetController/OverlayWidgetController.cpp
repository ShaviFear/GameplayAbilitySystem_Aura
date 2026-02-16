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
