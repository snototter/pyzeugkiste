import argparse
import datetime
import pytest
from pyzk import args as pzargs


def test_date_parsing():
    parser = argparse.ArgumentParser()
    parser.add_argument('day', action=pzargs.ValidateDate)
    parser.add_argument('--opt', action=pzargs.ValidateDate, default=None)

    # Invalid inputs
    with pytest.raises(ValueError):
        parser.parse_args([''])

    with pytest.raises(ValueError):
        parser.parse_args(['invalid'])

    with pytest.raises(ValueError):
        parser.parse_args(['not-a-date'])

    # Typos in the date/wrong order for YYYY-MM-DD:
    with pytest.raises(ValueError):
        parser.parse_args(['03-04-2000'])

    with pytest.raises(ValueError):
        parser.parse_args(['2000-03-04-'])

    with pytest.raises(ValueError):
        parser.parse_args(['2000--03-04'])

    with pytest.raises(ValueError):
        parser.parse_args(['03-04-2000'])

    with pytest.raises(ValueError):
        parser.parse_args(['2000-20-01'])

    # Typos in the date/wrong order for DD.MM.YYYY:
    with pytest.raises(ValueError):
        parser.parse_args(['01.02.2000.'])

    with pytest.raises(ValueError):
        parser.parse_args(['.01.02.2000'])

    with pytest.raises(ValueError):
        parser.parse_args(['01..02.2000'])

    with pytest.raises(ValueError):
        parser.parse_args(['2000.02.01'])

    with pytest.raises(ValueError):
        parser.parse_args(['01.15.2000'])

    # A None input is not allowed. `parse_args` does not invoke the action
    # with the default value.
    with pytest.raises(ValueError):
        parser.parse_args([None])

    # Standard date format: YYYY-MM-DD
    args = parser.parse_args(['2022-12-01'])
    assert datetime.date(2022, 12, 1) == args.day
    assert args.opt is None
    assert isinstance(args.day, datetime.date)

    args = parser.parse_args(['1234-07-20'])
    assert datetime.date(1234, 7, 20) == args.day

    args = parser.parse_args(['1234-7-3'])  # Without leading zeros
    assert datetime.date(1234, 7, 3) == args.day
    assert args.opt is None
    assert isinstance(args.day, datetime.date)

    # Correct format, but out-of-range (should be handled by
    # the datetime-internal value checks)
    with pytest.raises(ValueError):
      parser.parse_args(['2022-01-49'])

    # Standard date format: DD.MM.YYYY
    args = parser.parse_args(['2022-12-01'])
    assert datetime.date(2022, 12, 1) == args.day
    assert args.opt is None
    assert isinstance(args.day, datetime.date)

    args = parser.parse_args(['1.2.2134'])
    assert datetime.date(2134, 2, 1) == args.day
    assert args.opt is None
    assert isinstance(args.day, datetime.date)

    with pytest.raises(ValueError):
      parser.parse_args(['00.07.2022'])

    with pytest.raises(ValueError):
      parser.parse_args(['01.19.2022'])

    # Special input strings
    today = datetime.date.today()
    yesterday = today - datetime.timedelta(days=1)
    args = parser.parse_args(['today'])
    assert today == args.day
    assert isinstance(args.day, datetime.date)
    assert args.opt is None

    args = parser.parse_args(['yesterday'])
    assert yesterday == args.day
    assert isinstance(args.day, datetime.date)
    assert args.opt is None

    args = parser.parse_args(['yesterday', '--opt', 'today'])
    assert yesterday == args.day
    assert isinstance(args.day, datetime.date)
    assert today == args.opt
    assert isinstance(args.opt, datetime.date)


#TODO test_time
