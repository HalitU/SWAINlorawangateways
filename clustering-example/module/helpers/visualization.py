import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt

def process_visualization (ed_coords, gw_coords, rivers):    
    # Enddevices and sensors
    ed_x_y = []
    for ed_coord in ed_coords:
        ed_x_y.append ([ed_coord.lat, ed_coord.lon])
    gw_x_y = []
    for gw_key, gw_coord in gw_coords.items ():
        gw_x_y.append ([gw_coord [0], gw_coord [1]])

    # swap lat and lon stuff for visuals
    ed_x_y = [[y, x] for (x, y) in ed_x_y]
    gw_x_y = [[y, x] for (x, y) in gw_x_y]
    rivers = [[river.lon, river.lat] for river in rivers]

    # Use scatter plot to show the coordinates
    fig, ax = plt.subplots()
    ax.scatter (*zip(*ed_x_y), label='sensors', c='#6b5b95')
    ax.scatter (*zip(*gw_x_y), label='gateways', c= '#82b74b')
    if len (rivers) > 0:
        ax.scatter (*zip(*rivers), label='river', s=0.005, c='#ff7b25')
    ax.legend ()
    ax.grid (True)
    ax.set_xlabel('longitude')
    ax.set_ylabel('latitude')
    # plt.gca().invert_yaxis()

    plt.savefig ("clustering-example/output/env_visualization.png")

def process_visualization_with_removed (ed_coords, gw_coords, rivers, removed_coords):    
    # Enddevices and sensors
    ed_x_y = []
    for ed_coord in ed_coords:
        ed_x_y.append ([ed_coord.lat, ed_coord.lon])
    rmv_x_y = []
    for rmv_coord in removed_coords:
        rmv_x_y.append ([rmv_coord.lat, rmv_coord.lon])        
    gw_x_y = []
    for gw_key, gw_coord in gw_coords.items ():
        gw_x_y.append ([gw_coord [0], gw_coord [1]])

    # swap lat and lon stuff for visuals
    ed_x_y = [[y, x] for (x, y) in ed_x_y]
    rmv_x_y = [[y, x] for (x, y) in rmv_x_y]
    gw_x_y = [[y, x] for (x, y) in gw_x_y]
    rivers = [[river.lon, river.lat] for river in rivers]

    # Use scatter plot to show the coordinates
    fig, ax = plt.subplots()
    if len (rivers) > 0:
        ax.scatter (*zip(*rivers), label='river', s=0.005, c='#ff7b25')
    ax.scatter (*zip(*ed_x_y), label='sensors', c='#6b5b95')
    ax.scatter (*zip(*rmv_x_y), label='removed sensors', c='#d64161')
    ax.scatter (*zip(*gw_x_y), label='gateways', c= '#82b74b')
    ax.legend ()
    ax.grid (True)
    ax.set_xlabel('longitude')
    ax.set_ylabel('latitude')
    # plt.gca().invert_yaxis()

    plt.savefig ("clustering-example/output/env_visualization_with_removed.png")
