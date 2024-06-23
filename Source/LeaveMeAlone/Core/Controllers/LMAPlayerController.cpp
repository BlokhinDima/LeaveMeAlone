// LeaveMeAlone Games. All rights reserved.


#include "Core/Controllers/LMAPlayerController.h"

ALMAPlayerController::ALMAPlayerController()
{
}


void ALMAPlayerController::BeginPlay()
{
	Super::BeginPlay();

	SetInputMode(FInputModeGameOnly());
	bShowMouseCursor = false;
}