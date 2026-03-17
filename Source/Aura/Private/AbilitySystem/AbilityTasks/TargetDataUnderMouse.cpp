// Copyright Shavi


#include "AbilitySystem/AbilityTasks/TargetDataUnderMouse.h"
#include "AbilitySystemComponent.h"

UTargetDataUnderMouse* UTargetDataUnderMouse::CreateTargetDataUnderMouse(UGameplayAbility* OwningAbility)
{
	// Создаем новый экземпляр задачи внутри указанной способности
	UTargetDataUnderMouse* MyObj = NewAbilityTask<UTargetDataUnderMouse>(OwningAbility);
	return MyObj; // Возвращаем объект, чтобы его можно было использовать в Блюпринте или коде
}

void UTargetDataUnderMouse::Activate()
{
	// Проверяем, управляет ли этим персонажем живой игрок на данном компьютере
	const bool bIsLocallyControlled = Ability->GetCurrentActorInfo()->IsLocallyControlled();

	if (bIsLocallyControlled)
	{
		// Если мы игрок — считываем положение мыши и отправляем серверу
		SendMouseCursorData();
	}
	else
	{
		// Мы на сервере. Нам нужно подготовиться к приему данных от клиента.
		const FGameplayAbilitySpecHandle SpecHandle = GetAbilitySpecHandle();
		const FPredictionKey ActivationPredictionKey = GetActivationPredictionKey();

		// Подписываемся на делегат: "Когда придут данные для этой способности, вызови функцию OnTargetDataReplicatedCallback"
		AbilitySystemComponent.Get()->AbilityTargetDataSetDelegate(SpecHandle, ActivationPredictionKey).AddUObject(this, &UTargetDataUnderMouse::OnTargetDataReplicatedCallback);

		// Проверяем, вдруг данные уже прилетели по сети до того, как мы успели подписаться
		bool bCalledDelegate = AbilitySystemComponent.Get()->CallReplicatedTargetDataDelegatesIfSet(SpecHandle, ActivationPredictionKey);

		if (!bCalledDelegate)
		{
			// Если данных еще нет, говорим системе: "Мы ждем данные от удаленного игрока"
			SetWaitingOnRemotePlayerData();
		}
	}
}

void UTargetDataUnderMouse::SendMouseCursorData()
{
	// Создаем "окно предсказания" (Prediction). Это говорит GAS: 
	// "Я делаю это на клиенте, сервер скоро подтвердит, не блокируй выполнение"
	FScopedPredictionWindow ScopedPrediction(AbilitySystemComponent.Get());

	// Получаем контроллер игрока из информации о способности
	APlayerController* PC = Ability->GetCurrentActorInfo()->PlayerController.Get();
	FHitResult CursorHit;
	// Делаем стандартный трейс (луч) под курсором мыши
	PC->GetHitResultUnderCursor(ECC_Visibility, false, CursorHit);

	// Создаем контейнер для данных (Handle)
	FGameplayAbilityTargetDataHandle DataHandle;

	// Создаем конкретный тип данных — "Попадание по одной цели"
	// Используем 'new', так как GAS сама управляет памятью этих структур
	FGameplayAbilityTargetData_SingleTargetHit* Data = new FGameplayAbilityTargetData_SingleTargetHit();
	// Записываем наш результат трейса (весь HitResult) в эту структуру
	Data->HitResult = CursorHit;
	// Добавляем структуру в общий контейнер
	DataHandle.Add(Data);

	// КЛЮЧЕВАЯ ФУНКЦИЯ: Отправляем упакованные данные на сервер
	AbilitySystemComponent->ServerSetReplicatedTargetData(
		GetAbilitySpecHandle(),
		GetActivationPredictionKey(),
		DataHandle,
		FGameplayTag(),
		AbilitySystemComponent->ScopedPredictionKey);

	// Если задача еще активна (не отменена), вызываем делегат "ValidData" для Блюпринтов
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		ValidData.Broadcast(DataHandle);
	}
}

void UTargetDataUnderMouse::OnTargetDataReplicatedCallback(const FGameplayAbilityTargetDataHandle& DataHandle, FGameplayTag ActivationTag)
{
	// Говорим серверу: "Данные получены, мы их обработали, можно удалить их из сетевого буфера"
	AbilitySystemComponent->ConsumeClientReplicatedTargetData(GetAbilitySpecHandle(), GetActivationPredictionKey());

	// Вызываем делегат на сервере. Теперь серверная копия способности тоже знает, куда нажал игрок
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		ValidData.Broadcast(DataHandle);
	}
}