#!/bin/env python3
import pyproj
import numpy as np

def wgs84_to_utm(lat, lon):
    # Determine UTM zone by longitude
    utm_zone = int((lon + 180) / 6) + 1
    
    # Determine northern or southern hemisphere
    is_northern = lat >= 0
    
    # Create EPSG code for the UTM zone
    epsg_code = f"EPSG:{32600 + utm_zone if is_northern else 32700 + utm_zone}"

    # Create transformer for WGS84 to UTM
    transformer_to_utm = pyproj.Transformer.from_crs("EPSG:4326", epsg_code, always_xy=True)
    
    # Transform the coordinates
    easting, northing = transformer_to_utm.transform(lon, lat)
    
    return easting, northing, utm_zone, 'N' if is_northern else 'S'

def utm_to_wgs84(easting, northing, utm_zone, hemisphere):
    # Create the EPSG code for UTM
    is_northern = hemisphere == 'N'
    epsg_code = f"EPSG:{32600 + utm_zone if is_northern else 32700 + utm_zone}"

    # Create transformer for UTM to WGS84
    transformer_to_wgs84 = pyproj.Transformer.from_crs(epsg_code, "EPSG:4326", always_xy=True)

    # Transform the coordinates
    lon, lat = transformer_to_wgs84.transform(easting, northing)
    
    return lat, lon

def main():
    try:
        # Base HM WGS84 position / Read input coordinates
        lat = 48.154413   # float(input("Enter Latitude (degrees): "))
        lon = 33.409740   # float(input("Enter Longitude (degrees): "))

        # bounds (i.e. size of lab / map image of the lab)
        x_bound =  7.5  # meters
        y_bound = 10.0  # meters

        # base station offsets
        gnb_easting_offset = 3.5
        gnb_northing_offset = 5.5

        # create arrays with offsets for UEs (currenty up to 14 UEs)
        easting_offsets = np.tile(np.array([0.75, 5.75]), 7).tolist()  # individual offset for each two UEs
        northing_offsets = np.repeat(np.linspace(2.75, 8, 7),2).tolist() # individual offset for each two UEs
        

        # base station position
        easting, northing, zone, hemisphere = wgs84_to_utm(lat, lon)
        # Convert back to WGS-84
        new_lat, new_lon = utm_to_wgs84(easting+gnb_easting_offset, northing+gnb_northing_offset, zone, hemisphere)
        print(f"*.gNB[0].mobility.initialLatitude = {new_lat}deg")
        print(f"*.gNB[0].mobility.initialLongitude = {new_lon}deg")
        min_easting = easting
        min_northing = northing
        max_easting = easting
        max_northing = northing

        # UE position with offsets
        for e_offset, n_offset, i in zip(easting_offsets, northing_offsets, range(len(easting_offsets))):
            # Apply offsets
            new_easting = easting + e_offset
            new_northing = northing + n_offset

            if new_easting < min_easting:
                min_easting = new_easting
            if new_northing < min_northing:
                min_northing = new_northing
            if new_easting > max_easting:
                max_easting = new_easting
            if new_northing > max_northing:
                max_northing = new_northing
            
            # Convert back to WGS-84
            new_lat, new_lon = utm_to_wgs84(new_easting, new_northing, zone, hemisphere)
            
            # Print the result
            print(f"*.ue[{i}].mobility.initialLatitude = {new_lat}deg")
            print(f"*.ue[{i}].mobility.initialLongitude = {new_lon}deg")

        # check if the bounds are correct
        assert max_easting - min_easting <= x_bound, "Easting bounds exceed the defined x_bound."
        assert max_northing - min_northing <= y_bound, "Northing bounds exceed the defined y_bound."

        print("# config for OsgCoordConverterLocal")
        print("*.coordConverter.typename = \"OsgCoordConverterLocal\"")
        print(f"*.coordConverter.srs_code = \"+proj=utm +zone={zone} +ellps=WGS84 +datum=WGS84 +units=m +no_defs\"")
        print(f"*.coordConverter.offset_x = {min_easting*-1.0}m")
        print(f"*.coordConverter.offset_y = {min_northing*-1.0}m")
        print(f"*.coordConverter.xBound = {x_bound}m")
        print(f"*.coordConverter.yBound = {y_bound}m")

    except ValueError:
        print("Please enter valid numeric latitude and longitude values.")

if __name__ == "__main__":
    main()
