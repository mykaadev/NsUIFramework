#pragma once
#include "NsUIFrameworkTypes.h"
#include "Blueprint/UserWidget.h"
#include "NsUILayout.generated.h"

class UNsUILayer;

UCLASS()
class NSUIFRAMEWORK_API UNsUILayout : public UUserWidget
{
    GENERATED_BODY()
public:
    UNsUILayout(const FObjectInitializer& Initializer);
    virtual TSharedRef<SWidget> RebuildWidget() override;
    UNsUILayer* GetLayerWidget(ENsUILayer Layer) const;
    const FSlateBrush& GetCursorBrush() const { return CursorBrush; }
protected:
    UPROPERTY(EditDefaultsOnly, Category="NsUI")
    FSlateBrush CursorBrush;
private:
    UPROPERTY()
    TMap<ENsUILayer, TObjectPtr<UNsUILayer>> Layers;
};
