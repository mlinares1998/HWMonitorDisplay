"""
PC HARDWARE MONITOR DISPLAY FOR ARDUINO
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
"""
#Importamos las librerias
import urllib.request, json, serial, os, time, sys, math, subprocess,atexit,datetime,time
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
    print("PC HARDWARE MONITOR DISPLAY FOR ARDUINO HELPER\n")
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
    send_data_B3 = ""
    send_data_B4 = ""
    send_data_B5 = ""
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
                if data[x]["SensorName"] == "Memory Clock":
                    RAM_CLK = (data[x]["SensorValue"].replace(',','.'))
                    send_data_B1 +=("RC" + ":" + str(math.ceil(round(float(RAM_CLK),-1) * 2)) + ":")
                if data[x]["SensorName"] == "Memory Controller Clock (UCLK)":
                    RAM_CONTROLLER_CLK = (data[x]["SensorValue"].replace(',','.'))
                    send_data_B1 +=("RCC" + ":" + str((math.ceil(float(RAM_CONTROLLER_CLK)))) + ":")
                if data[x]["SensorName"] == "Page File Usage":
                    PAGE_FILE_USAGE = (data[x]["SensorValue"].replace(',','.'))
                    send_data_B1 +=("PF" + ":" + str((math.ceil(float(PAGE_FILE_USAGE)))) + ":")
                if data[x]["SensorName"] == "GPU Temperature":
                    GPU_TEMP = (data[x]["SensorValue"].replace(',','.'))
                    send_data_B1 +=("GT" + ":" + str((math.ceil(float(GPU_TEMP)))) + ":")
                if data[x]["SensorName"] == "GPU Clock":
                    GPU_CLK = (data[x]["SensorValue"].replace(',','.'))
                    send_data_B1 +=("GC" + ":" + str((math.ceil(float(GPU_CLK)))) + ":")
                if data[x]["SensorName"] == "GPU Memory Clock":
                    VRAM_CLK = (data[x]["SensorValue"].replace(',','.'))
                    send_data_B1 +=("VRC" + ":" + str((math.ceil(float(VRAM_CLK)))) + ":")
                if data[x]["SensorName"] == "GPU D3D Memory Dedicated":
                    VRAM_USED = (data[x]["SensorValue"].replace(',','.'))
                    send_data_B1 +=("UV" + ":" + str((math.ceil(float(VRAM_USED)))) + ":")
                if data[x]["SensorName"] == "CPU Core Voltage (SVI2 TFN)":
                    CPU_VCORE = (data[x]["SensorValue"].replace(',','.'))
                    send_data_B2 +=("CVT" + ":" + str(round(float(CPU_VCORE),2)) + ":")
                if data[x]["SensorName"] == "SoC Voltage (SVI2 TFN)":
                    CPU_VSOC = (data[x]["SensorValue"].replace(',','.'))
                    send_data_B1 +=("CVS" + ":" + str(round(float(CPU_VSOC),2)) + ":")
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
                #2nd Chunck
                if data[x]["SensorName"] == "CPU (Tctl/Tdie)":
                    CPU_TEMP = (data[x]["SensorValue"].replace(',','.'))
                    send_data_B2 +=("CT" + ":" + str((math.ceil(float(CPU_TEMP)))) + ":")
                if data[x]["SensorName"] == "Average Effective Clock":
                    CPU_CLK = (data[x]["SensorValue"].replace(',','.'))
                    send_data_B2 +=("CCA" + ":" + str((math.ceil(float(CPU_CLK)))) + ":")
                if data[x]["SensorName"] == "Core 0 T0 Effective Clock":
                    CPU_CLK_C0 = (data[x]["SensorValue"].replace(',','.'))
                    send_data_B2 +=("C0C" + ":" + str((math.ceil(float(CPU_CLK_C0)))) + ":")
                if data[x]["SensorName"] == "Core 0 VID":
                    CPU_VID_C0 = (data[x]["SensorValue"].replace(',','.'))
                    send_data_B2 +=("C0V" + ":" + str(round(float(CPU_VID_C0),2)) + ":")
                if data[x]["SensorName"] == "Core 1 T0 Effective Clock":
                    CPU_CLK_C1 = (data[x]["SensorValue"].replace(',','.'))
                    send_data_B2 +=("C1C" + ":" + str((math.ceil(float(CPU_CLK_C1)))) + ":")
                if data[x]["SensorName"] == "Core 1 VID":
                    CPU_VID_C1 = (data[x]["SensorValue"].replace(',','.'))
                    send_data_B2 +=("C1V" + ":" + str(round(float(CPU_VID_C1),2)) + ":")
                if data[x]["SensorName"] == "Core 2 T0 Effective Clock":
                    CPU_CLK_C2 = (data[x]["SensorValue"].replace(',','.'))
                    send_data_B2 +=("C2C" + ":" + str((math.ceil(float(CPU_CLK_C2)))) + ":")
                if data[x]["SensorName"] == "Core 2 VID":
                    CPU_VID_C2 = (data[x]["SensorValue"].replace(',','.'))
                    send_data_B2 +=("C2V" + ":" + str(round(float(CPU_VID_C2),2)) + ":")
                if data[x]["SensorName"] == "Core 3 T0 Effective Clock":
                    CPU_CLK_C3 = (data[x]["SensorValue"].replace(',','.'))
                    send_data_B2 +=("C3C" + ":" + str((math.ceil(float(CPU_CLK_C3)))) + ":")
                if data[x]["SensorName"] == "Core 3 VID":
                    CPU_VID_C3 = (data[x]["SensorValue"].replace(',','.'))
                    send_data_B2 +=("C3V" + ":" + str(round(float(CPU_VID_C3),2)) + ":")
                if data[x]["SensorName"] == "Total CPU Usage":
                    CPU_USAGE = (data[x]["SensorValue"].replace(',','.'))
                    send_data_B2 +=("CUA" + ":" + str((math.ceil(float(CPU_USAGE)))) + ":")
                
                #3RD Chunk
                if data[x]["SensorName"] == "Core 4 T0 Effective Clock":
                    CPU_CLK_C4 = (data[x]["SensorValue"].replace(',','.'))
                    send_data_B3 +=("C4C" + ":" + str((math.ceil(float(CPU_CLK_C4)))) + ":")
                if data[x]["SensorName"] == "Core 4 VID":
                    CPU_VID_C4 = (data[x]["SensorValue"].replace(',','.'))
                    send_data_B3 +=("C4V" + ":" + str(round(float(CPU_VID_C4),2)) + ":")
                if data[x]["SensorName"] == "Core 5 T0 Effective Clock":
                    CPU_CLK_C5 = (data[x]["SensorValue"].replace(',','.'))
                    send_data_B3 +=("C5C" + ":" + str((math.ceil(float(CPU_CLK_C5)))) + ":")
                if data[x]["SensorName"] == "Core 5 VID":
                    CPU_VID_C5 = (data[x]["SensorValue"].replace(',','.'))
                    send_data_B3 +=("C5V" + ":" + str(round(float(CPU_VID_C5),2)) + ":")
                if data[x]["SensorName"] == "Core 6 T0 Effective Clock":
                    CPU_CLK_C6 = (data[x]["SensorValue"].replace(',','.'))
                    send_data_B3 +=("C6C" + ":" + str((math.ceil(float(CPU_CLK_C6)))) + ":")
                if data[x]["SensorName"] == "Core 6 VID":
                    CPU_VID_C6 = (data[x]["SensorValue"].replace(',','.'))
                    send_data_B3 +=("C6V" + ":" + str(round(float(CPU_VID_C6),2)) + ":")
                if data[x]["SensorName"] == "Core 7 T0 Effective Clock":
                    CPU_CLK_C7 = (data[x]["SensorValue"].replace(',','.'))
                    send_data_B3 +=("C7C" + ":" + str((math.ceil(float(CPU_CLK_C7)))) + ":")
                if data[x]["SensorName"] == "Core 7 VID":
                    CPU_VID_C7 = (data[x]["SensorValue"].replace(',','.'))
                    send_data_B3 +=("C7V" + ":" + str(round(float(CPU_VID_C7),2)) + ":")
                if data[x]["SensorName"] == "Core 8 T0 Effective Clock":
                    CPU_CLK_C8 = (data[x]["SensorValue"].replace(',','.'))
                    send_data_B3 +=("C8C" + ":" + str((math.ceil(float(CPU_CLK_C8)))) + ":")
                if data[x]["SensorName"] == "Core 8 VID":
                    CPU_VID_C8 = (data[x]["SensorValue"].replace(',','.'))
                    send_data_B3 +=("C8V" + ":" + str(round(float(CPU_VID_C8),2)) + ":")
                if data[x]["SensorName"] == "Core 9 T0 Effective Clock":
                    CPU_CLK_C9 = (data[x]["SensorValue"].replace(',','.'))
                    send_data_B3 +=("C9C" + ":" + str((math.ceil(float(CPU_CLK_C9)))) + ":")
                if data[x]["SensorName"] == "Core 9 VID":
                    CPU_VID_C9 = (data[x]["SensorValue"].replace(',','.'))
                    send_data_B3 +=("C9V" + ":" + str(round(float(CPU_VID_C9),2)) + ":")

                #4Th Chunk
                if data[x]["SensorName"] == "Core 10 T0 Effective Clock":
                    CPU_CLK_C10 = (data[x]["SensorValue"].replace(',','.'))
                    send_data_B4 +=("C10C" + ":" + str((math.ceil(float(CPU_CLK_C10)))) + ":")
                if data[x]["SensorName"] == "Core 10 VID":
                    CPU_VID_C10 = (data[x]["SensorValue"].replace(',','.'))
                    send_data_B4 +=("C10V" + ":" + str(round(float(CPU_VID_C10),2)) + ":")
                if data[x]["SensorName"] == "Core 11 T0 Effective Clock":
                    CPU_CLK_C11 = (data[x]["SensorValue"].replace(',','.'))
                    send_data_B4 +=("C11C" + ":" + str((math.ceil(float(CPU_CLK_C11)))) + ":")
                if data[x]["SensorName"] == "Core 11 VID":
                    CPU_VID_C11 = (data[x]["SensorValue"].replace(',','.'))
                    send_data_B4 +=("C11V" + ":" + str(round(float(CPU_VID_C11),2)) + ":")
                if data[x]["SensorName"] == "Core 12 T0 Effective Clock":
                    CPU_CLK_C12 = (data[x]["SensorValue"].replace(',','.'))
                    send_data_B4 +=("C12C" + ":" + str((math.ceil(float(CPU_CLK_C12)))) + ":")
                if data[x]["SensorName"] == "Core 12 VID":
                    CPU_VID_C12 = (data[x]["SensorValue"].replace(',','.'))
                    send_data_B4 +=("C12V" + ":" + str(round(float(CPU_VID_C12),2)) + ":")
                if data[x]["SensorName"] == "Core 13 T0 Effective Clock":
                    CPU_CLK_C13 = (data[x]["SensorValue"].replace(',','.'))
                    send_data_B4 +=("C13C" + ":" + str((math.ceil(float(CPU_CLK_C13)))) + ":")
                if data[x]["SensorName"] == "Core 13 VID":
                    CPU_VID_C13 = (data[x]["SensorValue"].replace(',','.'))
                    send_data_B4 +=("C13V" + ":" + str(round(float(CPU_VID_C13),2)) + ":")
                if data[x]["SensorName"] == "Core 14 T0 Effective Clock":
                    CPU_CLK_C14 = (data[x]["SensorValue"].replace(',','.'))
                    send_data_B4 +=("C14C" + ":" + str((math.ceil(float(CPU_CLK_C14)))) + ":")
                if data[x]["SensorName"] == "Core 14 VID":
                    CPU_VID_C14 = (data[x]["SensorValue"].replace(',','.'))
                    send_data_B4 +=("C14V" + ":" + str(round(float(CPU_VID_C14),2)) + ":")
                if data[x]["SensorName"] == "Core 15 T0 Effective Clock":
                    CPU_CLK_C15 = (data[x]["SensorValue"].replace(',','.'))
                    send_data_B4 +=("C15C" + ":" + str((math.ceil(float(CPU_CLK_C15)))) + ":")
                if data[x]["SensorName"] == "Core 15 VID":
                    CPU_VID_C15 = (data[x]["SensorValue"].replace(',','.'))
                    send_data_B4 +=("C15V" + ":" + str(round(float(CPU_VID_C15),2)) + ":")

                #5Th Chunk
                if data[x]["SensorName"] == "CPU Core Power":
                    CPU_POWER = (data[x]["SensorValue"].replace(',','.'))
                    send_data_B5 +=("CW" + ":" + str(round(float(CPU_POWER),1)) + ":")
                if data[x]["SensorName"] == "CPU SoC Power":
                    SOC_POWER = (data[x]["SensorValue"].replace(',','.'))
                    send_data_B5 +=("SW" + ":" + str(round(float(SOC_POWER),1)) + ":")
                if data[x]["SensorName"] == "Core+SoC Power":
                    CPU_SOC_POWER = (data[x]["SensorValue"].replace(',','.'))
                    send_data_B5 +=("CWT" + ":" + str(round(float(CPU_SOC_POWER),1)) + ":")
                if data[x]["SensorName"] == "CPU SoC Power":
                    GPU_POWER = (data[x]["SensorValue"].replace(',','.'))
                    send_data_B5 +=("GW" + ":" + str(round(float(GPU_POWER),1)) + ":")
                if data[x]["SensorName"] == "CPU FAN":
                    CPU_FAN = (data[x]["SensorValue"].replace(',','.'))
                    send_data_B5 +=("CPF" + ":" + str(CPU_FAN) + ":")
                if data[x]["SensorName"] == "CHA1 FAN":
                    CH1_FAN = (data[x]["SensorValue"].replace(',','.'))
                    send_data_B5 +=("CH1" + ":" + str(CH1_FAN) + ":")
                if data[x]["SensorName"] == "CHA2 FAN":
                    CH2_FAN = (data[x]["SensorValue"].replace(',','.'))
                    send_data_B5 +=("CH2" + ":" + str(CH2_FAN) + ":")
                if data[x]["SensorName"] == "GPU FAN":
                    GPU_FAN = (data[x]["SensorValue"].replace(',','.'))
                    send_data_B5 +=("GPF" + ":" + str(GPU_FAN) + ":")
                
        if (reading == bytes('H\r\n', 'UTF-8')):
            arduino.write(b'OK')
        elif (reading == bytes('A\r\n', 'UTF-8')):
            string = "<" + send_data_B1 + ">"
            stringbytes = bytes(str(string), 'UTF-8')
            arduino.write(stringbytes)
            print(stringbytes)
        elif (reading == bytes('B\r\n', 'UTF-8')):
            string = "<" + send_data_B2 + ">"
            stringbytes = bytes(str(string), 'UTF-8')
            arduino.write(stringbytes)
            print(stringbytes)
        elif (reading == bytes('C\r\n', 'UTF-8')):
            string = "<" + send_data_B3 + ">"
            stringbytes = bytes(str(string), 'UTF-8')
            arduino.write(stringbytes)
            print(stringbytes)
        elif (reading == bytes('D\r\n', 'UTF-8')):
            string = "<" + send_data_B4 + ">"
            stringbytes = bytes(str(string), 'UTF-8')
            arduino.write(stringbytes)
            print(stringbytes)
        elif (reading == bytes('E\r\n', 'UTF-8')):
            #Date Printing
            date = datetime.datetime.now()
            YEAR = date.strftime("%Y")
            send_data_B5 +=("YY" + ":" + YEAR + ":")
            MONTH = date.strftime("%m")
            send_data_B5 +=("MM" + ":" + MONTH + ":")
            DAY = date.strftime("%d")
            send_data_B5 +=("DD" + ":" + DAY + ":")
            HOUR = date.strftime("%H")
            send_data_B5 +=("HH" + ":" + HOUR + ":")
            MINUTE = date.strftime("%M")
            send_data_B5 +=("MN" + ":" + MINUTE + ":")
            SECOND = date.strftime("%S")
            send_data_B5 +=("SS" + ":" + SECOND + ":")
            GMT = time.strftime("%z", time.gmtime())
            send_data_B5 +=("GM" + ":" + GMT + ":")
            string = "<" + send_data_B5 + ">"
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
