#include "Enemies/Coordination/EnemyRole.h"

float FEnemyRoleWeights::Get(const EEnemyRole Role) const
{
	switch (Role)
	{
	case EEnemyRole::Pressure: return Pressure;
	case EEnemyRole::Flank: return Flank;
	case EEnemyRole::Cover: return Cover;
	}
	return 0.f;
}
