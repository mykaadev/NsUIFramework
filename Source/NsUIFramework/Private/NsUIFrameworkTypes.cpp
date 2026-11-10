#include "NsUIFrameworkTypes.h"

#include "Selection/NsUISelectionMode.h"

DEFINE_LOG_CATEGORY(LogNsUI);

FNsUIWidgetConfig::FNsUIWidgetConfig()
{
    Layer = ENsUILayer::Menu;
    InputPolicy = ENsUIInputPolicy::UIOnly;
    FocusPolicy = ENsUIFocusPolicy::Children;
    bShowMouseCursor = true;
}
