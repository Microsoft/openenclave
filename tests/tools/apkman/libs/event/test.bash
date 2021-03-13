#!/bin/bash
if ! command -v nc &> /dev/null
then
   echo "netcat program not found. Skipping test."
   exit 0
fi

host/event_host enc/event_enc --once &
sleep 0.2 && echo Hi | nc 0.0.0.0 12345

