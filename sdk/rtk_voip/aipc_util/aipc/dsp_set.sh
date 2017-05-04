#!/bin/sh
aipc_util -z
aipc_util -e
aipc_util -R 0x94000000
aipc_util -w -d 0xb0c00000  -i ./nfjrom
aipc_util -b
