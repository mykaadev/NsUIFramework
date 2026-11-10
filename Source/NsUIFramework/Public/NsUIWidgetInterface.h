#pragma once

#include "CoreMinimal.h"
#include "NsUIFrameworkTypes.h"
#include "Types/SlateEnums.h"
#include "UObject/Interface.h"
#include "NsUIWidgetInterface.generated.h"

/** The optional contract of a widget that NsUIFramework opens. Every event has a default, so a widget implements only what it needs */
UINTERFACE(BlueprintType)
class NSUIFRAMEWORK_API UNsUIWidgetInterface : public UInterface
{
    GENERATED_BODY()
};

/** The optional contract of a widget that NsUIFramework opens */
class NSUIFRAMEWORK_API INsUIWidgetInterface
{
    GENERATED_BODY()

// Functions
public:
    /** Returns how the widget behaves. The framework reads it when the widget opens and on RefreshConfig */
    UFUNCTION(BlueprintNativeEvent, Category = "NsUI")
    FNsUIWidgetConfig GetConfig() const;

    /** Returns the first focus of the widget. The framework reads it when the widget gets the focus and has no earlier focus */
    UFUNCTION(BlueprintNativeEvent, Category = "NsUI")
    FNsUIFocusRequest GetFocus() const;

    /** The widget opened. Context is the object given to OpenWidget. SelectionMode is null when the widget has no selection mode */
    UFUNCTION(BlueprintNativeEvent, Category = "NsUI")
    void OnOpened(UObject* Context, class UNsUISelectionMode* SelectionMode);

    /** The widget gets input. Resumed is true when it had input before */
    UFUNCTION(BlueprintNativeEvent, Category = "NsUI")
    void OnActivated(bool bResumed);

    /** The widget loses input, because a higher widget opened. It stays open */
    UFUNCTION(BlueprintNativeEvent, Category = "NsUI")
    void OnDeactivated();

    /** The widget closed. Release resources here */
    UFUNCTION(BlueprintNativeEvent, Category = "NsUI")
    void OnClosed();

    /** The focus moved inside the widget. Use it for the focus look and the focus sound */
    UFUNCTION(BlueprintNativeEvent, Category = "NsUI")
    void OnFocusChanged(class UWidget* OldFocus, class UWidget* NewFocus);

    /** Return true to use Confirm before the focused control gets it */
    UFUNCTION(BlueprintNativeEvent, Category = "NsUI")
    bool HandleConfirm();

    /** Return true to use Back before the framework closes the widget */
    UFUNCTION(BlueprintNativeEvent, Category = "NsUI")
    bool HandleBack();

    /** Return true to use a direction before the focus moves, for example to pan a map */
    UFUNCTION(BlueprintNativeEvent, Category = "NsUI")
    bool HandleNavigate(EUINavigation Direction);

    /** Return true to use the next or previous tab action */
    UFUNCTION(BlueprintNativeEvent, Category = "NsUI")
    bool HandleTab(bool bNext);

    /** Return true to play an outro before the widget closes. Then call FinishCloseWidget when the outro ends */
    UFUNCTION(BlueprintNativeEvent, Category = "NsUI")
    bool BeginClose();
};
