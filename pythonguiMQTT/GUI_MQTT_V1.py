import tkinter as tk
from tkinter import messagebox
from influxdb_client import InfluxDBClient, Point
from influxdb_client.client.write_api import SYNCHRONOUS
import threading
import paho.mqtt.client as mqtt
import webbrowser

class App:
    def __init__(self, root):
        # Configuración de InfluxDB
        self.token = TOKEN
        self.org = "PDS_IA"
        self.bucket = "MQTT_IA"
        self.url = "http://localhost:8086"
        self.client_influx = InfluxDBClient(url=self.url, token=self.token, org=self.org)
        self.write_api = self.client_influx.write_api(write_options=SYNCHRONOUS)

        # Lista de ubicaciones y sus tópicos
        self.topicos = {
            "LaboratorioIA/Temperatura [°C]": "temperatura [°C]",
            "LaboratorioIA/Humedad [%]": "humedad [%]",
            "LaboratorioIA/CO_ppm": "co_ppm"
        }

        self.client_mqtt = None
        self.mqtt_thread = None

        # Configuración de la interfaz gráfica
        self.root = root
        self.root.title("Sistema IOT")
        
        self.create_widgets()

    def create_widgets(self):
        """Crear los widgets de la interfaz gráfica"""
        # Frame para los detalles del broker MQTT
        mqtt_frame = tk.LabelFrame(self.root, text="MQTT Broker", padx=10, pady=10)
        mqtt_frame.grid(row=0, column=0, padx=10, pady=10)

        tk.Label(mqtt_frame, text="Host:").grid(row=0, column=0)
        self.host_entry = tk.Entry(mqtt_frame)
        self.host_entry.grid(row=0, column=1)

        tk.Label(mqtt_frame, text="Puerto:").grid(row=1, column=0)
        self.port_entry = tk.Entry(mqtt_frame)
        self.port_entry.grid(row=1, column=1)

        tk.Label(mqtt_frame, text="Usuario:").grid(row=2, column=0)
        self.user_entry = tk.Entry(mqtt_frame)
        self.user_entry.grid(row=2, column=1)

        tk.Label(mqtt_frame, text="Contraseña:").grid(row=3, column=0)
        self.pass_entry = tk.Entry(mqtt_frame, show="*")
        self.pass_entry.grid(row=3, column=1)

        # Botones para conectar/desconectar
        connect_button = tk.Button(mqtt_frame, text="Conectar", command=self.start_mqtt)
        connect_button.grid(row=4, column=0, columnspan=2, pady=5)

        disconnect_button = tk.Button(mqtt_frame, text="Desconectar", command=self.stop_mqtt)
        disconnect_button.grid(row=5, column=0, columnspan=2)

        # Botón para abrir Grafana
        grafana_button = tk.Button(mqtt_frame, text="Abrir Grafana", command=self.abrir_grafana)
        grafana_button.grid(row=7, column=0, columnspan=2, pady=5)

        # Área para mostrar mensajes de MQTT
        self.text_box = tk.Text(self.root, height=15, width=50)
        self.text_box.grid(row=1, column=0, padx=10, pady=10)

    # Función que se ejecuta cuando el cliente se conecta al broker
    def on_connect(self, client, userdata, flags, rc):
        if rc == 0:
            messagebox.showinfo("MQTT", "Conectado exitosamente al broker")
            # Suscribirse a los tópicos
            for topico in self.topicos.keys():
                client.subscribe(topico)
                self.text_box.insert(tk.END, f"Suscrito al tópico: {topico}\n")
            self.text_box.insert(tk.END, f"Para ver los datos, ingresar a la pagina\n")
        else:
            messagebox.showerror("MQTT", f"Conexión fallida con el código de resultado {rc}")

    # Función que se ejecuta cuando un mensaje es recibido
    def on_message(self, client, userdata, msg):
        medicion = self.topicos.get(msg.topic)
        if medicion:
            ubicacion = msg.topic.split('/')[0]
            payload = msg.payload.decode()

            # Procesar el payload
            try:
                valor = float(payload)
            except ValueError:
                valor = None

            if valor is not None:
                punto = Point(medicion).tag("ubicacion", ubicacion).field("valor", valor)
                self.write_api.write(bucket=self.bucket, org=self.org, record=punto)
                # Mostrar el mensaje en la GUI
                # self.text_box.insert(tk.END, f"Guardado en InfluxDB: {valor} ' (Medición: {medicion},ubicacion:{ubicacion})\n")
            else:
                self.text_box.insert(tk.END, f"Error al convertir el valor: {payload}\n")

    # Función para iniciar la conexión MQTT
    def start_mqtt(self):
        host = self.host_entry.get()
        port = int(self.port_entry.get())
        user = self.user_entry.get()
        password = self.pass_entry.get()

        self.client_mqtt = mqtt.Client()
        self.client_mqtt.username_pw_set(user, password)
        self.client_mqtt.on_connect = self.on_connect
        self.client_mqtt.on_message = self.on_message
        self.client_mqtt.connect(host, port)

        self.mqtt_thread = threading.Thread(target=self.client_mqtt.loop_forever)
        self.mqtt_thread.start()

    # Función para detener la conexión MQTT
    def stop_mqtt(self):
        if self.client_mqtt:
            self.client_mqtt.disconnect()
            self.client_mqtt = None
            self.mqtt_thread = None
            self.text_box.insert(tk.END, "Desconectado del broker\n")

    def abrir_grafana(self):
        webbrowser.open("http://localhost:3000/d/bdy3k261tfpj4c/mqtt-becacin?orgId=1&refresh=5s&from=now-1h&to=now")

# Main
if __name__ == "__main__":
    root = tk.Tk()
    app = App(root)
    root.mainloop()