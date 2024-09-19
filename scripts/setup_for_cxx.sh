Delimeter="$RED-----------------------------------------------------------$WHITE"
 Indent="$WHITE           $WHITE-"
    Info="$BLUE   Info    $WHITE-"
Success="$GREEN   Success $WHITE-"
Building="$BLUE   Build   $WHITE-"
  Warn="$YELLOW   Warning $WHITE-"
   Failed="$RED   Failed  $WHITE-"
    Error="$RED   Error   $WHITE-"

if [ "$Platform" == "Linux" ] ; then
  PLATFORM_LINKER_OPTIONS="-lpthread -lX11 -ldl -lGL"
  PLATFORM_DEFINES=""
  PLATFORM_DEFINES="$PLATFORM_DEFINES -D BONSAI_LINUX"
  # PLATFORM_DEFINES="$PLATFORM_DEFINES -DBONSAI_SLOW"
  PLATFORM_INCLUDE_DIRS=""
  PLATFORM_CXX_OPTIONS="-ggdb"

  # TODO(Jesse): What does -fPIC acutally do?  I found the option documented,
  # but with no explanation of what it's doing.  Apparently it's unsupported on
  # Windows, so hopefully it's not necessary for anything.
  #
  # Turns out that -fPIC turns on rip-relative addressing (among other things?)
  # such that functions work regardless of where they're loaded into memory.
  # This is important (obviously) for dynamically-loaded libs and ASLR.
  #
  # https://clang.llvm.org/docs/ClangCommandLineReference.html
  # @unsupported_fPIC_flag_windows
  #
  SHARED_LIBRARY_FLAGS="-shared -fPIC"
  PLATFORM_EXE_EXTENSION=""
  PLATFORM_LIB_EXTENSION=".so"

elif [[ "$Platform" == "Windows" ]] ; then

  # advapi32 : tracelogging
  # winmm : timePeriodBegin // timePeriodEnd (scheduling)

  PLATFORM_LINKER_OPTIONS="-lAdvapi32 -lWinmm -lgdi32 -luser32 -lopengl32 -lglu32 -fuse-ld=lld -W1,/debug,/pdb:name.pdb"


  PLATFORM_DEFINES="-D _CRT_SECURE_NO_WARNINGS -D BONSAI_WIN32"
  PLATFORM_INCLUDE_DIRS=""
  PLATFORM_CXX_OPTIONS="-g -gcodeview"

  # @unsupported_fPIC_flag_windows
  SHARED_LIBRARY_FLAGS="-shared"

  PLATFORM_EXE_EXTENSION=".exe"
  PLATFORM_LIB_EXTENSION=".dll"
else
  echo "Unsupported Platform ($Platform), exiting." && exit 1
fi

# TODO(Jesse, tags: build_pipeline): Investigate -Wcast-align situation

  # -fsanitize=address

# Note(Jesse): Using -std=c++17 so I can mark functions with [[nodiscard]]

# TODO(Jesse): Figure out how to standardize on a compiler across machines such that
# we can remove -Wno-unknown-warning-optins
CXX_OPTIONS="
  --std=c++17
  -ferror-limit=2000
  -mssse3
  -mavx
  -mavx2
  -mfma

  -Weverything

  -Wno-reserved-identifier
  -Wno-reserved-id-macro

  -Wno-unknown-warning-option
  -Wno-unsafe-buffer-usage

  -Wno-exit-time-destructors
  -Wno-c++98-compat-pedantic

  -Wno-gnu-anonymous-struct
  -Wno-nested-anon-types

  -Wno-missing-prototypes
  -Wno-zero-as-null-pointer-constant
  -Wno-format-nonliteral
  -Wno-cast-qual
  -Wno-unused-function
  -Wno-four-char-constants
  -Wno-old-style-cast
  -Wno-float-equal
  -Wno-global-constructors
  -Wno-cast-align

  -Wno-switch-enum
  -Wno-switch-default
  -Wno-covered-switch-default

  -Wno-undef
  -Wno-c99-extensions
  -Wno-dollar-in-identifier-extension

  -Wno-class-varargs

  -Wno-unused-value
  -Wno-unused-variable
  -Wno-unused-but-set-variable
  -Wno-unused-parameter

  -Wno-implicit-int-float-conversion
  -Wno-extra-semi-stmt
  -Wno-reorder-init-list
  -Wno-unused-macros

  -Wno-padded
  -Wno-gnu-zero-variadic-macro-arguments

  -Wno-atomic-implicit-seq-cst

  -Wno-nonportable-system-include-path
  -Wno-nonportable-include-path
"

function SetOutputBinaryPathBasename()
{
  base_file="${1##*/}"
  output_basename="$2/${base_file%%.*}"
}

function ColorizeTitle()
{
  echo -e " $YELLOW$1$WHITE"
  echo -e ""
}

declare -a build_job_pids
declare -a build_job_names

# Call like: TrackPid "job name" <pid>
TrackPid() {
    local name=$1
    local pid=$2
    # echo "$name -- $pid"
    build_job_pids=(${build_job_pids[@]} $pid)
    build_job_names=(${build_job_names[@]} $name)
}

WaitForTrackedPids() {
    while [ ${#build_job_pids[@]} -ne 0 ]; do
        # echo "Waiting for pids: ${build_job_pids[@]}"
        local range=$(eval echo {0..$((${#build_job_pids[@]}-1))})
        local i
        for i in $range; do
            if ! kill -0 ${build_job_pids[$i]} 2> /dev/null; then

                exit_code=0
                wait ${build_job_pids[$i]} || exit_code=$?

                if [ $exit_code -eq 0 ]; then
                  echo -e "$Success ${build_job_names[$i]}"
                else
                  echo -e "$Failed ${build_job_names[$i]}"
                  exit 1
                fi

                unset build_job_pids[$i]
                unset build_job_names[$i]
            fi
        done
         # Expunge nulls created by unset.
        build_job_pids=("${build_job_pids[@]}")
        build_job_names=("${build_job_names[@]}")
        sleep 0.25
    done
}

