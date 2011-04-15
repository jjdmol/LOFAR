#!/usr/bin/env python
# P.Donker ASTRON feb-2010

from string import upper
import socket

# class STATION can be called with
# 'ALL'       for all stations
# 'NL'        for all NL stations
# 'IS'        for all IS stations
# 'hostname'  for specific station (CS001c)
#             must start with 'CS' or 'RS'
# 'hostip'    for specific station (10.151.1.1)
#             must start with '10.151' or '10.209'
#
#   example:
#   import stations
#   station = stations.STATION('ALL')
#
# after initiate the class can be called with the following functions
# host lcu(next=False)    returns LCU IP  | if next=True,                  |
# host ec(next=False)     returns LCU IP  | first the hostIP is build with |
# host psu(next=False)    returns LCU IP  | active station, after that     |
# host ipmi(next=False)   returns LCU IP  | nextStation() is called        |
# isnext nextStation()    increments the Station counter, this function is
#                         also called by above functions if next = True
#                         isnext=True if next station is available otherwise False

class STATION:

    nlStations = ['1','2','3','4','5','6','7',
                '19','20','21','22','23',
                '33','34','35','36','37','38','39',
                '49','50','51','53','55',
                '65','66','67','68',
                '129','131','132','134','135',
                '145','149','150','151','152','154',
                '161','162','166','167','168','169','170','171',
                '177','180','182','183','184','185','186','187','188','189',
                '193','195','198','199','200','201']
    
    #nlStations = []
    #for i in range(255):
    #    nlStations.append(str(i)) 


    isStations = ['1','2','3','4','5','6','7','8']

    deviceIP = {'LCU':'1','EC':'3','PSU':'5','IPMI':'11','NAA':'53'}
    
    def help(self):
        print "Unknown argument, usage:"
        print "'ALL'       for all stations"
        print "'NL'        for all NL stations"
        print "'IS'        for all IS stations"
        print "'hostname'  for specific station (CS001c)"
        print "            must start with 'CS' or 'RS'"
        print "'hostip'    for specific station (10.151.1.1)"
        print "            must start with '10.151' or '10.209'"    
    
  
    def isLCU(self):
        # get ip-adres of LCU
        local_host = socket.gethostbyname(socket.gethostname())
        ip = local_host.split('.')
        if ip[2] in self.nlStations:
            if ip[0] == '10':
                if (ip[1] == '151') or (ip[1] == '209'):
                    return(local_host[:local_host.rfind('.')+1])
        return(None)

    def stations(self, station, device='LCU'):
        if station == None: return ()
        station = upper(station)
        dIP = self.deviceIP[upper(device)]
        st = ()
        if station == 'TEST':
            ip = '10.87.2.239' 
            st += ip,
            return(st)
        if station == 'ALL':
            for s in self.nlStations:
                ip = '10.151.%s.%s' %(s,dIP)
                st += ip,
            for s in self.isStations:
                ip = '10.209.%s.%s' %(s,dIP)
                st += ip,
            return(st)

        if station == 'NL':
            for s in self.nlStations:
                ip = '10.151.%s.%s' %(s,dIP)
                st += ip,
            return(st)

        if station == 'IS':
            for s in self.isStations:
                ip = '10.209.%s.%s' %(s,dIP)
                st += ip,
            return(st)

        if station == 'THIS':
            s = self.isLCU()
            if s is None:
                print 'Error: this script can only run on a LCU'
                return(())
            st += s+dIP,
            return(st)

        if station.count('.') == 3:
            sts = station.split('.')
            if sts[0] == '10':
                if sts[1] == '151':
                    if sts[2] in self.nlStations:
                        st += station[:station.rfind('.')+1]+dIP,
                        return(st)
                if sts[1] == '209':
                    if sts[2] in self.isStations:
                        st += station[:station.rfind('.')+1]+dIP,
                        return(st)
            print 'Error: not a valid station IP'
            return(())
        prefix = upper(station[:2])
        if prefix == 'CS' or prefix == 'RS' or prefix == 'DE' or prefix == 'FR' or prefix == 'SE' or prefix == 'UK':
            try:
                host = socket.gethostbyname(station[:5] + 'C')
                if host.count('.') == 3:
                    st += host[:host.rfind('.')+1]+dIP,
                    return(st)
            except:
                print 'Error: %s not in NameTable' %(station)
            return(())
        self.help()
        

    def lcu(self, station=None):
        return(self.stations(station,'LCU'))

    def ec(self, station=None):
        return(self.stations(station,'EC'))

    def psu(self, station=None):
        return(self.stations(station,'PSU'))

    def ipmi(self, station=None):
        return(self.stations(station,'IPMI'))

    def naa(self, station=None):
        return(self.stations(station,'NAA'))
