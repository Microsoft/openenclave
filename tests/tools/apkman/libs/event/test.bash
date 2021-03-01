#!/bin/bash
host/event_host enc/event_enc --once &
sleep 0.2 && echo Hi | nc 0.0.0.0 12345

