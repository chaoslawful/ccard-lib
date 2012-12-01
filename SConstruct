import os

def SymLink(target, source, env):
    os.symlink(os.path.abspath(str(source[0])), os.path.abspath(str(target[0])))

ver = '0.1'
name = 'ccard-lib'
libdir = '/usr/local/lib/'

env = Environment(
        #CC = "clang",
        CCFLAGS = ["-Wall", "-Wextra", "-Werror", "-g3", "-Iinclude/", "-std=c99"],
        )

srcs = Glob("src/*.c")
libname = name + '.' + ver
slname = env.subst('$LIBPREFIX') + name + env.subst('$LIBSUFFIX')
dlname = env.subst('$SHLIBPREFIX') + name + env.subst('$SHLIBSUFFIX')

sl = env.StaticLibrary(libname, srcs)
dl = env.SharedLibrary(libname, srcs)

env.Install(libdir, [sl, dl])

# make version-less link to static and shared libraries
env.Command(libdir+slname, libdir+str(sl[0]), SymLink)
env.Command(libdir+dlname, libdir+str(dl[0]), SymLink)

# make 'install' the alias of library directory target
env.Alias('install', libdir)

#############################################################
# swig for php make and install
swigEnv = Environment(SWIGFLAGS=['-php'],
        SHLIBPREFIX="")

import commands
# php inlcudes
phpconfig = commands.getoutput("php-config --includes")
phpLibExtDir = commands.getoutput("php-config --extension-dir")

swigSl = swigEnv.SharedLibrary(
        'php_card.so', 
        ['ext/php_ccard.i'],  
        LIBS=['ccard-lib'],
        CCFLAGS = ["-I./include", phpconfig.split()])

def silentRemoveFile(file):
    try:
        os.remove(file);
    except:
        pass

# Remove useless files during clean
if swigEnv.GetOption('clean'):
    topdir = env.Dir("#").abspath
    silentRemoveFile(os.path.join(topdir, "ext/ccard.php"));
    silentRemoveFile(os.path.join(topdir, "ext/php_ccard.h"));

swigEnv.Install(phpLibExtDir, [swigSl])
swigEnv.Alias('install-php', phpLibExtDir)

# vi:ft=python ts=4 sw=4 et fdm=marker

