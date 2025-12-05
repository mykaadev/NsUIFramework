#pragma once

#include "NsUIFrameworkTypes.h"
#include "Blueprint/UserWidget.h"
#include "NsUILayout.generated.h"

class UOverlay;
class UNsUILayer;
class UNsUICursor;

UCLASS()
class NSUIFRAMEWORK_API UNsUILayout : public UUserWidget
{
    GENERATED_BODY()

public:
    UNsUILayout(const FObjectInitializer& ObjectInitializer);

    virtual TSharedRef<class SWidget> RebuildWidget() override;

    UNsUILayer* GetLayerWidget(ENsUILayer Layer) const;
    UNsUICursor* GetCursorWidget() const { return CursorWidget; }

    void UpdateCursorPosition(const FVector2D& ScreenPosition);
    void SetCursorVisibility(bool bVisible);

protected:
    UPROPERTY(EditDefaultsOnly, Category = "NsUI")
    TSubclassOf<UNsUILayer> LayerClass;

    UPROPERTY(EditDefaultsOnly, Category = "NsUI")
    FSlateBrush CursorBrush;

private:
    UPROPERTY()
    TObjectPtr<UOverlay> RootOverlay;

    UPROPERTY()
    TMap<ENsUILayer, TObjectPtr<UNsUILayer>> Layers;

    UPROPERTY()
    TObjectPtr<UNsUICursor> CursorWidget;

    void BuildLayer(ENsUILayer Layer, int32 ZOrder);
};
