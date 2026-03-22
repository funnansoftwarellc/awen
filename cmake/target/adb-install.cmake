# cmake -P script invoked by the adb-install custom target.
# Uninstalls the existing package (signature-agnostic), then installs the new APK.
# ADB, APK, and PACKAGE are passed via -D on the command line.

# Uninstall — ignore failure (package may not be installed at all).
execute_process(
    COMMAND "${ADB}" uninstall "${PACKAGE}"
    OUTPUT_QUIET ERROR_QUIET
)

execute_process(
    COMMAND "${ADB}" install "${APK}"
    RESULT_VARIABLE _result
)
if(NOT _result EQUAL 0)
    message(FATAL_ERROR "adb install failed (exit code ${_result})")
endif()
