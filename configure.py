#!/usr/bin/env python3

import argparse
import os
import platform

cflags  = "-g -Wall -Wextra -Wpedantic -Werror -fPIC "
cflags += "-Wno-gnu-zero-variadic-macro-arguments "
cflags += "-Wno-unused-parameter "
cflags += "-std=c11 -fcolor-diagnostics "

plat = platform.system()
if plat == 'Darwin':
  cflags += '-DPLATFORM_DARWIN '

ninja_vars = {
  "builddir" : "build",
  "cc"       : "clang",
  "cflags"   : cflags,
  "ldflags"  : "-L$builddir",
}

ninjafile_base = """
rule cc
  command = $cc -MMD -MT $out -MF $out.d $cflags -c $in -o $out
  description = CC $out
  depfile = $out.d
  deps = gcc

rule link
  command = $cc $ldflags -o $out $in
  description = LINK $out

rule shlib
  command = $cc $ldflags -shared -o $out $in
  description = SHLIB $out

"""

def get_san_flags(desc):
  if desc is None:
    return ""

  flags = ""
  sans = desc.split(',')
  for san in sans:
    flags += " -fsanitize=%s" % san
    if san == "undefined":
      flags += " -fno-sanitize-recover=undefined"
  return flags

def strip_src_ext(src_file):
  parts = src_file.split(".")
  if len(parts) != 2 or parts[1] not in ["c","m"]:
    return None
  return parts[0], parts[1]

class BuildEnv:
  def __init__(self, vars):
    self.vars = vars
    self.progs = []
    self.objs = []
    self.obj_ext_map = {}
    self.shared_libs = []

  def IsWindows(self):
    return os.name == 'nt'

  def IsDarwin(self):
    return platform.system() == 'Darwin'

  def src_helper(self, src):
    objects = []
    for f in src:
      obj_name, ext = strip_src_ext(f)
      objects.append(obj_name)
      if obj_name not in self.objs:
        self.objs.append(obj_name)
        self.obj_ext_map[obj_name] = ext
    return objects

  def Program(self, name, src, **kwargs):
    objects = self.src_helper(src)
    self.progs.append((name, objects, kwargs))

  def SharedLibrary(self, name, src):
    objects = self.src_helper(src)
    self.shared_libs.append((name, objects))

  def Test(self, name, src):
    return self.Program("test/%s" % name, src + ["test.c"])

  def write_ninja(self, fp):
    fp.write("# auto-generated by configure.py\n")

    for k,v in self.vars.items():
      fp.write("%s = %s\n" % (k,v))

    fp.write(ninjafile_base)

    fp.write("# objects\n")
    for obj in self.objs:
      ext = self.obj_ext_map[obj]
      fp.write("build $builddir/%s.o: cc %s.%s\n" % (obj, obj, ext))

    fp.write("\n# executables\n")
    for (name, objs, props) in self.progs:
      obj_line = " ".join(map(lambda x: "$builddir/%s.o" % x, objs))
      fw_part = ""
      if "frameworks" in props:
        fws = props["frameworks"]
        fw_part += "\n  ldflags = $ldflags"
        fw_part += "".join(map(lambda x: " -framework %s" % x, fws))

      ext = ".exe" if self.IsWindows() else ""
      fp.write("build $builddir/%s%s: link %s %s\n" % (name, ext, obj_line, fw_part))

    fp.write("\n# shared libraries\n")
    for (name, objs) in self.shared_libs:
      obj_line = " ".join(map(lambda x: "$builddir/%s.o" % x, objs))
      fp.write("build $builddir/%s: shlib %s\n" % (name, obj_line))


if __name__ == '__main__':
  parser = argparse.ArgumentParser()
  parser.add_argument('--sanitizers', '--san', dest='sanitizers', default=None)
  parser.add_argument('--config', default='debug')
  args = parser.parse_args()
  san_flags = get_san_flags(args.sanitizers)
  ninja_vars["cflags"] += san_flags
  ninja_vars["ldflags"] += san_flags

  if args.config == "release":
    ninja_vars["cflags"] += ' -O3'
  else:
    assert args.config == "debug", \
        "Invalid config %s" % args.config

  env = BuildEnv(ninja_vars)
  env.Program('bfi_demo', ['bfi_demo.c', 'bf_interpreter.c'])
  env.Test('test_bf_interpreter',
      ['test_bf_interpreter.c',
       'byte_buffer.c','bf_interpreter.c'])

  with open("build.ninja", "w") as f:
    env.write_ninja(f)
