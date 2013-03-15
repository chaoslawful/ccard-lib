AddOption('--prefix',
        dest='prefix',
        nargs=1, type='string',
        action='store',
        metavar='DIR',
        help='Installation prefix')

# build C source
SConscript('src/SConscript')

#############################################################
# swig for php make and install
if "install-php" in COMMAND_LINE_TARGETS:
    SConscript('ext/SConscript')

#############################################################
# build unit-test
SConscript('t/SConscript')

# vi:ft=python ts=4 sw=4 et fdm=marker
