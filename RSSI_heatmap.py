#heatmaps of RSSI and SNR

import pandas as pd
import matplotlib.pyplot as plt
import cartopy.crs as ccrs
import cartopy.feature as cfeature
import numpy as np
from scipy.interpolate import griddata
import contextily as ctx
import geopandas as gpd
from shapely.geometry import box

#load csv and extract data
df = pd.read_csv("test_log.csv") #replace with correct filename or path
gdf = gpd.GeoDataFrame(df, geometry = gpd.points_from_xy(df['Longitude'], df['Latitude']), crs = 'EPSG:4326')
gdf = gdf.to_crs(epsg = 3857)

x = gdf.geometry.x.values
y = gdf.geometry.y.values
rssi = gdf['RSSI'].values

#interpolate signal values to grid
xi = np.linspace(x.min(), x.max(), 200)
yi = np.linspace(y.min(), y.max(), 200)
xi, yi = np.meshgrid(xi, yi)
grid = griddata((x, y), rssi, (xi, yi), method = 'cubic')

#plot heatmap
fig, ax = plt.subplots(figsize = (10,8))
xrange = x.max() - x.min()
yrange = y.max() - y.min()
padding_x = xrange*0.05
padding_y = yrange*0.05
heatmap = ax.imshow(grid, extent = (x.min(), x.max(), y.min(), y.max()), origin = 'lower', cmap = 'Spectral', alpha = 0.6, zorder = 1)
#overlay basemap
ctx.add_basemap(ax, crs = 'EPSG:3857', source = ctx.providers.OpenStreetMap.Mapnik)
#show original data points
ax.scatter(x, y, c='white', s=10, edgecolor='black')
ax.set_xlim(x.min() - padding_x, x.max() + padding_x)
ax.set_ylim(y.min() - padding_y, y.max() + padding_y)
plt.colorbar(heatmap, label = 'RSSI (dBm)')
plt.title('RSSI Heatmap')
plt.tight_layout()
plt.show()
