#include "NsUISlateInputGuard.h"

namespace
{
    bool GForwardingSyntheticInputToSlate = false;
}

bool IsForwardingSyntheticInputToSlate()
{
    return GForwardingSyntheticInputToSlate;
}

FNsUISyntheticInputGuard::FNsUISyntheticInputGuard()
    : bPreviousState(GForwardingSyntheticInputToSlate)
{
    GForwardingSyntheticInputToSlate = true;
}

FNsUISyntheticInputGuard::~FNsUISyntheticInputGuard()
{
    GForwardingSyntheticInputToSlate = bPreviousState;
}
