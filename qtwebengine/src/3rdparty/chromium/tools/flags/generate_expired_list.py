#!/usr/bin/env python
# Copyright 2019 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Generates the list of expired flags as a C++ source file.

This program generates a data structure representing the set of flags that
expired before or as of the current Chromium milestone. Specifically, it reads
the flag metadata JSON file and emits a structure mapping flag internal names to
expiration milestones. This data structure is then linked into the built
Chromium, to be used to decide whether to show or hide a given flag in the flags
UI.

This program can be run with no arguments to run its own unit tests.
"""

from __future__ import print_function

import utils
import os
import sys


ROOT_PATH = os.path.join(os.path.dirname(__file__), '..', '..')


def get_chromium_version():
  """Parses the Chromium version out of //chrome/VERSION."""
  with open(os.path.join(ROOT_PATH, 'chrome', 'VERSION'),
            encoding='utf-8') as f:
    for line in f.readlines():
      key, value = line.strip().split('=')
      if key == 'MAJOR':
        return int(value)
  return None


def gen_file_header(prog_name, meta_name):
  """Returns the header for the generated expiry list file.

  The generated header contains at least:
  * A copyright message on the first line
  * A reference to this program (prog_name)
  * A reference to the input metadata file
  >>> 'The Chromium Authors' in gen_file_header('foo', 'bar')
  True
  >>> '/progname' in gen_file_header('/progname', '/dataname')
  True
  >>> '/dataname' in gen_file_header('/progname', '/dataname')
  True
  """
  return """// Copyright 2019 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This is a generated file. Do not edit it! It was generated by:
//   {prog_name}
// and it was generated from:
//   {meta_name}

#include "chrome/browser/expired_flags_list.h"

namespace flags {{
const ExpiredFlag kExpiredFlags[] = {{
""".format(prog_name=prog_name, meta_name=meta_name)


def gen_file_footer():
  return """
  {nullptr, 0},
};

}  // namespace flags"""


def gen_file_body(flags, mstone):
  """Generates the body of the flag expiration list.

  Flags appear in the list only if:
  * Their expiration mstone is not -1
  * Either the chrome version is unknown OR
  * Their expiration mstone is <= the chrome version

  >>> flags = [ { 'name': 'foo', 'expiry_milestone': 1 } ]
  >>> flags.append({ 'name': 'bar', 'expiry_milestone': 2 })
  >>> flags.append({ 'name': 'baz', 'expiry_milestone': -1 })
  >>> gen_file_body(flags, 1)
  '  {"foo", 1},'
  >>> gen_file_body(flags, 2)
  '  {"foo", 1},\\n  {"bar", 2},'
  >>> gen_file_body(flags, None)
  '  {"foo", 1},\\n  {"bar", 2},'
  """
  if mstone != None:
    flags = utils.keep_expired_by(flags, mstone)
  output = []
  for f in flags:
    if f['expiry_milestone'] != -1:
      name, expiry = f['name'], f['expiry_milestone']
      output.append('  {"' + name + '", ' + str(expiry) + '},')
  return '\n'.join(output)


def gen_expiry_file(program_name, metadata_name):
  output = gen_file_header(program_name, metadata_name)
  output += gen_file_body(utils.load_metadata(), get_chromium_version())
  output += gen_file_footer()
  return output


def write_if_changed(filename, contents):
  """Write contents into the named file if the file's content is different.

  This avoids updating the mtime if the file's contents haven't changed,
  which helps reduce spurious rebuilds.
  """
  try:
    with open(filename, 'r', encoding='utf-8') as f:
      current = f.read()
  except FileNotFoundError:
    # If the file doesn't exist yet, we'll always need to rewrite the file.
    current = None
  if not current or contents != current:
    with open(filename, 'w', encoding='utf-8') as f:
      f.write(contents)


def main():
  import doctest
  doctest.testmod()

  if len(sys.argv) < 3:
    print('{}: only ran tests'.format(sys.argv[0]))
    return

  output = gen_expiry_file(sys.argv[0], sys.argv[1])
  write_if_changed(sys.argv[2], output)


if __name__ == '__main__':
  main()
