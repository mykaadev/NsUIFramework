#pragma once

#include "NsUIFrameworkTypes.h"
#include "Blueprint/UserWidget.h"
#include "Components/Border.h"
#include "NsUILayer.generated.h"

class UPanelWidget;

UCLASS()
class NSUIFRAMEWORK_API UNsUILayer : public UUserWidget
{
    GENERATED_BODY()

public:
    UNsUILayer(const FObjectInitializer& ObjectInitializer);

    virtual TSharedRef<SWidget> RebuildWidget() override;

    void AddScreen(UUserWidget* Screen);
    void RemoveScreen(UUserWidget* Screen);
    void ClearScreens();

    void SetLayerType(ENsUILayer InLayer);
    ENsUILayer GetLayerType() const { return Layer; }

    UBorder* GetRootPanel() const { return RootPanel; }

private:
    UPROPERTY()
    TObjectPtr<UBorder> RootPanel;

    UPROPERTY(EditAnywhere, Category = "NsUI")
    ENsUILayer Layer;
};
