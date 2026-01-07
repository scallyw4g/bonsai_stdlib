#!/usr/bin/env bash

# SANITIZER="-fsanitize=undefined"
# SANITIZER="-fsanitize=address"
# SANITIZER="-fsanitize=thread"

BUILD_EVERYTHING=0

RunPoof=0

BuildExecutables=0
BuildObjects=0

stdlib_build_scripts='./scripts'
source $stdlib_build_scripts/preamble.sh
source $stdlib_build_scripts/setup_for_cxx.sh


OPTIMIZATION_LEVEL=""
EMCC=0


# NOTE(Jesse): Can't do this cause it fucks with the paths poof outputs
# ROOT="$(pwd)"
ROOT="."
SRC="$ROOT/src"
INCLUDE="$ROOT/external"
EXAMPLES="$ROOT/examples"
TESTS="$SRC/tests"
BIN="$ROOT/bin"
BIN_TEST="$BIN/tests"
BIN_GAME_LIBS="$BIN/game_libs"

BONSAI_INTERNAL='-D BONSAI_INTERNAL=1'
# BONSAI_INTERNAL='-D BONSAI_INTERNAL=1 -D BONSAI_SOFT_ASSERT'
# BONSAI_INTERNAL=''

EXECUTABLES_TO_BUILD="
  $SRC/examples/perlin.cpp
"

OBJECTS_TO_LINK_WITH="
  $BIN/objects/perlin_avx2.o
"

OBJECTS_TO_BUILD="
  $SRC/perlin_avx2.cpp
"

# COMPILER="clang++-19"
COMPILER="clang++"

function BuildObjects
{
  echo ""
  ColorizeTitle "Objects"
  for executable in $OBJECTS_TO_BUILD; do
    SetOutputBinaryPathBasename "$executable" "$BIN/objects"
    echo -e "$Building $executable"
    $COMPILER                                        \
      $SANITIZER                                     \
      $OPTIMIZATION_LEVEL                            \
      $CXX_OPTIONS                                   \
      $BONSAI_INTERNAL                               \
      $PLATFORM_CXX_OPTIONS                          \
      $PLATFORM_DEFINES                              \
      $PLATFORM_INCLUDE_DIRS                         \
      -I "$ROOT"                                     \
      -I "$SRC"                                      \
      -I "$INCLUDE"                                  \
      -c $executable                                 \
      -o "$output_basename"".o" &

    TrackPid "$executable" $!

  done
}

function BuildExecutables
{
  echo ""
  ColorizeTitle "Executables"
  for executable in $EXECUTABLES_TO_BUILD; do
    SetOutputBinaryPathBasename "$executable" "$BIN"
    echo -e "$Building $executable"
    $COMPILER                                       \
      $SANITIZER                                     \
      $OPTIMIZATION_LEVEL                            \
      $CXX_OPTIONS                                   \
      $BONSAI_INTERNAL                               \
      $PLATFORM_CXX_OPTIONS                          \
      $PLATFORM_LINKER_OPTIONS                       \
      $PLATFORM_DEFINES                              \
      $PLATFORM_INCLUDE_DIRS                         \
      -I "$ROOT"                                     \
      -I "$SRC"                                      \
      -I "$INCLUDE"                                  \
      $OBJECTS_TO_LINK_WITH                          \
      -o "$output_basename""$PLATFORM_EXE_EXTENSION" \
      $executable &

    TrackPid "$executable" $!

  done
}

function BuildWithClang
{
  which $COMPILER > /dev/null
  [ $? -ne 0 ] && echo -e "Please install $COMPILER" && exit 1

  echo -e ""
  echo -e "$Delimeter"

  [[ $BuildObjects     == 1     || $BUILD_EVERYTHING == 1 ]] && BuildObjects

  WaitForTrackedPids
  sync

  echo -e ""
  echo -e "$Delimeter"
  echo -e ""
  ColorizeTitle "Objects Complete"


  [[ $BuildExecutables == 1     || $BUILD_EVERYTHING == 1 ]] && BuildExecutables

  echo -e ""
  echo -e "$Delimeter"
  echo -e ""
  ColorizeTitle "Complete"

  WaitForTrackedPids
  sync

  echo -e ""
  echo -e "$Delimeter"
  echo -e ""

  echo -e ""
}

function BuildWithEMCC {
  which emcc > /dev/null 2&> 1
  [ $? -ne 0 ] && echo -e "$Error Please install emcc" && exit 1

  emcc                       \
    -s WASM=1                \
    -s LLD_REPORT_UNDEFINED  \
    -s FULL_ES3=1            \
    -s ALLOW_MEMORY_GROWTH=1 \
    -s ASSERTIONS=1          \
    -s DEMANGLE_SUPPORT=1    \
    -std=c++17               \
    -Wno-c99-designator      \
    -Wno-reorder-init-list   \
    -ferror-limit=2000       \
    -fno-exceptions          \
    -O2                      \
    -g4                      \
    --source-map-base /      \
    --emrun                  \
    -msse                    \
    -msimd128                \
    -DEMCC=1                 \
    -DWASM=1                 \
    $BONSAI_INTERNAL         \
    -I src                   \
    -I examples              \
    src/game_loader.cpp      \
    -o bin/wasm/platform.html

    # --embed-file shaders     \
    # --embed-file models      \
}


if [ ! -d "$BIN" ]; then
  mkdir "$BIN"
fi

if [ ! -d "$BIN/objects" ]; then
  mkdir "$BIN/objects"
fi


if [ ! -d "$BIN/wasm" ]; then
  mkdir "$BIN/wasm"
fi

function RunEntireBuild {

  if [ $RunPoof == 1 ]; then
    RunPoof
    [ $? -ne 0 ] && exit 1
  fi

  if [ $EMCC == 1 ]; then
    BuildWithEMCC
    [ $? -ne 0 ] && exit 1
  else
    BuildWithClang
    [ $? -ne 0 ] && exit 1
  fi

}


function RunPoofHelper {

   # -D _M_CEE_PURE \

   # -I "C:/Program Files (x86)/Microsoft Visual Studio/2019/Community/VC/Tools/MSVC/14.28.29333/include"        \
   # -I "C:/Program Files (x86)/Microsoft Visual Studio/2019/Community/VC/Tools/MSVC/14.28.29333/atlmfc/include" \

   # -I "C:/Program Files/LLVM/lib/clang/11.0.0/include"                                                         \
   # -I "C:/Program Files (x86)/Microsoft Visual Studio/2019/Community/VC/Tools/MSVC/14.28.29333/include"        \
   # -I "C:/Program Files (x86)/Microsoft Visual Studio/2019/Community/VC/Tools/MSVC/14.28.29333/atlmfc/include" \
   # -I "C:/Program Files (x86)/Windows Kits/10/Include/10.0.18362.0/ucrt"                                       \
   # -I "C:/Program Files (x86)/Windows Kits/10/include/10.0.18362.0/shared"                                     \
   # -I "C:/Program Files (x86)/Windows Kits/10/include/10.0.18362.0/um"                                         \
   # -I "C:/Program Files (x86)/Windows Kits/10/include/10.0.18362.0/winrt"                                      \

   which poof > /dev/null 2>&1
   if [ $? -eq 0 ]; then

   cmd="poof $COLOR_FLAG -D POOF_PREPROCESSOR -D BONSAI_PREPROCESSOR -I src/ -I external/ $PLATFORM_DEFINES $BONSAI_INTERNAL $@"

   echo "$cmd"
   $cmd

   else
     echo "poof not found, skipping."
   fi


}

function RunPoof
{
  echo -e ""
  echo -e "$Delimeter"
  echo -e ""

  ColorizeTitle "Poofing"

  # [ -d src/generated ] && rm -Rf src/generated
  # [ -d generated ] && rm -Rf generated

  # RunPoofHelper -o ./ src/poof_ctags_stub.cpp && echo -e "$Success poofed src/poof_ctags_stub.cpp" &
  # TrackPid "" $!

  # RunPoofHelper -o generated examples/ui_test/game.cpp && echo -e "$Success poofed examples/ui_test/game.cpp" &
  # TrackPid "" $!

  RunPoofHelper -o generated src/game_loader.cpp && echo -e "$Success poofed src/game_loader.cpp" &
  TrackPid "" $!

  # RunPoofHelper -o generated examples/turn_based/game.cpp && echo -e "$Success poofed examples/turn_based/game.cpp" &
  # TrackPid "" $!

  # RunPoofHelper -o generated examples/terrain_gen/game.cpp && echo -e "$Success poofed examples/terrain_gen/game.cpp" &
  # TrackPid "" $!

  # RunPoofHelper -o generated examples/the_wanderer/game.cpp && echo -e "$Success poofed examples/the_wanderer/game.cpp" &
  # TrackPid "" $!

  # RunPoofHelper -o generated examples/tools/voxel_synthesis_rule_baker/game.cpp && echo -e "$Success poofed examples/tools/voxel_synthesis_rule_baker/game.cpp" &
  # TrackPid "" $!

  # RunPoofHelper -o generated src/tools/asset_packer.cpp && echo -e "$Success poofed src/tools/asset_packer.cpp" &
  # TrackPid "" $!

  WaitForTrackedPids
  sync

  [ -d tmp ] && rm -Rf tmp
}


SetBuildAllFlags() {

  BuildExamples=1
  BuildExecutables=1
  BuildObjects=1
  BuildTests=1

  # NOTE(Jesse): These only build on linux.  I'm honestly not sure if it's
  # worth getting them to build on Windows
  if [ $Platform == "Linux" ]; then
    BuildDebugOnlyTests=1
  fi
}

if [ $# -eq 0 ]; then
  OPTIMIZATION_LEVEL="-O2"
  SetBuildAllFlags
fi

BundleRelease=0
while (( "$#" )); do
  CliArg=$1
  echo $CliArg

  case $CliArg in

    "BuildAll")
      SetBuildAllFlags
    ;;

    "BuildObjects")
      BuildObjects=1
    ;;

    "BuildExecutables")
      BuildExecutables=1
    ;;

    "RunPoof")
      RunPoof=1
    ;;

    "BuildWithEMCC")
      BuildWithEMCC=1
    ;;

    "-Od")
      OPTIMIZATION_LEVEL="-Od"
    ;;

    "-O0")
      OPTIMIZATION_LEVEL="-O0"
    ;;

    "-O1")
      OPTIMIZATION_LEVEL="-O1"
    ;;

    "-O2")
      OPTIMIZATION_LEVEL="-O2"
    ;;

    "-O3")
      OPTIMIZATION_LEVEL="-O3"
    ;;


    *)
      echo "Unrecognized Build Option ($CliArg), exiting." && exit 1
    ;;
  esac

  shift
done

time RunEntireBuild

