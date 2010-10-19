#! /usr/bin/python

def setup_db_queries():

    # Add "System Classes"

    yield "INSERT INTO Systems.SystemClasses(sc_id, sc_name, sc_description) VALUES (1, 'CEP'     , 'Central Processing Facilities');"
    yield "INSERT INTO Systems.SystemClasses(sc_id, sc_name, sc_description) VALUES (2, 'WAN'     , 'Wide Area Network');"
    yield "INSERT INTO Systems.SystemClasses(sc_id, sc_name, sc_description) VALUES (3, 'STATION' , 'Station');"

    # Add "Central Processing Facilities"

    for cep_id in [1]:
        si_id   = 100 + cep_id
        si_name = "CEP"
        si_description = "Central Processing Facilities"

        yield "INSERT INTO Systems.SystemInstances(si_id, si_name, si_description, sc_id) SELECT %(si_id)d, '%(si_name)s', '%(si_description)s', sc_id FROM Systems.SystemClasses WHERE sc_name = 'CEP';" % vars()

    # Add "Wide Area Network"

    for wan_id in [1]:
        si_id   = 200 + wan_id
        si_name = "WAN"
        si_description = "Wide Area Network"

        yield "INSERT INTO Systems.SystemInstances(si_id, si_name, si_description, sc_id) SELECT %(si_id)d, '%(si_name)s', '%(si_description)s', sc_id FROM Systems.SystemClasses WHERE sc_name = 'WAN';" % vars()

    # Add "Test Station" systems

    N_FTS = 2

    for fts_id in range(1, 1 + N_FTS):
        si_id   = 500 + fts_id
        si_name = "FTS-%d" % fts_id
        si_description = "First Test Station %d" % fts_id

        yield "INSERT INTO Systems.SystemInstances(si_id, si_name, si_description, sc_id) SELECT %(si_id)d, '%(si_name)s', '%(si_description)s', sc_id FROM Systems.SystemClasses WHERE sc_name = 'STATION';" % vars()

    # Add "Core Station" systems

    N_CS = 32

    for cs_id in range(1, 1 + N_CS):
        si_id   = 1000 + cs_id
        si_name = "CS-%d" % cs_id
        si_description = "Core Station %d" % cs_id

        yield "INSERT INTO Systems.SystemInstances(si_id, si_name, si_description, sc_id) SELECT %(si_id)d, '%(si_name)s', '%(si_description)s', sc_id FROM Systems.SystemClasses WHERE sc_name = 'STATION';" % vars()

    # Add "Remote Station" systems

    N_RS = 45

    for rs_id in range(1, 1 + N_RS):

        si_id = 2000 + rs_id
        si_name = "RS-%d" % rs_id
        si_description = "Remote Station %d" % rs_id

        yield "INSERT INTO Systems.SystemInstances(si_id, si_name, si_description, sc_id) SELECT %(si_id)d, '%(si_name)s', '%(si_description)s', sc_id FROM Systems.SystemClasses WHERE sc_name = 'STATION';" % vars()

    # Add Stable observable to each STATION ...

    yield "INSERT INTO Observations.Observables(obs_name, obs_unit, si_id, type_id) SELECT 'stable', 'flag', si_id, type_id FROM Systems.SystemClasses NATURAL JOIN Systems.SystemInstances JOIN Observations.ObservableTypes ON sc_name = 'STATION' AND type_name = 'boolean';"

    # Add RCU Status to each RCU at each STATION ...

    # 6 Crates per station
    # 4 RSP Boards per crate
    # 4 APs per RSP board
    # 2 RCUs per AP --- 1 for each polarisation
    # 2 Antennas Types per RCU (possibly 3!)

    # E.g., there will be 6*4*4*2 RCUs per station

    for crate_id in range(6):
        for rsp_id in range(4):
            for ap_id in range(4):
                for rcu_id in range(2):
                    for (obs_suffix, obs_unit, obs_type) in [
                        ("Settings"  , 'BU', 'integer'),
                        ("MeanPower" , 'BU', 'float'),
                        ("PeakPower" , 'BU', 'float')]:

                        obs_name = "crate%d_rsp%d_ap%d_rcu%d_%s" %  (crate_id, rsp_id, ap_id, rcu_id, obs_suffix)

                        yield "INSERT INTO Observations.Observables(obs_name, obs_unit, si_id, type_id) SELECT '%(obs_name)s', '%(obs_unit)s', si_id, type_id FROM Systems.SystemClasses NATURAL JOIN Systems.SystemInstances JOIN Observations.ObservableTypes ON sc_name = 'STATION' AND type_name = '%(obs_type)s';" % vars()

def main():
    for query in setup_db_queries():
        print query

if __name__ == "__main__":
    main()
