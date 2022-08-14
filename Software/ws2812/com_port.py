# Test project for accessing serial port

import serial # pyserial library

print("COM port test python script...")

# Initialise serial port. COM5 was used to communicate with the basys dev board
ser = serial.Serial('COM5', baudrate = 115200, timeout = 1)

while 1:
   
   command = input("Input a command: ")
   command = command.split(" ")
   
   print(command)
   
   if (len(command) > 1):
      val = int(command[1])
   
   command = command[0]
   
   if (command == "fade"):
      print("issuing rgb fade command")
      ser.write([0x4D, 0x30])
   elif (command == "test"):
      print("issuing test command")
      ser.write([0x4D, 0x31])
   elif (command == "manual"):
      ser.write([0x4D, 0x32])
   elif (command == "blink"):
      ser.write([0x4D, 0x33])
   elif (command == "off"):
      ser.write([0x4D, 0x34])
   elif(command == "red"):
      print("issuing red command")
      ser.write([0x52, val])
   elif(command == "green"):
      print("issuing red command")
      ser.write([0x47, val])
   elif(command == "blue"):
      print("issuing red command")
      ser.write([0x42, val])
   
   # Read line, decoding to ascii is optional
   dat = ser.readline().decode('ascii')
   if (len(dat) != 0):
      print("Return from system: " + dat)
   