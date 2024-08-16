import click
import datetime as dt
from datetime import timedelta
import numpy as np
import glob as glob
import os
from pathlib import Path

from nsidc0630_params import resolutions


def remove_files(prefix, projections, ltods, channels, satellite, date, file_regex):
    """
    Deletes all but the most recent duplicates of files with requested
    characteristics.  This function relies on the processing date part of the
    filenames to be sorted so that the most recent file will be last in the
    sorted list. Should be called separately clean up sets of data, premet or
    spatial metadata files.

    prefix : string, NSIDC authID used as filename prefix in the data files
    projections : list of projection strings, e.g. 'N25'
    ltods : list of ltod letters, any of 'E', 'M', 'A', 'D'
    channels : list of channel ids (including polarization)
    satellite : list of platform/sensors, any of 'F16', 'F17', 'F18', 'AMSR2', 'SMAP'
    date : string, date to search for, yyyymmdd
    file_regex : full path and filename regex to look for

    """
    for projection in projections:
        for ltod in ltods:
            for channel in channels:
                cur_regex = file_regex % (
                    prefix,
                    projection,
                    satellite,
                    ltod,
                    channel,
                    date
                )
                files = np.sort(glob.glob(cur_regex))
                if (len(files)) > 1:
                    for cur_regex in files[0:-1]:
                        print('Removing file: ', cur_regex)
                        os.remove(cur_regex)


def datetime_to_date(_ctx, _param, value: dt.datetime) -> dt.date:
    """
    Click callback that converts a `dt.datetime` to `dt.date`
    """
    return value.date()

def daterange(start_date, end_date):
    """
    Generator to efficiently return the span of dates by day

    start_date : dt.datetime, begin date
    end_date : dt.datetime, end date
    
    """
    for n in range(int((end_date - start_date).days)):
        yield start_date + dt.timedelta(n)


@click.command()
@click.option(
    '-s',
    '--start-date',
    type=click.DateTime(formats=['%Y%m%d', '%Y-%m-%d']),
    default=str(dt.datetime.today().date() - timedelta(days=10)),
    show_default=True,
    help='Start date of MOD09GA tiles to process.',
    callback=datetime_to_date,
)
@click.option(
    '-e',
    '--end-date',
    type=click.DateTime(formats=['%Y%m%d', '%Y-%m-%d']),
    default=str(dt.datetime.today().date() - timedelta(days = 0)),
    show_default=True,
    help='End date of MOD09GA tiles to process.',
    callback=datetime_to_date,
)
@click.option(
    '-i',
    '--input-dir',
    type=click.Path(
        file_okay=False, dir_okay=True, exists=False, path_type=Path
    ),
    envvar='MOD09GA_NRT_DIR',
    show_default=True,
    help='Absolute directory to existing MOD09GA granule files set by MOD09GA_NRT_DIR'
         ' environment variable. Date and tile ID subdirectories will be added'
         ' (e.g. 2023.10.03/h08v04).',
)
@click.option(
    '-p',
    '--platforms',
    type=click.Choice(['F16', 'F17', 'F18', 'AMSR2', 'SMAP']),
    default=['F16'],
    multiple=True,
    help='Platforms for which duplicate files are to be removed.',
)
def remove_duplicate_files(start_date, end_date, input_dir, platforms):
    if end_date < start_date:
        raise ValueError('The start date of processing: ' +
                         start_date.strftime('%m/%d/%Y') + '  is after the end date: ' +
                         end_date.strftime('%m/%d/%Y'))
    for day in daterange(start_date, end_date + dt.timedelta(days=1)):
        for platform in platforms:
            if platform in ['F16', 'F17', 'F18']:
                channels = ['19H', '19V', '22V', '37H', '37V', '91H', '91V']
                platform = platform + '_SSMIS'
                prefix = 'NSIDC0630'
            elif platform in ['AMSR2']:
                channels = ['6.9H', '6.9V', '10.7H', '10.7V', '18H', '18V', '23H', '23V', '36H', '36V', '89H', '89V']
                platform = 'GCOMW1_' + platform
                prefix = 'NSIDC0630'
            elif platform in ['SMAP']:
                channels = ['1.4H', '1.4V', '1.4F']
                platform = platform + '_LRM'
                prefix = 'NSIDC0738'
            else:
                raise ValueError('Unknown platform: ' + platform)
            for resolution in resolutions.values():
                projections = resolution['projections']
                ltods = resolution.get('ltods')
                day_str = day.strftime('%Y%m%d')
                file_regex = os.path.join(input_dir, resolution['file_regex'])
                remove_files(prefix, projections, ltods, channels, platform, day_str, file_regex)
                file_regex = os.path.join(input_dir, resolution['premet_file_regex'])
                remove_files(prefix, projections, ltods, channels, platform, day_str, file_regex)
                file_regex = os.path.join(input_dir, resolution['spatial_file_regex'])
                remove_files(prefix, projections, ltods, channels, platform, day_str, file_regex)


if __name__ == "__main__":
    """Executed from the command line"""
    remove_duplicate_files()
