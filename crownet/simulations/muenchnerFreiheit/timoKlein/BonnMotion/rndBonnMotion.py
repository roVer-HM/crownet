#!/usr/bin/python3

import os
import random
import argparse
import sys

def main(arg_dict):
    if arg_dict['seed'] is None:
        seed = random.randrange(sys.maxsize)
    else:
        seed = arg_dict['seed']

    rnd = random.Random(seed)

    if not os.path.isfile(arg_dict['input']):
        print(f"input {arg_dict['input']} is not a file ")
        sys.exit(-1)

    with open(arg_dict['input'], 'r') as f:
        lines = f.readlines()
        # sort lines based on first field (first timestep)
        lines.sort(key=lambda row: float(row.split(' ')[0]))

    draw = set()
    while len(draw) < arg_dict['n']:
        draw.add(rnd.randint(0, len(lines) - 1))

    draw_list = list(draw)
    draw_list.sort()
    data=list()
    for d in draw_list:
        data.append(lines[d])


    with open(arg_dict['output'], 'w') as f:
        # if arg_dict['log-settings']:
        #     f.write(f"# Args: {arg_dict}. With seed={seed}\n")
        for d in data:
            f.write(d)

    second_wave=list()
    for idx, row in enumerate(data):
        time = float(row.split(' ')[0])
        if (time >= arg_dict['time']):
            second_wave.append(idx)

    print()
    print(f"# new Config")
    print(f"[Config conf_{arg_dict['output'].split('.')[-1]}]")
    print(f"extends = {arg_dict['extend']}")
    print(f"*.p.app[0..{min(second_wave)-1}].startTime = 0.0")
    print(f"*.p.app[{min(second_wave)}..{max(second_wave)}].startTime = {arg_dict['time']}")
    print()


    # print(f"done.")

def str2bool(v):
    # see https://stackoverflow.com/a/43357954
    if isinstance(v, bool):
        return v
    if v.lower() in ('yes', 'true', 't', 'y', '1'):
        return True
    elif v.lower() in ('no', 'false', 'f', 'n', '0'):
        return False
    else:
        raise argparse.ArgumentTypeError('Boolean value expected.')


def parse_args(args):
    parser = argparse.ArgumentParser(prog="BonnMotion Helper",
                                     description="Crate subset of trajectories from a given BonnMotion file")

    parser.add_argument("-i", "--input",
                        dest="input",
                        nargs="?",
                        required=True,
                        help="BonnMotion input file")

    parser.add_argument("-o", "--output",
                        dest="output",
                        nargs="?",
                        required=True,
                        help="Location of Output file")

    parser.add_argument("-s", "--seed",
                        dest="seed",
                        required=False,
                        type=int,
                        help="seed for Randomness. Set this to some long value to reproduce outputs.")

    parser.add_argument("-n",
                        dest="n",
                        required=True,
                        type=int,
                        nargs="?",
                        help="number of samples to draw. If bigger than input only the input is returned.")

    parser.add_argument("--log-settings",
                        dest="log-settings",
                        type=str2bool,
                        const=True,
                        default=True,
                        nargs="?",
                        help="add command info to output file.")
    parser.add_argument("--time",
                        dest="time",
                        type=float,
                        default=80.400000,
                        help="Set starttime of second wave of peds. default=80.400000")

    parser.add_argument("--extends",
                        dest="extend",
                        default="general",
                        help="Set name of parent configuration.")

    return parser.parse_args(args)


if __name__ == '__main__':
    # main(vars(parse_args(['--log-settings', '-i', 'mf_timo_klein_initial_peds/bn_0000.txt', '-o', 'out.txt', '-n', '3'])))
    main(vars(parse_args(sys.argv[1:])))
