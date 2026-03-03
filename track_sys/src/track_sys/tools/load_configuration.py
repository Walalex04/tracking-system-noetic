

import rospkg
import os
import yaml

rospack = rospkg.RosPack()

package_path = rospack.get_path('track_sys')

config_path = os.path.join(package_path, 'config', 'tracking_sys.yaml')

with open(config_path, 'r') as f:
    config = yaml.safe_load(f)



def get_path_aruco():
    return config["tracking_system"]["paths_files"]["output"][0]["aruco_files"]


def get_number_arucos():
    return config["tracking_system"]["aruco_configuration"][0]["amount_aruco"]

def get_marker_size():
    return config["tracking_system"]["aruco_configuration"][2]["marker_size"]