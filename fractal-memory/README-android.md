
Building for android
====================

First you need to get the source code for `SDL2`, and put it in
`fractal-memory/jni/SDL`, and `SDL2_ttf`, and put that in
`fractal-memory/jni/SDL_ttf`, and finally freetype's source goes in
`fractal-memory/jni/SDL_ttf/freetype`.

Then in the fractal-memory subdirectory (which has the
`AndroidManifest.xml`), you do:

    # First update local.properties (require "android" in path)
    android update project -p ./
    # Now build the C code (require "ndk-build" in path)
    ndk-build
    # Finally, build and run the app (require "ant" in path)
    ant debug install

