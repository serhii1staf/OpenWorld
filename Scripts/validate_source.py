#!/usr/bin/env python3
"""Static repository integrity only. Does NOT compile or validate UE APIs."""
import json
import pathlib
import re
import sys

ROOT = pathlib.Path(__file__).resolve().parents[1]
MODULE = ROOT / 'Source' / 'OpenWorld'


def validate(root=ROOT):
    errors = []
    module = root / 'Source' / 'OpenWorld'
    for name in ('OpenWorld.uproject', 'ContentManifest.json'):
        try:
            json.loads((root / name).read_text())
        except (OSError, ValueError) as exc:
            errors.append(f'{name}: {exc}')
    for path in sorted(module.rglob('*')):
        if path.suffix not in ('.h', '.cpp'):
            continue
        text = path.read_text()
        includes = re.findall(r'^\s*#include\s+"([^"]+)"', text, re.M)
        for inc in includes:
            if pathlib.Path(inc).name.startswith('OW') and not inc.endswith('.generated.h'):
                if not (module / inc).exists() and not (path.parent / inc).exists():
                    errors.append(f'{path.relative_to(root)}: unresolved project include {inc}')
        if path.suffix == '.h' and re.search(r'\b(?:UCLASS|USTRUCT|UENUM|UINTERFACE)\s*\(', text):
            expected = path.stem + '.generated.h'
            if not includes or includes[-1] != expected:
                errors.append(f'{path.relative_to(root)}: generated header must be the last include')
        if text.count('{') != text.count('}'):
            errors.append(f'{path.relative_to(root)}: unbalanced braces')
    suspicious = re.compile(r'(?:gh[pousr]_[A-Za-z0-9]{25,}|github_pat_[A-Za-z0-9_]{30,}|-----BEGIN (?:RSA |OPENSSH )?PRIVATE KEY-----)')
    for folder in ('Source', 'Scripts', 'Config', '.github', 'docs', 'Tests'):
        for path in (root / folder).rglob('*'):
            if path.is_file() and path.suffix in ('.h', '.cpp', '.cs', '.ini', '.py', '.ps1', '.md', '.yml', '.json'):
                if suspicious.search(path.read_text()):
                    errors.append(f'{path.relative_to(root)}: possible credential (value suppressed)')
    return errors


if __name__ == '__main__':
    problems = validate()
    for problem in problems:
        print('ERROR:', problem)
    print(f'Source integrity: {len(problems)} errors. Unreal compilation/runtime NOT tested.')
    sys.exit(bool(problems))
