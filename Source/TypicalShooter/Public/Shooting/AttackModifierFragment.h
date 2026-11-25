// Item fragment: while the item lies in a character's inventory it changes that character's attacks (R16, R5).

#pragma once

#include "CoreMinimal.h"
#include "ItemFragment.h"
#include "Shooting/AttackTypes.h"
#include "Shooting/ShotPattern.h"
#include "AttackModifierFragment.generated.h"

class UGameplayEffect;

USTRUCT(BlueprintType)
struct TYPICALSHOOTER_API FAttackModifier_ItemFragment : public FItemFragment
{
	GENERATED_BODY()

	/** Applied to the holder while the item is in their inventory, removed with the item. Use an infinite-duration
	 *  effect modifying AttackAttributeSet.Primary_* / Alternate_* (or AmmoAttributeSet reserves). */
	UPROPERTY(EditAnywhere, Category = "Item Fragment")
	TSubclassOf<UGameplayEffect> Effect;

	/** Replace the shot pattern of PatternSlot while the item is held (the last item added wins). */
	UPROPERTY(EditAnywhere, Category = "Item Fragment")
	bool bReplacePattern = false;

	UPROPERTY(EditAnywhere, Category = "Item Fragment", meta = (EditCondition = "bReplacePattern"))
	EAttackSlot PatternSlot = EAttackSlot::Primary;

	UPROPERTY(EditAnywhere, Category = "Item Fragment", meta = (EditCondition = "bReplacePattern"))
	FShotPattern Pattern;
};
