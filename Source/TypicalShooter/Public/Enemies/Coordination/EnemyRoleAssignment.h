// Distributes roles over the enemies of one group: quotas from the weights, only roles an enemy allows, and as few
// changes as possible (an enemy keeps its role while that does not break the quotas). Pure logic.

#pragma once

#include "CoreMinimal.h"
#include "Containers/StaticArray.h"
#include "Enemies/Coordination/EnemyRole.h"

namespace EnemyRoles
{
	constexpr int32 RoleCount = 3;
	constexpr uint8 AllRoles = (1 << RoleCount) - 1;

	constexpr uint8 Mask(const EEnemyRole Role) { return uint8(1 << static_cast<uint8>(Role)); }

	/** Wanted count per role, indexed by EEnemyRole. */
	using FQuotas = TStaticArray<int32, RoleCount>;

	struct FCandidate
	{
		/** Roles this enemy may take (EnemyRoles::Mask bits); 0 = any. */
		uint8 Allowed = AllRoles;
		/** Role from the previous assignment, if any. */
		TOptional<EEnemyRole> Current;
	};

	/** Largest-remainder split of GroupSize by the weights (ties go to the earlier role); all-zero weights = all Pressure. */
	TYPICALSHOOTER_API FQuotas ComputeQuotas(int32 GroupSize, const FEnemyRoleWeights& Weights);

	/**
	 * One role per candidate (same order). Keeps allowed current roles within the quotas, fills the rest starting with the
	 * most constrained enemies, and moves a flexible enemy to a free role when that lets a constrained one stay within the
	 * quotas. A role is over quota only when no allowed role has room.
	 */
	TYPICALSHOOTER_API TArray<EEnemyRole> Assign(const TArray<FCandidate>& Candidates, const FEnemyRoleWeights& Weights);
}
