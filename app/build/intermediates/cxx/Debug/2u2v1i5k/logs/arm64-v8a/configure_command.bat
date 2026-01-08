@echo off
"C:\\Users\\Bronson\\AppData\\Local\\Android\\Sdk\\cmake\\4.1.2\\bin\\cmake.exe" ^
  "-HC:\\Projects\\UMG-Android\\app\\src\\main\\cpp" ^
  "-DCMAKE_SYSTEM_NAME=Android" ^
  "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON" ^
  "-DCMAKE_SYSTEM_VERSION=24" ^
  "-DANDROID_PLATFORM=android-24" ^
  "-DANDROID_ABI=arm64-v8a" ^
  "-DCMAKE_ANDROID_ARCH_ABI=arm64-v8a" ^
  "-DANDROID_NDK=C:\\Users\\Bronson\\AppData\\Local\\Android\\Sdk\\ndk\\25.1.8937393" ^
  "-DCMAKE_ANDROID_NDK=C:\\Users\\Bronson\\AppData\\Local\\Android\\Sdk\\ndk\\25.1.8937393" ^
  "-DCMAKE_TOOLCHAIN_FILE=C:\\Users\\Bronson\\AppData\\Local\\Android\\Sdk\\ndk\\25.1.8937393\\build\\cmake\\android.toolchain.cmake" ^
  "-DCMAKE_MAKE_PROGRAM=C:\\Users\\Bronson\\AppData\\Local\\Android\\Sdk\\cmake\\4.1.2\\bin\\ninja.exe" ^
  "-DCMAKE_LIBRARY_OUTPUT_DIRECTORY=C:\\Projects\\UMG-Android\\app\\build\\intermediates\\cxx\\Debug\\2u2v1i5k\\obj\\arm64-v8a" ^
  "-DCMAKE_RUNTIME_OUTPUT_DIRECTORY=C:\\Projects\\UMG-Android\\app\\build\\intermediates\\cxx\\Debug\\2u2v1i5k\\obj\\arm64-v8a" ^
  "-DCMAKE_BUILD_TYPE=Debug" ^
  "-BC:\\Projects\\UMG-Android\\app\\.cxx\\Debug\\2u2v1i5k\\arm64-v8a" ^
  -GNinja
