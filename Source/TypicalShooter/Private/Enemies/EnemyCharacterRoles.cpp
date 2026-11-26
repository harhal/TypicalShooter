// AEnemyCharacter: what the coordinator may ask of this enemy class (allowed roles, distance bands per role).

#include "Enemies/EnemyCharacter.h"

#include "Enemies/Coordination/EnemyCoordinationSettings.h"
#include "Enemies/Coordination/EnemyRoleAssignment.h"

uint8 AEnemyCharacter::GetAllowedRoleMask() const
{
	uint8 Mask = 0;
	for (const EEnemyRole Allowed : AllowedRoles)
	{
		Mask |= EnemyRoles::Mask(Allowed);
	}
	return Mask;
}

FEnemyDistanceBand AEnemyCharacter::GetRoleBand(const EEnemyRole ForRole) const
{
	if (const FEnemyDistanceBand* Override = RoleBandOverrides.Find(ForRole))
	{
		return *Override;
	}
	return GetDefault<UEnemyCoordinationSettings>()->RoleBands.Get(ForRole);
}
