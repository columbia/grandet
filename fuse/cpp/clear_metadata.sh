#!/bin/bash

redis-cli KEYS "f:*" | xargs redis-cli DEL
redis-cli KEYS "x:*" | xargs redis-cli DEL
