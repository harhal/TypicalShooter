// Ground jump as a GAS ability (power grid R6c): always granted, independent of the power grid. Activates only when
// the character can jump (on the ground), so the jump button is free for the air attack while airborne.
// Local predicted: the owning client jumps at once, CharacterMovement replicates the move.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "JumpGameplayAbility.generated.h"

UCLASS()
class TYPICALSHOOTER_API UJumpGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UJumpGameplayAbility();

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags,
		FGameplayTagContainer* OptionalRelevantTags) const override;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	/** Releasing the button stops the jump (variable jump height if the movement allows holding). */
	virtual void InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
};
