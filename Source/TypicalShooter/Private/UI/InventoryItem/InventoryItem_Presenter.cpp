#include "UI/InventoryItem/InventoryItem_Presenter.h"

#include "UI/InventoryItem/InventoryItem_View.h"

void UInventoryItem_Presenter::SetModel(TScriptInterface<IInventoryItem_Model> InModel)
{
	Model = InModel;
	UpdateView();
}

void UInventoryItem_Presenter::SetView(TSharedPtr<IInventoryItem_View> InView)
{
	View = InView;
	UpdateView();
}

void UInventoryItem_Presenter::UpdateView() const
{
	if (!View.Get())
	{
		return;
	}
	
	FSlateBrush Icon = FSlateBrush();
	FString Name = FString();
	FString Description = FString();
	int32 Count = 0;
	
	if (IsValid(Model.GetObject()))
	{
		Icon = Model->GetIcon();
		Name = Model->GetName();
		Description = Model->GetDescription();
		Count = Model->GetCount();
	}
	
	View->SetIcon(Icon);
	View->SetName(Name);
	View->SetDescription(Description);
	View->SetCount(Count);
}