#!/usr/bin/env python
import sys
import subprocess

def parse(items):
    lvl = 0
    ret = []
    stack = []
    cur = ""
    lvl_stack = [[]]
    for ch in items:
        if ch == '{':
            lvl += 1

            # push new level
            stack += [cur]
            lvl_stack += [[]]
            cur = ""
        elif ch == '}':
            lvl -= 1

            # add current item to current level
            lvl_stack[-1] += [cur]
            # pop current level
            cur_lvl = lvl_stack[-1]
            lvl_stack = lvl_stack[:-1]

            # pop last item
            last_item = stack[-1]
            stack = stack[:-1]
            for cur_item in cur_lvl:
                lvl_stack[-1] += [last_item + cur_item]

            cur = ""
        elif ch == ' ':
            if cur:
                lvl_stack[-1] += [cur]
            cur = ""
        else:
            cur += ch
    if cur:
        lvl_stack[-1] += [cur]
    return lvl_stack[-1]

total = 0
for line in open(sys.argv[1]):
    line = line.strip()

    (name, items) = line.split(':')
    print name,

    list_items = parse(items)
    ret = subprocess.check_output("cloc --quiet --yaml --exclude-dir=external,build,examples,boost,etc,run %s | grep code | tail -n 1" % ' '.join(list_items), shell=True)
    print ret.strip(), list_items
    total += int(ret.split(':')[1])

print "TOTAL:", total
