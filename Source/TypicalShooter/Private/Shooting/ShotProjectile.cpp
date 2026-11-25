#include "Shooting/ShotProjectile.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"
#include "Shooting/ShotDamageReceiver.h"
#include "Shooting/ShotProjectileMovementComponent.h"
#include "UObject/ConstructorHelpers.h"

AShotProjectile::AShotProjectile()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PostPhysics;
	bReplicates = true;
	SetReplicatingMovement(true);

	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	Collision->InitSphereRadius(5.f);
	Collision->SetCollisionProfileName(TEXT("Projectile"));
	Collision->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Ignore); // other projectiles
	Collision->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
	Collision->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	Collision->CanCharacterStepUpOn = ECB_No;
	RootComponent = Collision;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(Collision);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh->SetRelativeScale3D(FVector(0.1f));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (SphereMesh.Succeeded())
	{
		Mesh->SetStaticMesh(SphereMesh.Object);
	}

	Movement = CreateDefaultSubobject<UShotProjectileMovementComponent>(TEXT("Movement"));
	Movement->SetUpdatedComponent(Collision);
	Movement->bRotationFollowsVelocity = true;
	Movement->bShouldBounce = false;
}

AShotProjectile* AShotProjectile::SpawnShot(UWorld* World, const TSubclassOf<AShotProjectile> Class,
	const FTransform& SpawnTransform, const FProjectileShotParams& Params, AActor* Owner, APawn* Instigator)
{
	if (!World || !Class)
	{
		return nullptr;
	}
	AShotProjectile* Projectile = World->SpawnActorDeferred<AShotProjectile>(Class, SpawnTransform, Owner, Instigator,
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (!Projectile)
	{
		return nullptr;
	}
	Projectile->Params = Params;
	Projectile->Params.DamageLossPerHit = FMath::Clamp(Params.DamageLossPerHit, 0.f, 1.f);
	if (!Projectile->Params.Damage.Instigator.IsValid())
	{
		Projectile->Params.Damage.Instigator = Instigator ? Instigator : Owner;
	}
	Projectile->Flight = FVector2D(Params.Speed, Params.GravityScale);
	Projectile->ApplyParams(); // before FinishSpawning: the movement component turns InitialSpeed into velocity there
	Projectile->FinishSpawning(SpawnTransform);
	return Projectile;
}

bool AShotProjectile::HandleShotHit(const FHitResult& Hit)
{
	if (HasAuthority())
	{
		if (IShotDamageReceiver* Receiver = IShotDamageReceiver::FindOnActor(Hit.GetActor()))
		{
			Receiver->ReceiveShotDamage(Params.Damage, Hit);
		}
	}
	Params.Damage.Scale(1.f - Params.DamageLossPerHit);
	return Params.DamageLossPerHit < 1.f && Params.Damage.HasDamage();
}

void AShotProjectile::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(AShotProjectile, Flight, COND_InitialOnly);
}

void AShotProjectile::PostNetReceiveVelocity(const FVector& NewVelocity)
{
	Movement->Velocity = NewVelocity;
}

void AShotProjectile::BeginPlay()
{
	Super::BeginPlay();
	Collision->IgnoreActorWhenMoving(GetOwner(), true);
	Collision->IgnoreActorWhenMoving(GetInstigator(), true);
	Movement->OnProjectileStop.AddDynamic(this, &AShotProjectile::OnStopped);
	LastLocation = GetActorLocation();
}

void AShotProjectile::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	const FVector Location = GetActorLocation();
	TraveledDistance += FVector::Dist(Location, LastLocation);
	LastLocation = Location;
	if (TraveledDistance >= Params.MaxTravelDistance)
	{
		Expire();
	}
}

void AShotProjectile::ApplyParams()
{
	Movement->InitialSpeed = Flight.X;
	Movement->MaxSpeed = Flight.X;
	Movement->ProjectileGravityScale = Flight.Y;
}

void AShotProjectile::Expire()
{
	if (HasAuthority())
	{
		Destroy();
	}
	else
	{
		SetActorHiddenInGame(true); // the server destroys the replicated actor
	}
}

void AShotProjectile::OnStopped(const FHitResult& Hit)
{
	Expire();
}

void AShotProjectile::OnRep_Flight()
{
	ApplyParams();
}
