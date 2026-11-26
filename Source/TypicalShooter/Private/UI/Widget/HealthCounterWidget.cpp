#include "UI/Widget/HealthCounterWidget.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GameFramework/PlayerController.h"
#include "Shooting/HealthDisplay.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Text/STextBlock.h"

UHealthCounterWidget::UHealthCounterWidget()
{
	Font = FCoreStyle::GetDefaultFontStyle("Bold", 22);
}

void UHealthCounterWidget::ReleaseSlateResources(const bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);
	Text.Reset();
}

TSharedRef<SWidget> UHealthCounterWidget::RebuildWidget()
{
	Text = SNew(STextBlock)
		.Font(Font)
		.ColorAndOpacity(ColorAndOpacity)
		.ShadowOffset(FVector2D(1.f, 1.f))
		.Text(TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateUObject(this, &UHealthCounterWidget::GetText)));
	return Text.ToSharedRef();
}

FText UHealthCounterWidget::GetText() const
{
	const APlayerController* Player = GetOwningPlayer();
	const UAbilitySystemComponent* Asc = Player
		? UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Player->GetPawn()) : nullptr;
	return Asc ? HealthDisplay::GetHealthText(*Asc) : FText::GetEmpty();
}
