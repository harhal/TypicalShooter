// HUD health counter: the owning player's health, read every frame from the pawn's ASC
// (text from HealthDisplay::GetHealthText).

#pragma once

#include "CoreMinimal.h"
#include "Components/Widget.h"
#include "Fonts/SlateFontInfo.h"
#include "HealthCounterWidget.generated.h"

class STextBlock;

UCLASS()
class TYPICALSHOOTER_API UHealthCounterWidget : public UWidget
{
	GENERATED_BODY()

public:
	UHealthCounterWidget();

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
