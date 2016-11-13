#!/usr/bin/env python2.7
# -*- coding: utf-8 -*-
# date: 03/11/16
# author: Camble
# version: 1.0a
# name: I2C stuff
# description:
# source: https://github.com/Camble/RPi-I2C

import smbus
import time

bus = smbus.SMBus(1)

int voltage = 0;

# This is the address we setup in the Arduino Program
address = 0x04

def writeNumber(value):
    bus.write_byte(address, value)
	# bus.write_byte_data(address, 0, value)
	return -1

def getVoltage():
	# write V to request voltage
	sleep(0.05)
    data = ""
    for i in range(0, 5):
    	data[i] = chr(bus.read_byte(address))
    return data

def readNumber():
	number = bus.read_byte(address)
	# number = bus.read_byte_data(address, 1)
	return number

while True:
	var = input("Request? ")
	if not var:
		continue

	writeNumber(var)
	print "Sent \"" + var + "\" to the ATTiny"
	time.sleep(0.1)

  voltages = getVoltage()
	print "Average voltage: " + voltages[0]
  print "Values used: ",
  for i in range(len(voltages)):
    print voltages[i],
