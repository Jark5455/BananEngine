import hashlib
import os
import pathlib
import shutil
import sys
import subprocess
import enum

OUT_PATH = pathlib.Path('BananBuild')
RELEASE_PATH = pathlib.Path('BananBuild/Release')
RELEASE_DEPENDENCY_PATH = pathlib.Path('BananBuild/Release/DepCache')
RELEASE_BUILDCACHE_PATH = pathlib.Path('BananBuild/Release/BuildCache')
RELEASE_BUILDCACHE_BIN_PATH = pathlib.Path('BananBuild/Release/BuildCache/bin')
RELEASE_BUILDCACHE_LIB_PATH = pathlib.Path('BananBuild/Release/DepCache/out/lib')
RELEASE_BUILDCACHE_INCLUDE_PATH = pathlib.Path('BananBuild/Release/DepCache/out/include')
DEBUG_PATH = pathlib.Path('BananBuild/Debug')
DEBUG_DEPENDENCY_PATH = pathlib.Path('BananBuild/Debug/DepCache')
DEBUG_BUILDCACHE_PATH = pathlib.Path('BananBuild/Debug/BuildCache')
DEBUG_BUILDCACHE_BIN_PATH = pathlib.Path('BananBuild/Debug/BuildCache/bin')
DEBUG_BUILDCACHE_LIB_PATH = pathlib.Path('BananBuild/Debug/DepCache/out/lib')
DEBUG_BUILDCACHE_INCLUDE_PATH = pathlib.Path('BananBuild/Debug/DepCache/out/include')

BUILD_TYPE = sys.argv[1] if len(sys.argv) > 1 else 'Debug'

DEPENDENCY_PATH = pathlib.Path('')
BUILDCACHE_PATH = pathlib.Path('')
BIN_PATH = pathlib.Path('')
LIB_PATH = pathlib.Path('')
INCLUDE_PATH = pathlib.Path('')

if BUILD_TYPE == 'Release':
    DEPENDENCY_PATH = RELEASE_DEPENDENCY_PATH
    BUILDCACHE_PATH = RELEASE_BUILDCACHE_PATH
    BIN_PATH = RELEASE_BUILDCACHE_BIN_PATH
    LIB_PATH = RELEASE_BUILDCACHE_LIB_PATH
    INCLUDE_PATH = RELEASE_BUILDCACHE_INCLUDE_PATH
elif BUILD_TYPE == 'Debug':
    DEPENDENCY_PATH = DEBUG_DEPENDENCY_PATH
    BUILDCACHE_PATH = DEBUG_BUILDCACHE_PATH
    BIN_PATH = DEBUG_BUILDCACHE_BIN_PATH
    LIB_PATH = DEBUG_BUILDCACHE_LIB_PATH
    INCLUDE_PATH = DEBUG_BUILDCACHE_INCLUDE_PATH
else:
    raise Exception('Invalid specified build type')


def safe_copy(src, dst):
    # Make sure that writing data remains in the project directory when COPYING
    cwd = pathlib.Path(os.getcwd())
    dst_abs = dst.resolve()

    if cwd not in dst_abs.parents:
        raise Exception('Attempted write to : ' + dst + ' which is outside the project directory')

    if src.is_dir():
        shutil.copytree(src, dst)
    elif src.is_file():
        shutil.copy(src, dst)
    else:
        raise Exception('Not a file or directory')


def safe_delete(pth):
    # Make sure that writing data remains in the project directory when DELETING
    cwd = pathlib.Path(os.getcwd())
    dir_abs = pth.resolve()

    if cwd not in dir_abs.parents:
        raise Exception('Attempted write to : ' + pth + ' which is outside the project directory')

    shutil.rmtree(pth)


def hash_sourcefile(path):
    with open(path, 'rb') as f:
        return hashlib.file_digest(f, 'md5').hexdigest()


def hash_directory(path):
    digest = hashlib.md5()

    for root, dirs, files in os.walk(str(path)):
        for names in files:
            file_path = os.path.join(root, names)

            # Hash the path and add to the digest to account for empty files/directories
            digest.update(hashlib.md5(file_path[len(str(path)):].encode()).digest())

            if os.path.isfile(file_path):
                with open(file_path, 'rb') as f_obj:
                    while True:
                        buf = f_obj.read(1024 * 1024)
                        if not buf:
                            break
                        digest.update(buf)

    return digest.hexdigest()


def check_source_hash(sourcefile, md5hash):
    try:
        with open(str(BUILDCACHE_PATH.joinpath(sourcefile.with_suffix('.md5'))), 'r') as hash_file:
            return hash_file.read() == md5hash
    except:
        return False


def check_dependency_hash(depname, md5hash):
    try:
        with open(DEPENDENCY_PATH.joinpath(depname + '.md5'), 'r') as hash_file:
            return hash_file.read() == md5hash
    except:
        return False


def mkdirs():
    if not OUT_PATH.exists():
        OUT_PATH.mkdir(parents=True)
    if not RELEASE_PATH.exists():
        RELEASE_PATH.mkdir(parents=True)
    if not RELEASE_DEPENDENCY_PATH.exists():
        RELEASE_DEPENDENCY_PATH.mkdir(parents=True)
    if not RELEASE_BUILDCACHE_PATH.exists():
        RELEASE_BUILDCACHE_PATH.mkdir(parents=True)
    if not RELEASE_BUILDCACHE_BIN_PATH.exists():
        RELEASE_BUILDCACHE_BIN_PATH.mkdir(parents=True)
    if not RELEASE_BUILDCACHE_LIB_PATH.exists():
        RELEASE_BUILDCACHE_LIB_PATH.mkdir(parents=True)
    if not RELEASE_BUILDCACHE_INCLUDE_PATH.exists():
        RELEASE_BUILDCACHE_INCLUDE_PATH.mkdir(parents=True)
    if not DEBUG_PATH.exists():
        DEBUG_PATH.mkdir(parents=True)
    if not DEBUG_DEPENDENCY_PATH.exists():
        DEBUG_DEPENDENCY_PATH.mkdir(parents=True)
    if not DEBUG_BUILDCACHE_PATH.exists():
        DEBUG_BUILDCACHE_PATH.mkdir(parents=True)
    if not DEBUG_BUILDCACHE_BIN_PATH.exists():
        DEBUG_BUILDCACHE_BIN_PATH.mkdir(parents=True)
    if not DEBUG_BUILDCACHE_LIB_PATH.exists():
        DEBUG_BUILDCACHE_LIB_PATH.mkdir(parents=True)
    if not DEBUG_BUILDCACHE_INCLUDE_PATH.exists():
        DEBUG_BUILDCACHE_INCLUDE_PATH.mkdir(parents=True)


def build_dependencies():
    cmake_name = 'cmake'
    if os.name == 'nt':
        cmake_name = 'cmake.exe'

    global BUILD_TYPE

    # GLM
    glm_path = pathlib.Path('third-party/glm')
    glm_hash = hash_directory(glm_path)
    if not check_dependency_hash('glm', glm_hash):
        safe_copy(glm_path.joinpath('glm'), INCLUDE_PATH.joinpath('glm'))

        with open(DEPENDENCY_PATH.joinpath('glm.md5'), 'w') as hash_file:
            hash_file.write(glm_hash)

    # STB
    stb_path = pathlib.Path('third-party/stb')
    stb_hash = hash_directory(stb_path)
    if not check_dependency_hash('stb', stb_hash):
        stb_include_path = INCLUDE_PATH.joinpath('stb/')

        if not stb_include_path.exists():
            stb_include_path.mkdir()

        for hdr in stb_path.glob('*.h'):
            safe_copy(hdr, stb_include_path.joinpath(hdr.name))

        with open(DEPENDENCY_PATH.joinpath('stb.md5'), 'w') as hash_file:
            hash_file.write(stb_hash)

    # VOLK
    volk_path = pathlib.Path('third-party/volk')
    volk_hash = hash_directory(volk_path)
    if not check_dependency_hash('volk', volk_hash):
        volk_workingdir = DEPENDENCY_PATH.joinpath('volk')
        if volk_workingdir.exists():
            safe_delete(volk_workingdir)
        safe_copy(volk_path, volk_workingdir)

        print('=' * os.get_terminal_size().columns)
        print('COMPILING VOLK')
        print('=' * os.get_terminal_size().columns)

        volk_build_dir = volk_workingdir.joinpath('build')
        volk_build_dir.mkdir()

        assert subprocess.run([cmake_name, ".."], cwd=volk_build_dir).returncode == 0
        assert subprocess.run([cmake_name, "--build", "."], cwd=volk_build_dir).returncode == 0

        if os.name == 'posix':
            safe_copy(volk_build_dir.joinpath('libvolk.a'), LIB_PATH.joinpath('libvolk.a'))
        elif os.name == 'nt':
            safe_copy(volk_build_dir.joinpath('libvolk.lib'), LIB_PATH.joinpath('libvolk.lib'))

        volk_include_path = INCLUDE_PATH.joinpath('volk/')
        if not volk_include_path.exists():
            volk_include_path.mkdir()

        safe_copy(volk_path.joinpath('volk.h'), volk_include_path.joinpath('volk.h'))

        with open(DEPENDENCY_PATH.joinpath('volk.md5'), 'w') as hash_file:
            hash_file.write(volk_hash)

    # ASSIMP
    assimp_path = pathlib.Path('third-party/assimp')
    assimp_hash = hash_directory(assimp_path)
    if not check_dependency_hash('assimp', assimp_hash):
        # rebuild if hash check fails
        assimp_workingdir = DEPENDENCY_PATH.joinpath('assimp')
        if assimp_workingdir.exists():
            safe_delete(assimp_workingdir)
        safe_copy(assimp_path, assimp_workingdir)

        print('=' * os.get_terminal_size().columns)
        print('COMPILING ASSIMP')
        print('=' * os.get_terminal_size().columns)

        assimp_build_dir = assimp_workingdir.joinpath('build')
        assimp_build_dir.mkdir()

        assert subprocess.run([cmake_name, "-DASSIMP_BUILD_ZLIB=ON", "-DASSIMP_BUILD_MINIZIP=ON", ".."],
                              cwd=assimp_build_dir).returncode == 0

        if BUILD_TYPE == 'Debug':
            assert subprocess.run([cmake_name, "--build", ".", "--config", "debug"],
                                  cwd=assimp_build_dir).returncode == 0
        elif BUILD_TYPE == 'Release':
            assert subprocess.run([cmake_name, "--build", ".", "--config", "release"],
                                  cwd=assimp_build_dir).returncode == 0

        if os.name == 'posix':
            safe_copy(assimp_build_dir.joinpath('bin/libassimp.so'), LIB_PATH.joinpath('libassimp.so'))
        elif os.name == 'nt':
            safe_copy(assimp_build_dir.joinpath('bin/libassimp.dll'), LIB_PATH.joinpath('libassimp.dll'))

        safe_copy(assimp_workingdir.joinpath('include/assimp'), INCLUDE_PATH.joinpath('assimp'))

        with open(DEPENDENCY_PATH.joinpath('assimp.md5'), 'w') as hash_file:
            hash_file.write(assimp_hash)

    # OPENEXR
    openexr_path = pathlib.Path('third-party/openexr')
    openexr_hash = hash_directory(openexr_path)
    if not check_dependency_hash('openexr', openexr_hash):
        # rebuild if hash check fails
        openexr_workingdir = DEPENDENCY_PATH.joinpath('openexr')
        if openexr_workingdir.exists():
            safe_delete(openexr_workingdir)
        safe_copy(openexr_path, openexr_workingdir)

        print('=' * os.get_terminal_size().columns)
        print('COMPILING OPENEXR')
        print('=' * os.get_terminal_size().columns)

        openexr_build_dir = openexr_workingdir.joinpath('build')
        openexr_build_dir.mkdir()

        assert subprocess.run(
            [cmake_name, "-DOPENEXR_FORCE_INTERNAL_IMATH=ON", "-DOPENEXR_FORCE_INTERNAL_DEFLATE=ON", "..",
             "--install-prefix", openexr_build_dir.resolve().joinpath("install")],
            cwd=openexr_build_dir).returncode == 0
        assert subprocess.run([cmake_name, "--build", ".", "--target", "install", "--config", BUILD_TYPE],
                              cwd=openexr_build_dir).returncode == 0

        if os.name == 'posix':
            safe_copy(openexr_build_dir.joinpath('install/lib/libImath.so'), LIB_PATH.joinpath('libImath.so'))
            safe_copy(openexr_build_dir.joinpath('install/lib/libIex.so'), LIB_PATH.joinpath('libIex.so'))
            safe_copy(openexr_build_dir.joinpath('install/lib/libIlmThread.so'), LIB_PATH.joinpath('libIlmThread.so'))
            safe_copy(openexr_build_dir.joinpath('install/lib/libOpenEXR.so'), LIB_PATH.joinpath('libOpenEXR.so'))
            safe_copy(openexr_build_dir.joinpath('install/lib/libOpenEXRCore.so'),
                      LIB_PATH.joinpath('libOpenEXRCore.so'))
            safe_copy(openexr_build_dir.joinpath('install/lib/libOpenEXRUtil.so'),
                      LIB_PATH.joinpath('libOpenEXRUtil.so'))

        elif os.name == 'nt':
            safe_copy(openexr_build_dir.joinpath('install/lib/libImath.dll'), LIB_PATH.joinpath('libImath.dll'))
            safe_copy(openexr_build_dir.joinpath('install/lib/libIex.dll'), LIB_PATH.joinpath('libIex.dll'))
            safe_copy(openexr_build_dir.joinpath('install/lib/libIlmThread.dll'), LIB_PATH.joinpath('libIlmThread.dll'))
            safe_copy(openexr_build_dir.joinpath('install/lib/libOpenEXR.dll'), LIB_PATH.joinpath('libOpenEXR.dll'))
            safe_copy(openexr_build_dir.joinpath('install/lib/libOpenEXRCore.dll'),
                      LIB_PATH.joinpath('libOpenEXRCore.dll'))
            safe_copy(openexr_build_dir.joinpath('install/lib/libOpenEXRUtil.dll'),
                      LIB_PATH.joinpath('libOpenEXRUtil.dll'))

        safe_copy(openexr_build_dir.joinpath('install/include/Imath'), INCLUDE_PATH.joinpath('Imath'))
        safe_copy(openexr_build_dir.joinpath('install/include/OpenEXR'), INCLUDE_PATH.joinpath('OpenEXR'))

        with open(DEPENDENCY_PATH.joinpath('openexr.md5'), 'w') as hash_file:
            hash_file.write(openexr_hash)

    # SDL
    sdl_path = pathlib.Path('third-party/sdl')
    sdl_hash = hash_directory(sdl_path)
    if not check_dependency_hash('sdl', sdl_hash):
        # rebuild if hash check fails
        sdl_workingdir = DEPENDENCY_PATH.joinpath('sdl')
        if sdl_workingdir.exists():
            safe_delete(sdl_workingdir)
        safe_copy(sdl_path, sdl_workingdir)

        print('=' * os.get_terminal_size().columns)
        print('COMPILING SDL')
        print('=' * os.get_terminal_size().columns)

        sdl_build_dir = sdl_workingdir.joinpath('build')
        sdl_build_dir.mkdir()

        assert subprocess.run([cmake_name, "-DCMAKE_BUILD_TYPE=" + BUILD_TYPE, ".."], cwd=sdl_build_dir).returncode == 0
        assert subprocess.run([cmake_name, "--build", "."], cwd=sdl_build_dir).returncode == 0

        if BUILD_TYPE == 'Debug':
            if os.name == 'posix':
                safe_copy(sdl_build_dir.joinpath('libSDL2-2.0d.so'), LIB_PATH.joinpath('libSDL2-2.0d.so'))
            elif os.name == 'nt':
                safe_copy(sdl_build_dir.joinpath('libSDL2-2.0d.dll'), LIB_PATH.joinpath('libSDL2-2.0d.dll'))
        elif BUILD_TYPE == 'Release':
            if os.name == 'posix':
                safe_copy(sdl_build_dir.joinpath('libSDL2-2.0.so'), LIB_PATH.joinpath('libSDL2-2.0.so'))
            elif os.name == 'nt':
                safe_copy(sdl_build_dir.joinpath('libSDL2-2.0.dll'), LIB_PATH.joinpath('libSDL2-2.0.dll'))

        safe_copy(sdl_build_dir.joinpath('include/SDL2'), INCLUDE_PATH.joinpath('SDL2'))

        with open(DEPENDENCY_PATH.joinpath('sdl.md5'), 'w') as hash_file:
            hash_file.write(sdl_hash)


def compile_file(filepath, flags, includes):
    includes.append('./')

    if not BUILDCACHE_PATH.joinpath(filepath).parent.exists():
        BUILDCACHE_PATH.joinpath(filepath).parent.mkdir(parents=True)

    if os.name == 'posix':
        includes = [x for include in includes for x in ('-I', include)]
        cmd = ['clang++', '-std=c++20', '-c'] + flags + includes + ['-o', str(BUILDCACHE_PATH.joinpath(filepath.with_suffix('.o'))), str(filepath)]
        print(' '.join(cmd))
        assert subprocess.run(cmd).returncode == 0
    elif os.name == 'nt':
        includes = [x for include in includes for x in ('/I', include)]
        cmd = ['clang-cl.exe', '/std:c++20', '/c'] + flags + includes + ['/Fo:', str(BUILDCACHE_PATH.joinpath(filepath.stem)), str(filepath)]
        print(' '.join(cmd))
        assert subprocess.run(cmd).returncode == 0


def link_target(target_name, target_type, files, flags, libs):

    if not BIN_PATH.joinpath(target_name).parent.exists():
        BIN_PATH.joinpath(target_name).parent.mkdir(parents=True)

    if os.name == 'posix':
        libs = ['-l' + lib for lib in libs]

        if target_type == 'executable':
            cmd = ['clang++'] + flags + libs + ['-L', str(LIB_PATH), '-L', str(BIN_PATH), '-o', str(BIN_PATH.joinpath(target_name))] + files
            print(' '.join(cmd))
            assert subprocess.run(cmd).returncode == 0
        elif target_type == 'library':
            cmd = ['clang++', '-shared'] + flags + libs + ['-L', str(LIB_PATH), '-L', str(BIN_PATH), '-o', str(BIN_PATH.joinpath('lib' + target_name + '.so'))] + files
            print(' '.join(cmd))
            assert subprocess.run(cmd).returncode == 0
        else:
            raise Exception('Invalid target type')
    elif os.name == 'nt':
        libs = ['lib' + lib + '.dll' for lib in libs]

        if target_type == 'executable':
            cmd = ['lld-link.exe'] + flags + ['/LIBPATH:', str(LIB_PATH), '/LIBPATH:', str(BIN_PATH), '/OUT:', str(BIN_PATH.joinpath(target_name + '.exe'))] + files + libs
            print(' '.join(cmd))
            assert subprocess.run(cmd).returncode == 0
        elif target_type == 'library':
            cmd = ['lld-link.exe', '/DLL'] + flags + ['/LIBPATH:', str(LIB_PATH), '/LIBPATH:', str(BIN_PATH), '/OUT:', str(BIN_PATH.joinpath('lib' + target_name + '.dll'))] + files + libs
            print(' '.join(cmd))
            assert subprocess.run(cmd).returncode == 0
        else:
            raise Exception('Invalid target type')


def build_banan():

    # Build BananStlExt

    BananStlExtChanges = False
    for src in pathlib.Path('stlext').glob('*.cpp'):
        hash = hash_sourcefile(src)
        if not check_source_hash(src, hash):
            flags = ['-fcf-protection', '-fstack-protector', '-fsanitize=address']
            if (os.name == 'posix'):
                flags.extend(['-fPIC', '-Wall', '-Wextra'])
                if BUILD_TYPE == 'Debug':
                    flags.extend(['-g', '-DNDEBUG'])
                elif BUILD_TYPE == 'Release':
                    flags.extend(['-Werror', '-O3'])
            elif os.name == 'nt':
                flags.append('/W4')
                if BUILD_TYPE == 'Debug':
                    flags.extend(['/Zi', '/DNDEBUG'])
                elif BUILD_TYPE == 'Release':
                    flags.extend(['/WX', '/O2'])

            compile_file(src, flags, [])

            with open(str(BUILDCACHE_PATH.joinpath(src.with_suffix('.md5'))), 'w') as hash_file:
                hash_file.write(hash)

            BananStlExtChanges = True

    if BananStlExtChanges:
        flags = ['-fsanitize=address']
        if os.name == 'nt' and BUILD_TYPE == 'Debug':
            flags.extend(['/DEBUG', '/PDB'])

        files = [str(BUILDCACHE_PATH.joinpath(src.with_suffix('.o'))) for src in pathlib.Path('stlext').glob('*.cpp')]
        link_target('BananStlExt', 'library', files, flags, [])

    # Build BananEngineCore

    BananEngineCoreChanges = False
    for src in pathlib.Path('core').glob('*.cpp'):
        hash = hash_sourcefile(src)
        if not check_source_hash(src, hash):
            includes = [str(INCLUDE_PATH.joinpath('assimp')), str(INCLUDE_PATH.joinpath('glm')), str(INCLUDE_PATH.joinpath('Imath')), str(INCLUDE_PATH.joinpath('OpenEXR')), str(INCLUDE_PATH.joinpath('SDL2')), str(INCLUDE_PATH.joinpath('stb')), str(INCLUDE_PATH.joinpath('volk'))]
            flags = ['-fcf-protection', '-fstack-protector', '-fsanitize=address']
            if (os.name == 'posix'):
                flags.extend(['-fPIC', '-Wall', '-Wextra'])
                if BUILD_TYPE == 'Debug':
                    flags.extend(['-g', '-DNDEBUG'])
                elif BUILD_TYPE == 'Release':
                    flags.extend(['-Werror', '-O3'])
            elif os.name == 'nt':
                flags.append('/W4')
                if BUILD_TYPE == 'Debug':
                    flags.extend(['/Zi', '/DNDEBUG'])
                elif BUILD_TYPE == 'Release':
                    flags.extend(['/WX', '/O2'])

            compile_file(src, flags, includes)

            with open(str(BUILDCACHE_PATH.joinpath(src.with_suffix('.md5'))), 'w') as hash_file:
                hash_file.write(hash)

            BananEngineCoreChanges = True

    if BananEngineCoreChanges:
        flags = ['-fsanitize=address']
        if os.name == 'nt' and BUILD_TYPE == 'Debug':
            flags.extend(['/DEBUG', '/PDB'])

        files = [str(BUILDCACHE_PATH.joinpath(src.with_suffix('.o'))) for src in pathlib.Path('core').glob('*.cpp')]
        link_target('BananEngineCore', 'library', files, flags, ['assimp', 'OpenEXR', 'SDL2-2.0', 'volk', 'BananStlExt'])

    # Build BananEngineTest

    BananEngineTestChanges = False
    for src in pathlib.Path('test').rglob('*.cpp'):
        hash = hash_sourcefile(src)
        if not check_source_hash(src, hash):
            includes = [str(INCLUDE_PATH.joinpath('assimp')), str(INCLUDE_PATH.joinpath('glm')), str(INCLUDE_PATH.joinpath('Imath')), str(INCLUDE_PATH.joinpath('OpenEXR')), str(INCLUDE_PATH.joinpath('SDL2')), str(INCLUDE_PATH.joinpath('stb')), str(INCLUDE_PATH.joinpath('volk'))]
            flags = ['-fcf-protection', '-fstack-protector', '-fsanitize=address']
            if (os.name == 'posix'):
                flags.extend(['-fPIC', '-Wall', '-Wextra'])
                if BUILD_TYPE == 'Debug':
                    flags.extend(['-g', '-DNDEBUG'])
                elif BUILD_TYPE == 'Release':
                    flags.extend(['-Werror', '-O3'])
            elif os.name == 'nt':
                flags.append('/W4')
                if BUILD_TYPE == 'Debug':
                    flags.extend(['/Zi', '/DNDEBUG'])
                elif BUILD_TYPE == 'Release':
                    flags.extend(['/WX', '/O2'])

            compile_file(src, flags, includes)

            with open(str(BUILDCACHE_PATH.joinpath(src.with_suffix('.md5'))), 'w') as hash_file:
                hash_file.write(hash)

            BananEngineTestChanges = True

    if BananEngineTestChanges:
        flags = ['-fsanitize=address']
        if os.name == 'nt' and BUILD_TYPE == 'Debug':
            flags.extend(['/DEBUG', '/PDB'])

        files = [str(BUILDCACHE_PATH.joinpath(src.with_suffix('.o'))) for src in pathlib.Path('test').rglob('*.cpp')]
        link_target('BananEngineTest', 'executable', files, flags, ['BananEngineCore', 'BananStlExt', 'SDL2-2.0', 'volk'])

if __name__ == '__main__':
    mkdirs()
    build_dependencies()
    build_banan()
