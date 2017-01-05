#!/usr/bin/env python2

import csv
import sys
import time
import os
import getopt

"""
# Parses a perf stat file and returns a dictionary with the parsed
# values from that file.
#
# @param file The file to parse
# @returns a dictionary with the metric names as keys and the corresponding
           metrics as values
"""
def parse_file(filePath):
    measurements = {}
    reader = csv.reader(open(filePath, "rb"))

    next(reader) # skip first row
    for row in reader:
        measurements[row[2]] = row[0]

    return measurements

"""
# Executes perf stat runs and generates csv files with the corresponding
# metrics, one file for each run.
#
# @param generations The number of generations for Conway's Game of Life
# @param inputFile An input file with a starting configuration for Conway's Game of Life
# @param numberOfRuns The number of runs to execute
# @param outputDir The directory where the generated csv files should be stored
# @returns None
"""
def do_perf_runs(binary = "life-hash_table", generations = 100, inputFile = "f0.l", numberOfRuns = 5, outputDir = "measurements/perf/" + str(time.time())):
    if not os.path.exists(outputDir):
        os.makedirs(outputDir)

    print "Collecting metrics for %d generations, inputFile '%s'" % (generations, inputFile)
    print "%d runs, writing output to '%s'\n" % (numberOfRuns, outputDir)

    for i in range(0, numberOfRuns):
        print "Run %d..." % i
        os.system("(perf stat -e cycles:u -e cycles:k -e instructions:u -e branch-misses -e L1-dcache-load-misses:u -e LLC-loads:u -e LLC-stores:u -e LLC-load-misses:u -e LLC-store-misses:u -x, ./" + binary + " " + str(generations) + " < " + inputFile + " | sort > /dev/null) > " + outputDir + "/run_" + str(i) + ".csv 2>&1")

    print "\nDone.\n"

"""
# Iterates over previously generated csv files and returns a dictionary with
# the results of each file.
#
# @param sourceDir The directory containing the files to iterate over
# @returns a dictionary with the results of each file
"""
def collect_results(sourceDir):
    measurements = {}

    for resultsFile in os.listdir(sourceDir):
        if os.path.isdir(sourceDir + "/" + resultsFile):
            continue

        currentMeasurements = parse_file(sourceDir + "/" + resultsFile)
        for key in currentMeasurements.keys():
            if key in measurements:
                values = measurements[key]
                values.append(int(currentMeasurements[key]))
                measurements[key] = values
            else:
                measurements[key] = [int(currentMeasurements[key])]

    return measurements

"""
# Returns the average of a list of values.
#
# @param values A list containing numeric items
# @returns the average of the list's values
"""
def avg(values):
    return sum(values) / len(values)

"""
# Returns the median of a list of values.
#
# @param values A list containing numeric items
# @returns the median of the list's values
"""
def median(values):
    values.sort()
    return values[len(values) / 2]

"""
# Returns aggregated results for a measurements dictionary.
#
# @param measurements A dictionary containing perf stats metrics
# @returns a dictionary containing the aggregated results of the provided measurements
"""
def aggregate_results(measurements):
    aggregates = {}

    for key in measurements.keys():
        aggregates[key] = {}
        aggregates[key]["avg"] = avg(measurements[key])
        aggregates[key]["median"] = median(measurements[key])

    return aggregates

"""
# Writes the given results to a csv file in the provided output directory.
#
# @param results The results to write
# @param outputDirectory The directory to write the results file to
# @param fileName The file name of the results file
# @returns None
"""
def write_results(results, outputDirectory, fileName = "aggregates.csv"):
    if not os.path.exists(outputDirectory):
        os.makedirs(outputDirectory)

    writer = csv.DictWriter(open(outputDirectory + "/" + fileName, "wb"), ["metric", "avg", "median"])
    writer.writeheader()
    for metric, aggregates in results.items():
        writer.writerow({"metric": metric, "avg": aggregates["avg"], "median": aggregates["median"]})

"""
# Cleans up previous builds and re-compiles the binary that should be evaluated.
#
# @param target The Makefile target to execute
# @returns None
"""
def build(target):
    os.system("make clean")
    os.system("make " + target)

"""
# Returns a usage message for this script.
# @returns the usage message for this scripts
"""
def usage_message():
    return "test.py -b <binary> -t <makefile-target> -g <generations> -i <input-file> -r <runs> -o <output-directory>"

def main(argv):
    binary = "life-hash_table"
    generations = 100
    inputFile = "f0.l"
    runs = 5
    outputDirectory = "measurements/perf/stats"
    target = "life-hash_table"

    try:
        opts, args = getopt.getopt(argv, "b:g:i:r:o:t:h", ["binary=", "gens=", "input=", "runs=", "output=", "target", "help"])
    except getopt.GetoptError:
        print usage_message()
        sys.exit(2)
    for opt, arg in opts:
        if opt in ("-h", "help"):
            print usage_message()
            sys.exit()
        elif opt in ("-b", "binary="):
            binary = args
        elif opt in ("-g", "gens="):
            generations = int(arg)
        elif opt in ("-i", "input="):
            inputFile = arg
        elif opt in ("-r", "runs="):
            runs = int(arg)
        elif opt in ("-o", "output="):
            outputDirectory = arg
        elif opt in ("-t", "target="):
            target = arg

    build(target)
    do_perf_runs(binary, generations, inputFile, runs, outputDirectory)
    results = aggregate_results(collect_results(outputDirectory))
    print results
    write_results(results, outputDirectory + "/aggregates", "aggregates.csv")

if __name__ == "__main__":
    main(sys.argv[1:])
