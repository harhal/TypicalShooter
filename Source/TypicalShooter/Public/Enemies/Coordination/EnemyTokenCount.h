// How many attack tokens an enemy group shares: a fraction of the group size, clamped. Pure math.

#pragma once

#include "CoreMinimal.h"
#include "EnemyTokenCount.generated.h"

USTRUCT(BlueprintType)
struct TYPICALSHOOTER_API FEnemyTokenCountSettings
{
	GENERATED_BODY()

	/** Share of the group that may attack at once (rounded). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tokens", meta = (ClampMin = "0", ClampMax = "1"))
	float TokenFraction = 0.4f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tokens", meta = (ClampMin = "0"))
	int32 MinTokens = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tokens", meta = (ClampMin = "0"))
	int32 MaxTokens = 3;
};

namespace EnemyCoordination
{
	/** clamp(round(TokenFraction * GroupSize), MinTokens, MaxTokens), never more than GroupSize. */
	TYPICALSHOOTER_API int32 TokenCount(int32 GroupSize, const FEnemyTokenCountSettings& Settings);
}
