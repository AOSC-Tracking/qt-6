#!/usr/bin/env python3
# Copyright 2021 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""A utility for generating the structured metrics validator.

Takes as input a structured.xml file describing all events and produces a C++
header and implementation file exposing validator function calls.
"""

import argparse
import sys

import codegen
from sync import model

parser = argparse.ArgumentParser(
    description='Generate structured metrics validator')
parser.add_argument('--input', help='Path to structured.xml')
parser.add_argument('--output', help='Path to generated files.')


def main():
  args = parser.parse_args()
  structured = model.Model(open(args.input, encoding='utf-8').read())

  codegen.ValidatorHeaderTemplate(
      args.output, 'structured_metrics_validator.h').write_file()

  codegen.ValidatorImplTemplate(structured, args.output,
                                'structured_metrics_validator.cc').write_file()

  return 0


if __name__ == '__main__':
  sys.exit(main())
