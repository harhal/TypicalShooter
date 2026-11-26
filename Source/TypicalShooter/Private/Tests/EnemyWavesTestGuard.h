// PIE tests that are not about enemies run Lvl_FirstPerson without enemy waves: enqueue DisableEnemyWaves() before
// FStartPIECommand and RestoreEnemyWaves() after FEndPlayMapCommand.

#pragma once

#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR

#include "Enemies/EnemyWaveSpawner.h"
#include "Tests/AutomationCommon.h"

namespace EnemyWavesTestGuard
{
	inline void EnqueueSetWaves(const bool bEnabled)
	{
		ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([bEnabled]()
		{
			AEnemyWaveSpawner::SetWavesEnabled(bEnabled);
			return true;
		}));
	}

	inline void DisableEnemyWaves() { EnqueueSetWaves(false); }
	inline void RestoreEnemyWaves() { EnqueueSetWaves(true); }
}

#endif
