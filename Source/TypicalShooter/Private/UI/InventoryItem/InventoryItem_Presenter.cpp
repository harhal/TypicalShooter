#include "UI/InventoryItem/InventoryItem_Presenter.h"

#include "UI/InventoryItem/InventoryItem_View.h"

void UInventoryItem_Presenter::SetModel(const TScriptInterface<IInventoryItem_Model>& InModel)
{
	Model = InModel;
	UpdateView();
}

void UInventoryItem_Presenter::SetView(const TSharedPtr<IInventoryItem_View>& InView)
{
	View = InView;
	UpdateView();
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

	if (IsValid(Model.GetObject()))
	{
		Icon = Model->GetItemIcon();
		Name = Model->GetItemName();
		Description = Model->GetItemDescription();
		Count = Model->GetItemCount();
	}

	PinnedView->SetIcon(Icon);
	PinnedView->SetName(Name);
	PinnedView->SetDescription(Description);
	PinnedView->SetCount(Count);
}
