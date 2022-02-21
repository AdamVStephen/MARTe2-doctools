#!/usr/bin/env bash
#
export M2TOOLS_DIR=../../MARTe2-tools/Build/x86-linux/Source/
CFGTODOT=$(realpath $M2TOOLS_DIR/CfgToDot.ex)

find .. -name "*.cfg" | head -1 |
while read cfg
do
  location=$(dirname $cfg)
  filename=$(basename $cfg)
  echo "Process $filename in $location"
  pushd $location
  pwd
  #echo "$CFGTODOT -i ${filename} -o sta_"
  $CFGTODOT -i ${filename} -o sta_ >> convert.log
  ls -1 *.gv |
  while read gvfile
  do
    pngfile=$(echo $gvfile | sed -e 's/\.gv/\.png/g')
    dot -Tpng ${gvfile} -o ${pngfile}
  done
  #ldd $CFGTODOT
  #ls -l
  popd
done

