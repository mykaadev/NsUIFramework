#pragma once

#include "NsUIFrameworkTypes.h"


#include "Blueprint/UserWidget.h"
#include "NsUIDebugOverlay.generated.h"

class UOverlay;
class UHorizontalBox;
class UVerticalBox;
class UTextBlock;

UCLASS()
class UNsUIDebugOverlay : public UUserWidget
{
    GENERATED_BODY()

public:
    UNsUIDebugOverlay(const FObjectInitializer& ObjectInitializer);

    virtual TSharedRef<SWidget> RebuildWidget() override;
    void SetSnapshots(const TArray<FNsUILayerSnapshot>& Snapshots);

private:
    UPROPERTY()
    TObjectPtr<UHorizontalBox> RootBox;

    void BuildEntries(const TArray<FNsUILayerSnapshot>& Snapshots);
    UVerticalBox* AddLayerColumn(const FString& Title);
};