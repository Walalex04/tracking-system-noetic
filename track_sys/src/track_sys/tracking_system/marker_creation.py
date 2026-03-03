
import cv2 as cv
from cv2 import aruco
import matplotlib.pyplot as plt
from track_sys.tools.load_configuration import get_path_aruco, get_number_arucos, get_marker_size
import rospkg
import os



"""

route_arauco = get_route_aruco()


## SELECCIONAR DICCIONARIO DE MARCADORES 

dict = cv.aruco.getPredefinedDictionary(cv.aruco.DICT_4X4_250)


## SETEAR PARAMETROS DE DETECCION

markerId = 0
markerSize = 250 ## TAMAÑO DEL MARCADOR EN PIXELES
num_images_generated = 20

for id in range(num_images_generated):
    marker = cv.aruco.generateImageMarker(dict, id, markerSize)
    cv.imwrite(os.path.join(route_arauco, ("code_arauco_" + str(id) + ".jpg")), marker)

## MOSTRAR MARCADOR 

#plt.imshow(marker, cmap="gray")
#plt.show()


#generating differents aRuco and save it

"""

def generate_markers():
    rospack = rospkg.RosPack()
    pkg_path = rospack.get_path("track_sys")

    route = get_path_aruco()
    amount_arucos = get_number_arucos()
    markerSize = get_marker_size()

    output_dir = os.path.join(pkg_path, route)
    print(f"the path is {output_dir}")
    os.makedirs(output_dir, exist_ok=True)


    """
    TO DO:
        make dictionary for relationship bewteen de string with objec dictioary
    """

    dictionary = aruco.getPredefinedDictionary(cv.aruco.DICT_4X4_250) 

    for id in range(amount_arucos):
        marker = aruco.drawMarker(dictionary, id, markerSize)
        cv.imwrite(os.path.join(output_dir, ("code_arauco_" + str(id) + ".jpg")), marker)

    print("generation aruco ....")
