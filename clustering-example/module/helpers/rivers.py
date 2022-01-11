import geopandas as gpd

class River:
    def __init__ (self, name, lat, lon, height):
        self.name = name
        self.lat = lat
        self.lon = lon
        self.height = height

def read_river_data ():
    ########################
    # Add the rivers
    rivers = []
    current_file = "clustering-example/data/river/Erg_river_Hydro.shp" #988 rows
    shapefile = gpd.read_file(current_file)
    # Convert 4326 lat/lon to km from reference point
    shapefile = shapefile.to_crs (epsg=4326)
    
    # Points of all rivers
    for linestring in shapefile ['geometry']:
        # Points of a single river
        for coord in linestring.coords:
            # Convert to distance with reference
            lon = coord [0]
            lat = coord [1]
            # Add them to river list
            rivers.append (River ('dummy', lat, lon, 0))
    ########################

    return rivers