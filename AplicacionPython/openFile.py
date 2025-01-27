import tkinter as tk
from tkinter import filedialog
import matplotlib.pyplot as plt
import numpy as np

def abrir_archivo(dist1,dist2,dist3,dist4,dist5,dist6,dist7,dist8):
    # Abrir cuadro de diálogo para seleccionar un archivo
    archivo_path = filedialog.askopenfilename(title="Abrir archivo", filetypes=[("Archivos de texto", "*.txt")])
    distancias_intra = [dist5,dist6,dist7,dist8]
    distancias_extra = [dist1,dist2,dist3,dist4]
    if archivo_path:
        try:
            # Leer el archivo
            with open(archivo_path, "r") as archivo:
                lineas = archivo.readlines()
                
                # Tomamos la primera línea y la procesamos
                for linea in lineas:
                    datos = linea.strip().split(";")  # Dividimos por punto y coma
                    if len(datos) == 10:
                        # Tomamos los 8 últimos datos
                        datos_ultimos_8 = list(map(float, datos[-8:]))

                        datos_extra = [datos_ultimos_8[4],datos_ultimos_8[5],datos_ultimos_8[6],datos_ultimos_8[7]]
                        datos_intra = [datos_ultimos_8[0],datos_ultimos_8[1],datos_ultimos_8[2],datos_ultimos_8[3]]

                        # Llamamos a la función para graficar
                        graficar(distancias_extra, distancias_intra, datos_extra,datos_intra,datos[0],datos[1])
                        #break
                    else:
                        print("La línea no tiene 10 datos.")
        except Exception as e:
            print(f"Error al leer el archivo: {e}")
        
def graficar(di_e,di_i,da_e,da_i, velo, angl):
    plt.plot(di_e, da_e, marker='o', linestyle='-', color='b', label='Intrados')
    plt.plot(di_i, da_i, marker='o', linestyle='-', color='r', label='Extrados')
    #plt.plot(distancias[4:], datos[4:], marker='o', linestyle='-', color='b', label='Intrados')
    #plt.plot(distancias[:4], datos[:4], marker='o', linestyle='-', color='r', label='Extrados')   
    plt.title(f"Presión a {velo} km/h y {angl}º")
    plt.xlabel("Distancia (mm)")
    plt.ylabel("Presión (Pa)")
    plt.legend()
    plt.grid(True)
    plt.show()