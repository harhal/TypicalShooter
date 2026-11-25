#include "UI/Widget/AmmoCounterWidget.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GameFramework/PlayerController.h"
#include "Shooting/AmmoDisplay.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Text/STextBlock.h"

UAmmoCounterWidget::UAmmoCounterWidget()
{
	Font = FCoreStyle::GetDefaultFontStyle("Bold", 22);
}

void UAmmoCounterWidget::ReleaseSlateResources(const bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);
	Text.Reset();
}

TSharedRef<SWidget> UAmmoCounterWidget::RebuildWidget()
{
	Text = SNew(STextBlock)
		.Font(Font)
		.ColorAndOpacity(ColorAndOpacity)
		.ShadowOffset(FVector2D(1.f, 1.f))
		.Text(TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateUObject(this, &UAmmoCounterWidget::GetText)));
	return Text.ToSharedRef();
}

FText UAmmoCounterWidget::GetText() const
{
	const APlayerController* Player = GetOwningPlayer();
	const UAbilitySystemComponent* Asc = Player
		? UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Player->GetPawn()) : nullptr;
	return Asc ? AmmoDisplay::GetReserveText(*Asc) : FText::GetEmpty();
}
