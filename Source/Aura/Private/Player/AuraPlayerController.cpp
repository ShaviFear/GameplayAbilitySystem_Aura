// Copyright Shavi


#include "Player/AuraPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Interaction/EnemyInterface.h"

AAuraPlayerController::AAuraPlayerController()
{
	bReplicates = true;
}

void AAuraPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	CursorTrace();
}

void AAuraPlayerController::CursorTrace()
{
	FHitResult CursorHit;
	GetHitResultUnderCursor(ECC_Visibility, false, CursorHit);
	if (!CursorHit.bBlockingHit) return;

	LastActor = ThisActor;
	ThisActor = Cast<IEnemyInterface>(CursorHit.GetActor());

	/**
	* Line trace from a cursor. There are several scenarios:
	* A. LastActor in null && ThisActor is null
	*	- Do nothing
	* B. LastActor in null && ThisActor is valid
	*	- Highlight ThisActor
	* C. LastActor is valid && ThisActor is null
	*	- UnHighlight LastActor
	* D. Both actor are valid, but LastActor != ThisActor
	*	- UnHighlight LastActor and Highlight ThisActor
	* E. Both actor are valid, but LastActor == ThisActor
	*	- Do nothing
	*
	*/

	if (LastActor == nullptr)
	{
		if (ThisActor != nullptr)
		{
			//Case B
			ThisActor->HighlightActor();
		}
		else
		{
			//Case A - both are NULL, do nothing
		}
	}
	else //LastActor is valid
	{
		if (ThisActor == nullptr)
		{
			//Case C
			LastActor->UnhighlightActor();
		}
		else //Both actors are valid 
		{
			if (LastActor != ThisActor) //but LastActor != ThisActor
			{
				//Case D
				LastActor->UnhighlightActor();
				ThisActor->HighlightActor();
			}
			else //but LastActor == ThisActor
			{
				//Case E - do nothing
			}
		}
	}
}


void AAuraPlayerController::BeginPlay()
{
	Super::BeginPlay();
	check(AuraContext);

	// Получаем доступ к подсистеме ввода локального игрока (синглтон-сервис для работы с Enhanced Input)
	UEnhancedInputLocalPlayerSubsystem* SubSystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if (SubSystem)
	{
		SubSystem->AddMappingContext(AuraContext, 0);
	}

	// Включаем отображение курсора мыши на экране
	bShowMouseCursor = true;
	// Устанавливаем внешний вид курсора (стандартная стрелочка)
	DefaultMouseCursor = EMouseCursor::Default;

	// Настройка режима ввода: позволяем взаимодействовать и с миром (игра), и с виджетами (UI)
	FInputModeGameAndUI InputModeData;
	// Мышь не "запирается" внутри окна игры (удобно для оконного режима)
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	// Не скрывать курсор, когда игрок зажимает кнопку мыши (например, для вращения камеры)
	InputModeData.SetHideCursorDuringCapture(false);
	// Применяем созданные настройки режима ввода к контроллеру
	SetInputMode(InputModeData);
}

// Эта функция вызывается один раз, чтобы "подключить" провода от кнопок к функциям.
void AAuraPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// Кастим стандартный компонент ввода к Enhanced версии. CastChecked выдаст ошибку, если это не так
	UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent);

	// Привязываем конкретный InputAction (MoveAction) к функции Move
	EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AAuraPlayerController::Move);
}

// Здесь происходит математика: мы определяем, где у персонажа "вперед", учитывая поворот камеры.
void AAuraPlayerController::Move(const FInputActionValue& InputActionValue)
{
	// Получаем вектор направления из ввода (X = влево/вправо, Y = вперед/назад)
	const FVector2D InputAxisVector = InputActionValue.Get<FVector2D>();
	// Узнаем, куда сейчас смотрит контроллер (камера)
	const FRotator Rotation = GetControlRotation();
	// Нам нужен только поворот по оси Z (Yaw), чтобы персонаж не пытался "идти" вверх в небо
	const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

	// Вычисляем вектор "Вперед" относительно нашего поворота (ось X в локальных координатах)
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	// Вычисляем вектор "Вправо" относительно нашего поворота (ось Y в локальных координатах)
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	// Пытаемся получить саму пешку (персонажа), которой управляем
	if (APawn* ControlledPawn = GetPawn<APawn>())
	{
		// Добавляем движение вперед/назад (InputAxisVector.Y) по направлению ForwardDirection
		ControlledPawn->AddMovementInput(ForwardDirection, InputAxisVector.Y);
		// Добавляем движение влево/вправо (InputAxisVector.X) по направлению RightDirection
		ControlledPawn->AddMovementInput(RightDirection, InputAxisVector.X);
	}

	/*Подведем итог:В мире игры(3D) : Перед — это X.Право — это Y.
	В векторе ввода(2D) : Вперед / Назад — это координата Y.Лево / Право — это координата X.
	Поэтому в коде мы «скрещиваем» их :
	Направление X(Мир) + Сила Y(Ввод) = Движение вперед.
	Направление Y(Мир) + Сила X(Ввод) = Движение вбок(Strafing).
	Лайфхак для редактора : Посмотри в левый нижний угол вьюпорта Unreal Engine.
	Ты увидишь там стрелочки(Gizmo).Красная стрелка(X) всегда смотрит туда, куда «смотрит» пустой уровень по умолчанию.*/
}

