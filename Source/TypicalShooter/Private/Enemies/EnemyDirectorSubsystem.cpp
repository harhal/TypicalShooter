#include "Enemies/EnemyDirectorSubsystem.h"

#include "Enemies/EnemyCharacter.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"

void UEnemyDirectorSubsystem::Register(AEnemyCharacter* Enemy)
{
	if (Enemy)
	{
		Enemies.AddUnique(Enemy);
	}
}

void UEnemyDirectorSubsystem::Unregister(AEnemyCharacter* Enemy)
{
	Enemies.Remove(Enemy);
}

TArray<AEnemyCharacter*> UEnemyDirectorSubsystem::GetEnemies() const
{
	TArray<AEnemyCharacter*> Live;
	for (AEnemyCharacter* Enemy : Enemies)
	{
		if (IsValid(Enemy))
		{
			Live.Add(Enemy);
		}
	}
	return Live;
}

APawn* UEnemyDirectorSubsystem::FindTargetFor(const AActor* Enemy) const
{
	if (!Enemy)
	{
		return nullptr;
	}
	APawn* Nearest = nullptr;
	double NearestDistSq = TNumericLimits<double>::Max();
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APawn* Pawn = It->IsValid() ? (*It)->GetPawn() : nullptr;
		const double DistSq = Pawn ? FVector::DistSquared(Pawn->GetActorLocation(), Enemy->GetActorLocation()) : 0.0;
		if (IsValid(Pawn) && DistSq < NearestDistSq)
		{
			Nearest = Pawn;
			NearestDistSq = DistSq;
		}
	}
	return Nearest;
}
