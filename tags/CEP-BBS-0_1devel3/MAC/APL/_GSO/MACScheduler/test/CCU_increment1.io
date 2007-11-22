[io]
COM1 = { SOCKET, LOFAR, 8192, "client localhost 27000" } // MACScheduler
COM2 = { SOCKET, LOFAR, 8192, "server localhost 27001" } // BeamServer
COM3 = { SOCKET, LOFAR, 8192, "server localhost 27010" } // VBStartDaemon
COM4 = { SOCKET, LOFAR, 8192, "client localhost 27003" } // VirtualBackend
COM5 = { SOCKET, LOFAR, 8192, "client localhost 26999" } // THPVSSBridge

// For serial ports: COM1 = { COM1, ccc , "Baud=1200 parity=N data=8 stop=1 octs=on" }

