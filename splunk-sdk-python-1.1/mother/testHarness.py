#!/usr/bin/env python


import splunklib.client as client
import splunklib.results as results
from time import sleep
import sys

HOST = "lx-pmsrch-p03p"
PORT = 8089
USERNAME = "admin"
PASSWORD = "0911ifka"

# Create a Service instance and log in 
service = client.connect(
    host=HOST,
    port=PORT,
    username=USERNAME,
    password=PASSWORD)

   
# Run a normal search--search everything, return 1st 10 events
kwargs_normalsearch = {"exec_mode": "normal"}
searchquery_normal = "search parsons"
job = service.jobs.create(searchquery_normal, **kwargs_normalsearch)

# A normal search returns the job's SID right away, so we need to poll for completion
while True:
    job.refresh()
    stats = {"isDone": job["isDone"],
             "doneProgress": float(job["doneProgress"])*100,
              "scanCount": int(job["scanCount"]),
              "eventCount": int(job["eventCount"]),
              "resultCount": int(job["resultCount"])}
    status = ("\r%(doneProgress)03.1f%%   %(scanCount)d scanned   "
              "%(eventCount)d matched   %(resultCount)d results") % stats

    sys.stdout.write(status)
    sys.stdout.flush()
    if stats["isDone"] == "1":
        sys.stdout.write("\n\nDone!\n\n")
        break
    sleep(2)

# Get properties of the job
print "Search job properties"
print "Search job ID:        ", job["sid"]
print "The number of events: ", job["eventCount"]
print "The number of results:", job["resultCount"]
print "Search duration:      ", job["runDuration"], "seconds"
print "This job expires in:  ", job["ttl"], "seconds"
print "------------------------------------------\n"
print "Search results:\n"

# Get the results and display them
for result in results.ResultsReader(job.results()):
    print result

