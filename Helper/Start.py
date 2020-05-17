#Importamos las librerias
import urllib.request, json, serial, os, time, sys, math, subprocess,atexit
import serial.tools.list_ports

clear_cmd = lambda: os.system('cls')
ip = "localhost"
port = "55555"
arduino = None
HWiNFO64_dir = str(os.getcwd()) + "\\HWiNFO64"
RSM_dir = str(os.getcwd()) + "\\Remote Sensor Monitor"
selected_com = None
com_list = None
def welcome():
    global selected_com
    global com_list
    print("Arduino HWMONITOR Helper\n")
    print("Working Dir:" + str(os.getcwd()))
    print("COM PORT LIST:\n")
    com_list = [comport.device for comport in serial.tools.list_ports.comports()]
    com_index = 0
    for x in com_list:
        print(str(com_index + 1) + ") " + com_list[com_index])
        com_index += 1
    selected_com = input("Select COM PORT:")
    if ((selected_com.isdigit() == False) or (int(selected_com) > com_index) or (int(selected_com) < 1)):
        print("\nInvalid COM Port")
        selected_com = 0
        time.sleep(5)
        welcome()
    launchprogrmans()
    return

def launchprogrmans():
    global arduino
    global selected_com
    global com_list
    SW_MINIMIZE = 6
    info = subprocess.STARTUPINFO()
    info.dwFlags = subprocess.STARTF_USESHOWWINDOW
    info.wShowWindow = SW_MINIMIZE
    print("Waiting for programs...\n")
    print("HWINFO START\n")
    subprocess.Popen(HWiNFO64_dir + "\\HWiNFO64.exe", startupinfo=info)
    time.sleep(10)
    print("HWINFO OK\n")
    print("REMOTE SENSOR MONITOR START\n")
    subprocess.Popen(RSM_dir + "\\Remote Sensor Monitor.exe --ohm=0 --AIDA64=0 --gpuz=0", startupinfo=info)
    time.sleep(5)
    arduino = serial.Serial(str(com_list[int(selected_com) - 1]),115200)
    return

def killprograms(start):
    print("Killing Programs...")
    subprocess.call(['taskkill.exe', '/F', '/IM', 'HWiNFO64.exe'])
    subprocess.call(['taskkill.exe', '/F', '/IM', 'Remote Sensor Monitor.exe'])
    if start == False:
        exit()

def SendJSON():
    global ip
    global port
    global arduino
    send_data_B1 = ""
    send_data_B2 = ""
    try:
        with urllib.request.urlopen("http://" + str(ip) + ":" + str(port)) as url:
            data = json.loads(url.read().decode())
            reading = arduino.readline()
            for x in range (len(data)):
            #First Chunk
                if data[x]["SensorName"] == "Physical Memory Used":
                    send_data_B1 +=("UR" + ":" + str(data[x]["SensorValue"]) + ":")
                if data[x]["SensorName"] == "Physical Memory Available":
                    send_data_B1 +=("FR" + ":" + str(data[x]["SensorValue"]) + ":")
                if data[x]["SensorName"] == "GPU D3D Memory Dedicated":
                    VRAM_USED = (data[x]["SensorValue"].replace(',','.'))
                    send_data_B1 +=("UV" + ":" + str((math.ceil(float(VRAM_USED)))) + ":")
                if data[x]["SensorName"] == "Memory Clock":
                    RAM_CLK = (data[x]["SensorValue"].replace(',','.'))
                    send_data_B1 +=("RC" + ":" + str(math.ceil(round(float(RAM_CLK),-1) * 2)) + ":")
                if data[x]["SensorName"] == "Memory Controller Clock (UCLK)":
                    RAM_CONTROLLER_CLK = (data[x]["SensorValue"].replace(',','.'))
                    send_data_B1 +=("RCC" + ":" + str((math.ceil(float(RAM_CONTROLLER_CLK)))) + ":")
                if data[x]["SensorName"] == "Page File Usage":
                    PAGE_FILE_USAGE = (data[x]["SensorValue"].replace(',','.'))
                    send_data_B1 +=("PF" + ":" + str((math.ceil(float(PAGE_FILE_USAGE)))) + ":")
                if data[x]["SensorName"] == "Average Effective Clock":
                    CPU_CLK = (data[x]["SensorValue"].replace(',','.'))
                    send_data_B1 +=("CCA" + ":" + str((math.ceil(float(CPU_CLK)))) + ":")
                if data[x]["SensorName"] == "Core 0 T0 Effective Clock":
                    CPU_CLK_C0 = (data[x]["SensorValue"].replace(',','.'))
                    send_data_B1 +=("C0C" + ":" + str((math.ceil(float(CPU_CLK_C0)))) + ":")
                if data[x]["SensorName"] == "Core 0 VID":
                    CPU_VID_C0 = (data[x]["SensorValue"].replace(',','.'))
                    send_data_B1 +=("C0V" + ":" + str(round(float(CPU_VID_C0),2)) + ":")
                if data[x]["SensorName"] == "Core 1 T0 Effective Clock":
                    CPU_CLK_C1 = (data[x]["SensorValue"].replace(',','.'))
                    send_data_B1 +=("C1C" + ":" + str((math.ceil(float(CPU_CLK_C1)))) + ":")
                if data[x]["SensorName"] == "Core 1 VID":
                    CPU_VID_C1 = (data[x]["SensorValue"].replace(',','.'))
                    send_data_B1 +=("C1V" + ":" + str(round(float(CPU_VID_C1),2)) + ":")
                if data[x]["SensorName"] == "Core 2 T0 Effective Clock":
                    CPU_CLK_C2 = (data[x]["SensorValue"].replace(',','.'))
                    send_data_B1 +=("C2C" + ":" + str((math.ceil(float(CPU_CLK_C2)))) + ":")
                if data[x]["SensorName"] == "Core 2 VID":
                    CPU_VID_C2 = (data[x]["SensorValue"].replace(',','.'))
                    send_data_B1 +=("C2V" + ":" + str(round(float(CPU_VID_C2),2)) + ":")
                if data[x]["SensorName"] == "Core 3 T0 Effective Clock":
                    CPU_CLK_C3 = (data[x]["SensorValue"].replace(',','.'))
                    send_data_B1 +=("C3C" + ":" + str((math.ceil(float(CPU_CLK_C3)))) + ":")
                if data[x]["SensorName"] == "Core 3 VID":
                    CPU_VID_C3 = (data[x]["SensorValue"].replace(',','.'))
                    send_data_B1 +=("C3V" + ":" + str(round(float(CPU_VID_C3),2)) + ":")
                if data[x]["SensorName"] == "Total CPU Usage":
                    CPU_USAGE = (data[x]["SensorValue"].replace(',','.'))
                    send_data_B1 +=("CUA" + ":" + str((math.ceil(float(CPU_USAGE)))) + ":")
                if data[x]["SensorName"] == "CPU (Tctl/Tdie)":
                    CPU_TEMP = (data[x]["SensorValue"].replace(',','.'))
                    send_data_B1 +=("CT" + ":" + str((math.ceil(float(CPU_TEMP)))) + ":")
                if data[x]["SensorName"] == "CPU Core Voltage (SVI2 TFN)":
                    CPU_VCORE = (data[x]["SensorValue"].replace(',','.'))
                    send_data_B1 +=("CVT" + ":" + str(round(float(CPU_VCORE),2)) + ":")
                if data[x]["SensorName"] == "SoC Voltage (SVI2 TFN)":
                    CPU_VSOC = (data[x]["SensorValue"].replace(',','.'))
                    send_data_B1 +=("CVS" + ":" + str(round(float(CPU_VSOC),2)) + ":")
                if data[x]["SensorName"] == "GPU Temperature":
                    GPU_TEMP = (data[x]["SensorValue"].replace(',','.'))
                    send_data_B1 +=("GT" + ":" + str((math.ceil(float(GPU_TEMP)))) + ":")
                if data[x]["SensorName"] == "GPU Clock":
                    GPU_CLK = (data[x]["SensorValue"].replace(',','.'))
                    send_data_B1 +=("GC" + ":" + str((math.ceil(float(GPU_CLK)))) + ":")
                if data[x]["SensorName"] == "GPU Memory Clock":
                    VRAM_CLK = (data[x]["SensorValue"].replace(',','.'))
                    send_data_B1 +=("VRC" + ":" + str((math.ceil(float(VRAM_CLK)))) + ":")
                if data[x]["SensorName"] == "SoC Voltage (SVI2 TFN)":
                    GPU_VCORE = (data[x]["SensorValue"].replace(',','.'))
                    send_data_B1 +=("GV" + ":" + str(round(float(GPU_VCORE),2)) + ":")
                if data[x]["SensorName"] == "Frames per Second":
                    GPU_FPS = (data[x]["SensorValue"].replace(',','.'))
                    send_data_B1 +=("FP" + ":" + str((math.ceil(float(GPU_FPS)))) + ":")
                if data[x]["SensorName"] == "GPU D3D Usage":
                    GPU_USAGE = (data[x]["SensorValue"].replace(',','.'))
                    send_data_B1 +=("GU" + ":" + str((math.ceil(float(GPU_USAGE)))) + ":")
                if data[x]["SensorClass"] == "Network: Atheros/Qualcomm QCA9377 802.11ac Wireless Network Adapter" and data [x]["SensorName"] == "Current DL rate":
                    DW_SPEED = math.ceil(float(data[x]["SensorValue"].replace(',','.'))) // 125
                    send_data_B1 +=("DW" + ":" + str(DW_SPEED) + ":")
                if data[x]["SensorClass"] == "Network: Atheros/Qualcomm QCA9377 802.11ac Wireless Network Adapter" and data [x]["SensorName"] == "Current UP rate":
                    UP_SPEED = math.ceil(float(data[x]["SensorValue"].replace(',','.'))) // 125
                    send_data_B1 +=("UP" + ":" + str(UP_SPEED) + ":")

        if (reading == bytes('<WAITING FOR HELPER>\r\n', 'UTF-8')):
            arduino.write(b'OK')
        elif (reading == bytes('<WAITINGB1>\r\n', 'UTF-8')):
            string = "<" + send_data_B1 + ">"
            stringbytes = bytes(str(string), 'UTF-8')
            arduino.write(stringbytes)
            print(stringbytes)
        elif (reading == bytes('<WAITINGB2>\r\n', 'UTF-8')):
            string = "<" + send_data_B2 + ">"
            stringbytes = bytes(str(string), 'UTF-8')
            arduino.write(stringbytes)
            print(stringbytes)
        else:
            arduino.write(b'a')
    except ConnectionRefusedError:
        print("Overhead, RETRY")
        time.sleep(1)
        arduino.close()
        killprograms(True)
        launchprogrmans()
        SendJSON()
    except KeyboardInterrupt:
        arduino.close()
        killprograms(False)
        
    except serial.SerialException:
        killprograms(False)
    except urllib.error.HTTPError:
        print("Overhead, RETRY")
        time.sleep(1)
        arduino.close()
        killprograms(True)
        launchprogrmans()
        SendJSON()


killprograms(True)   
welcome()
print("ARDUINO OK")
print("Press CTRL + C to close...")
while True:
    SendJSON()
