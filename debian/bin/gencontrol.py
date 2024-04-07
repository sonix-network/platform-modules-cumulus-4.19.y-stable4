#!/usr/bin/env python

import sys
sys.path.append(sys.argv[1] + "/lib/python")

from debian_linux.config import ConfigCoreDump
from debian_linux.debian import Changelog, PackageDescription, VersionLinux
from debian_linux.gencontrol import Gencontrol as Base
from debian_linux.utils import Templates

import os.path, re, codecs

class Gencontrol(Base):
    def __init__(self, config):
        super(Gencontrol, self).__init__(ConfigCoreDump(fp = open(config, "rb")), Templates(["debian/templates"]))

        config_entry = self.config['version',]
        self.version = VersionLinux(config_entry['source'])
        self.abiname = config_entry['abiname']
        self.vars = {
            'upstreamversion': self.version.linux_upstream,
            'version': self.version.linux_version,
            'source_upstream': self.version.upstream,
            'abiname': self.abiname,
        }
        changelog_version = Changelog()[0].version
        self.package_version = '%s' % (changelog_version.complete)

    def _setup_makeflags(self, names, makeflags, data):
        for src, dst, optional in names:
            if src in data or not optional:
                makeflags[dst] = data[src]

    def do_main_setup(self, vars, makeflags, extra):
        super(Gencontrol, self).do_main_setup(vars, makeflags, extra)
        makeflags.update({
            'VERSION': self.version.linux_version,
            'UPSTREAMVERSION': self.version.linux_upstream,
            'ABINAME': self.abiname,
            'SOURCEVERSION': self.version.complete,
            'GENCONTROL_ARGS': '-v%s' % self.package_version,
        })

    def do_main_makefile(self, makefile, makeflags, extra):
        fs_enabled = [featureset
                      for featureset in self.config['base', ]['featuresets']
                      if self.config.merge('base', None, featureset).get('enabled', True)]
        for featureset in fs_enabled:
            makeflags_featureset = makeflags.copy()
            makeflags_featureset['FEATURESET'] = featureset

        triplet_enabled = []
        for arch in iter(self.config['base', ]['arches']):
            for featureset in self.config['base', arch].get('featuresets', ()):
                if self.config.merge('base', None, featureset).get('enabled', True):
                    for flavour in self.config['base', arch, featureset]['flavours']:
                        triplet_enabled.append('%s_%s_%s' %
                                               (arch, featureset, flavour))

        makeflags = makeflags.copy()
        makeflags['ALL_FEATURESETS'] = ' '.join(fs_enabled)
        makeflags['ALL_TRIPLETS'] = ' '.join(triplet_enabled)
        super(Gencontrol, self).do_main_makefile(makefile, makeflags, extra)

    def do_main_packages(self, packages, vars, makeflags, extra):
        packages['source']['Build-Depends'].extend(
            ['linux-support-%s' % self.abiname,
             # We don't need this installed, but it ensures that after an
             # ABI bump linux is auto-built before linux-latest on each
             # architecture.
             'linux-headers-%s-all' % self.abiname]
        )

    arch_makeflags = (
        ('kernel-arch', 'KERNEL_ARCH', False),
    )

    def do_arch_setup(self, vars, makeflags, arch, extra):
        config_base = self.config.merge('base', arch)
        self._setup_makeflags(self.arch_makeflags, makeflags, config_base)

    def do_featureset_setup(self, vars, makeflags, arch, featureset, extra):
        config_base = self.config.merge('base', arch, featureset)

    flavour_makeflags_base = (
        ('compiler', 'COMPILER', False),
        ('kernel-arch', 'KERNEL_ARCH', False),
        ('cflags', 'CFLAGS_KERNEL', True),
        ('override-host-type', 'OVERRIDE_HOST_TYPE', True),
    )

    flavour_makeflags_build = (
        ('image-file', 'IMAGE_FILE', True),
    )

    flavour_makeflags_image = (
        ('type', 'TYPE', False),
        ('install-stem', 'IMAGE_INSTALL_STEM', True),
    )

    flavour_makeflags_other = (
        ('localversion', 'LOCALVERSION', False),
        ('localversion-image', 'LOCALVERSION_IMAGE', True),
    )

    def do_flavour_setup(self, vars, makeflags, arch, featureset, flavour, extra):
        config_base = self.config.merge('base', arch, featureset, flavour)
        config_build = self.config.merge('build', arch, featureset, flavour)
        config_image = self.config.merge('image', arch, featureset, flavour)

        vars['flavour'] = vars['localversion'][1:]

        vars['localversion-image'] = vars['localversion']
        override_localversion = config_image.get('override-localversion', None)
        if override_localversion is not None:
            vars['localversion-image'] = vars['localversion_headers'] + '-' + override_localversion

        self._setup_makeflags(self.flavour_makeflags_base, makeflags, config_base)
        self._setup_makeflags(self.flavour_makeflags_build, makeflags, config_build)
        self._setup_makeflags(self.flavour_makeflags_image, makeflags, config_image)
        self._setup_makeflags(self.flavour_makeflags_other, makeflags, vars)

    def do_flavour_packages(self, packages, makefile, arch, featureset, flavour, vars, makeflags, extra):
        if self.version.linux_modifier is None:
            try:
                vars['abiname'] = '-%s' % self.config['abi', arch]['abiname']
            except KeyError:
                vars['abiname'] = self.abiname
            makeflags['ABINAME'] = vars['abiname']

        config_description = self.config.merge('description', arch, featureset, flavour)
        vars['class'] = config_description['hardware']
        vars['longclass'] = config_description.get('hardware-long') or vars['class']

        templates = []
        templates.extend(self.templates["control.image"])
        if self.config.get_merge('build', arch, featureset, flavour,
                                 'debug-info', False):
            templates.extend(self.templates["control.image-dbg"])
            makeflags['DEBUG'] = True
        image_fields = {'Description': PackageDescription()}

        desc_parts = self.config.get_merge('description', arch, featureset, flavour, 'parts')
        if desc_parts:
            # XXX: Workaround, we need to support multiple entries of the same name
            parts = list(set(desc_parts))
            parts.sort()
            desc = image_fields['Description']
            for part in parts:
                desc.append(config_description['part-long-' + part])
                desc.append_short(config_description.get('part-short-' + part, ''))

        packages_dummy = []
        packages_dummy.append(self.process_real_image(templates[0], image_fields, vars))
        packages_dummy.extend(self.process_packages(templates[1:], vars))
        self.merge_packages(packages, packages_dummy, arch)

        cmds_binary_arch = ["$(MAKE) -f debian/rules.real binary-arch-flavour %s" % makeflags]
        cmds_build = ["$(MAKE) -f debian/rules.real build-arch %s" % makeflags]
        cmds_setup = ["$(MAKE) -f debian/rules.real setup-flavour %s" % makeflags]
        makefile.add('binary-arch_%s_%s_%s_real' % (arch, featureset, flavour), cmds=cmds_binary_arch)
        makefile.add('build-arch_%s_%s_%s_real' % (arch, featureset, flavour), cmds=cmds_build)
        makefile.add('setup_%s_%s_%s_real' % (arch, featureset, flavour), cmds=cmds_setup)

        # Substitute kernel version etc. into maintainer scripts,
        # translations and lintian overrides
        def substitute_file(template, target, append=False):
            with codecs.open(target, 'a' if append else 'w',
                            'utf-8') as f:
                f.write(self.substitute(self.templates[template], vars))
        for name in ['postinst', 'postrm', 'triggers']:
            substitute_file('image.%s' % name,
                            'debian/platform-modules-%s%s.%s' %
                            (vars['abiname'], vars['localversion'], name))

    def merge_packages(self, packages, new, arch):
        for new_package in new:
            name = new_package['Package']
            if name in packages:
                package = packages.get(name)
                package['Architecture'].add(arch)

                for field in 'Depends', 'Provides', 'Suggests', 'Recommends', 'Conflicts':
                    if field in new_package:
                        if field in package:
                            v = package[field]
                            v.extend(new_package[field])
                        else:
                            package[field] = new_package[field]

            else:
                new_package['Architecture'] = arch
                packages.append(new_package)

    def process_real_image(self, entry, fields, vars):
        entry = self.process_package(entry, vars)
        for key, value in fields.items():
            if key in entry:
                real = entry[key]
                real.extend(value)
            elif value:
                entry[key] = value
        return entry

if __name__ == '__main__':
    Gencontrol(sys.argv[1] + "/config.defines.dump")()
