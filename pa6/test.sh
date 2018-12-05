#!/bin/sh
#  test.sh
#  Created by Joshua Higginbotham on 11/8/15.

# This is only provided for your convenience.
# The tests used in this file may or may not
# match up with what is called for for the report,
# so be sure to read that part of the handout.
# But you're free to modify this script however
# you need to, it's not graded.

echo "Running tests..."
rm output.txt
for i in 1 10 25 50 100 250 500
do
  for j in 1 5 10 25 50 100 150 200
  do
    echo "10000,$i,$j" >> output.txt
    ./client -n 10000 -w $i -b $j
  done
done
echo "Finished!"
