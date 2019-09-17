#!/usr/bin/env python3
import time
import subprocess
import argparse
import re


def get_logs():
    # Assume file is short to fit in memory. Otherwise need to seek from the end
    with open('/var/log/kern.log') as f:
        return ''.join(list(f)[-3:])


def get_jiffies(logs):
    # parsing jiffies and tasklet jiffies from logs
    jiffy = []
    match_jiffies = re.search('jiffies = (\d+)', logs)
    match_tasklet_jiffies = re.search('tasklet jiffies = (\d+)', logs)

    try:
        jiffy.append(match_jiffies.group(1))
    except:
        jiffy.append('no result')

    try:
        jiffy.append(match_tasklet_jiffies.group(1))
    except:
        jiffy.append('no result')

    # returning list
    return jiffy


def main(args):
    parser = argparse.ArgumentParser(description='Test jiffies')
    parser.add_argument('-t', '--time',type=int, default='100', required=True,
                        help='Time in seconds between stopping module and running it again')
    parser.add_argument('-m', '--module', required=True,
                        help='kernel module which should be started')
    parser.add_argument('-u', '--user', default='TEST', help="name of user who's run the module")
    parser.add_argument('-f', '--freq', type=int, required=True,
                        help='Kernel timer interrupt frequency')
    
    args = parser.parse_args()
    int_freq = int(args.freq)
    timer = int(args.time)
    module_name = args.module
    username = 'username="%s"'.format(args.user)

    subprocess.call(['insmod', module_name, username])
    subprocess.call(['rmmod', module_name])

    jiffies_1 = get_jiffies(get_logs())

    print('jiffies :', jiffies_1[0])
    print('tasklet jiffies:', jiffies_1[1])
    print('sleep for:', timer, 's..')

    time.sleep(timer)
    subprocess.call(['insmod', module_name, username])
    subprocess.call(['rmmod', module_name])
    jiffies_2 = get_jiffies(get_logs())

    print('jiffies :', jiffies_2[0])
    print('tasklet jiffies:', jiffies_2[1])

    jiff_diff = int(jiffies_2[0]) - int(jiffies_1[0])
    tasklet_jiff_diff = int(jiffies_2[1]) - int(jiffies_1[1])
    seconds = jiff_diff / int_freq
    seconds_tasklet = tasklet_jiff_diff / int_freq

    print('time in seconds evaluated from jiffies: {:.2f} s'.format(seconds))
    print('time in seconds evaluated from tasklet_jiffies: {:.2f} s'.format(seconds_tasklet))

    
if __name__ == '__main__':
    from sys import argv
    main(argv) 
    
    


