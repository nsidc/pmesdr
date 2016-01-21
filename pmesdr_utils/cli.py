from click import command, argument, group, CommandCollection
import pmesdr_utils.inspector as inspector

@group()
def make_png_cli():
    pass

@make_png_cli.command()
@argument('var_name')
@argument('filename')
def make_png(filename, var_name):
    """
    Makes a png browse image of the requested variable
    """
    inspector.make_png(filename, var_name)

@group()
def make_geotiff_cli():
    pass

@make_geotiff_cli.command()
@argument('var_name')
@argument('filename')
def make_geotiff(filename, var_name):
    """
    Makes a geotiff file with the requested variable
    """
    inspector.make_geotiff(filename, var_name)

cli = CommandCollection(sources=[make_png_cli, make_geotiff_cli])

if __name__ == '__main__':
    cli()

