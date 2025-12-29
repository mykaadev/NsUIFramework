#pragma once

#include "NsUIFrameworkTypes.h"
#include "Blueprint/UserWidget.h"
#include "NsUILayout.generated.h"

UCLASS()
class NSUIFRAMEWORK_API UNsUILayout : public UUserWidget
{
    GENERATED_BODY()

public:
    UNsUILayout(const FObjectInitializer& ObjectInitializer);

    virtual TSharedRef<class SWidget> RebuildWidget() override;

    class UNsUILayer* GetLayerWidget(ENsUILayer Layer) const;

    class UNsUICursor* GetCursorWidget() const { return CursorWidget; }
    FVector2D GetCursorAnchor() const { return CursorAnchor; }
    FVector2D GetCursorWidgetSnapAnchor() const { return CursorWidgetSnapAnchor; }

    void SetCursorAnchor(const FVector2D& InAnchor);
    void SetCursorVisibility(bool bVisible);

private:

    void BuildLayer(ENsUILayer Layer, int32 ZOrder);

protected:
    UPROPERTY(EditDefaultsOnly, Category = "NsUI")
    TSubclassOf<class UNsUILayer> LayerClass;

    UPROPERTY(EditDefaultsOnly, Category = "NsUI")
    FSlateBrush CursorBrush;

private:
    UPROPERTY()
    TObjectPtr<class UOverlay> RootOverlay;

    UPROPERTY()
    TMap<ENsUILayer, TObjectPtr<class UNsUILayer>> Layers;

    UPROPERTY()
    TObjectPtr<class UNsUICursor> CursorWidget;

    UPROPERTY()
    FVector2D CursorAnchor;

    UPROPERTY()
    FVector2D CursorWidgetSnapAnchor;
};
