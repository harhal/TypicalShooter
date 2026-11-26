#include "Enemies/Coordination/EnemyTokenCount.h"

int32 EnemyCoordination::TokenCount(const int32 GroupSize, const FEnemyTokenCountSettings& Settings)
{
	if (GroupSize <= 0)
	{
		return 0;
	}
	const int32 Scaled = FMath::RoundToInt(Settings.TokenFraction * GroupSize);
	const int32 Clamped = FMath::Clamp(Scaled, Settings.MinTokens, FMath::Max(Settings.MinTokens, Settings.MaxTokens));
	return FMath::Min(Clamped, GroupSize);
}
