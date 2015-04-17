import numpy
import csv
import datetime
from pprint import pprint

from pylab import *

intervals = []
samples = []
markers = []

last_trigger = None
with open("yorkshire-pig-trial1.log") as f:
	reader = csv.reader(f, delimiter=' ')
	reader.next()
	for row in reader:
		if row:
			time = datetime.datetime.strptime(row[0].strip(':'), '%Y-%m-%d-%H-%M-%f')
			samples.append(row[1])
			markers.append(row[2])

			if float(row[2]) > 0 and last_trigger:
				intervals.append((time - last_trigger).microseconds / 1000)
				last_trigger = time

			if float(row[2]) > 0 and not last_trigger:
				last_trigger = time

intervals = numpy.array(intervals)
interval_mean = numpy.mean(intervals)
interval_std_dev = numpy.std(intervals)

print interval_mean
print interval_std_dev


samples = numpy.array(samples)
markers = numpy.array(markers)

t = numpy.arange(0.0, 1000000, 0.016)[:len(samples)]
print len(samples)
print len(t)
print len(markers)
plot(t, samples, 'b-', t, markers, 'ro')

xlabel('time (s)')
ylabel('voltage (V)')
title('yorkshire-pig-trial1')
grid(True)
savefig("test.png")
show()