#pragma once
#include "Blueprint/UserWidget.h"
#include "NsUIScreen.h"
#include "NsUISmokeTestWidgets.generated.h"

class UVerticalBox;
class AActor;

UCLASS()
class NSUIFRAMEWORKEDITOR_API UNsUISmokeTestRootWidget : public UUserWidget, public INsUIScreen
{
    GENERATED_BODY()
public:
    virtual TSharedRef<SWidget> RebuildWidget() override;
    virtual bool HandleNsScreenCancel_Implementation() override;
private:
    UFUNCTION()
    void Close();
    UFUNCTION()
    void OpenMenu();
    UFUNCTION()
    void OpenModal();
    UFUNCTION()
    void OpenExternal();
    UFUNCTION()
    void AddControl();
    UFUNCTION()
    void RemoveControl();
    UFUNCTION()
    void OpenActors();
    UFUNCTION()
    void OpenLocations();
    UPROPERTY()
    TObjectPtr<UVerticalBox> Controls;
};

UCLASS()
class NSUIFRAMEWORKEDITOR_API UNsUISmokeWorldWidget : public UUserWidget, public INsUIScreen
{
    GENERATED_BODY()
public:
    virtual FNsUIScreenConfig GetNsScreenConfig_Implementation() const override;
    virtual void OnNsScreenOpened_Implementation(UObject* Context, UNsUISelectionMode* Mode) override;
    virtual void OnNsScreenClosed_Implementation() override;
    virtual bool HandleNsScreenCancel_Implementation() override;
    virtual bool HandleNsScreenConfirm_Implementation() override;
    bool bActors = true;
private:
    UPROPERTY()
    TObjectPtr<UNsUISelectionMode> Selection;
    TArray<TWeakObjectPtr<AActor>> Actors;
};
