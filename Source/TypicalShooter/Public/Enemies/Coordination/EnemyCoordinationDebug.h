// Debug drawing of the enemy coordination (server world), toggled by TypicalShooter.Enemies.Debug.Coordination 1:
// role and token above each enemy, a line to its stand point, the sector rays around each target and its flank view cone.

#pragma once

#include "CoreMinimal.h"

class UEnemyDirectorSubsystem;

namespace EnemyCoordinationDebug
{
	TYPICALSHOOTER_API bool IsEnabled();

	/** Draws one frame of the director's groups. */
	TYPICALSHOOTER_API void Draw(const UEnemyDirectorSubsystem& Director);
}
