#pragma once

/** RAII guard around synthetic Slate input forwarding to prevent recursion. */
struct FNsUISyntheticInputGuard
{
    FNsUISyntheticInputGuard();
    ~FNsUISyntheticInputGuard();

private:
    bool bPreviousState;
};

/** Query whether synthetic Slate input is currently being forwarded. */
bool IsForwardingSyntheticInputToSlate();
