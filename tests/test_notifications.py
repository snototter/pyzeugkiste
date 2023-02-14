import datetime
import pytest
from pyzeugkiste import notifications as pnot
from pathlib import Path


def test_size_str():
    fn = Path(__file__).parent.resolve() / 'data' / 'test-invalid.toml'
    assert 14 == pnot.file_size(fn)

    assert '3B' == pnot.byte_size_str(3, '')
    assert '1.0 KB' == pnot.byte_size_str(1024)
    assert '1.5_KB' == pnot.byte_size_str(1536, '_')

    bsz = 1024 * 1024
    assert '1.0 MB' == pnot.byte_size_str(bsz, ' ')
    bsz *= 1024
    assert '1.0 GB' == pnot.byte_size_str(bsz, ' ')
    bsz *= 1024
    assert '1.0 TB' == pnot.byte_size_str(bsz, ' ')
    tb1 = bsz
    bsz *= 1024
    assert '1024.0 TB' == pnot.byte_size_str(bsz, ' ')
    bsz += tb1
    assert '1025.0 TB' == pnot.byte_size_str(bsz, ' ')


def test_syslog():
    print(pnot.query_systemd_status())

    print(pnot.query_syslog(unit='unknown-unit'))
    assert pnot.query_systemd_status(unit='unknown').strip() == 'Unit unknown.service could not be found.'
    assert False
