#!/bin/bash

#recipients=mhardman@nsidc.org brodzik@nsidc.org
recipients=maddenp@colorado.edu

source /etc/profile.d/modules.sh
set -ex
repo=$(readlink -f $(dirname $(readlink -f $0))/..)
cd $repo
git pull
node=janus-compile$(shuf -i1-4 -n1)
subject="measures-byu unit tests $(date)"
ssh $node $repo/janus_batchfiles/unit_test_local.sh 2>&1 | mail -s "$subject" $recipients
