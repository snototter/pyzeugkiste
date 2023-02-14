import datetime
import subprocess
from io import StringIO
from pathlib import Path
from sys import platform
from typing import Union

def ensure_parent_path_exists(filename: Union[Path, str]) -> Path:
    """
    TODO doc
    """
    if filename is None:
        return None

    if isinstance(filename, str):
        filename = Path(filename)

    parent = filename.parent.resolve()

    if not parent.exists():
        parent.mkdir(parents=True)

    return parent


def query_systemd_status(
        unit: str = None,
        max_lines: int = -1,
        output_log_file: Union[Path, str] = None) -> str:
  # TODO doc
    assert platform == 'linux'
    ensure_parent_path_exists(output_log_file)

    cmd = ['/bin/systemctl', 'status', '--full', '--no-pager']
    if unit is not None:
        cmd.append(unit)

    if max_lines > 0:
        cmd.append(f'-n{max_lines}')


    tgt = StringIO() if (output_log_file is None) else open(output_log_file, 'w')
    proc = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    tgt.write(proc.stdout.decode())

    if output_log_file is None:
        tgt.seek(0)
        return tgt.read()
    else:
        tgt.close()
        return None


def query_syslog(
        unit: str = None,
        since: Union[datetime.datetime, str] = None,
        until: Union[datetime.datetime, str] = None,
        output_log_file: Union[Path, str] = None) -> str:
    """
    Extracts the system log.

    TODO: if file is none -> return a string

    Collects the system log and stores it in the given file. Optionally, a
    particular system unit and time range can be specified.

    Args:
      output_log_file: Path to the output file. This file will be
        **overwritten**. If the path does not exist, it will be created with
        default permissions.
      unit: The name of the system unit to extract the log for. If not provided,
        the whole system log will be collected.
      since: TODO doc
      until: TODO doc

    Returns:
      A single :class:`str` holding the collected log entries if
      no ``output_log_file`` has been specified. Otherwise, the log will be
      stored in the given file and this method returns ``None``.
    """
    assert platform == 'linux'
    ensure_parent_path_exists(output_log_file)

    cmd = ['/bin/journalctl', '--no-pager']
    if unit is not None:
        cmd.append('--unit')
        cmd.append(unit)

    if since is not None:
        cmd.append('--since')
        cmd.append(str(since))
    if until is not None:
        cmd.append('--until')
        cmd.append(str(until))

    # StringIO has no `fileno` attribute, thus the context manager approach won't
    # work & we need to read the pipe ourselves:
    # # with open(output_log_file, 'w') as log:
    # #     subprocess.run(cmd, stdout=log)
    tgt = StringIO() if (output_log_file is None) else open(output_log_file, 'w')
    proc = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    tgt.write(proc.stdout.decode())

    if output_log_file is None:
        tgt.seek(0)
        return tgt.read()
    else:
        tgt.close()
        return None


def byte_size_str(num_bytes: int, delimiter: str = ' ') -> str:
    """
    Returns a human-friendly representation of the given number of bytes,
    e.g. "10.1 MB".

    Args:
      num_bytes: Number of bytes.
      delimiter: Delimiter (such as a space) between the number and the
        abbreviation of the unit.
    """
    units = ['B', 'KB', 'MB', 'GB', 'TB']
    idx = 0
    while True:
        if (idx == len(units) - 1) or (num_bytes < 1024):
            break
        num_bytes /= 1024
        idx += 1
    if idx > 0:
        return f'{num_bytes:.1f}{delimiter}{units[idx]}'
    else:
        return f'{int(num_bytes)}{delimiter}{units[0]}'


def file_size(filename: Union[Path, str]) -> int:
    """
    Returns the file size in byte.

    Uses `os.stat <https://docs.python.org/3/library/os.html#os.stat>`__
    to query the file size.

    Args:
      filename: Path to the file.
    """
    if isinstance(filename, str):
        filename = Path(filename)

    if not filename.exists():
        return 0

    return filename.stat().st_size
