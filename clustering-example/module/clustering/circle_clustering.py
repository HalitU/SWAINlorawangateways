import numpy as np
import geopy.distance
import math
from pyproj import Proj, transform
from decimal import Decimal

class Intersection:
    def __init__ (self, name, lat, lon, height):
        self.name = name
        self.lat = lat
        self.lon = lon
        self.height = height

class CircleClustering:
    distance_threshold = None
    distance_error_margin = None
    rivers = None
    lat_margin = None
    lon_margin = None

    def clustering_algorithm_set_cover (self, sensors):
        # Initially all sensors are chilling
        labels = [-1] * len (sensors)
        # Covered will be used to find the appropriate sets
        # Initially none is covered
        covered = [False] * len (sensors)
        # Alone is used to check if a sensor will be in a set with an intersection
        # Initially all of them are alone and sad
        alone = [True] * len (sensors)

        ##########
        # Find all of the intersection points, which are potential gateway points
        # If there is no intersection, then the sensor is alone
        # So after this process, all the sensors not-covered will be separated into their own clusters
        ##########
        
        # Intersection points between all circles
        intersectionPoints = self.getIntersectionPoints (sensors)
        print ("Number of intersection points: ", len (intersectionPoints))

        # Find the list of sensors covered by these intersections
        intersectionSets = {}
        for i_ix, intersection in enumerate (intersectionPoints):
            intersectionSets [i_ix] = []
            for s_ix, sensor in enumerate (sensors):
                if self.containedInCircle (sensor, intersection):
                    intersectionSets [i_ix].append (s_ix)
                    # Since sensor is within an intersection, it means it cannot be alone
                    alone [s_ix] = False
            assert len (intersectionSets [i_ix]) > 0
        
        # Sort these points according to the number sensors they might cover
        # Starting from the top dictionary cover sensors with gateways and create clusters
        print ("Sorting the sensor assignment dictionary and evaluating coverage")
        for key in sorted(intersectionSets, key=lambda k: len(intersectionSets[k]), reverse=True):
            values = intersectionSets [key]
            # Cover these sensors and create a cluster point if nothing is covered no need to do anything
            new_cluster_exists = False
            for value in values:
                if covered [value] == False:
                    covered [value] = True
                    labels [value] = key
                    new_cluster_exists = True

        # Lastly convert the assigned labels of the sensors into clusters
        # If any of the label is still -1, that means no intersection exists, assign a new cluster value
        print ("Finalizing the clustering process")
        clusters = {}
        max_key = len (intersectionPoints)
        for ix, label in enumerate (labels):
            if label == -1:
                # Create a singular cluster
                clusters [max_key] = [sensors [ix].name]
                # Do not forget to update the label
                labels [ix] = max_key
                # Next key for the next singular cluster
                max_key += 1
            else:
                if label not in clusters:
                    clusters [label] = []
                clusters [label].append (sensors [ix].name)

        print ("Clustering done. Number of clusters: ", len (clusters.keys ()))
        # Return labels and clusters
        return labels, clusters

    def intersectionArea (self, circles):
        if len (circles) == 1:
            return [circles [0].lat, circles [0].lon, circles [0].height]

        # Intersection points between all circles
        intersectionPoints = self.getIntersectionPoints (circles)

        # Removing points which are not included in all of the circles
        intersectionPoints = [point for point in intersectionPoints if self.containedInAllCircles (circles, point) == True]

        if len (intersectionPoints) == 0:
            return []

        # print ("Number of points which are within all circles: ", len (intersectionPoints))

        # TODO: calculating area for better locations
        # TODO: choosing a point within the intersection points, which should also be close to the rivers!
        # TODO: cover no river intersection case if it exists

        ###################
        # Choosing a point close to the rivers
        ###################
        # Get the rivers which are within the range of current sensors
        river_intersections = []
        for river in self.rivers:
            if self.containedInAllCircles (circles, river):
                river_intersections.append (river)

        # If a river exists, just choose one of the river points
        # otherwise chose one from the original intersections
        if len (river_intersections) > 0:
            intersectionPoints = river_intersections

        # How to integrate the height
        # for now take average of all sensors
        average_height = np.average ([point.height for point in circles])

        # Choose a random point
        # random_index = np.random.choice (len (intersectionPoints))

        # Instead of using a random point, take average of all possible points
        avg_lat = 0.0
        avg_lon = 0.0
        for river_point in intersectionPoints:
            avg_lat += river_point.lat
            avg_lon += river_point.lon
        avg_lat = avg_lat / len (intersectionPoints)
        avg_lon = avg_lon / len (intersectionPoints)
        print ("Average location: ", avg_lat, " ", avg_lon)

        return [avg_lat, avg_lon, average_height]
        # lat (e.g 42) - lon (e.g. 28) - height
        # return  [
        #     intersectionPoints [random_index].lat,
        #     intersectionPoints [random_index].lon,
        #     average_height
        # ]

    def getIntersectionPoints (self, circles):
        intersections = []
        for c_ix, circle_source in enumerate (circles):
            for circle_target in circles [c_ix+1:]:
                # self.circleCircleIntersection_test_cases (circle_source, circle_target)
                intersection = self.circleCircleIntersection (circle_source, circle_target)
                if len (intersection) > 0:
                    intersections.append (intersection [0])
                    intersections.append (intersection [1])
        return intersections

    ## New math formulas
    def convert_to_geocentric (self, p):
        p_lat, p_lon = p.lat, p.lon
        p_lat_r, p_lon_r = math.radians (p_lat), math.radians (p_lon)
        p_x = Decimal (math.cos (p_lon_r) * math.cos (p_lat_r))
        p_y = Decimal (math.sin (p_lon_r) * math.cos (p_lat_r))
        p_z = Decimal (math.sin (p_lat_r))
        return p_x, p_y, p_z


    def convert_km_to_radians (self, m):
        # 1 km = 0.539957 nautical miles
        # 1 m = 0.000539957 nm
        return math.radians ((m * 0.000539957) / 60)

    ##

    def circleCircleIntersection (self, p1, p2):
        #############################
        # debugging
        # https://gis.stackexchange.com/questions/48937/calculating-intersection-of-two-circles
        # Real distance between the coordinates
        d = geopy.distance.distance ((p1.lat, p1.lon), (p2.lat, p2.lon)).m
        radius1 = self.distance_threshold
        radius2 = self.distance_threshold

        # 1 - Convert lat lon to geocentric coordinates
        x1 = self.convert_to_geocentric (p1)
        x2 = self.convert_to_geocentric (p2)

        # 2 - Convert ranges into angles along the earth sphere
        r1 = Decimal (self.convert_km_to_radians (radius1))
        r2 = Decimal (self.convert_km_to_radians (radius2))

        # 3 - intersection between sphere and earths surface
        q = Decimal (np.dot (x1, x2))

        # 4 - 
        intersections = []
        if q**2 != 1:
            a = (Decimal(math.cos(r1)) - Decimal(math.cos(r2))*q) / (1 - q**2)
            b = (Decimal(math.cos(r2)) - Decimal(math.cos(r1))*q) / (1 - q**2)

            n = np.cross(x1, x2)

            x0_1 = [a*f for f in x1]
            x0_2 = [b*f for f in x2]
            x0 = [sum(f) for f in zip(x0_1, x0_2)]
 
            if (np.dot(x0, x0) <= 1) & (np.dot(n,n) != 0): # This is to secure that (1 - np.dot(x0, x0)) / np.dot(n,n) > 0
                t = Decimal(math.sqrt((1 - np.dot(x0, x0)) / np.dot(n,n)))
                t1 = t
                t2 = -t

                i1 = x0 + t1*n
                i2 = x0 + t2*n

                i1_lat = math.degrees( math.asin(i1[2]))
                i1_lon = math.degrees( math.atan2(i1[1], i1[0] ) )
                ip1 = (i1_lat, i1_lon)

                i2_lat = math.degrees( math.asin(i2[2]))
                i2_lon = math.degrees( math.atan2(i2[1], i2[0] ) )
                ip2 = (i2_lat, i2_lon)
                intersections = [Intersection ('dummy', ip1 [0], ip1 [1], 0), Intersection ('dummy', ip2 [0], ip2 [1], 0)]
                # intersections = [ip1, ip2]
            elif (np.dot(n,n) == 0):
                #print ("The centers of the circles can be neither the same point nor antipodal points.")
                pass
            else:
                #print ("The circles do not intersect")            
                pass
        else:
            # print ("The centers of the circles can be neither the same point nor antipodal points.")            
            pass

        if len (intersections) > 0:
            expected_threshold = self.distance_threshold + self.distance_error_margin
            # Checking the consistency for first point with intersections
            final_dist = geopy.distance.distance ((intersections [0].lat, intersections [0].lon), (p1.lat, p1.lon)).m
            assert final_dist <= self.distance_threshold + self.distance_error_margin, "Distance was: " + str (final_dist) + " flat distance was: " + str (d) + " maximum allowed: " + str (expected_threshold)
            final_dist = geopy.distance.distance ((intersections [1].lat, intersections [1].lon), (p1.lat, p1.lon)).m
            assert final_dist <= self.distance_threshold + self.distance_error_margin, "Distance was: " + str (final_dist) + " flat distance was: " + str (d) + " maximum allowed: " + str (expected_threshold)
            # Checking the consistency for second point with intersections
            final_dist = geopy.distance.distance ((intersections [0].lat, intersections [0].lon), (p2.lat, p2.lon)).m
            assert final_dist <= self.distance_threshold + self.distance_error_margin, "Distance was: " + str (final_dist) + " flat distance was: " + str (d) + " maximum allowed: " + str (expected_threshold)
            final_dist = geopy.distance.distance ((intersections [1].lat, intersections [1].lon), (p2.lat, p2.lon)).m
            assert final_dist <= self.distance_threshold + self.distance_error_margin, "Distance was: " + str (final_dist) + " flat distance was: " + str (d) + " maximum allowed: " + str (expected_threshold)
           
        return intersections

    def circleCircleIntersection_test_cases (self, p1, p2):
        #############################
        # debugging
        # https://gis.stackexchange.com/questions/48937/calculating-intersection-of-two-circles
        # Real distance between the coordinates
        p1 = [37.673442, -90.234036]
        p2 = [36.109997, -90.953669]
        radius1 = 107.5 * 1.852 * 1000 #self.distance_threshold
        radius2 = 145 * 1.852 * 1000 #self.distance_threshold

        print (radius1)
        print (radius2)

        d = geopy.distance.distance ((p1.lat, p1.lon), (p2.lat, p2.lon)).m

        # 1 - Convert lat lon to geocentric coordinates
        x1 = self.convert_to_geocentric (p1)
        x2 = self.convert_to_geocentric (p2)

        # 2 - Convert ranges into angles along the earth sphere
        r1 = Decimal (self.convert_km_to_radians (radius1))
        r2 = Decimal (self.convert_km_to_radians (radius2))

        # 3 - intersection between sphere and earths surface
        q = Decimal (np.dot (x1, x2))

        # 4 - 
        intersections = []
        if q**2 != 1:
            a = (Decimal(math.cos(r1)) - Decimal(math.cos(r2))*q) / (1 - q**2)
            b = (Decimal(math.cos(r2)) - Decimal(math.cos(r1))*q) / (1 - q**2)

            n = np.cross(x1, x2)

            x0_1 = [a*f for f in x1]
            x0_2 = [b*f for f in x2]
            x0 = [sum(f) for f in zip(x0_1, x0_2)]
 
            if (np.dot(x0, x0) <= 1) & (np.dot(n,n) != 0): # This is to secure that (1 - np.dot(x0, x0)) / np.dot(n,n) > 0
                t = Decimal(math.sqrt((1 - np.dot(x0, x0)) / np.dot(n,n)))
                t1 = t
                t2 = -t

                i1 = x0 + t1*n
                i2 = x0 + t2*n

                print (math.degrees( math.asin(i1[2])))
                print (math.degrees( math.atan2 (i1[2], math.sqrt (i1[0]**2 + i1[1]**2))))

                i1_lat = math.degrees( math.asin(i1[2]))
                i1_lon = math.degrees( math.atan2(i1[1], i1[0] ) )
                ip1 = (i1_lat, i1_lon)

                i2_lat = math.degrees( math.asin(i2[2]))
                i2_lon = math.degrees( math.atan2(i2[1], i2[0] ) )
                ip2 = (i2_lat, i2_lon)
                intersections = [ip1, ip2]
            elif (np.dot(n,n) == 0):
                print ("The centers of the circles can be neither the same point nor antipodal points.")
            else:
                print ("The circles do not intersect")            
        else:
            print ("The centers of the circles can be neither the same point nor antipodal points.")            

        print (intersections)

        if len (intersections) > 0:
            dist = geopy.distance.great_circle ((intersections [0].lat, intersections [0].lon), (p1.lat, p1.lon)).m
            dist2 = geopy.distance.great_circle ((intersections [1].lat, intersections [1].lon), (p2.lat, p2.lon)).m
            print (dist)
            print (dist2)
            assert dist <= self.distance_threshold, "Distance was: " + str (dist) + ", " + str (dist2) + " main distance was: " + str (d)
        
        assert (False)

        #############################

        d = geopy.distance.distance ((p1.lat, p1.lon), (p2.lat, p2.lon)).m
        r1 = self.distance_threshold
        r2 = self.distance_threshold

        # assert d <= (r1 + r2), "Distance:" + str (d) + ", radius: " + str (r1) + '\n' + listToString (p1) + '\n' + listToString (p2)
        if d > (r1 + r2):
            return []

        a = (r1 * r1 - r2 * r2 + d * d) / (2 * d)
        h = math.sqrt (r1 * r1 - a * a)
        x0 = p1.lat + a * (p2.lat - p1.lat) / d
        y0 = p1.lon + a * (p2.lon - p1.lon) / d
        rx = -(p2.lon - p1.lon) * (h / d)
        ry = -(p2.lat - p1.lat) * (h / d)

        intersections = [[x0 + rx, y0 - ry], [x0 - rx, y0 + ry ]]

        ## debugging
        dist = geopy.distance.distance ((intersections [0].lat, intersections [0].lon), (p1.lat, p1.lon)).m
        dist2 = geopy.distance.distance ((intersections [1].lat, intersections [1].lon), (p1.lat, p1.lon)).m
        assert dist <= self.distance_threshold, "Distance was: " + str (dist) + ", " + str (dist2) + " main distance was: " + str (d)
        ##

        return intersections

    # Checks whether a point is within the range of all circles (aka sensor ranges)
    def containedInAllCircles (self, circles, point):
        for circle in circles:
            # Since checking all rivers are costly we can use a short calculation
            # to eliminate some potential easy to see out of ranges!
            lat_s, lon_s = circle.lat, circle.lon
            lat_t, lon_t = point.lat, point.lon
            if abs (lat_s - lat_t) > self.lat_margin:
                return False
            if abs (lon_s - lon_t) > self.lon_margin:
                return False
            # If above does not work, we have to do the real calculation with the library
            dist = geopy.distance.distance ([circle.lat, circle.lon], [point.lat, point.lon]).m
            if dist > self.distance_threshold + self.distance_error_margin:
                return False
        return True

    def containedInCircle (self, point, circle):
        dist = geopy.distance.distance ([circle.lat, circle.lon], [point.lat, point.lon]).m
        if dist > self.distance_threshold + self.distance_error_margin:
            return False
        return True        

    def listToString(self, s): 
        # initialize an empty string
        str1 = ""     
        # traverse in the string  
        for ele in s: 
            str1 += str (ele) + " "    
        # return string  
        return str1 

    def rivers_within_gateway_count (self, gateways, rivers):
        total_in_range = 0
        for river in rivers:
            for gw_key, gateway in gateways.items ():
                lat_s, lon_s = river.lat, river.lon
                lat_t, lon_t = gateway [0], gateway [1]
                # If both margins are OK
                if abs (lat_s - lat_t) <= self.lat_margin and abs (lon_s - lon_t) <= self.lon_margin:                    
                    dist = geopy.distance.distance ([lat_t, lon_t], [lat_s, lon_s]).m
                    # If distance threshold for signal range is OK
                    if dist <= self.distance_threshold + self.distance_error_margin:
                        # Then this river point is within the range
                        total_in_range += 1
        return total_in_range