#!/usr/bin/env python2.7
# -*- coding: utf-8 -*-
# date: 03/11/16
# author: Camble
# version: 1.0a
# name: I2C stuff
# description:
# source: https://github.com/Camble/RPi-I2C

import smbus
import struct
import time

bus = smbus.SMBus(1)

voltage = 0;

# This is the address we setup in the Arduino Program
address = 0x04

def writeNumber(value):
  bus.write_byte(address, value)
  # bus.write_byte_data(address, 0, value)
  return -1

def getVoltage():
  # write V to request voltage
  time.sleep(0.05)
  data = [None] * 2
  for i in range(2):
    data = data + chr(bus.read_byte(address))
  #value = struct.unpack("<h", data)
  #value = struct.unpack(">h", data)
  value  = data[0] << 8 | data[1]
  return value

while True:
  var = input("Request? ")
  if not var:
    continue

  # writeNumber(var)
  print "Sent \"" + str(var) + "\" to the ATTiny"
  time.sleep(0.1)

  voltage = getVoltage()
  print "Average voltage: " + str(voltage)
  #print "Values used: ",
  #for i in range(len(voltages)):
  #  print voltages[i],
