// HUD ammo counter: one line per attack slot, "<label>: clip / reserve", read every frame from the owning
// player's pawn ASC (text from AmmoDisplay::GetSlotText).

#pragma once

#include "CoreMinimal.h"
#include "Components/Widget.h"
#include "Fonts/SlateFontInfo.h"
#include "Shooting/AttackTypes.h"
#include "AmmoCounterWidget.generated.h"

class STextBlock;

UCLASS()
class TYPICALSHOOTER_API UAmmoCounterWidget : public UWidget
{
	GENERATED_BODY()

public:
	UAmmoCounterWidget();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance")
	FSlateFontInfo Font;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance")
	FSlateColor ColorAndOpacity = FSlateColor(FLinearColor::White);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance")
	FText PrimaryLabel;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance")
	FText AlternateLabel;

	virtual void ReleaseSlateResources(bool bReleaseChildren) override;

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	TSharedRef<STextBlock> MakeLine(EAttackSlot AttackSlot) const;
	FText GetLine(EAttackSlot AttackSlot) const;

	TSharedPtr<SWidget> Root;
};
