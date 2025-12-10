#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Border.h"
#include "Components/HorizontalBox.h"
#include "Components/VerticalBox.h"
#include "NsUIFrameworkTypes.h"
#include "NsUIDebugOverlay.generated.h"


UCLASS()
class UNsUIDebugOverlay : public UUserWidget
{
    GENERATED_BODY()

public:
    UNsUIDebugOverlay(const FObjectInitializer& ObjectInitializer);

    virtual TSharedRef<SWidget> RebuildWidget() override;

    void SetSnapshots(const TArray<FNsUILayerSnapshot>& Snapshots);

protected:
    void BuildEntries(const TArray<FNsUILayerSnapshot>& Snapshots);
    UVerticalBox* AddLayerColumn(const FString& Title);

    UPROPERTY(Transient)
    UHorizontalBox* RootBox;

    UPROPERTY(Transient)
    UBorder* BackgroundBorder;
};

