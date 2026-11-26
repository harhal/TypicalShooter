#include "Enemies/Coordination/EnemyRoleStand.h"

const FEnemyDistanceBand& FEnemyRoleBands::Get(const EEnemyRole Role) const
{
	switch (Role)
	{
	case EEnemyRole::Flank: return Flank;
	case EEnemyRole::Cover: return Cover;
	default: return Pressure;
	}
}

void EnemyRoleStand::Apply(FEnemyStandQuery& Query, const FInput& Input)
{
	const float Min = Input.Band.Min;
	const float Max = FMath::Max(Input.Band.Min, Input.Band.Max);
	const bool bWaiting = !Input.bHasToken;

	Query.MinDistance = Min;
	Query.MaxDistance = Input.Role == EEnemyRole::Pressure && bWaiting ? 0.5f * (Min + Max) : Max;
	Query.SectorYaw = Input.SectorYaw;
	Query.SectorHalfWidth = Input.SectorHalfWidth;
	Query.MinSeparation = Input.MinSeparation;
	Query.bSectorRequiredToStay = !(Input.Role == EEnemyRole::Cover && bWaiting);
}
