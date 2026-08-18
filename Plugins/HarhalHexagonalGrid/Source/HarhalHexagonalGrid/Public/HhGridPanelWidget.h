// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HhAxialCoord.h"
#include "HhGridPanel.h"
#include "Components/PanelSlot.h"
#include "Components/PanelWidget.h"
#include "UObject/Object.h"
#include "HhGridPanelWidget.generated.h"

class SHhGridPanel;

UCLASS()
class UHhGridPanelSlotWidget : public UPanelSlot
{
	GENERATED_BODY()
 
public:
	UPROPERTY(EditAnywhere, Category = "HHex")
	FHhAxialCoord Coord;
 
private:	
	SHhGridPanel::FSlot* Slot;
	
public:
	
	FHhAxialCoord GetCoord() const;

	UFUNCTION(BlueprintCallable, Category="HHex")
	void SetCoord(const FHhAxialCoord& InCoord);
	
	virtual void SynchronizeProperties() override;
 
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
 
	void BuildSlot(TSharedRef<SHhGridPanel> GridPanel);
	
#if WITH_EDITOR
	virtual void SynchronizeFromTemplate(const UPanelSlot* const TemplateSlot) override;
#endif //WITH_EDITOR
};
 
UCLASS()
class UHhGridPanelWidget : public UPanelWidget
{
	GENERATED_BODY()
 
public:
	UPROPERTY(EditAnywhere, Category = "HHex")
	float HexSize = 50.f;
	
	UPROPERTY(EditAnywhere, Category = "HHex")
	int32 HorizontalSize = 3.f;
	
	UPROPERTY(EditAnywhere, Category = "HHex")
	int32 VerticalSize = 3.f;
	
public: 
	UFUNCTION(BlueprintCallable, Category = "HHex")
	UHhGridPanelSlotWidget* AddChildToGrid(UWidget* Content, const FHhAxialCoord& Coord);
	
	virtual UClass* GetSlotClass() const override { return UHhGridPanelSlotWidget::StaticClass(); }
	
    virtual void ReleaseSlateResources(bool bReleaseChildren) override;
 
protected:
	virtual void OnSlotAdded(UPanelSlot* InSlot) override;
	virtual void OnSlotRemoved(UPanelSlot* InSlot) override;
 
protected:
	TSharedPtr<SHhGridPanel> SlateGridPanel;
	
	TMap<FHhAxialCoord, TWeakObjectPtr<UHhGridPanelSlotWidget>> ChildSlots;
	
protected:
	// UWidget interface
	virtual TSharedRef<SWidget> RebuildWidget() override;
	// End of UWidget interface
};
