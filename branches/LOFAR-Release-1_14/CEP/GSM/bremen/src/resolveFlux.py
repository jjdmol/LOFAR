#!/usr/bin/python
from math import log10
from src.resolve import BasicResolver


class FluxResolver(BasicResolver):
    """
    Resolve a 2-to-2 problem by taking fluxes in account.
    Resolve if the distance in the flux space is different by a factor of 10.
    """

    def get_flux_distance(self, i1, i2):
        """
        Get de Ruiter distance in the coordinates + flux space.
        """
        dist = 1e-12
        for i in range(1, 6, 2):
            dist = dist + (i1[i] - i2[i])*(i1[i] - i2[i]) / \
                          (i1[i+1] * i1[i+1] + i2[i+1] * i2[i+1])
        return dist

    def resolve(self, detections, sources):
        """
        Run resolver.
        """
        solution = []
        if (len(detections) == 2 and len(sources) == 2):
            matr = [
                [self.get_flux_distance(detections[0], sources[0]),
                 self.get_flux_distance(detections[0], sources[1])],
                [self.get_flux_distance(detections[1], sources[0]),
                 self.get_flux_distance(detections[1], sources[1])]
            ]
            if    (log10(matr[0][0] / matr[0][1]) > 1
               and log10(matr[1][0] / matr[1][1]) < -1
               and matr[0][1] < 1
               and matr[1][0] < 1
               and matr[0][0] > 1
               and matr[1][1] > 1):
                solution = [[detections[0][0], sources[1][0]],
                            [detections[1][0], sources[0][0]]]
                ok = True
            elif  (log10(matr[0][0] / matr[0][1]) < -1
               and log10(matr[1][0] / matr[1][1]) > 1
               and matr[0][0] < 1
               and matr[1][1] < 1
               and matr[0][1] > 1
               and matr[1][0] > 1):
                solution = [[detections[0][0], sources[0][0]],
                            [detections[1][0], sources[1][0]]]
                ok = True
            else:
                ok = False
        else:
            ok = False

        return ok, solution
