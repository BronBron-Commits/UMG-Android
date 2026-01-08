@echo off
"C:\\Users\\Bronson\\AppData\\Local\\Android\\Sdk\\cmake\\4.1.2\\bin\\cmake.exe" ^
  "-HC:\\Projects\\UMG-Android\\app\\src\\main\\cpp" ^
  "-DCMAKE_SYSTEM_NAME=Android" ^
  "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON" ^
  "-DCMAKE_SYSTEM_VERSION=24" ^
  "-DANDROID_PLATFORM=android-24" ^
  "-DANDROID_ABI=x86" ^
  "-DCMAKE_ANDROID_ARCH_ABI=x86" ^
  "-DANDROID_NDK=C:\\Users\\Bronson\\AppData\\Local\\Android\\Sdk\\ndk\\25.2.9519653" ^
  "-DCMAKE_ANDROID_NDK=C:\\Users\\Bronson\\AppData\\Local\\Android\\Sdk\\ndk\\25.2.9519653" ^
  "-DCMAKE_TOOLCHAIN_FILE=C:\\Users\\Bronson\\AppData\\Local\\Android\\Sdk\\ndk\\25.2.9519653\\build\\cmake\\android.toolchain.cmake" ^
  "-DCMAKE_MAKE_PROGRAM=C:\\Users\\Bronson\\AppData\\Local\\Android\\Sdk\\cmake\\4.1.2\\bin\\ninja.exe" ^
  "-DCMAKE_LIBRARY_OUTPUT_DIRECTORY=C:\\Projects\\UMG-Android\\app\\build\\intermediates\\cxx\\Debug\\6cu2c5o1\\obj\\x86" ^
  "-DCMAKE_RUNTIME_OUTPUT_DIRECTORY=C:\\Projects\\UMG-Android\\app\\build\\intermediates\\cxx\\Debug\\6cu2c5o1\\obj\\x86" ^
  "-DCMAKE_BUILD_TYPE=Debug" ^
  "-BC:\\Projects\\UMG-Android\\app\\.cxx\\Debug\\6cu2c5o1\\x86" ^
  -GNinja
