#include "UI/InventoryItem/InventoryItem_Presenter.h"

#include "InventoryItem.h"
#include "PowerGrid/PowerGridComponent.h"
#include "UI/InventoryItem/InventoryItem_View.h"
#include "UI/PowerGrid/PowerGridDisplay.h"

void UInventoryItem_Presenter::SetModel(const TScriptInterface<IInventoryItem_Model>& InModel)
{
	UnbindPowerGrid();
	Model = InModel;
	BindPowerGrid();
	UpdateView();
}

void UInventoryItem_Presenter::SetView(const TSharedPtr<IInventoryItem_View>& InView)
{
	View = InView;
	UpdateView();
}

void UInventoryItem_Presenter::BeginDestroy()
{
	UnbindPowerGrid();
	Super::BeginDestroy();
}

void UInventoryItem_Presenter::BindPowerGrid()
{
	const UInventoryItem* Item = Cast<UInventoryItem>(Model.GetObject());
	UPowerGridComponent* Grid = Item ? PowerGridDisplay::FindGridOf(*Item) : nullptr;
	if (!Grid)
	{
		return;
	}
	PowerGrid = Grid;
	PowerGridChangedHandle = Grid->GetPowerGridChangedDelegate().AddWeakLambda(this, [this](UPowerGridComponent*)
	{
		UpdateView();
	});
}

void UInventoryItem_Presenter::UnbindPowerGrid()
{
	if (UPowerGridComponent* Grid = PowerGrid.Get())
	{
		Grid->GetPowerGridChangedDelegate().Remove(PowerGridChangedHandle);
	}
	PowerGrid.Reset();
	PowerGridChangedHandle.Reset();
}

void UInventoryItem_Presenter::UpdateView() const
{
	const TSharedPtr<IInventoryItem_View> PinnedView = View.Pin();
	if (!PinnedView.IsValid())
	{
		return;
	}

	FSlateBrush Icon;
	FText Name;
	FText Description;
	int32 Count = 0;
	FItemPowerInfo PowerInfo;

	if (IsValid(Model.GetObject()))
	{
		Icon = Model->GetItemIcon();
		Name = Model->GetItemName();
		Description = Model->GetItemDescription();
		Count = Model->GetItemCount();
	}
	if (const UInventoryItem* Item = Cast<UInventoryItem>(Model.GetObject()); IsValid(Item))
	{
		PowerInfo = PowerGridDisplay::DescribeItem(*Item, PowerGrid.Get());
	}

	PinnedView->SetIcon(Icon);
	PinnedView->SetName(Name);
	PinnedView->SetDescription(Description);
	PinnedView->SetCount(Count);
	PinnedView->SetPowerInfo(PowerInfo);
}
