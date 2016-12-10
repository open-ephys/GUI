#
# This file is used by the "Post-build shell script" in the Xcode section of
# open-ephys.jucer.  It copies required files to the OS X application bundle
# (open-ephys.app) and updates shared library links to point to the bundled
# versions.
#

from glob import iglob
import os
from os import environ as env
import shutil
from subprocess import check_call, check_output


xcrun = '/usr/bin/xcrun'

executable = os.path.join(env['CONFIGURATION_BUILD_DIR'],
                          env['EXECUTABLE_PATH'])

frameworks_folder = os.path.join(env['CONFIGURATION_BUILD_DIR'],
                                 env['FRAMEWORKS_FOLDER_PATH'])

resources_folder = os.path.join(env['CONFIGURATION_BUILD_DIR'],
                                env['UNLOCALIZED_RESOURCES_FOLDER_PATH'])


def copy_if_needed(src, dst):
    if (not os.path.isfile(dst) or
        os.path.getmtime(src) > os.path.getmtime(dst)):
        shutil.copy(src, dst)


def change_install_name(target, old_name, new_name):
    check_call([xcrun,
               'install_name_tool',
               '-change',
               old_name,
               new_name,
               target])


# Copy Rhythm-related files to bundle Resources folder
for path in ('Bitfiles/rhd2000.bit',
             'Bitfiles/rhd2000_usb3.bit',
             'DLLs/libokFrontPanel.dylib'):
    src = os.path.join(env['PROJECT_DIR'], '../../Resources', path)
    dst = os.path.join(resources_folder, os.path.basename(src))
    copy_if_needed(src, dst)

# Create bundle Frameworks folder
if not os.path.isdir(frameworks_folder):
    os.mkdir(frameworks_folder)

# Extract paths to linked HDF5 and ZeroMQ shared libraries from bundle
# executable
local_libs = check_output([xcrun, 'dyldinfo', '-dylibs', executable])
local_libs = tuple(lib.strip() for lib in local_libs.split('\n')
                   if ('/lib/libhdf5' in lib) or ('/lib/libzmq' in lib))

# Copy shared libraries to bundle Frameworks folder
for src in local_libs:
    dst = os.path.join(frameworks_folder, os.path.basename(src))
    copy_if_needed(src, dst)

# Change install names of shared libraries in bundle executable to point to
# copies in bundle Frameworks folder
for old_name in local_libs:
    new_name = '@executable_path/../Frameworks/' + os.path.basename(old_name)
    change_install_name(executable, old_name, new_name)

# Change install names in shared libraries themselves so that inter-library
# links (e.g. from libhdf5_cpp to libhdf5) point to copies in bundle Frameworks
# folder
for dylib in iglob(frameworks_folder + '/*.dylib'):
    for old_name in local_libs:
        new_name = '@loader_path/' + os.path.basename(old_name)
        # Since we don't know whether the shared library was linked with the
        # "-headerpad_max_install_names" flag, ensure that the new install name
        # is no longer than the old one
        assert len(new_name) <= len(old_name)
        change_install_name(dylib, old_name, new_name)
