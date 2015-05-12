from click import command, argument, group, CommandCollection
import pmesdr_utils.inspector as inspector

@group()
def make_png_cli():
    pass

@make_png_cli.command()
@argument('filename')
def make_png(filename):
    """Given a pmesdr nc file, makes a png browse image of the tb data"""
    inspector.make_png(filename)

@group()
def make_geotiff_cli():
    pass

@make_geotiff_cli.command()
@argument('grid')
@argument('filename')
def make_geotiff(grid, filename):
    """Makes the image in filename into a geotiff"""
    inspector.make_geotiff(grid, filename)

cli = CommandCollection(sources=[make_png_cli, make_geotiff_cli])

if __name__ == '__main__':
    cli()

