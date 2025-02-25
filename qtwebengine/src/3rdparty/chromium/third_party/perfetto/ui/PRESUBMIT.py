# Copyright (C) 2018 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

from __future__ import print_function
import time
import subprocess
from os.path import relpath, dirname, join


USE_PYTHON3 = True


def RunAndReportIfLong(func, *args, **kargs):
  start = time.time()
  results = func(*args, **kargs)
  end = time.time()
  limit = 0.5  # seconds
  name = func.__name__
  runtime = end - start
  if runtime > limit:
    print("{} took >{:.2}s ({:.2}s)".format(name, limit, runtime))
  return results


def CheckChange(input, output):
  results = []
  results += RunAndReportIfLong(CheckEslint, input, output)
  results += RunAndReportIfLong(CheckImports, input, output)
  results += RunAndReportIfLong(CheckAnyRachet, input, output)
  return results


def CheckChangeOnUpload(input_api, output_api):
  return CheckChange(input_api, output_api)


def CheckChangeOnCommit(input_api, output_api):
  return CheckChange(input_api, output_api)


def CheckEslint(input_api, output_api):
  path = input_api.os_path
  ui_path = input_api.PresubmitLocalPath()
  module_path = path.join(ui_path, 'node_modules', '.bin', 'eslint')
  lint_path = path.join(ui_path, 'eslint')

  if not path.exists(module_path):
    repo_root = input_api.change.RepositoryRoot()
    install_path = path.join(repo_root, 'tools', 'install-build-deps')
    return [
        output_api.PresubmitError(
            f"eslint not found. Please first run\n $ {install_path} --ui")
    ]

  def file_filter(x):
    return input_api.FilterSourceFile(
        x,
        files_to_check=[r'.*\.ts$', r'.*\.js$'],
        files_to_skip=[r'.*\.eslintrc\.js'])

  files = input_api.AffectedSourceFiles(file_filter)

  if not files:
    return []
  paths = [f.AbsoluteLocalPath() for f in files]

  cmd = [lint_path] + paths
  if subprocess.call(cmd):
    s = ' '.join(cmd)
    return [output_api.PresubmitError(f"eslint errors. Run: $ {s}")]
  return []


def CheckImports(input_api, output_api):
  path = input_api.os_path
  ui_path = input_api.PresubmitLocalPath()
  check_imports_path = join(dirname(ui_path), 'tools', 'check_imports')

  def file_filter(x):
    return input_api.FilterSourceFile(
        x, files_to_check=[r'.*\.ts$', r'.*\.js$'])

  files = input_api.AffectedSourceFiles(file_filter)

  if not files:
    return []

  if subprocess.call([check_imports_path]):
    return [output_api.PresubmitError(f"")]
  return []


def CheckAnyRachet(input_api, output_api):
  path = input_api.os_path
  ui_path = input_api.PresubmitLocalPath()
  check_ratchet_path = join(dirname(ui_path), 'tools', 'check_ratchet')

  def file_filter(x):
    return input_api.FilterSourceFile(x, files_to_check=[r'.*\.ts$'])

  files = input_api.AffectedSourceFiles(file_filter)

  if not files:
    return []

  if subprocess.call([check_ratchet_path]):
    return [output_api.PresubmitError(f"")]
  return []
