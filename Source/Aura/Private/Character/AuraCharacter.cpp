// Copyright Shavi


#include "Character/AuraCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Player/AuraPlayerState.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "Player/AuraPlayerController.h"
#include "UI/HUD/AuraHUD.h"

AAuraCharacter::AAuraCharacter()
{
	// 1. Отключаем привязку поворота персонажа к повороту контроллера(камеры).
	// Если это true, персонаж всегда будет смотреть туда же, куда и камера. 
	// В Top-Down нам это не нужно: камера смотрит вниз, а персонаж — куда бежит.
	bUseControllerRotationPitch = false; // Не наклонять персонажа вверх/вниз вместе с камерой
	bUseControllerRotationYaw = false; // Не вращать персонажа влево/вправо вместе с камерой
	bUseControllerRotationRoll = false; // Не заваливать персонажа на бок
	
	// 2. Настраиваем автоматический поворот персонажа в сторону движения.
	// Когда мы нажимаем "Влево", персонаж сам плавно развернется лицом на запад.
	GetCharacterMovement()->bOrientRotationToMovement = true;

	// 3. Устанавливаем скорость этого разворота.
	// 400 градусов в секунду по оси Yaw (вокруг себя) — это достаточно быстро, чтобы персонаж не казался неповоротливым.
	GetCharacterMovement()->RotationRate = FRotator(0.f, 400.f, 0.f);

	// 4. Ограничиваем движение персонажа плоскостью.
	// Это гарантирует, что персонаж будет скользить только по поверхности пола.
	GetCharacterMovement()->bConstrainToPlane = true;

	// 5. Принудительно "примагничиваем" персонажа к плоскости в самом начале игры.
	// Помогает избежать ситуации, когда персонаж завис в паре миллиметров над землей при спавне.
	GetCharacterMovement()->bSnapToPlaneAtStart = true;
	
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());

	ViewCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ViewCamera"));
	ViewCamera->SetupAttachment(CameraBoom);
}

void AAuraCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	//Init ability actor info for the Server
	InitAbilityActorInfo();
	
}

void AAuraCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	//Init ability actor info for the Client
	InitAbilityActorInfo();
}

void AAuraCharacter::InitAbilityActorInfo()
{
	AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
	check(AuraPlayerState);
	// 1. Сообщаем ASC, кто его Владелец (PlayerState) и кто его Аватар (этот Character)
	AuraPlayerState->GetAbilitySystemComponent()->InitAbilityActorInfo(AuraPlayerState, this);
	// 2. Вызываем метод привязывающий наши callback функции к необходимым делегатам, после того, как параметры инициализированы
	Cast<UAuraAbilitySystemComponent>(AuraPlayerState->GetAbilitySystemComponent())->AbilityActorInfoSet();
	// 2. Кэшируем указатели локально для удобства(чтобы не тянуться в PlayerState каждый раз)
	AbilitySystemComponent = AuraPlayerState->GetAbilitySystemComponent();
	AttributeSet = AuraPlayerState->GetAttribureSet();

	// 3. Инициализируем UI. 
	// На сервере GetController() вернет контроллер. 
	// На клиенте это сработает только для ТОГО игрока, который управляет этим персонажем (Locally Controlled).
	if (AAuraPlayerController* PlayerController = Cast<AAuraPlayerController>(GetController()))
	{
		if (AAuraHUD* AuraHUD = Cast<AAuraHUD>(PlayerController->GetHUD()))
		{
			AuraHUD->InitOverlay(PlayerController, AuraPlayerState, AbilitySystemComponent, AttributeSet);
		}
	}
}
