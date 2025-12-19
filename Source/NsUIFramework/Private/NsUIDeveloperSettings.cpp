#include "NsUIDeveloperSettings.h"

UNsUIDeveloperSettings::UNsUIDeveloperSettings()
{
    CategoryName = TEXT("Game");

    ConfirmKeys = { EKeys::Enter, EKeys::Virtual_Accept }; // Virtual key for platform mappings
    CancelKeys = { EKeys::Escape, EKeys::Virtual_Back };   // Virtual key for platform mappings

    NavigateUpKeys = { EKeys::Up, EKeys::Gamepad_DPad_Up };
    NavigateDownKeys = { EKeys::Down, EKeys::Gamepad_DPad_Down };
    NavigateLeftKeys = { EKeys::Left, EKeys::Gamepad_DPad_Left };
    NavigateRightKeys = { EKeys::Right, EKeys::Gamepad_DPad_Right };
    NavigateNextKeys = { EKeys::Tab };
    NavigatePreviousKeys = { EKeys::Tab, EKeys::BackSpace };

    ConfirmMouseButtons = { EKeys::LeftMouseButton };
}
