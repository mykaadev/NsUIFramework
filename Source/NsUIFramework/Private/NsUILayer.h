#pragma once

#include "Blueprint/UserWidget.h"
#include "NsUILayer.generated.h"

class UOverlay;

UCLASS(meta=(ScriptName="NsUILayerPanel"))
class NSUIFRAMEWORK_API UNsUILayer : public UUserWidget
{
    GENERATED_BODY()

public:
    UNsUILayer(const FObjectInitializer& ObjectInitializer);

    virtual TSharedRef<SWidget> RebuildWidget() override;

    void AddScreen(UUserWidget* Screen);

private:
    UPROPERTY()
    TObjectPtr<UOverlay> RootOverlay;

};
