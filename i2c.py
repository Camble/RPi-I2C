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
	# voltage = bus.read_byte(address)
	# or voltage = bus.read_byte_data(address, 1)
	# return voltage

def readNumber():
	number = bus.read_byte(address)
	# number = bus.read_byte_data(address, 1)
	return number

while True:
	var = input("Enter 1 – 9: ")
	if not var:
		continue

	writeNumber(var)
	print "RPI: Hi Arduino, I sent you ", var
	# sleep one second
	time.sleep(1)

	number = readNumber()
	print "Arduino: Hey RPI, I received a digit ", number
	print
