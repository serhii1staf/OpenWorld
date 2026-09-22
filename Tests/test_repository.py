import importlib.util
import json
from pathlib import Path
import re
import unittest

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('validate_source', ROOT / 'Scripts/validate_source.py')
validator = importlib.util.module_from_spec(spec)
spec.loader.exec_module(validator)


class RepositoryIntegrity(unittest.TestCase):
    def test_static_integrity(self):
        self.assertEqual(validator.validate(), [])

    def test_engine_version_and_module(self):
        project = json.loads((ROOT / 'OpenWorld.uproject').read_text())
        self.assertEqual(project['EngineAssociation'], '5.6')
        self.assertEqual(project['Modules'][0]['Name'], 'OpenWorld')
        self.assertTrue((ROOT / 'Source/OpenWorld/OpenWorld.Build.cs').exists())

    def test_configured_native_classes_exist(self):
        cfg = ''.join(p.read_text() for p in (ROOT / 'Config').glob('*.ini'))
        headers = '\n'.join(p.read_text() for p in (ROOT / 'Source').rglob('*.h'))
        for class_name in re.findall(r'/Script/OpenWorld\.(\w+)', cfg):
            self.assertRegex(headers, r'\b[AU]' + class_name + r'\b')

    def test_generated_headers_last(self):
        for path in (ROOT / 'Source').rglob('*.h'):
            includes = re.findall(r'#include\s+"([^"]+)"', path.read_text())
            if any('.generated.h' in inc for inc in includes):
                self.assertEqual(includes[-1], path.stem + '.generated.h')

    def test_release_gate_not_bypassed(self):
        manifest = json.loads((ROOT / 'ContentManifest.json').read_text())
        self.assertIn(manifest['status'], ('content-required', 'integration-ready'))
        if not manifest['assets']:
            self.assertEqual(manifest['status'], 'content-required')
        self.assertIn('ValidateContent.ps1', (ROOT / 'Scripts/Build.ps1').read_text())

    def test_self_hosted_only_manual(self):
        workflow = (ROOT / '.github/workflows/windows.yml').read_text()
        self.assertIn('workflow_dispatch:', workflow)
        self.assertNotIn('pull_request', workflow)
        self.assertNotIn('pull_request_target', workflow)
        self.assertIn('environment: windows-release', workflow)
        self.assertIn('--draft --prerelease', workflow)

    def test_native_tests_present_not_claimed_executed(self):
        native = (ROOT / 'Source/OpenWorld/Tests/OWSystemTests.cpp').read_text()
        self.assertEqual(native.count('IMPLEMENT_SIMPLE_AUTOMATION_TEST('), 4)
        self.assertIn('index.json', (ROOT / 'Scripts/Test.ps1').read_text())

    def test_no_fake_engine_assets(self):
        for path in (ROOT / 'Content').rglob('*'):
            if path.suffix in ('.umap', '.uasset'):
                header = path.read_bytes()[:80]
                self.assertTrue(header.startswith(b'\xc1\x83\x2a\x9e') or header.startswith(b'version https://git-lfs.github.com/spec/v1'), str(path))

    def test_all_native_target_types_exist(self):
        for target, kind in [('OpenWorld', 'Game'), ('OpenWorldEditor', 'Editor'), ('OpenWorldServer', 'Server')]:
            self.assertIn('TargetType.' + kind, (ROOT / 'Source' / (target + '.Target.cs')).read_text())

    def test_license_scope(self):
        self.assertTrue((ROOT / 'LICENSE').exists())
        self.assertTrue((ROOT / 'docs/CONTENT.md').exists())


if __name__ == '__main__':
    unittest.main()
