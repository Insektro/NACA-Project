import serial

def leer_puerto_com(mypuerto, speed, nombre,samples, baudrate=115200):
    # Configuración de la conexión serie
    puerto = 'COM' + str(mypuerto)
    print(puerto)
    ser = serial.Serial(puerto, baudrate, timeout=1)  # Abre el puerto con el baudrate indicado
    
    # Asegurarse de que el puerto esté abierto correctamente
    if not ser.is_open:
        print(f"Error al abrir el puerto {puerto}")
        return
    
    print(f"Lectura de datos en el puerto {puerto}...")
    noDatos = True
    try:
        for i in samples:
            with open(f"{nombre}.txt", "a") as archivo:  # Abrimos el archivo en modo 'append'
                while noDatos:
                    # Leer los datos desde el puerto serial
                    datos = ser.readline().decode('utf-8').strip()  # Lee una línea y decodifica a UTF-8
                    
                    # Si los datos no están vacíos y contienen el formato esperado
                    if datos and len(datos.split(';')) == 9:
                        archivo.write(speed + ';')
                        archivo.write(datos + '\n')  # Guardamos los datos con un salto de línea
                        print(f"Datos guardados: {datos}")
                        noDatos = False
                    else:
                        print("Datos no válidos recibidos.")
    
    except KeyboardInterrupt:
        print("\nLectura interrumpida por el usuario.")
    
    finally:
        ser.close()  # Cerramos el puerto cuando se termina