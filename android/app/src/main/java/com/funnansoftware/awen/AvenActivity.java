package com.funnansoftware.awen;

import org.libsdl.app.SDLActivity;

/**
 * Main Android activity for awen-pong.
 *
 * Extends SDL3's SDLActivity to override the default library list.  SDL3 is
 * built as a static library by vcpkg (VCPKG_LIBRARY_LINKAGE=static), so its
 * code is linked directly into libawen-pong.so — there is no separate libSDL3.so.
 * Only "awen-pong" needs to be loaded; SDL3's JNI entry points are exported from
 * libawen-pong.so automatically.
 *
 * SDL3's Java runtime files (SDLActivity.java, SDL.java, etc.) must be
 * present in src/main/java/org/libsdl/app/.  Run the Gradle task
 * "syncSdl3AndroidSources" once after vcpkg has installed SDL3 for Android
 * to copy them from the vcpkg buildtrees into the project:
 *
 *   ./gradlew syncSdl3AndroidSources
 */
public class AvenActivity extends SDLActivity
{
    @Override
    protected String[] getLibraries()
    {
        return new String[] {
            "awen-pong"
        };
    }
}
