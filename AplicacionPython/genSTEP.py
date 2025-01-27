import numpy as np
import cadquery as cq


def naca4_coordinates(m, p, xx, l, num_points=1000):
    x = np.linspace(0, 1, num_points)
    yt = 5 * xx * (0.2969 * np.sqrt(x) - 0.1260 * x - 0.3516 * x**2 + 0.2843 * x**3 - 0.1015 * x**4)
    yc = np.where(x < p, m/p**2 * (2*p*x - x**2), m/(1-p)**2 * (1 - 2*p + 2*p*x - x**2))
    dyc_dx = np.where(x < p, 2*m/p**2 * (p - x), 2*m/(1-p)**2 * (p - x))
    theta = np.arctan(dyc_dx)

    xu = (x - yt * np.sin(theta))*l
    xl = (x + yt * np.sin(theta))*l
    yu = (yc + yt * np.cos(theta))*l
    yl = (yc - yt * np.cos(theta))*l

    upper_surface = np.vstack((xu, yu)).T
    lower_surface = np.vstack((xl, yl)).T

    coordinates = np.concatenate((upper_surface, lower_surface[::-1]))

    return coordinates
'''
# Define the NACA 4-digit code parameters
m = 0.04  # Maximum camber
p = 0.40001   # Position of maximum camber
xx = 0.20 # Maximum thickness
l = 150 # Cord length (mm)
'''

def generar(m,p,xx,l):
    # Generate the coordinates of the NACA airfoil
    puntos = naca4_coordinates(m, p, xx, l)

    # Crear un objeto Workplane
    wp = cq.Workplane("XY")

    # Crear la figura uniendo los puntos
    wp = wp.polyline(puntos).close().extrude(100)

    # Exportar a archivo STEP
    cq.exporters.export(wp, f'naca1mm{int(m*100)}{int(p*10)}{int(xx*100)}.step')