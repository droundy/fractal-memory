
Building for android
====================

    # First update local.properties (require "android" in path)
    android update project -p ./
    # Now build the C code (require "ndk-build" in path)
    ndk-build
    # Finally, build and run the app (require "ant" in path)
    ant debug install

