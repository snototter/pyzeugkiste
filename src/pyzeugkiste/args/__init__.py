"""Collection of custom validators for `argparse`."""
import argparse
import datetime


class ValidateDate(argparse.Action):
    """
    Action class for `argparse` to parse command line parameters into a valid
    `datetime.date` object.

    A date can be specified by:
    * `YYYY-MM-DD`
    * `DD.MM.YYYY`
    * Special strings: `today` and `yesterday`
    """
    def __call__(self, parser, args, value, option_string=None):
        val = None
        if value is None:
            raise ValueError('Date input cannot be `None`!')
        try:
            vstrip = value.strip().lower()
            if vstrip == 'today':
                val = datetime.date.today()
            elif vstrip == 'yesterday':
                val = datetime.date.today() - datetime.timedelta(days=1)
            elif '.' in value:
                val = datetime.datetime.strptime(value, '%d.%m.%Y').date()
            else:
                val = datetime.datetime.strptime(value, '%Y-%m-%d').date()
        except ValueError:
            raise ValueError(f'Invalid date input "{value}". Supported formats are "YYYY-MM-DD" and "DD.MM.YYYY", as well as "today"/"yesterday".') from None
        setattr(args, self.dest, val)


class ValidateTime(argparse.Action):
    """
    Action class for `argparse` to parse command line parameters into a valid
    `datetime.time` object.

    A time can be specified by:
    * `HH:MM` or `HHMM`
    * `HH:MM:SS` or `HHMMSS`
    """
    def __call__(self, parser, args, value, option_string=None):
        val = None
        if value is None:
            raise ValueError('Time input cannot be `None`!')
        try: #TODO support millisec & microsec %S.123 or %S.123456
            vstrip = value.strip().lower()
            if ':' in value:
                if vstrip.count(':') == 2:
                    val = datetime.datetime.strptime(value, '%H:%M:%S').time()
                else:
                    val = datetime.datetime.strptime(value, '%H:%M').time()
            else:
                if len(vstrip) == 4:
                    val = datetime.datetime.strptime(value, '%H%M').time()
                else:
                    val = datetime.datetime.strptime(value, '%H%M%S').time()
        except ValueError:
            raise ValueError(f'Invalid time input "{value}". Supported formats are "HH:MM" or "HH:MM:SS" or "HHMM" or "HHMMSS".') from None
        setattr(args, self.dest, val)
