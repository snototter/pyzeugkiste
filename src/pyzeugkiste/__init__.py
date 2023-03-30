#try:
from ._core import __version__, __doc__, __werkzeugkiste_version__

__all__ = ['args', 'config', 'files', 'geo', 'strings']
#except ModuleNotFoundError:
#  __version__ = 'fallback'
#  __doc__ = 'TODO'
