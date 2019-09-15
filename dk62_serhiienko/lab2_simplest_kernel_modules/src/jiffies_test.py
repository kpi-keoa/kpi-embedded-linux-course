#!/usr/bin/env python3
import time
import subprocess
import argparse
import re


def get_logs():
    kern_log = subprocess.Popen(['cat', '/var/log/kern.log'], stdout=subprocess.PIPE)
    # getting last 5 logs from kern.log
    output = subprocess.check_output(('tail', '-5'), stdin=kern_log.stdout)
    kern_log.wait()
    return str(output)


def get_jiffies(logs):
    # parsing jiffies and tasklet jiffies from logs
    jiffy = []
    match_jiffies = re.search('jiffies = (\d+)', logs)
    match_tasklet_jiffies = re.search('tasklet jiffies = (\d+)', logs)
    if match_jiffies:
        jiffy.append(match_jiffies.group(1))
    else:
        jiffy.append("no result")

    if match_tasklet_jiffies:
        jiffy.append(match_tasklet_jiffies.group(1))
    else:
        jiffy.append("no result")

    # returning list
    return jiffy


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Test jiffies")
    parser.add_argument("-t", default="100", required=True,
                        help="Time in seconds between stopping module and running it again")
    parser.add_argument("-m", required=True,
                        help="kernel module which should be started")
    parser.add_argument("-u", default="TEST", help="name of user who's run the module")
    parser.add_argument("-f", required=True,
                        help="Kernel interrupt frequency")

    args = parser.parse_args()
    int_freq = int(args.f)
    timer = int(args.t)
    module_name = args.m
    username = "username=" + args.u

    subprocess.call(['insmod', module_name, username])
    subprocess.call(['rmmod', module_name])
    jiffies_1 = get_jiffies(get_logs())

    print("jiffies :", jiffies_1[0])
    print("tasklet jiffies:", jiffies_1[1])
    print("sleep for:", timer, "s..")

    time.sleep(timer)
    subprocess.call(['insmod', module_name, username])
    subprocess.call(['rmmod', module_name])
    jiffies_2 = get_jiffies(get_logs())

    print("jiffies :", jiffies_2[0])
    print("tasklet jiffies:", jiffies_2[1])

    jiff_diff = int(jiffies_2[0]) - int(jiffies_1[0])
    tasklet_jiff_diff = int(jiffies_2[1]) - int(jiffies_1[1])
    seconds = jiff_diff / int_freq
    seconds_tasklet = tasklet_jiff_diff / int_freq

    print("time in seconds evaluated from jiffies:", seconds, "s")
    print("time in seconds evaluated from tasklet_jiffies:", seconds_tasklet, "s")


