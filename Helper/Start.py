#Importamos las librerias
import urllib.request, json, serial, os, time, sys, math, subprocess,atexit
import serial.tools.list_ports

clear_cmd = lambda: os.system('cls')
ip = "localhost"
port = "55555"
arduino = None
HWiNFO64_dir = str(os.getcwd()) + "\\HWiNFO64"
RSM_dir = str(os.getcwd()) + "\\Remote Sensor Monitor"

def welcome():

    global arduino
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
    arduino = serial.Serial(str(com_list[int(selected_com) - 1]),115200)
    launchprogrmans()
    return

def launchprogrmans():
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
    send_data = ""
    try:
        with urllib.request.urlopen("http://" + str(ip) + ":" + str(port)) as url:
            data = json.loads(url.read().decode())
            reading = arduino.readline()
            for x in range (len(data)):
                if data[x]["SensorName"] == "Physical Memory Used":
                    send_data +=("UR" + ":" + str(data[x]["SensorValue"]) + ":")
                if data[x]["SensorName"] == "Physical Memory Available":
                    send_data +=("FR" + ":" + str(data[x]["SensorValue"]) + ":")
                if data[x]["SensorName"] == "Average Effective Clock":
                    CPU_CLK = (data[x]["SensorValue"].replace(',','.'))
                    send_data +=("CC" + ":" + str((math.ceil(float(CPU_CLK)))) + ":")
                if data[x]["SensorName"] == "Total CPU Usage":
                    CPU_USAGE = (data[x]["SensorValue"].replace(',','.'))
                    send_data +=("CU" + ":" + str((math.ceil(float(CPU_USAGE)))) + ":")
                if data[x]["SensorName"] == "CPU (Tctl/Tdie)":
                    CPU_TEMP = (data[x]["SensorValue"].replace(',','.'))
                    send_data +=("CT" + ":" + str((math.ceil(float(CPU_TEMP)))) + ":")
                if data[x]["SensorName"] == "CPU Core Voltage (SVI2 TFN)":
                    CPU_VCORE = (data[x]["SensorValue"].replace(',','.'))
                    send_data +=("CV" + ":" + str(round(float(CPU_VCORE),2)) + ":")
                if data[x]["SensorName"] == "GPU Temperature":
                    GPU_TEMP = (data[x]["SensorValue"].replace(',','.'))
                    send_data +=("GT" + ":" + str((math.ceil(float(GPU_TEMP)))) + ":")
                if data[x]["SensorName"] == "GPU Clock":
                    GPU_CLK = (data[x]["SensorValue"].replace(',','.'))
                    send_data +=("GC" + ":" + str((math.ceil(float(GPU_CLK)))) + ":")
                if data[x]["SensorName"] == "SoC Voltage (SVI2 TFN)":
                    GPU_VCORE = (data[x]["SensorValue"].replace(',','.'))
                    send_data +=("GV" + ":" + str(round(float(GPU_VCORE),2)) + ":")
                if data[x]["SensorName"] == "Frames per Second":
                    GPU_FPS = (data[x]["SensorValue"].replace(',','.'))
                    send_data +=("FP" + ":" + str((math.ceil(float(GPU_FPS)))) + ":")
        if (reading == bytes('<WAITING FOR HELPER>\r\n', 'UTF-8')):
            arduino.write(b'OK')
        if (reading == bytes('<WAITING>\r\n', 'UTF-8')):
            string = "<" + send_data + ">"
            stringbytes = bytes(str(string), 'UTF-8')
            arduino.write(stringbytes)
        else:
            arduino.write(b'a')
    except ConnectionRefusedError:
        print("Overhead, RETRY")
        time.sleep(1)
        killprograms(True)
        launchprogrmans()
        SendJSON()
    except KeyboardInterrupt:
        killprograms(False)
        
    except serial.SerialException:
        killprograms(False)
    except urllib.error.HTTPError:
        print("Overhead, RETRY")
        time.sleep(1)
        killprograms(True)
        launchprogrmans()
        SendJSON()


killprograms(True)   
welcome()
print("ARDUINO OK")
print("Press CTRL + C to close...")
while True:
    SendJSON()
