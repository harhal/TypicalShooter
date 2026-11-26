#include "Enemies/Coordination/EnemyCoordinationDebug.h"

#include "DrawDebugHelpers.h"
#include "Enemies/Coordination/EnemyCoordinationSettings.h"
#include "Enemies/EnemyAIController.h"
#include "Enemies/EnemyCharacter.h"
#include "Enemies/EnemyDirectorSubsystem.h"
#include "GameFramework/Pawn.h"
#include "HAL/IConsoleManager.h"

namespace EnemyCoordinationDebugImpl
{
	TAutoConsoleVariable<bool> CVarDebug(TEXT("TypicalShooter.Enemies.Debug.Coordination"), false,
		TEXT("1: draw enemy coordination (roles, attack tokens, stand points, sectors, flank view cone)."));

	FColor RoleColor(const EEnemyRole Role)
	{
		switch (Role)
		{
		case EEnemyRole::Flank: return FColor::Orange;
		case EEnemyRole::Cover: return FColor::Cyan;
		default: return FColor::Red;
		}
	}

	FVector AtYaw(const FVector& Center, const float Yaw, const float Distance)
	{
		return Center + FVector(Distance, 0.f, 0.f).RotateAngleAxis(Yaw, FVector::UpVector);
	}

	void DrawViewCone(const UWorld& World, const APawn& Target)
	{
		const float ViewYaw = Target.GetViewRotation().Yaw;
		const float Half = GetDefault<UEnemyCoordinationSettings>()->Sectors.FlankViewHalfAngle;
		const FVector Center = Target.GetActorLocation();
		DrawDebugLine(&World, Center, AtYaw(Center, ViewYaw - Half, 600.f), FColor::White);
		DrawDebugLine(&World, Center, AtYaw(Center, ViewYaw + Half, 600.f), FColor::White);
	}

	void DrawMember(const UWorld& World, const UEnemyDirectorSubsystem& Director, const APawn& Target, const AEnemyCharacter& Enemy)
	{
		const FEnemyAssignment* Assignment = Director.GetAssignment(&Enemy);
		if (!Assignment)
		{
			return;
		}
		const FColor Color = RoleColor(Assignment->Role);
		const FVector Center = Target.GetActorLocation();
		const float Reach = Enemy.GetRoleBand(Assignment->Role).Max;
		DrawDebugLine(&World, Center, AtYaw(Center, Assignment->SectorYaw, Reach), Color);

		if (const AEnemyAIController* Controller = Cast<AEnemyAIController>(Enemy.GetController()))
		{
			DrawDebugLine(&World, Enemy.GetActorLocation(), Controller->GetStandPoint(), Color, false, -1.f, 0, 2.f);
			if (!Controller->GetMoveGoal().Equals(Controller->GetStandPoint(), 1.f))
			{
				// Going around the target: the current arc waypoint.
				DrawDebugLine(&World, Enemy.GetActorLocation(), Controller->GetMoveGoal(), FColor::White);
				DrawDebugSphere(&World, Controller->GetMoveGoal(), 20.f, 6, FColor::White);
			}
		}
		const bool bToken = Director.HasAttackToken(&Enemy);
		const FString Label = FString::Printf(TEXT("%s%s"), *UEnum::GetDisplayValueAsText(Assignment->Role).ToString(), bToken ? TEXT(" [TOKEN]") : TEXT(""));
		DrawDebugString(&World, Enemy.GetActorLocation() + FVector(0.f, 0.f, 120.f), Label, nullptr, bToken ? FColor::Yellow : Color, 0.f, true);
	}
}

bool EnemyCoordinationDebug::IsEnabled()
{
	return EnemyCoordinationDebugImpl::CVarDebug.GetValueOnGameThread();
}

void EnemyCoordinationDebug::Draw(const UEnemyDirectorSubsystem& Director)
{
#if ENABLE_DRAW_DEBUG
	const UWorld* World = Director.GetWorld();
	if (!World)
	{
		return;
	}
	for (const APawn* Target : Director.GetGroupTargets())
	{
		EnemyCoordinationDebugImpl::DrawViewCone(*World, *Target);
		for (const AEnemyCharacter* Enemy : Director.GetGroupMembers(Target))
		{
			EnemyCoordinationDebugImpl::DrawMember(*World, Director, *Target, *Enemy);
		}
	}
#endif
}
