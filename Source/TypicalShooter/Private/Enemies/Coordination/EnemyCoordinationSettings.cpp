#include "Enemies/Coordination/EnemyCoordinationSettings.h"

int32 UEnemyCoordinationSettings::GetTokenCount(const int32 GroupSize) const
{
	return EnemyCoordination::TokenCount(GroupSize, Tokens);
}
