#!/usr/bin/env python2

from __future__ import division

import csv
import sys
import time
import os
import getopt
import math

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
# @param runnable The thing (binary, script) to run
# @param generations The number of generations for Conway's Game of Life
# @param inputFile An input file with a starting configuration for Conway's Game of Life
# @param numberOfRuns The number of runs to execute
# @param outputDir The directory where the generated csv files should be stored
# @returns None
"""
def do_perf_runs(runnable = "life-hash_table", generations = 100, inputFile = "f0.l", numberOfRuns = 5, outputDir = "measurements/perf/" + str(time.time())):
    if not os.path.exists(outputDir):
        os.makedirs(outputDir)
    
    # add current directory prefix for binaries/scripts in current working dir
    if os.path.exists(runnable) and os.path.basename(runnable) == runnable:
        runnable = "./"+runnable
    
    print "Collecting metrics for `%s %d < %s | sort`" % (runnable, generations, inputFile)
    print "%d runs, writing output to '%s'\n" % (numberOfRuns, outputDir)

    for i in range(1, numberOfRuns+1):
        print "Run %d..." % i
        os.system("(perf stat -e cycles:u -e cycles:k -e instructions:u -e branch-misses -e L1-dcache-load-misses:u -e LLC-loads:u -e LLC-stores:u -e LLC-load-misses:u -e LLC-store-misses:u -x, " + runnable + " " + str(generations) + " < " + inputFile + " | sort > /dev/null) > " + outputDir + "/run_" + str(i) + ".csv 2>&1")

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
            val = int(currentMeasurements[key]) if not currentMeasurements[key] in ["<not supported>", "<not counted>"] else 0
            if key in measurements:
                values = measurements[key]
                values.append(val)
                measurements[key] = values
            else:
                measurements[key] = [val]

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
    return values[len(values) // 2]

"""
# Returns the variance of a list of values.
#
# @param values A list containing numeric items
# @returns the variance of the list's values
"""
def sd(values):
    mean = avg(values)
    var = sum((x - mean) ** 2 for x in values) / (len(values) - 1)
    return math.sqrt(var)

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
        aggregates[key]["sd"] = sd(measurements[key])
        aggregates[key]["min"] = min(measurements[key])
        aggregates[key]["max"] = max(measurements[key])
        
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

    writer = csv.DictWriter(open(outputDirectory + "/" + fileName, "wb"), ["metric", "avg", "median", "sd", "min", "max"])
    writer.writeheader()
    for metric, aggregates in results.items():
        writer.writerow({"metric": metric, "avg": aggregates["avg"], "median": aggregates["median"], "sd": aggregates["sd"], "min": aggregates["min"], "max": aggregates["max"]})

"""
# Returns a usage message for this script.
# @returns the usage message for this scripts
"""
def usage_message():
    return "test.py -p <runnable> -g <generations> -i <input-file> -r <runs> -o <output-directory>"

def main(argv):
    binary = "life-hash_table"
    generations = 100
    inputFile = "f0.l"
    runs = 5
    outputDirectory = "measurements/perf/stats"
    
    try:
        opts, args = getopt.getopt(argv, "p:g:i:r:o:h", ["runnable=", "gens=", "input=", "runs=", "output=", "help"])
    except getopt.GetoptError:
        print usage_message()
        sys.exit(2)
    for opt, arg in opts:
        if opt in ("-h", "help"):
            print usage_message()
            sys.exit()
        elif opt in ("-p", "runnable="):
            runnable = arg
        elif opt in ("-g", "gens="):
            generations = int(arg)
        elif opt in ("-i", "input="):
            inputFile = arg
        elif opt in ("-r", "runs="):
            runs = int(arg)
        elif opt in ("-o", "output="):
            outputDirectory = arg
    
    if (runs < 1):
        raise Exception("runs option argument must be at least 1")
    
    do_perf_runs(runnable, generations, inputFile, runs, outputDirectory)
    
    results = collect_results(outputDirectory)
    
    if runs > 1:
        results = aggregate_results(results)
        write_results(results, outputDirectory + "/aggregates", "aggregates.csv")
    
    print results

if __name__ == "__main__":
    main(sys.argv[1:])
