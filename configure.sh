#!/bin/sh

# Syntax and structure borrowed from cvc5's configure.sh script

CONF_FILE=Makefile.conf

usage () {
cat <<EOF
Usage: $0 [<option> ...]

Configures the CMAKE build environment.

-h, --help              display this message and exit
--prefix=STR            install directory       (default: /usr/local/)
--build-dir=STR         custom build directory  (default: build)
--with-msat             build with MathSAT which has a custom non-BSD compliant license.  (default : off)
                        Required for interpolant based model checking
--debug                 build debug with debug symbols (default: off)
--python                compile with python bindings (default: off)
--static-lib            build a static library (default: shared)
--static                build a static executable (default: dynamic); implies --static-lib
--with-profiling        build with gperftools for profiling (default: off)
EOF
  exit 0
}

die () {
    echo "*** $0: $*" 1>&2
    exit 1
}

build_dir=build
install_prefix=default
build_type=default
with_msat=default
debug=default
python=default
lib_type=SHARED
static_exec=NO
with_profiling=default

buildtype=Debug

while [ $# -gt 0 ]
do
    case $1 in
        -h|--help) usage;;
        --prefix) die "missing argument to $1 (see -h)" ;;
        --prefix=*)
            install_prefix=${1##*=}
            # Check if install_prefix is an absolute path and if not, make it
            # absolute.
            case $install_prefix in
                /*) ;;                                      # absolute path
                *) install_prefix=$(pwd)/$install_prefix ;; # make absolute path
            esac
            ;;
        --build-dir) die "missing argument to $1 (see -h)" ;;
        --build-dir=*)
            build_dir=${1##*=}
            # Check if build_dir is an absolute path and if not, make it
            # absolute.
            case $build_dir in
                /*) ;;                                      # absolute path
                *) build_dir=$(pwd)/$build_dir ;; # make absolute path
            esac
            ;;
        --with-msat) with_msat=ON;;
        --debug)
            debug=yes;
            buildtype=Debug
            ;;
        --python)
            python=yes
            ;;
        --static-lib)
            lib_type=STATIC
            ;;
        --static)
            static_exec=YES;
            lib_type=STATIC;
            ;;
        --with-profiling) with_profiling=ON;;
        *) die "unexpected argument: $1";;
    esac
    shift
done


cmake_opts="-DCMAKE_BUILD_TYPE=$buildtype -DWASIM_LIB_TYPE=${lib_type} -DWASIM_STATIC_EXEC=${static_exec}"

[ $install_prefix != default ] \
    && cmake_opts="$cmake_opts -DCMAKE_INSTALL_PREFIX=$install_prefix"

[ $with_msat != default ] \
    && cmake_opts="$cmake_opts -DWITH_MSAT=$with_msat"

[ $python != default ] \
    && cmake_opts="$cmake_opts -DBUILD_PYTHON_BINDINGS=ON"

[ $with_profiling != default ] \
    && cmake_opts="$cmake_opts -DWITH_PROFILING=$with_profiling"

root_dir=$(pwd)

[ -e "$build_dir" ] && rm -r "$build_dir"

mkdir -p "$build_dir"
cd "$build_dir" || exit 1

[ -e CMakeCache.txt ] && rm CMakeCache.txt

echo "Running with cmake options: $cmake_opts"

cmake "$root_dir" $cmake_opts 2>&1
