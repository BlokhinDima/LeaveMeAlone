// LeaveMeAlone Games. All rights reserved.


#include "Core/Characters/DefaultCharacter.h"
#include "Core/Components/LMAWeaponComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/DecalComponent.h"
#include "Components/InputComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
ADefaultCharacter::ADefaultCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Stamina = MaxStamina;

	SpringArm = CreateDefaultSubobject<USpringArmComponent>("SpringArm");
	SpringArm->SetupAttachment(GetRootComponent());
	SpringArm->SetUsingAbsoluteRotation(true);
	SpringArm->TargetArmLength = DefaultArmLength;
	SpringArm->SetRelativeRotation(FRotator(YRotation, 0.0f, 0.0f));
	SpringArm->bDoCollisionTest = false;
	SpringArm->bEnableCameraLag = true;

	CurrentZoom = DefaultArmLength;

	Camera = CreateDefaultSubobject<UCameraComponent>("Camera");
	Camera->SetupAttachment(SpringArm);
	Camera->SetFieldOfView(FOV);
	Camera->bUsePawnControlRotation = false;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	HealthComponent = CreateDefaultSubobject<ULMAHealthComponent>("HealthComponent");

	WeaponComponent = CreateDefaultSubobject<ULMAWeaponComponent>("Weapon");
}

// Called when the game starts or when spawned
void ADefaultCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (GetWorld())
	{
		if (IsValid(CursorMaterial))
		{
			Cursor = UGameplayStatics::SpawnDecalAtLocation(GetWorld(), CursorMaterial, CursorSize, FVector(0));
		}
	}

	HealthComponent->OnDeath.AddUObject(this, &ADefaultCharacter::OnDeath);
	OnHealthChanged(HealthComponent->GetHealth());
	HealthComponent->OnHealthChanged.AddUObject(this, &ADefaultCharacter::OnHealthChanged);

	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, FString::Printf(TEXT("Stamina = %f"), Stamina));
}

// Called every frame
void ADefaultCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!(HealthComponent->IsDead()))
	{
		RotationPlayerOnCursor();
	}

	if (!InSprintState && Stamina < MaxStamina)
	{
		Stamina = FMath::Clamp(Stamina + StaminaRestoreCoeff, 0.0f, MaxStamina);

		if (Stamina == MaxStamina)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, FString::Printf(TEXT("Stamina = %f"), Stamina));
		}
	}
}

// Called to bind functionality to input
void ADefaultCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAxis("MoveForward", this, &ADefaultCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ADefaultCharacter::MoveRight);
	PlayerInputComponent->BindAction("ZoomIn", IE_Pressed, this, &ADefaultCharacter::ZoomIn);
	PlayerInputComponent->BindAction("ZoomOut", IE_Pressed, this, &ADefaultCharacter::ZoomOut);
	PlayerInputComponent->BindAction("Sprint", IE_Repeat, this, &ADefaultCharacter::Sprint);
	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &ADefaultCharacter::TurnOnSprintState);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &ADefaultCharacter::TurnOffSprintState);
	PlayerInputComponent->BindAction("Fire", IE_Pressed, WeaponComponent, &ULMAWeaponComponent::StartAutoFire);
	PlayerInputComponent->BindAction("Fire", IE_Released, WeaponComponent, &ULMAWeaponComponent::EndAutoFire);
	PlayerInputComponent->BindAction("Reload", IE_Pressed, WeaponComponent, &ULMAWeaponComponent::Reload);
}

void ADefaultCharacter::MoveForward(float Value)
{
	AddMovementInput(GetActorForwardVector(), Value);
}

void ADefaultCharacter::MoveRight(float Value)
{
	AddMovementInput(GetActorRightVector(), Value);
}

void ADefaultCharacter::Sprint()
{
	Stamina = FMath::Clamp(Stamina - StaminaReductionCoeff, 0.0f, MaxStamina);

	if (Stamina  == 0.0f)
	{
		TurnOffSprintState();
	}
	
	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, FString::Printf(TEXT("Stamina = %f"), Stamina));
}

void ADefaultCharacter::ZoomIn()
{
	if (CurrentZoom - ZoomStep >= MaxZoom)
	{
		CurrentZoom -= ZoomStep;
		SpringArm->TargetArmLength = CurrentZoom;
	}
}

void ADefaultCharacter::ZoomOut()
{
	if (CurrentZoom + ZoomStep <= MinZoom)
	{
		CurrentZoom += ZoomStep;
		SpringArm->TargetArmLength = CurrentZoom;
	}
}

void ADefaultCharacter::OnDeath()
{
	Cursor->DestroyRenderState_Concurrent();

	PlayAnimMontage(DeathMontage);
	GetCharacterMovement()->DisableMovement();
	SetLifeSpan(5.0f);

	if (Controller)
	{
		Controller->ChangeState(NAME_Spectating);
	}

	OnCharacterDeath.Broadcast();
}

void ADefaultCharacter::RotationPlayerOnCursor()
{
	if (GetWorld())
	{
		APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
		if (PlayerController)
		{
			FHitResult HitResult;
			PlayerController->GetHitResultUnderCursor(ECC_GameTraceChannel1, true, HitResult);
			float FindRotatorResultYaw = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), HitResult.Location).Yaw;
			SetActorRotation(FQuat(FRotator(0.0f, FindRotatorResultYaw, 0.0f)));
			if (Cursor)
			{
				Cursor->SetWorldLocation(HitResult.Location);
			}
		}
	}
}

void ADefaultCharacter::OnHealthChanged(float NewHealth)
{
}

void ADefaultCharacter::TurnOnSprintState()
{ 
	GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
	InSprintState = true; 
}

void ADefaultCharacter::TurnOffSprintState()
{ 
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	InSprintState = false; 
}