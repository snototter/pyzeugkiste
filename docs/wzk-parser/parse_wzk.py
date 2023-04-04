"""
Set up a virtual environment & install the python-only C++ parser:

```bash
python3.8 -m venv venv
source venv/bin/activate
python -m pip install -U pip
python -m pip install git+https://github.com/robotpy/cxxheaderparser.git
```

This script locates the werkzeugkiste header files, assuming that the
project has already been set up in development mode (i.e. werkzeugkiste
has been fetched via CMake).

```bash
python parse_wzk.py
```

"""

import argparse
import re
from pathlib import Path
import tempfile
import subprocess
import sys
import json


def __parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        '-n', '--ns', dest='namespace', default=None, type=str,
        help='Specify a single namespace to process, e.g. "config"')
    return parser.parse_args()


def __param_str(parsed_params) -> str:
    pstr = '('
    delim = False
    for p in parsed_params:
        if p['name'] is None:
            continue
        if delim:
            pstr += ', '
        pstr += p['name']
        delim = True
    pstr +=')'
    return pstr


def __process_parse_result(data: dict):
    # namespaces = data['namespace']['namespaces']['werkzeugkiste']['namespaces']
    namespaces = data['namespace']['namespaces']
    if 'werkzeugkiste' not in namespaces:
        print('--> Namespace not declared in this header')
        return
    
    # print(json.dumps(namespaces['werkzeugkiste'], sort_keys=True, indent=2))
    namespaces = namespaces['werkzeugkiste']['namespaces']
    for ns in namespaces:
#        print(namespaces[ns])

        cnts = {}
        for fx in namespaces[ns]['functions']:
            name = fx['name']['segments'][0]['name']
            params = __param_str(fx['parameters'])
            fxs = f'{name}{params}'
            if fxs in cnts:
                cnts[fxs] += 1
            else:
                cnts[fxs] = 1

        print('### Functions')
        if len(cnts) == 0:
            print('* No functions')
        else:
            for fxs in cnts:
                overload = f', {cnts[fxs]} overloads' if (cnts[fxs] > 1) else ''
                print(f'* [ ] `{fxs}`{overload}')

        print('\n### Classes')
        num_classes = 0
        for cls in namespaces[ns]['classes']:
            num_classes += 1
            cnts = {}
            name = cls['class_decl']['typename']['segments'][0]['name']
            for meth in cls['methods']:
                # Skip c'tor and destructor
                if meth['constructor'] or meth['destructor']:
                    continue
                fxname = meth['name']['segments'][0]['name']
                access = f"[{meth['access']}] " if (meth['access'] != 'public') else ''
                params = __param_str(meth['parameters'])
                fxs = f'{access}{fxname}{params}'
                if fxs in cnts:
                    cnts[fxs] += 1
                else:
                    cnts[fxs] = 1

            print(f'Class `{name}`')
            if len(cnts) == 0:
                print('* No methods')
            else:
                for fxs in cnts:
                    overload = f', {cnts[fxs]} overloads' if (cnts[fxs] > 1) else ''
                    print(f'* [ ] `{fxs}`{overload}')
                    if not fxs.startswith('['):
                        print('  * [ ] Bindings available')
                        print('  * [ ] Tested')
                        print('  * [ ] Documented')
            print()

        if num_classes == 0:
            print('* No classes')

    print('\n\n')
    


def parse_header(filename: Path):
    print(f'Header file: {filename}')
    tmp = tempfile.NamedTemporaryFile()
    # Read the whole file
    with open(filename, 'r') as infile, open(tmp.name, 'w') as outfile:
        # Strip macros which would cause cxx parser to crash
        regex = re.compile(r"WERKZEUGKISTE_.*_EXPORT")
        skip_multiple_lines = False
        for line in infile:
            line = regex.sub('', line.strip())
            if line.endswith('\\'):
                # print(f'Skipping {line}')
                skip_multiple_lines = True
            else:
                if ('WZKREG_TNSPEC' in line):
                    # Macro usage is just a single line in types.h
                    continue
                if skip_multiple_lines:
                    # The final line (w/o backslash) should also be skipped:
                    skip_multiple_lines = False
                    # print(f'Skipping {line}')
                    continue
            if skip_multiple_lines:
                continue
            outfile.write(line + '\n')

#    with open(tmp.name) as f:
#        contents = f.read()
#        print(contents)
    proc = subprocess.run([
        sys.executable, '-m', 'cxxheaderparser', '--mode', 'json', f'{tmp.name}'],
        stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    js_dump = proc.stdout.decode('utf-8')
    try:
        data = json.loads(js_dump)
    except json.decoder.JSONDecodeError as e:
        print(js_dump)
        raise e
    __process_parse_result(data)


if __name__ == '__main__':
    args = __parse_args()
    
    wzk_incdir = Path(__file__).absolute().parent.parent.parent / 'build' / '_deps' / 'werkzeugkiste-src' / 'include' / 'werkzeugkiste'
    if args.namespace is not None:
        wzk_incdir = wzk_incdir / args.namespace

    for f in list(wzk_incdir.rglob("*.h")):
        try:
            parse_header(f)
        except KeyError as e:
            print(f'--> KeyError while parsing: {e}\n\n')

