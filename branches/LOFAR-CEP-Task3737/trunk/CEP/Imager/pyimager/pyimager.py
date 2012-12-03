#!/usr/bin/env python

import argparse
import lofar.pyimager.algorithms as algorithms

def main():
    parser = argparse.ArgumentParser(description="Python imager")
    subparsers = parser.add_subparsers(help="operation to perform")

    subparser = subparsers.add_parser("empty", help="create an empty image")
    subparser.add_argument("-d", "--data-processor", dest="processor", \
        metavar="PROCESSOR", default="casa", help="data processor to use")
    subparser.add_argument("-B", type=float, help="maximum baseline length (m)")
    subparser.add_argument("ms", help="input measurement set")
    subparser.add_argument("image", help="output image")
    subparser.set_defaults(func=algorithms.empty)

    subparser = subparsers.add_parser("mfclean", help="multi-field Clark clean")
    subparser.add_argument("-d", "--data-processor", dest="processor", \
        metavar="PROCESSOR", default="casa", help="data processor to use")
    subparser.add_argument("-B", type=float, default=0.0, help="maximum \
        baseline length (m)")
    subparser.add_argument("-p", "--padding", type=float, default=1.0, \
        help="image plane padding factor (>= 1.0)")
#    subparser.add_argument("-g", choices=["awz", "aw", "w"], help="gridder to use")
#    subparser.add_argument("-G", dest="gridder_options", action="append", metavar="OPTION", help="gridder specific option")
    subparser.add_argument("ms", help="input measurement set")
    subparser.add_argument("image", help="output image")
    subparser.set_defaults(func=algorithms.mfclean)

    args = parser.parse_args()
    args.func(args)

if __name__ == "__main__":
    main()
