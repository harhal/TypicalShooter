#include "Shooting/TargetDummy.h"

#include "TS_GAS.h"
#include "TypicalShooter.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/StaticMesh.h"
#include "Shooting/GasDamageReceiverComponent.h"
#include "Shooting/HealthAttributeSet.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

ATargetDummy::ATargetDummy()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	Collision = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Collision"));
	Collision->InitCapsuleSize(40.f, 90.f);
	Collision->SetCollisionProfileName(UCollisionProfile::BlockAllDynamic_ProfileName);
	RootComponent = Collision;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(Collision);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh->SetRelativeScale3D(FVector(0.8f, 0.8f, 1.8f));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	if (CylinderMesh.Succeeded())
	{
		Mesh->SetStaticMesh(CylinderMesh.Object);
	}

	HealthText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("HealthText"));
	HealthText->SetupAttachment(Collision);
	HealthText->SetRelativeLocation(FVector(0.f, 0.f, 110.f));
	HealthText->SetHorizontalAlignment(EHTA_Center);
	HealthText->SetWorldSize(24.f);

	AbilitySystem = CreateDefaultSubobject<UTS_GAS>(TEXT("AbilitySystem"));
	AbilitySystem->SetIsReplicated(true);
	AbilitySystem->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	HealthSet = CreateDefaultSubobject<UHealthAttributeSet>(TEXT("HealthSet"));
	DamageReceiver = CreateDefaultSubobject<UGasDamageReceiverComponent>(TEXT("DamageReceiver"));
}

UAbilitySystemComponent* ATargetDummy::GetAbilitySystemComponent() const
{
	return AbilitySystem;
}

float ATargetDummy::GetHealth() const
{
	return AbilitySystem->GetNumericAttribute(UHealthAttributeSet::GetHealthAttribute());
}

void ATargetDummy::BeginPlay()
{
	Super::BeginPlay();

	AbilitySystem->InitAbilityActorInfo(this, this);
	AbilitySystem->GetGameplayAttributeValueChangeDelegate(UHealthAttributeSet::GetHealthAttribute())
		.AddUObject(this, &ATargetDummy::OnHealthChanged);
	AbilitySystem->GetGameplayAttributeValueChangeDelegate(UHealthAttributeSet::GetMaxHealthAttribute())
		.AddWeakLambda(this, [this](const FOnAttributeChangeData&) { UpdateHealthText(); });

	if (HasAuthority())
	{
		AbilitySystem->SetNumericAttributeBase(UHealthAttributeSet::GetMaxHealthAttribute(), MaxHealth);
		AbilitySystem->SetNumericAttributeBase(UHealthAttributeSet::GetHealthAttribute(), MaxHealth);
	}
	UpdateHealthText();
}

void ATargetDummy::OnHealthChanged(const FOnAttributeChangeData& Data)
{
	UpdateHealthText();
	const float Lost = Data.OldValue - Data.NewValue;
	if (!HasAuthority() || Lost <= 0.f)
	{
		return;
	}

	TotalDamageTaken += Lost;
	UE_LOG(LogTypicalShooter, Log, TEXT("Target dummy %s took %.1f damage, health %.1f"), *GetName(), Lost, Data.NewValue);
	if (Data.NewValue <= 0.f)
	{
		++Defeats;
		UE_LOG(LogTypicalShooter, Log, TEXT("Target dummy %s depleted (%d times), refilling"), *GetName(), Defeats);
		// Refill outside of the effect execution that changed health.
		GetWorldTimerManager().SetTimerForNextTick(this, &ATargetDummy::Refill);
	}
}

void ATargetDummy::Refill()
{
	AbilitySystem->SetNumericAttributeBase(UHealthAttributeSet::GetHealthAttribute(), MaxHealth);
}

void ATargetDummy::UpdateHealthText() const
{
	const float Max = AbilitySystem->GetNumericAttribute(UHealthAttributeSet::GetMaxHealthAttribute());
	HealthText->SetText(FText::FromString(FString::Printf(TEXT("%.0f / %.0f"), FMath::CeilToFloat(GetHealth()), Max)));
}
