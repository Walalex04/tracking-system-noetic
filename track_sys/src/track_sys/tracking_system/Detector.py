import cv2
from cv2 import aruco
import numpy as np


class Detector:

    def __init__(self):
        """
        Inicialización del detector
        """

        self.dictionary = aruco.Dictionary_get(aruco.DICT_4X4_250)
        self.detectorParms = aruco.DetectorParameters_create()

        # Variable persistente para el trail
        self.last_frame = None
        self.trail_mask = None   # ← capa del trail
        self.last_positions = {}   # ← guardamos última posición por ID

    def detect_position_robots(self, frame):

        markerCorners, markerIds, _ = aruco.detectMarkers(
            frame,
            self.dictionary,
            parameters=self.detectorParms
        )

        pos_robot_detect = {}

        if markerIds is not None and len(markerIds) > 0:

            ids = markerIds.flatten()

            for i, mid in enumerate(ids):
                points = markerCorners[i][0]
                cx = int(points.mean(axis=0)[0])
                cy = int(points.mean(axis=0)[1])

                pos_robot_detect[str(mid)] = (cx, cy)

        return pos_robot_detect


    def detect_trail(self, frame):

        markerCorners, markerIds, _ = aruco.detectMarkers(
            frame,
            self.dictionary,
            parameters=self.detectorParms
        )

        if self.trail_mask is None:
            self.trail_mask = np.zeros_like(frame)

        if markerIds is not None and len(markerIds) > 0:

            ids = markerIds.flatten()

            for i, mid in enumerate(ids):

                points = markerCorners[i][0]
                cx = int(points.mean(axis=0)[0])
                cy = int(points.mean(axis=0)[1])

                current_pos = (cx, cy)

                # Si ya teníamos una posición previa → dibujamos línea
                if mid in self.last_positions:
                    prev_pos = self.last_positions[mid]

                    cv2.line(
                        self.trail_mask,
                        prev_pos,
                        current_pos,
                        (0, 125, 255),
                        3
                    )

                # Actualizamos última posición
                self.last_positions[mid] = current_pos

        combined = cv2.add(frame, self.trail_mask)

        return combined

    def detect_pos_arena(self, frame):
        pass