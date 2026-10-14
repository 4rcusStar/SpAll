// StartupTrace.h — opt-in milestone log for application startup.
//
// A GUI-subsystem binary has no console, so a startup problem — especially a hang
// before the first window exists — leaves nothing to inspect. With
// SPALL_STARTUP_TRACE=1 set, milestones are appended to spall-startup-trace.log next
// to the executable, which shows exactly how far startup got.
#pragma once

namespace spall {

/// Append one milestone line. Does nothing unless SPALL_STARTUP_TRACE is set.
void startupTrace(const char* milestone);

}  // namespace spall
