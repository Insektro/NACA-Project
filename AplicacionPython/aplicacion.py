# Title: aplicacion.py
# Author: Alberto Rived
# Version 1.0
# Brief: Aplication for generate airfoil, read pressure and plot


import UART
import openFile
import genSTEP
import drawNaca
import tkinter as tk
from tkinter import ttk
from functools import partial


def mostrar_pantalla1():
    limpiar_ventana()
    ventana.geometry("300x300")

    label1 = tk.Label(ventana, text="Parámetro M:")
    label1.grid(row=0, column=0)
    entry1 = ttk.Entry()
    entry1.grid(row=0, column=1)
    
    label2 = tk.Label(ventana, text="Parámetro P:")
    label2.grid(row=1, column=0)
    entry2 = ttk.Entry()
    entry2.grid(row=1, column=1)
    
    label3 = tk.Label(ventana, text="Parámetro XX:")
    label3.grid(row=2, column=0)
    entry3 = ttk.Entry()
    entry3.grid(row=2, column=1)
    
    label4 = tk.Label(ventana, text="Longitud cuerda:")
    label4.grid(row=3, column=0)
    entry4 = ttk.Entry()
    entry4.grid(row=3, column=1)
    
    # Botón para ejecutar la función
    button = tk.Button(ventana, text="Exportar stl", command=partial(export_stl,entry1, entry2, entry3, entry4))
    button.grid(row=5, column=0)
    
    button2 = tk.Button(ventana, text="Visualizar NACA", command=partial(draw_naca,entry1, entry2, entry3, entry4))
    button2.grid(row=5, column=1)
    
    btn_retorno = tk.Button(ventana, text="Volver al Menú Principal", command=mostrar_menu_principal)
    btn_retorno.grid(row=7, columnspan=2)
    
def export_stl(entry1,entry2,entry3,entry4):
    param1 = float(entry1.get())
    param2 = float(entry2.get())
    param3 = float(entry3.get())
    param4 = float(entry4.get())
    
    genSTEP.generar(param1,param2,param3,param4)

def draw_naca(entry1,entry2,entry3,entry4): 
    param1 = float(entry1.get())
    param2 = float(entry2.get())
    param3 = float(entry3.get())
    param4 = float(entry4.get())
    
    drawNaca.plot_naca4(param1,param2,param3,param4)

def mostrar_pantalla2():

    limpiar_ventana()
    ventana.geometry("300x300")

    label1 = tk.Label(ventana, text="Puerto COM:")
    label1.grid(row=0, column=0)
    entry1 = ttk.Entry()
    entry1.grid(row=0, column=1)  

    label2 = tk.Label(ventana, text="Velocidad (km/h):")
    label2.grid(row=1, column=0)
    entry2 = ttk.Entry()
    entry2.grid(row=1, column=1) 

    label3 = tk.Label(ventana, text="Nombre archivo:")
    label3.grid(row=2, column=0)
    entry3 = ttk.Entry()
    entry3.grid(row=2, column=1) 

    label4 = tk.Label(ventana, text="Número de muestras:")
    label4.grid(row=3, column=0)
    entry4 = ttk.Entry()
    entry4.grid(row=3, column=1)   
    
    button = tk.Button(ventana, text="Leer", command=partial(leerUART,entry1,entry2,entry3,entry4))
    button.grid(row=4, columnspan=2)

    btn_retorno = tk.Button(ventana, text="Volver al Menú Principal", command=mostrar_menu_principal)
    btn_retorno.grid(row=5, columnspan=2)
    
def leerUART(entry1,entry2,entry3,entry4):
        param1 = entry1.get()
        param2 = entry2.get()
        param3 = entry3.get()
        param4 = entry4.get()
        UART.leer_puerto_com(param1,param2,param3,param4)
    
    
def mostrar_pantalla3():
    limpiar_ventana()
    ventana.geometry("700x300")
    label1 = tk.Label(ventana, text="Distancias Extradós:")
    label1.grid(row=0, column=0)
    entry1 = ttk.Entry()
    entry1.grid(row=0, column=1)
    entry2 = ttk.Entry()
    entry2.grid(row=0, column=2) 
    entry3 = ttk.Entry()
    entry3.grid(row=0, column=3) 
    entry4 = ttk.Entry()
    entry4.grid(row=0, column=4)     
    label2 = tk.Label(ventana, text="Distancias Intradós:")
    label2.grid(row=1, column=0)
    entry5 = ttk.Entry()
    entry5.grid(row=1, column=1)
    entry6 = ttk.Entry()
    entry6.grid(row=1, column=2) 
    entry7 = ttk.Entry()
    entry7.grid(row=1, column=3) 
    entry8 = ttk.Entry()
    entry8.grid(row=1, column=4)  
    
    button = tk.Button(ventana, text="Abrir", command=partial(abrirArch,entry1,entry2,entry3,entry4,entry5,entry6,entry7,entry8))
    button.grid(row=3, columnspan=2)

    btn_retorno = tk.Button(ventana, text="Volver al Menú Principal", command=mostrar_menu_principal)
    btn_retorno.grid(row=4, columnspan=2)

def abrirArch(entry1,entry2,entry3,entry4,entry5,entry6,entry7,entry8):
        param1 = float(entry1.get())
        param2 = float(entry2.get())
        param3 = float(entry3.get())
        param4 = float(entry4.get())
        param5 = float(entry5.get())
        param6 = float(entry6.get())
        param7 = float(entry7.get())
        param8 = float(entry8.get())
        openFile.abrir_archivo(param1,param2,param3,param4,param5,param6,param7,param8)

def mostrar_menu_principal():
    limpiar_ventana()
    ventana.geometry("500x300")
    btn1 = tk.Button(ventana, text="Crear NACA", height = 4, width = 15, command=mostrar_pantalla1)
    btn1.place(x=30,y=30)
    btn2 = tk.Button(ventana, text="Medir", height = 4, width = 15, command=mostrar_pantalla2)
    btn2.place(x=170,y=30)
    btn3 = tk.Button(ventana, text="Abrir archivo", height = 4, width = 15, command=mostrar_pantalla3)
    btn3.place(x=310,y=30)


def limpiar_ventana():
    for widget in ventana.winfo_children():
        widget.destroy()
        

# Crear la ventana principal
ventana = tk.Tk()
ventana.geometry("1000x600")
ventana.title("NACA Project")

# Mostrar el menú principal al iniciar
mostrar_menu_principal()

# Iniciar el bucle principal
ventana.mainloop()
