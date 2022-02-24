#!/usr/bin/env bash
SYGENSYS=/home/adam/git-wd-Sygensys/UKAEA-Sygensys/doc/
find . -type d |
while read d;
do
	if [ -f "${SYGENSYS}/${d}/README.md" ]
	then
		echo "Found a README.md for $d"
		#ls -l "${SYGENSYS}/${d}/README.md"
		cp -p "${SYGENSYS}/${d}/README.md" "${d}/"
	fi
done
