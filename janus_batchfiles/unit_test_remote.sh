#!/bin/bash

source /etc/profile.d/modules.sh
set -ex
repo=$(readlink -f $(dirname $(readlink -f $0))/..)
cd $repo
git pull
node=janus-compile$(shuf -i1-4 -n1)
ssh $node $repo/janus_batchfiles/unit_test_local
