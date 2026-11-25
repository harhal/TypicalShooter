#include "UI/Widget/AmmoCounterWidget.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GameFramework/PlayerController.h"
#include "Shooting/AmmoDisplay.h"
#include "Shooting/AttackAttributeSet.h"
#include "Styling/CoreStyle.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "AmmoCounterWidget"

UAmmoCounterWidget::UAmmoCounterWidget()
{
	Font = FCoreStyle::GetDefaultFontStyle("Bold", 22);
	PrimaryLabel = LOCTEXT("Primary", "Primary");
	AlternateLabel = LOCTEXT("Alternate", "Alternate");
}

void UAmmoCounterWidget::ReleaseSlateResources(const bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);
	Root.Reset();
}

TSharedRef<SWidget> UAmmoCounterWidget::RebuildWidget()
{
	Root = SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()[MakeLine(EAttackSlot::Primary)]
		+ SVerticalBox::Slot().AutoHeight()[MakeLine(EAttackSlot::Alternate)];
	return Root.ToSharedRef();
}

TSharedRef<STextBlock> UAmmoCounterWidget::MakeLine(const EAttackSlot AttackSlot) const
{
	return SNew(STextBlock)
		.Font(Font)
		.ColorAndOpacity(ColorAndOpacity)
		.ShadowOffset(FVector2D(1.f, 1.f))
		.Text(TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateUObject(this, &UAmmoCounterWidget::GetLine, AttackSlot)));
}

FText UAmmoCounterWidget::GetLine(const EAttackSlot AttackSlot) const
{
	const APlayerController* Player = GetOwningPlayer();
	const UAbilitySystemComponent* Asc = Player
		? UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Player->GetPawn()) : nullptr;
	if (!Asc || !Asc->GetSet<UAttackAttributeSet>())
	{
		return FText::GetEmpty();
	}
	const FText& Label = AttackSlot == EAttackSlot::Primary ? PrimaryLabel : AlternateLabel;
	return FText::Format(LOCTEXT("Line", "{0}: {1}"), Label, AmmoDisplay::GetSlotText(*Asc, AttackSlot));
}

#undef LOCTEXT_NAMESPACE
