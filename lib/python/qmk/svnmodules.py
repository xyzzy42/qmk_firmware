"""Functions for working with QMK's SVN checkouts.
"""
import json
import shutil
from pathlib import Path

from milc import cli


def _load_config():
    """Load .svnmodules.json from the QMK root.

    Returns a dict mapping paths to their SVN config (url, revision).
    """
    config_path = Path('.svnmodules.json')
    if not config_path.exists():
        return {}

    with open(config_path, 'r') as f:
        return json.load(f)


def is_svn_available():
    """Check if svn is available on PATH.
    """
    return shutil.which('svn') is not None


def status():
    """Returns a dictionary of SVN checkouts.

    Each entry is a dict of the form:

        {
            'name': 'checkout_path',
            'status': None/False/True,
            'revision': '<svn revision number>'
        }

    status is None when the checkout doesn't exist, True when it's checked out.
    (False is reserved for future use, e.g. revision mismatch.)
    """
    config = _load_config()
    checkouts = {}

    for path, info in config.items():
        entry = {'name': path, 'status': None, 'revision': None}
        checkout_dir = Path(path)

        if checkout_dir.exists() and (checkout_dir / '.svn').exists():
            result = cli.run(['svn', 'info', '--show-item', 'revision', str(checkout_dir)], timeout=30)
            if result.returncode == 0 and result.stdout.strip():
                entry['revision'] = result.stdout.strip()
                if info.get('revision') and entry['revision'] != str(info['revision']):
                    entry['status'] = False
                else:
                    entry['status'] = True
            else:
                entry['status'] = False
        checkouts[path] = entry

    return checkouts


def update(paths=None):
    """Update SVN checkouts.

        paths
            A string containing a single path or a list of paths.
            If None, update all configured SVN checkouts.
    """
    config = _load_config()

    if paths is None:
        paths = list(config.keys())
    elif isinstance(paths, str):
        paths = [paths]

    for path in paths:
        if path not in config:
            cli.log.warning('SVN checkout %s not found in .svnmodules.json', path)
            continue

        info = config[path]
        checkout_dir = Path(path)
        url = info['url']
        revision = info.get('revision')

        if checkout_dir.exists() and (checkout_dir / '.svn').exists():
            # Update existing checkout
            cmd = ['svn', 'update', str(checkout_dir)]
            if revision:
                cmd.extend(['-r', str(revision)])
            cli.run(cmd, capture_output=False, check=True)
        else:
            # Fresh checkout
            if checkout_dir.exists():
                shutil.rmtree(checkout_dir)
            cmd = ['svn', 'checkout', url, str(checkout_dir)]
            if revision:
                cmd.extend(['-r', str(revision)])
            cli.run(cmd, capture_output=False, check=True)
