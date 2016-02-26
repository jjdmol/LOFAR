#!/usr/bin/python

def replace_host(location, cluster_name, hosts):
  """
    Returns location, with its hostname replaced by one
    of `hosts', but only if the host matches `cluster_name':
      cluster_name:... -> hosts[0]:...
      other:... -> other:...

    The hosts array is rotated to obtain a round-robin allocation
    through repeated use.
  """

  host, dir = location.split(":", 2)

  if host == cluster_name:
    host = hosts.pop(0)
    hosts.append(host)
   
  return "%s:%s" % (host,dir)

def process_parset(parset, cluster_name, hosts):
  data_products = [ "Correlated", "CoherentStokes", "IncoherentStokes" ]

  for dp in data_products:
    key = "Observation.DataProducts.Output_%s.locations" % (dp,)
    if not parset.isDefined(key):
      continue

    # obtain current locations
    locations = parset._getStringVector1(key, True)

    # replace global fs references
    locations = [replace_host(x, cluster_name, hosts) for x in locations]

    # update locations field
    parset.replace(key, "[%s]" % (", ".join(locations),))

if __name__ == "__main__":
  import sys
  from optparse import OptionParser
  from lofar.parameterset import PyParameterSet

  # Command-line arguments
  parser = OptionParser("%prog [options] < parset")
  parser.add_option("-C", "--cluster", dest="cluster", type="string", default="cep4",
                    help="Cluster name to replace")
  parser.add_option("-H", "--hosts", dest="hosts", type="string", default="",
                    help="Pool of host names to use (space separated)")

  (options, args) = parser.parse_args()

  if not options.cluster or not options.hosts:
    print "Require both --cluster and --hosts."
    parser.print_help()
    sys.exit(1)

  hosts = options.hosts.split()

  # Read from stdin ...
  parset = PyParameterSet("/dev/stdin", False)

  # ... process ...
  process_parset(parset, options.cluster, hosts)

  # Write to stdout ...
  print str(parset)

