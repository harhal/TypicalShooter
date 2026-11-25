// HUD ammo counter: the owning player's reserve per ammo kind, read every frame from the pawn's ASC
// (text from AmmoDisplay::GetReserveText).

#pragma once

#include "CoreMinimal.h"
#include "Components/Widget.h"
#include "Fonts/SlateFontInfo.h"
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

	virtual void ReleaseSlateResources(bool bReleaseChildren) override;

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	FText GetText() const;

	TSharedPtr<STextBlock> Text;
};
